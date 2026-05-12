# =============================================================================
# Faster two-camera SNN line regression
# =============================================================================
#
# Main differences from realtimeTake3:
#   - cam0 and cam1 are batched together for inference
#   - current one-output slope/intercept checkpoints run in two forwards total
#   - optional two-output checkpoint path runs slope+intercept in one forward
#   - event accumulation uses bincount instead of repeated advanced-index +=
#
# The SNN is intended to replace the Hough line estimate by returning the same
# line parameters:
#       x = slope * y + intercept
# =============================================================================

import time
import threading

import dv_processing as dv
import numpy as np
import torch
from spikingjelly.activation_based import functional

from model_definition import CONFIG, SNN_Net


# =============================================================================
# Settings
# =============================================================================

ENABLE_VISUALIZATION = True

EVENT_WINDOW_US = 1_000
PRINT_INTERVAL_S = 0.50
VIS_INTERVAL_S = 0.05
VIS_STRIDE = 2
INACTIVITY_RESET_S = 0.10
ENABLE_TF32 = True
EVENT_BRIGHTNESS_GAIN = 100.0

# Set this only when measuring exact GPU timing. It slows the live loop.
SYNC_FOR_TIMING = False

# If you train a two-output model, set this to the checkpoint path and leave the
# separate slope/intercept paths as fallback.
TWO_OUTPUT_WEIGHTS_PATH = None

INTERCEPT_WEIGHTS_PATH = (
    r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True"
    r"\models\interceptTest\may12intercept.pth"
)

SLOPE_WEIGHTS_PATH = (
    r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True"
    r"\models\slopeTest\may12slope.pth"
)

CAMERA_0_NAME = None
CAMERA_1_NAME = None

CAM0_TOP_MASK_Y = 70
CAM0_BOTTOM_MASK_Y = 170

CAM1_TOP_MASK_Y = 30
CAM1_BOTTOM_MASK_Y = 162


if ENABLE_VISUALIZATION:
    try:
        import cv2 as cv
    except ImportError:
        cv = None
        ENABLE_VISUALIZATION = False
else:
    cv = None


DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")

if DEVICE.type == "cuda":
    torch.backends.cudnn.benchmark = True
    if ENABLE_TF32:
        torch.backends.cuda.matmul.allow_tf32 = True
        torch.backends.cudnn.allow_tf32 = True


# =============================================================================
# Model helpers
# =============================================================================

def build_model(output_dim=1):
    model = SNN_Net(
        tau=CONFIG["tau"],
        final_tau=CONFIG["final_tau"],
        hidden=CONFIG["hidden"],
        norm_type=CONFIG["norm_type"],
        learnable_norm=CONFIG["learnable_norm"],
        init_scale=CONFIG["init_scale"],
    )

    if output_dim != 1:
        model.fc_out = torch.nn.Linear(CONFIG["hidden"], output_dim, bias=True)

    if DEVICE.type == "cuda":
        return model.to(device=DEVICE, memory_format=torch.channels_last).eval()

    return model.to(DEVICE).eval()


def reset_models(*models):
    for model in models:
        if model is not None:
            functional.reset_net(model)


def _load_weights(model, weights_path):
    model.load_state_dict(torch.load(weights_path, map_location=DEVICE))
    model.eval()
    return model


def load_models():
    if TWO_OUTPUT_WEIGHTS_PATH:
        model = _load_weights(build_model(output_dim=2), TWO_OUTPUT_WEIGHTS_PATH)
        reset_models(model)
        print("Loaded two-output SNN checkpoint:", TWO_OUTPUT_WEIGHTS_PATH)
        return model, None, None

    intercept_model = _load_weights(build_model(output_dim=1), INTERCEPT_WEIGHTS_PATH)
    slope_model = _load_weights(build_model(output_dim=1), SLOPE_WEIGHTS_PATH)
    reset_models(intercept_model, slope_model)
    print("Loaded separate slope/intercept SNN checkpoints")
    return None, intercept_model, slope_model


# =============================================================================
# Camera setup
# =============================================================================

