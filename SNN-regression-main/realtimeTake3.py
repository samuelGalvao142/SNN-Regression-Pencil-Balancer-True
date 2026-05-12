# =============================================================================
# SNN Implementation for real time control of mechanical systems from image
# processing and event cameras
# =============================================================================
# Fast threaded two-camera SNN regression testing version
#
# Includes:
#   - two camera support
#   - one background reader thread per camera
#   - masked/clamped event input region
#   - clamped visualization line
#   - brightness gain for event display
#   - reduced CUDA synchronization delay
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

# Reader thread update window
EVENT_WINDOW_US = 1_000

# Main loop update rates
PRINT_INTERVAL_S = 0.50
VIS_INTERVAL_S = 0.05

VIS_STRIDE = 2
INACTIVITY_RESET_S = 0.10
ENABLE_TF32 = True

# Visualization brightness
EVENT_BRIGHTNESS_GAIN = 100.0

# Set True only if you want accurate timing numbers.
# False is faster for real-time control/testing.
SYNC_FOR_TIMING = False

# -----------------------------------------------------------------------------
# Camera device names
# -----------------------------------------------------------------------------
# If auto-discovery works, leave these as None.
# If auto-discovery fails, manually enter camera names/serials.
# -----------------------------------------------------------------------------

CAMERA_0_NAME = None
CAMERA_1_NAME = None

# -----------------------------------------------------------------------------
# Clamp / mask region for each camera
# -----------------------------------------------------------------------------
# Only events inside:
#       top_mask_y <= y <= bottom_mask_y
# are accumulated into the SNN input frame.
#
# The predicted line is also drawn only between these two y-values.
# -----------------------------------------------------------------------------

CAM0_TOP_MASK_Y = 70
CAM0_BOTTOM_MASK_Y = 170

CAM1_TOP_MASK_Y = 30
CAM1_BOTTOM_MASK_Y = 162

# =============================================================================
# OpenCV
# =============================================================================

if ENABLE_VISUALIZATION:
    try:
        import cv2 as cv
    except ImportError:
        cv = None
        ENABLE_VISUALIZATION = False
else:
    cv = None

# =============================================================================
# Torch device
# =============================================================================

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")

if DEVICE.type == "cuda":
    torch.backends.cudnn.benchmark = True

    if ENABLE_TF32:
        torch.backends.cuda.matmul.allow_tf32 = True
        torch.backends.cudnn.allow_tf32 = True


# =============================================================================
# Model helpers
# =============================================================================

def build_model():
    model = SNN_Net(
        tau=CONFIG["tau"],
        final_tau=CONFIG["final_tau"],
        hidden=CONFIG["hidden"],
        norm_type=CONFIG["norm_type"],
        learnable_norm=CONFIG["learnable_norm"],
        init_scale=CONFIG["init_scale"],
    )

    if DEVICE.type == "cuda":
        return model.to(device=DEVICE, memory_format=torch.channels_last).eval()

    return model.to(DEVICE).eval()


def reset_models(*models):
    for model in models:
        functional.reset_net(model)


# =============================================================================
# Camera opening
# =============================================================================

def open_two_cameras():
    """
    Opens two DVS/event cameras.

    If CAMERA_0_NAME and CAMERA_1_NAME are provided, those are used directly.
    Otherwise, the function tries to auto-discover two connected cameras.
    """

    if CAMERA_0_NAME is not None and CAMERA_1_NAME is not None:
        if CAMERA_0_NAME == CAMERA_1_NAME:
            raise ValueError("CAMERA_0_NAME and CAMERA_1_NAME must be different.")

        capture0 = dv.io.camera.open(CAMERA_0_NAME)
        capture1 = dv.io.camera.open(CAMERA_1_NAME)

        return capture0, capture1

    devices = dv.io.camera.discover()

    print("Discovered devices:")
    for i, dev in enumerate(devices):
        print(f"  [{i}] {dev}")

    if len(devices) < 2:
        raise RuntimeError(
            f"Need two event cameras, but only found {len(devices)} device(s)."
        )

    # If this fails, manually set CAMERA_0_NAME and CAMERA_1_NAME above.
    capture0 = dv.io.camera.open(devices[0])
    capture1 = dv.io.camera.open(devices[1])

    return capture0, capture1


