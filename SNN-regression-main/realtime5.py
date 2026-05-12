# =============================================================================
# Faster single-camera SNN line regression
# =============================================================================
#
# Use this for the cam0-trained setup before returning to two-camera control.
# It keeps the same Hough-replacement output convention:
#
#       x = slope * y + intercept
#
# Compared with realtime.py:
#   - optional two-output checkpoint path: one forward for [slope, intercept]
#   - current separate checkpoints still work: one slope forward + one intercept
#   - faster/correct event accumulation with np.bincount
#   - cam0-style y-mask and clamped visualization line
#   - avoids GPU sync unless explicitly requested for timing
# =============================================================================

import time

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

# Set True only when you want accurate timing. It slows the live loop.
SYNC_FOR_TIMING = False

# If auto-open picks the wrong camera, set this to the cam0 device name/serial.
CAMERA_NAME = None

# Match the cam0 region used in realtimeTake3.
TOP_MASK_Y = 70
BOTTOM_MASK_Y = 170

# If you train a two-output model, set this and realtime5 will do one model
# forward total. Output order is expected to be [slope_norm, intercept_norm].
TWO_OUTPUT_WEIGHTS_PATH = None

INTERCEPT_WEIGHTS_PATH = (
    r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True"
    r"\models\interceptTest\may12intercept.pth"
)

SLOPE_WEIGHTS_PATH = (
    r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True"
    r"\models\slopeTest\may12slope.pth"
)


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
        model.fc_out = torch.nn.Linear(CONFIG["hidden"], output_dim, bias=False)

    if DEVICE.type == "cuda":
        return model.to(device=DEVICE, memory_format=torch.channels_last).eval()
    return model.to(DEVICE).eval()


def reset_models(*models):
    for model in models:
        if model is not None:
            functional.reset_net(model)


def load_models():
    if TWO_OUTPUT_WEIGHTS_PATH:
        model = build_model(output_dim=2)
        model.load_state_dict(torch.load(TWO_OUTPUT_WEIGHTS_PATH, map_location=DEVICE))
        model.eval()
        reset_models(model)
        print("Loaded two-output SNN checkpoint:", TWO_OUTPUT_WEIGHTS_PATH)
        return model, None, None

    intercept_model = build_model(output_dim=1)
    slope_model = build_model(output_dim=1)

    intercept_model.load_state_dict(torch.load(INTERCEPT_WEIGHTS_PATH, map_location=DEVICE))
    slope_model.load_state_dict(torch.load(SLOPE_WEIGHTS_PATH, map_location=DEVICE))

    intercept_model.eval()
    slope_model.eval()
    reset_models(intercept_model, slope_model)
    print("Loaded separate slope/intercept SNN checkpoints")
    return None, intercept_model, slope_model


# =============================================================================
# Event accumulation and inference
# =============================================================================

def accumulate_events_bincount(pos_frame, neg_frame, xs, ys, ps, width):
    if xs.size == 0:
        return

    flat = ys.astype(np.int64, copy=False) * int(width) + xs.astype(np.int64, copy=False)

    pos_flat = flat[ps == 1]
    if pos_flat.size:
        pos_frame.reshape(-1)[:] += np.bincount(
            pos_flat,
            minlength=pos_frame.size,
        ).astype(np.float32, copy=False)

    neg_flat = flat[ps == 0]
    if neg_flat.size:
        neg_frame.reshape(-1)[:] += np.bincount(
            neg_flat,
            minlength=neg_frame.size,
        ).astype(np.float32, copy=False)


def denormalize_prediction(slope_raw, intercept_raw):
    slope = (slope_raw * 0.40) - 0.20
    intercept = intercept_raw * 280.0
    return slope, intercept


def run_snn_inference(frame_cpu, frame_device, two_output_model, intercept_model, slope_model):
    if DEVICE.type == "cuda":
        frame_device.copy_(frame_cpu, non_blocking=True)

    if two_output_model is not None:
        out = two_output_model(frame_device)
        if SYNC_FOR_TIMING and DEVICE.type == "cuda":
            torch.cuda.synchronize()
        raw = out.detach().to("cpu").reshape(2)
        slope_raw = float(raw[0])
        intercept_raw = float(raw[1])
    else:
        b_out = intercept_model(frame_device)
        m_out = slope_model(frame_device)
        if SYNC_FOR_TIMING and DEVICE.type == "cuda":
            torch.cuda.synchronize()
        slope_raw = float(m_out.detach().to("cpu").reshape(()))
        intercept_raw = float(b_out.detach().to("cpu").reshape(()))

    slope, intercept = denormalize_prediction(slope_raw, intercept_raw)
    return slope_raw, intercept_raw, slope, intercept


# =============================================================================
# Visualization
# =============================================================================

def draw_estimated_line(canvas, slope, intercept):
    y0_full = max(0, min(H - 1, int(TOP_MASK_Y)))
    y1_full = max(0, min(H - 1, int(BOTTOM_MASK_Y)))
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