def open_two_cameras():
    if CAMERA_0_NAME is not None and CAMERA_1_NAME is not None:
        if CAMERA_0_NAME == CAMERA_1_NAME:
            raise ValueError("CAMERA_0_NAME and CAMERA_1_NAME must be different.")
        return dv.io.camera.open(CAMERA_0_NAME), dv.io.camera.open(CAMERA_1_NAME)

    devices = dv.io.camera.discover()
    print("Discovered devices:")
    for i, dev in enumerate(devices):
        print(f"  [{i}] {dev}")

    if len(devices) < 2:
        raise RuntimeError(f"Need two event cameras, but only found {len(devices)}.")

    return dv.io.camera.open(devices[0]), dv.io.camera.open(devices[1])


def make_camera_state(capture, name, top_mask_y, bottom_mask_y):
    capture.setEventsRunning(True)
    capture.setFramesRunning(False)

    if not capture.isEventStreamAvailable():
        raise RuntimeError(f"No event stream detected for {name}")

    W, H = capture.getEventResolution()
    VIS_W = (W + VIS_STRIDE - 1) // VIS_STRIDE
    VIS_H = (H + VIS_STRIDE - 1) // VIS_STRIDE

    latest_frame_np = np.zeros((2, H, W), dtype=np.float32)

    if ENABLE_VISUALIZATION:
        window_name = f"SNN Regression Line Estimate - {name}"
        cv.namedWindow(window_name, cv.WINDOW_NORMAL)
        preview_bgr = np.zeros((VIS_H, VIS_W, 3), dtype=np.uint8)
        preview_green_f32 = np.zeros((VIS_H, VIS_W), dtype=np.float32)
        preview_red_f32 = np.zeros((VIS_H, VIS_W), dtype=np.float32)
    else:
        window_name = None
        preview_bgr = None
        preview_green_f32 = None
        preview_red_f32 = None

    return {
        "name": name,
        "capture": capture,
        "W": W,
        "H": H,
        "VIS_W": VIS_W,
        "VIS_H": VIS_H,
        "latest_frame_np": latest_frame_np,
        "window_name": window_name,
        "preview_bgr": preview_bgr,
        "preview_green_f32": preview_green_f32,
        "preview_red_f32": preview_red_f32,
        "top_mask_y": int(top_mask_y),
        "bottom_mask_y": int(bottom_mask_y),
        "last_event_wall_time": time.perf_counter(),
        "last_window_us": 0,
        "has_frame": False,
        "lock": threading.Lock(),
        "stop_event": threading.Event(),
    }


# =============================================================================
# Event reader
# =============================================================================

def accumulate_events_bincount(local_pos, local_neg, xs, ys, ps, width):
    if xs.size == 0:
        return

    flat = ys.astype(np.int64, copy=False) * int(width) + xs.astype(np.int64, copy=False)

    pos_flat = flat[ps == 1]
    if pos_flat.size:
        local_pos.reshape(-1)[:] += np.bincount(
            pos_flat,
            minlength=local_pos.size,
        ).astype(np.float32, copy=False)

    neg_flat = flat[ps == 0]
    if neg_flat.size:
        local_neg.reshape(-1)[:] += np.bincount(
            neg_flat,
            minlength=local_neg.size,
        ).astype(np.float32, copy=False)