# =============================================================================
# Visualization
# =============================================================================

def draw_estimated_line(cam_state, canvas, slope, intercept):
    """
    Draw the SNN-regression line only inside the y-mask region:

        x = slope*y + intercept

    slope and intercept should already be denormalized.

    The line is clamped between:
        y = top_mask_y
        y = bottom_mask_y
    """

    H = cam_state["H"]
    VIS_W = cam_state["VIS_W"]
    VIS_H = cam_state["VIS_H"]

    top_mask_y = cam_state["top_mask_y"]
    bottom_mask_y = cam_state["bottom_mask_y"]

    y0_full = max(0, min(H - 1, int(top_mask_y)))
    y1_full = max(0, min(H - 1, int(bottom_mask_y)))

    if y0_full > y1_full:
        y0_full, y1_full = y1_full, y0_full

    x0_full = slope * y0_full + intercept
    x1_full = slope * y1_full + intercept

    x0 = int(round(x0_full / VIS_STRIDE))
    y0 = int(round(y0_full / VIS_STRIDE))

    x1 = int(round(x1_full / VIS_STRIDE))
    y1 = int(round(y1_full / VIS_STRIDE))

    clipped, pt1, pt2 = cv.clipLine(
        (0, 0, VIS_W, VIS_H),
        (x0, y0),
        (x1, y1)
    )

    if clipped:
        cv.line(canvas, pt1, pt2, (0, 255, 255), 2, cv.LINE_AA)

    # Draw top/bottom clamp lines
    y_top_vis = int(round(y0_full / VIS_STRIDE))
    y_bottom_vis = int(round(y1_full / VIS_STRIDE))

    cv.line(
        canvas,
        (0, y_top_vis),
        (VIS_W - 1, y_top_vis),
        (255, 255, 255),
        1,
        cv.LINE_AA
    )

    cv.line(
        canvas,
        (0, y_bottom_vis),
        (VIS_W - 1, y_bottom_vis),
        (255, 255, 255),
        1,
        cv.LINE_AA
    )


def update_preview(cam_state, pos_frame_snapshot, neg_frame_snapshot, slope, intercept, latency_ms):
    """
    Update visualization window for one camera.
    """

    if not ENABLE_VISUALIZATION:
        return

    preview_bgr = cam_state["preview_bgr"]
    preview_green_f32 = cam_state["preview_green_f32"]
    preview_red_f32 = cam_state["preview_red_f32"]

    np.clip(
        pos_frame_snapshot[::VIS_STRIDE, ::VIS_STRIDE] * EVENT_BRIGHTNESS_GAIN,
        0,
        255,
        out=preview_green_f32
    )

    np.clip(
        neg_frame_snapshot[::VIS_STRIDE, ::VIS_STRIDE] * EVENT_BRIGHTNESS_GAIN,
        0,
        255,
        out=preview_red_f32
    )

    preview_bgr[:, :, 0].fill(0)
    np.copyto(preview_bgr[:, :, 1], preview_green_f32, casting="unsafe")
    np.copyto(preview_bgr[:, :, 2], preview_red_f32, casting="unsafe")

    draw_estimated_line(cam_state, preview_bgr, slope, intercept)

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
# Camera state / buffers
# =============================================================================