def update_preview(pos_events, neg_events, slope, intercept, latency_ms):
    if not ENABLE_VISUALIZATION:
        return

    np.clip(
        pos_events[::VIS_STRIDE, ::VIS_STRIDE] * EVENT_BRIGHTNESS_GAIN,
        0,
        255,
        out=preview_green_f32,
    )
    np.clip(
        neg_events[::VIS_STRIDE, ::VIS_STRIDE] * EVENT_BRIGHTNESS_GAIN,
        0,
        255,
        out=preview_red_f32,
    )

    preview_bgr[:, :, 0].fill(0)
    np.copyto(preview_bgr[:, :, 1], preview_green_f32, casting="unsafe")
    np.copyto(preview_bgr[:, :, 2], preview_red_f32, casting="unsafe")

    draw_estimated_line(preview_bgr, slope, intercept)

    cv.putText(
        preview_bgr,
        f"m={slope:.3f}  b={intercept:.1f}  {latency_ms:.2f} ms",
        (10, 24),
        cv.FONT_HERSHEY_SIMPLEX,
        0.45,
        (255, 255, 255),
        1,
        cv.LINE_AA,
    )

    cv.imshow("SNN Line Estimate - realtime5", preview_bgr)
    cv.waitKey(1)


# =============================================================================
# Main
# =============================================================================

two_output_model, intercept_model, slope_model = load_models()

capture = dv.io.camera.open(CAMERA_NAME) if CAMERA_NAME else dv.io.camera.open()
capture.setEventsRunning(True)
capture.setFramesRunning(False)

if not capture.isEventStreamAvailable():
    raise RuntimeError("No event camera detected")

W, H = capture.getEventResolution()
print("Resolution:", (W, H))
print("Single-camera realtime5 SNN regression started")
print(f"Mask: y = {TOP_MASK_Y} to {BOTTOM_MASK_Y}")
print("Device:", DEVICE)
print("SYNC_FOR_TIMING:", SYNC_FOR_TIMING)

VIS_W = (W + VIS_STRIDE - 1) // VIS_STRIDE
VIS_H = (H + VIS_STRIDE - 1) // VIS_STRIDE

top_mask_y = max(0, min(H - 1, int(TOP_MASK_Y)))
bottom_mask_y = max(0, min(H - 1, int(BOTTOM_MASK_Y)))
if top_mask_y > bottom_mask_y:
    top_mask_y, bottom_mask_y = bottom_mask_y, top_mask_y

if DEVICE.type == "cuda":
    frame_cpu = torch.empty((1, 2, H, W), dtype=torch.float32, pin_memory=True)
    frame_device = torch.empty(
        (1, 2, H, W),
        dtype=torch.float32,
        device=DEVICE,
        memory_format=torch.channels_last,
    )
    frame_np = frame_cpu.numpy()
else:
    frame_np = np.zeros((1, 2, H, W), dtype=np.float32)
    frame_cpu = torch.from_numpy(frame_np)
    frame_device = frame_cpu

pos_frame = frame_np[0, 0]
neg_frame = frame_np[0, 1]

next_print_time = time.perf_counter()
next_vis_time = time.perf_counter()
last_event_wall_time = time.perf_counter()

if ENABLE_VISUALIZATION:
    cv.namedWindow("SNN Line Estimate - realtime5", cv.WINDOW_NORMAL)
    preview_bgr = np.zeros((VIS_H, VIS_W, 3), dtype=np.uint8)
    preview_green_f32 = np.zeros((VIS_H, VIS_W), dtype=np.float32)
    preview_red_f32 = np.zeros((VIS_H, VIS_W), dtype=np.float32)
else:
    print("OpenCV not available; visualization disabled.")


try:
    with torch.inference_mode():
        while capture.isRunning():
            cycle_start = time.perf_counter()

            pos_frame.fill(0)
            neg_frame.fill(0)

            window_start_ts = None
            window_end_ts = None

            while capture.isRunning():
                events = capture.getNextEventBatch()

                if events is None:
                    now = time.perf_counter()
                    if now - last_event_wall_time >= INACTIVITY_RESET_S:
                        reset_models(two_output_model, intercept_model, slope_model)
                        last_event_wall_time = now
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
                last_event_wall_time = time.perf_counter()

                mask = (ys >= top_mask_y) & (ys <= bottom_mask_y)
                accumulate_events_bincount(pos_frame, neg_frame, xs[mask], ys[mask], ps[mask], W)

                if window_end_ts - window_start_ts >= EVENT_WINDOW_US:
                    break

            if window_start_ts is None or window_end_ts is None:
                continue

            acquisition_end = time.perf_counter()

            slope_raw, intercept_raw, slope, intercept = run_snn_inference(
                frame_cpu,
                frame_device,
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
                acquisition_ms = (acquisition_end - cycle_start) * 1000.0
                inference_ms = (inference_end - acquisition_end) * 1000.0

                print(
                    f"Raw Prediction: m_norm = {slope_raw:.4f} | "
                    f"b_norm = {intercept_raw:.4f} || "
                    f"Denorm: m = {slope:.4f} | "
                    f"b = {intercept:.4f} | "
                    f"x = {slope:.4f}y + {intercept:.4f} | "
                    f"Window = {window_end_ts - window_start_ts} us | "
                    f"Total = {cycle_latency_ms:.2f} ms | "
                    f"Acquire = {acquisition_ms:.2f} ms | "
                    f"Infer = {inference_ms:.2f} ms"
                )
                next_print_time = now + PRINT_INTERVAL_S

            if need_visualization:
                cycle_latency_ms = (inference_end - cycle_start) * 1000.0
                update_preview(pos_frame, neg_frame, slope, intercept, cycle_latency_ms)
                next_vis_time = now + VIS_INTERVAL_S
                loop_hz = 1.0 / max(1e-9, inference_end - cycle_start)
                print(f"Main loop Hz: {loop_hz:.1f}")

except KeyboardInterrupt:
    print("\nStopping realtime5...")

finally:
    try:
        capture.close()
    except Exception:
        pass

    if ENABLE_VISUALIZATION:
        cv.destroyAllWindows()

    print("Closed camera and windows.")