def camera_reader_loop(cam_state):
    capture = cam_state["capture"]
    H = cam_state["H"]
    W = cam_state["W"]

    top_mask_y = max(0, min(H - 1, int(cam_state["top_mask_y"])))
    bottom_mask_y = max(0, min(H - 1, int(cam_state["bottom_mask_y"])))
    if top_mask_y > bottom_mask_y:
        top_mask_y, bottom_mask_y = bottom_mask_y, top_mask_y

    local_frame = np.zeros_like(cam_state["latest_frame_np"])
    local_pos = local_frame[0]
    local_neg = local_frame[1]

    while not cam_state["stop_event"].is_set() and capture.isRunning():
        local_pos.fill(0)
        local_neg.fill(0)

        window_start_ts = None
        window_end_ts = None

        while not cam_state["stop_event"].is_set() and capture.isRunning():
            events = capture.getNextEventBatch()

            if events is None:
                now = time.perf_counter()
                if now - cam_state["last_event_wall_time"] >= INACTIVITY_RESET_S:
                    cam_state["last_event_wall_time"] = now
                time.sleep(0.0001)
                continue

            events_np = events.numpy()
            if events_np.size == 0:
                continue

            xs = events_np["x"]
            ys = events_np["y"]
            ps = events_np["polarity"]
            ts = events_np["timestamp"]
            if ts.size == 0:
                continue

            if window_start_ts is None:
                window_start_ts = int(ts[0])

            window_end_ts = int(ts[-1])
            cam_state["last_event_wall_time"] = time.perf_counter()

            mask = (ys >= top_mask_y) & (ys <= bottom_mask_y)
            accumulate_events_bincount(local_pos, local_neg, xs[mask], ys[mask], ps[mask], W)

            if window_end_ts - window_start_ts >= EVENT_WINDOW_US:
                break

        if window_start_ts is None or window_end_ts is None:
            continue

        with cam_state["lock"]:
            np.copyto(cam_state["latest_frame_np"], local_frame)
            cam_state["last_window_us"] = int(window_end_ts - window_start_ts)
            cam_state["has_frame"] = True


def start_camera_thread(cam_state):
    thread = threading.Thread(target=camera_reader_loop, args=(cam_state,), daemon=True)
    thread.start()
    cam_state["thread"] = thread


def stop_camera_thread(cam_state):
    cam_state["stop_event"].set()
    if "thread" in cam_state:
        cam_state["thread"].join(timeout=1.0)


# =============================================================================
# Batched inference
# =============================================================================

def make_batch_buffers(cam0, cam1):
    if (cam0["W"], cam0["H"]) != (cam1["W"], cam1["H"]):
        raise RuntimeError(
            "Batched inference requires both cameras to have the same resolution. "
            f"Got cam0={(cam0['W'], cam0['H'])}, cam1={(cam1['W'], cam1['H'])}."
        )

    H = cam0["H"]
    W = cam0["W"]

    if DEVICE.type == "cuda":
        batch_cpu = torch.empty((2, 2, H, W), dtype=torch.float32, pin_memory=True)
        batch_device = torch.empty(
            (2, 2, H, W),
            dtype=torch.float32,
            device=DEVICE,
            memory_format=torch.channels_last,
        )
        batch_np = batch_cpu.numpy()
    else:
        batch_np = np.zeros((2, 2, H, W), dtype=np.float32)
        batch_cpu = torch.from_numpy(batch_np)
        batch_device = batch_cpu

    return batch_np, batch_cpu, batch_device


def copy_latest_batch(cam0, cam1, batch_np):
    with cam0["lock"]:
        if not cam0["has_frame"]:
            return False, 0, 0
        np.copyto(batch_np[0], cam0["latest_frame_np"])
        cam0_window_us = cam0["last_window_us"]

    with cam1["lock"]:
        if not cam1["has_frame"]:
            return False, 0, 0
        np.copyto(batch_np[1], cam1["latest_frame_np"])
        cam1_window_us = cam1["last_window_us"]

    return True, cam0_window_us, cam1_window_us


def denormalize_predictions(slope_raw, intercept_raw):
    slope = (slope_raw * 0.40) - 0.20
    intercept = intercept_raw * 280.0
    return slope, intercept


def run_batched_snn(batch_cpu, batch_device, two_output_model, intercept_model, slope_model):
    if DEVICE.type == "cuda":
        batch_device.copy_(batch_cpu, non_blocking=True)

    if two_output_model is not None:
        out = two_output_model(batch_device)
        if SYNC_FOR_TIMING and DEVICE.type == "cuda":
            torch.cuda.synchronize()
        raw = out.detach().to("cpu").reshape(2, 2).numpy()
        slope_raw = raw[:, 0]
        intercept_raw = raw[:, 1]
    else:
        b_out = intercept_model(batch_device)
        m_out = slope_model(batch_device)
        if SYNC_FOR_TIMING and DEVICE.type == "cuda":
            torch.cuda.synchronize()
        slope_raw = m_out.detach().to("cpu").reshape(2).numpy()
        intercept_raw = b_out.detach().to("cpu").reshape(2).numpy()

    slope, intercept = denormalize_predictions(slope_raw, intercept_raw)
    return slope_raw, intercept_raw, slope, intercept