def make_camera_state(capture, name, top_mask_y, bottom_mask_y):
    """
    Create frame buffers, preview buffers, metadata, and locks for one camera.
    """

    capture.setEventsRunning(True)
    capture.setFramesRunning(False)

    if not capture.isEventStreamAvailable():
        raise RuntimeError(f"No event stream detected for {name}")

    W, H = capture.getEventResolution()

    VIS_W = (W + VIS_STRIDE - 1) // VIS_STRIDE
    VIS_H = (H + VIS_STRIDE - 1) // VIS_STRIDE

    # Shared latest event frame updated by reader thread
    latest_frame_np = np.zeros((1, 2, H, W), dtype=np.float32)
    latest_pos_frame = latest_frame_np[0, 0]
    latest_neg_frame = latest_frame_np[0, 1]

    # Snapshot frame copied by main thread for inference
    if DEVICE.type == "cuda":
        frame_cpu = torch.empty(
            (1, 2, H, W),
            dtype=torch.float32,
            pin_memory=True
        )

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

        # Thread-updated latest frame
        "latest_frame_np": latest_frame_np,
        "latest_pos_frame": latest_pos_frame,
        "latest_neg_frame": latest_neg_frame,

        # Main-thread snapshot/inference frame
        "frame_cpu": frame_cpu,
        "frame_device": frame_device,
        "frame_np": frame_np,

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
# Threaded event reader
# =============================================================================

def camera_reader_loop(cam_state):
    """
    Background loop for one camera.

    This continuously drains events and updates cam_state['latest_frame_np'].
    The main thread only copies the latest frame and does not block on camera IO.
    """

    capture = cam_state["capture"]

    H = cam_state["H"]

    top_mask_y = max(0, min(H - 1, int(cam_state["top_mask_y"])))
    bottom_mask_y = max(0, min(H - 1, int(cam_state["bottom_mask_y"])))

    if top_mask_y > bottom_mask_y:
        top_mask_y, bottom_mask_y = bottom_mask_y, top_mask_y

    local_frame = np.zeros_like(cam_state["latest_frame_np"])
    local_pos = local_frame[0, 0]
    local_neg = local_frame[0, 1]

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

                # Small sleep avoids a hot CPU spin when no events arrive.
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

            # ------------------------------------------------------------
            # Regression clamp/mask
            # ------------------------------------------------------------
            mask = (ys >= top_mask_y) & (ys <= bottom_mask_y)

            xs = xs[mask]
            ys = ys[mask]
            ps = ps[mask]

            if xs.size > 0:
                local_pos[ys[ps == 1], xs[ps == 1]] += 1.0
                local_neg[ys[ps == 0], xs[ps == 0]] += 1.0

            if window_start_ts is not None and window_end_ts - window_start_ts >= EVENT_WINDOW_US:
                break

        if window_start_ts is None or window_end_ts is None:
            continue

        # Publish the completed local window to the shared latest frame.
        with cam_state["lock"]:
            np.copyto(cam_state["latest_frame_np"], local_frame)
            cam_state["last_window_us"] = int(window_end_ts - window_start_ts)
            cam_state["has_frame"] = True


def start_camera_thread(cam_state):
    thread = threading.Thread(
        target=camera_reader_loop,
        args=(cam_state,),
        daemon=True
    )

    thread.start()
    cam_state["thread"] = thread


def stop_camera_thread(cam_state):
    cam_state["stop_event"].set()

    if "thread" in cam_state:
        cam_state["thread"].join(timeout=1.0)


# =============================================================================
# Main-thread snapshot
# =============================================================================

def copy_latest_frame_for_inference(cam_state):
    """
    Copy the latest threaded camera frame into the main inference buffer.
    Returns:
        has_frame, pos_snapshot, neg_snapshot, window_us
    """

    with cam_state["lock"]:
        if not cam_state["has_frame"]:
            return False, None, None, 0

        np.copyto(cam_state["frame_np"], cam_state["latest_frame_np"])
        window_us = cam_state["last_window_us"]

    pos_snapshot = cam_state["frame_np"][0, 0]
    neg_snapshot = cam_state["frame_np"][0, 1]

    return True, pos_snapshot, neg_snapshot, window_us


# =============================================================================
# SNN inference
# =============================================================================

