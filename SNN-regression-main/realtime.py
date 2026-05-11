# =============================================================================
# SNN Implementation for real time control of mechanical systems from image
# processing and event cameras
# =============================================================================
# Disclaimer: The architecture is based on code provided by Geronimo Marin
# Hurtado, available at:
# https://github.com/Geronimo9177/snn-event-regression
# Most recent access: Feb 17, 2026
# =============================================================================
# This pipeline will have the purpose of processing and predicting the tilt
# angle of a pencil on top of a pencil balancing robot, from feedback provided
# by two Davis346 event cameras.
# Adaptations and real time implementation by Samuel Galvao, supervised by
# Professor Takashi Tanaka, at Purdue University.
# =============================================================================

import time

import dv_processing as dv
import numpy as np
import torch
from spikingjelly.activation_based import functional

from model_definition import CONFIG, SNN_Net

# out_port = serial.Serial('COM3', 115200)

EVENT_WINDOW_US = 2_000
PRINT_INTERVAL_S = 0.10
INACTIVITY_RESET_S = 0.10

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")

if DEVICE.type == "cuda":
    torch.backends.cudnn.benchmark = True


def build_model():
    model = SNN_Net(
        tau=CONFIG["tau"],
        final_tau=CONFIG["final_tau"],
        hidden=CONFIG["hidden"],
        norm_type=CONFIG["norm_type"],
        learnable_norm=CONFIG["learnable_norm"],
        init_scale=CONFIG["init_scale"],
    )
    return model.to(DEVICE).eval()


def reset_models(*models):
    for model in models:
        functional.reset_net(model)


bModel = build_model()
mModel = build_model()

bModel.load_state_dict(
    torch.load(
        r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True\models\lin_b\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth",
        map_location=DEVICE,
    )
)
mModel.load_state_dict(
    torch.load(
        r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True\models\lin_m\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth",
        map_location=DEVICE,
    )
)

reset_models(bModel, mModel)

capture = dv.io.camera.open()
capture.setEventsRunning(True)
capture.setFramesRunning(False)

if not capture.isEventStreamAvailable():
    raise RuntimeError("No event camera detected")

W, H = capture.getEventResolution()
print("Resolution:", (W, H))
print("Live inference started")

frame_np = np.zeros((1, 2, H, W), dtype=np.float32)
pos_frame = frame_np[0, 0]
neg_frame = frame_np[0, 1]
frame_cpu = torch.from_numpy(frame_np)
frame_device = frame_cpu if DEVICE.type == "cpu" else torch.empty_like(frame_cpu, device=DEVICE)

next_print_time = time.perf_counter()
last_event_wall_time = time.perf_counter()

with torch.inference_mode():
    while capture.isRunning():
        pos_frame.fill(0)
        neg_frame.fill(0)

        window_start_ts = None
        window_end_ts = None
        inference_start = time.perf_counter()

        while capture.isRunning():
            events = capture.getNextEventBatch()

            if events is None:
                now = time.perf_counter()
                if now - last_event_wall_time >= INACTIVITY_RESET_S:
                    reset_models(bModel, mModel)
                    last_event_wall_time = now
                continue

            events_np = events.numpy()
            if events_np.size == 0:
                continue

            xs = events_np["x"]
            ys = events_np["y"]
            ps = events_np["polarity"]
            ts = events_np["timestamp"]

            if window_start_ts is None:
                window_start_ts = int(ts[0])

            window_end_ts = int(ts[-1])
            last_event_wall_time = time.perf_counter()

            pos_frame[ys[ps == 1], xs[ps == 1]] += 1.0
            neg_frame[ys[ps == 0], xs[ps == 0]] += 1.0

            if window_end_ts - window_start_ts >= EVENT_WINDOW_US:
                break

        if window_start_ts is None:
            continue

        if DEVICE.type == "cuda":
            frame_device.copy_(frame_cpu, non_blocking=True)

        bOut = bModel(frame_device)
        mOut = mModel(frame_device)

        now = time.perf_counter()
        if now >= next_print_time:
            latency_ms = (now - inference_start) * 1000.0
            print(
                f"Prediction: m = {mOut.item():.4f} | "
                f"b = {bOut.item():.4f} | "
                f"x = {mOut.item():.4f}y + {bOut.item():.4f} | "
                f"Window = {window_end_ts - window_start_ts} us | "
                f"Latency: {latency_ms:.2f} ms"
            )
            next_print_time = now + PRINT_INTERVAL_S