# =============================================================================
# Visualization
# =============================================================================

def draw_estimated_line(cam_state, canvas, slope, intercept):
    H = cam_state["H"]
    VIS_W = cam_state["VIS_W"]
    VIS_H = cam_state["VIS_H"]

    y0_full = max(0, min(H - 1, int(cam_state["top_mask_y"])))
    y1_full = max(0, min(H - 1, int(cam_state["bottom_mask_y"])))
    if y0_full > y1_full:
        y0_full, y1_full = y1_full, y0_full

    x0 = int(round((slope * y0_full + intercept) / VIS_STRIDE))
    y0 = int(round(y0_full / VIS_STRIDE))
    x1 = int(round((slope * y1_full + intercept) / VIS_STRIDE))
    y1 = int(round(y1_full / VIS_STRIDE))

    clipped, pt1, pt2 = cv.clipLine((0, 0, VIS_W, VIS_H), (x0, y0), (x1, y1))
    if clipped:
        cv.line(canvas, pt1, pt2, (0, 255, 255), 2, cv.LINE_AA)

    cv.line(canvas, (0, y0), (VIS_W - 1, y0), (255, 255, 255), 1, cv.LINE_AA)
    cv.line(canvas, (0, y1), (VIS_W - 1, y1), (255, 255, 255), 1, cv.LINE_AA)


def update_preview(cam_state, frame_snapshot, slope, intercept, latency_ms):
    if not ENABLE_VISUALIZATION:
        return

    preview_bgr = cam_state["preview_bgr"]
    preview_green_f32 = cam_state["preview_green_f32"]
    preview_red_f32 = cam_state["preview_red_f32"]

    np.clip(
        frame_snapshot[0, ::VIS_STRIDE, ::VIS_STRIDE] * EVENT_BRIGHTNESS_GAIN,
        0,
        255,
        out=preview_green_f32,
    )
    np.clip(
        frame_snapshot[1, ::VIS_STRIDE, ::VIS_STRIDE] * EVENT_BRIGHTNESS_GAIN,
        0,
        255,
        out=preview_red_f32,
    )

    preview_bgr[:, :, 0].fill(0)
    np.copyto(preview_bgr[:, :, 1], preview_green_f32, casting="unsafe")
    np.copyto(preview_bgr[:, :, 2], preview_red_f32, casting="unsafe")

    draw_estimated_line(cam_state, preview_bgr, float(slope), float(intercept))

    cv.putText(
        preview_bgr,
        f"{cam_state['name']}  m={slope:.3f}  b={intercept:.1f}  {latency_ms:.2f} ms",
        (10, 24),
        cv.FONT_HERSHEY_SIMPLEX,
        0.45,
        (255, 255, 255),
        1,
        cv.LINE_AA,
    )
    cv.imshow(cam_state["window_name"], preview_bgr)
    cv.waitKey(1)


# =============================================================================
# Main
# =============================================================================