def run_snn_inference(cam_state, bModel, mModel):
    """
    Run slope and intercept inference for one camera.

    Output model values are denormalized using:

        intercept = intercept_raw * 280
        slope     = slope_raw * 0.40 - 0.20

    Then the visualized line is:

        x = slope*y + intercept
    """

    if DEVICE.type == "cuda":
        cam_state["frame_device"].copy_(
            cam_state["frame_cpu"],
            non_blocking=True
        )

    bOut = bModel(cam_state["frame_device"])
    mOut = mModel(cam_state["frame_device"])

    if SYNC_FOR_TIMING and DEVICE.type == "cuda":
        torch.cuda.synchronize()

    predictions = torch.stack(
        (
            mOut.reshape(()),
            bOut.reshape(())
        )
    ).to("cpu")

    slope_raw = float(predictions[0])
    intercept_raw = float(predictions[1])

    # ------------------------------------------------------------
    # Denormalize SNN regression outputs
    # ------------------------------------------------------------
    # Given:
    #   intercept = normalized_intercept * 280
    #   slope     = normalized_slope * 0.40 - 0.20
    #
    # After this:
    #   line is x = slope*y + intercept
    # ------------------------------------------------------------

    slope = (slope_raw * 0.40) - 0.20
    intercept = intercept_raw * 280.0

    return slope_raw, intercept_raw, slope, intercept


# =============================================================================
# Load models
# =============================================================================

# =============================================================================
# Load models
# =============================================================================

INTERCEPT_WEIGHTS_PATH = (
    r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True"
    r"\models\interceptTest\may12intercept.pth"
)

SLOPE_WEIGHTS_PATH = (
    r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True"
    r"\models\slopeTest\may12slope.pth"
)


def load_intercept_model():
    model = build_model()
    model.load_state_dict(
        torch.load(
            INTERCEPT_WEIGHTS_PATH,
            map_location=DEVICE,
        )
    )
    model.eval()
    return model


def load_slope_model():
    model = build_model()
    model.load_state_dict(
        torch.load(
            SLOPE_WEIGHTS_PATH,
            map_location=DEVICE,
        )
    )
    model.eval()
    return model


# Separate SNN states for Camera 0
bModel0 = load_intercept_model()
mModel0 = load_slope_model()

# Separate SNN states for Camera 1
bModel1 = load_intercept_model()
mModel1 = load_slope_model()

reset_models(bModel0, mModel0, bModel1, mModel1)

# =============================================================================
# Open cameras
# =============================================================================

capture0, capture1 = open_two_cameras()

cam0 = make_camera_state(
    capture0,
    "Camera 0",
    top_mask_y=CAM0_TOP_MASK_Y,
    bottom_mask_y=CAM0_BOTTOM_MASK_Y
)

cam1 = make_camera_state(
    capture1,
    "Camera 1",
    top_mask_y=CAM1_TOP_MASK_Y,
    bottom_mask_y=CAM1_BOTTOM_MASK_Y
)

print("Camera 0 Resolution:", (cam0["W"], cam0["H"]))
print("Camera 1 Resolution:", (cam1["W"], cam1["H"]))
print("Threaded two-camera SNN regression testing started")
print(f"Camera 0 mask: y = {cam0['top_mask_y']} to {cam0['bottom_mask_y']}")
print(f"Camera 1 mask: y = {cam1['top_mask_y']} to {cam1['bottom_mask_y']}")
print("Device:", DEVICE)
print("SYNC_FOR_TIMING:", SYNC_FOR_TIMING)

if not ENABLE_VISUALIZATION:
    print("OpenCV not available; visualization disabled.")

# Start reader threads
start_camera_thread(cam0)
start_camera_thread(cam1)

next_print_time = time.perf_counter()
next_vis_time = time.perf_counter()


# =============================================================================
# Main loop
# =============================================================================

try:
    with torch.inference_mode():
        while cam0["capture"].isRunning() and cam1["capture"].isRunning():
            cycle_start = time.perf_counter()

            # ------------------------------------------------------------
            # Grab latest frames from threaded readers
            # ------------------------------------------------------------

            cam0_ready, cam0_pos_snapshot, cam0_neg_snapshot, cam0_window_us = copy_latest_frame_for_inference(cam0)
            cam1_ready, cam1_pos_snapshot, cam1_neg_snapshot, cam1_window_us = copy_latest_frame_for_inference(cam1)

            if not cam0_ready or not cam1_ready:
                time.sleep(0.001)
                continue

            acquisition_end = time.perf_counter()

            # ------------------------------------------------------------
            # Run SNN regression inference for both cameras
            # ------------------------------------------------------------

            cam0_slope_raw, cam0_intercept_raw, cam0_slope, cam0_intercept = run_snn_inference(
                cam0,
                bModel0,
                mModel0
            )

            cam1_slope_raw, cam1_intercept_raw, cam1_slope, cam1_intercept = run_snn_inference(
                cam1,
                bModel1,
                mModel1
            )

            inference_end = time.perf_counter()

            now = time.perf_counter()

            need_print = now >= next_print_time
            need_visualization = ENABLE_VISUALIZATION and now >= next_vis_time

            # ------------------------------------------------------------
            # Print outputs
            # ------------------------------------------------------------

            if need_print:
                cycle_latency_ms = (inference_end - cycle_start) * 1000.0
                snapshot_ms = (acquisition_end - cycle_start) * 1000.0
                inference_ms = (inference_end - acquisition_end) * 1000.0

                print(
                    "\n"
                    f"Camera 0 Raw Prediction: "
                    f"m_norm = {cam0_slope_raw:.4f} | "
                    f"b_norm = {cam0_intercept_raw:.4f} || "
                    f"Denorm: m = {cam0_slope:.4f} | "
                    f"b = {cam0_intercept:.4f} | "
                    f"x = {cam0_slope:.4f}y + {cam0_intercept:.4f} | "
                    f"Window = {cam0_window_us} us"
                )

                print(
                    f"Camera 1 Raw Prediction: "
                    f"m_norm = {cam1_slope_raw:.4f} | "
                    f"b_norm = {cam1_intercept_raw:.4f} || "
                    f"Denorm: m = {cam1_slope:.4f} | "
                    f"b = {cam1_intercept:.4f} | "
                    f"x = {cam1_slope:.4f}y + {cam1_intercept:.4f} | "
                    f"Window = {cam1_window_us} us"
                )

                print(
                    f"Total main-loop = {cycle_latency_ms:.2f} ms | "
                    f"Snapshot = {snapshot_ms:.2f} ms | "
                    f"Infer = {inference_ms:.2f} ms"
                )

                next_print_time = now + PRINT_INTERVAL_S

            # ------------------------------------------------------------
            # Visualization
            # ------------------------------------------------------------

            if need_visualization:
                cycle_latency_ms = (inference_end - cycle_start) * 1000.0

                update_preview(
                    cam0,
                    cam0_pos_snapshot,
                    cam0_neg_snapshot,
                    cam0_slope,
                    cam0_intercept,
                    cycle_latency_ms
                )

                update_preview(
                    cam1,
                    cam1_pos_snapshot,
                    cam1_neg_snapshot,
                    cam1_slope,
                    cam1_intercept,
                    cycle_latency_ms
                )

                next_vis_time = now + VIS_INTERVAL_S
                loop_hz = 1.0 / max(1e-9, inference_end - cycle_start)
                print(f"Main loop Hz: {loop_hz:.1f}")

except KeyboardInterrupt:
    print("\nStopping threaded two-camera SNN regression...")

finally:
    stop_camera_thread(cam0)
    stop_camera_thread(cam1)

    try:
        cam0["capture"].close()
    except Exception:
        pass

    try:
        cam1["capture"].close()
    except Exception:
        pass

    if ENABLE_VISUALIZATION:
        cv.destroyAllWindows()

    print("Closed cameras and windows.")