def main():
    two_output_model, intercept_model, slope_model = load_models()

    capture0, capture1 = open_two_cameras()

    cam0 = make_camera_state(
        capture0,
        "Camera 0",
        top_mask_y=CAM0_TOP_MASK_Y,
        bottom_mask_y=CAM0_BOTTOM_MASK_Y,
    )
    cam1 = make_camera_state(
        capture1,
        "Camera 1",
        top_mask_y=CAM1_TOP_MASK_Y,
        bottom_mask_y=CAM1_BOTTOM_MASK_Y,
    )

    batch_np, batch_cpu, batch_device = make_batch_buffers(cam0, cam1)

    print("Camera 0 Resolution:", (cam0["W"], cam0["H"]))
    print("Camera 1 Resolution:", (cam1["W"], cam1["H"]))
    print("Batched two-camera SNN regression started")
    print(f"Camera 0 mask: y = {cam0['top_mask_y']} to {cam0['bottom_mask_y']}")
    print(f"Camera 1 mask: y = {cam1['top_mask_y']} to {cam1['bottom_mask_y']}")
    print("Device:", DEVICE)
    print("SYNC_FOR_TIMING:", SYNC_FOR_TIMING)

    if not ENABLE_VISUALIZATION:
        print("OpenCV not available; visualization disabled.")

    start_camera_thread(cam0)
    start_camera_thread(cam1)

    next_print_time = time.perf_counter()
    next_vis_time = time.perf_counter()

    try:
        with torch.inference_mode():
            while cam0["capture"].isRunning() and cam1["capture"].isRunning():
                cycle_start = time.perf_counter()

                ready, cam0_window_us, cam1_window_us = copy_latest_batch(cam0, cam1, batch_np)
                if not ready:
                    time.sleep(0.001)
                    continue

                acquisition_end = time.perf_counter()

                slope_raw, intercept_raw, slope, intercept = run_batched_snn(
                    batch_cpu,
                    batch_device,
                    two_output_model,
                    intercept_model,
                    slope_model,
                )

                inference_end = time.perf_counter()
                now = time.perf_counter()

                need_print = now >= next_print_time
                need_visualization = ENABLE_VISUALIZATION and now >= next_vis_time

                if need_print:
                    cycle_latency_ms = (inference_end - cycle_start) * 1000.0
                    snapshot_ms = (acquisition_end - cycle_start) * 1000.0
                    inference_ms = (inference_end - acquisition_end) * 1000.0

                    print(
                        "\n"
                        f"Camera 0 Raw Prediction: "
                        f"m_norm = {slope_raw[0]:.4f} | "
                        f"b_norm = {intercept_raw[0]:.4f} || "
                        f"Denorm: m = {slope[0]:.4f} | "
                        f"b = {intercept[0]:.4f} | "
                        f"x = {slope[0]:.4f}y + {intercept[0]:.4f} | "
                        f"Window = {cam0_window_us} us"
                    )
                    print(
                        f"Camera 1 Raw Prediction: "
                        f"m_norm = {slope_raw[1]:.4f} | "
                        f"b_norm = {intercept_raw[1]:.4f} || "
                        f"Denorm: m = {slope[1]:.4f} | "
                        f"b = {intercept[1]:.4f} | "
                        f"x = {slope[1]:.4f}y + {intercept[1]:.4f} | "
                        f"Window = {cam1_window_us} us"
                    )
                    print(
                        f"Total main-loop = {cycle_latency_ms:.2f} ms | "
                        f"Snapshot = {snapshot_ms:.2f} ms | "
                        f"Batched infer = {inference_ms:.2f} ms"
                    )
                    next_print_time = now + PRINT_INTERVAL_S

                if need_visualization:
                    cycle_latency_ms = (inference_end - cycle_start) * 1000.0
                    update_preview(cam0, batch_np[0], slope[0], intercept[0], cycle_latency_ms)
                    update_preview(cam1, batch_np[1], slope[1], intercept[1], cycle_latency_ms)
                    next_vis_time = now + VIS_INTERVAL_S
                    loop_hz = 1.0 / max(1e-9, inference_end - cycle_start)
                    print(f"Main loop Hz: {loop_hz:.1f}")

    except KeyboardInterrupt:
        print("\nStopping batched two-camera SNN regression...")

    finally:
        stop_camera_thread(cam0)
        stop_camera_thread(cam1)

        for cam in (cam0, cam1):
            try:
                cam["capture"].close()
            except Exception:
                pass

        if ENABLE_VISUALIZATION:
            cv.destroyAllWindows()

        print("Closed cameras and windows.")


if __name__ == "__main__":
    main()
