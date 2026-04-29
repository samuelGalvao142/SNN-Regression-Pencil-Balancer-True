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
# Currently running on event camera streamed data
# To do:
#   If output doesn't match camera movement, develop training rig and train SNN
# =============================================================================

import torch
import time
import math
import serial
import cv2 as cv
import numpy as np
import dv_processing as dv
from pathlib import Path
from spikingjelly.activation_based import functional
from model_definition import SNN_Net, CONFIG

PI = math.pi

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")
PROJECT_ROOT = Path(__file__).resolve().parents[1]
WEIGHTS_PATH = PROJECT_ROOT / "models" / f"model_{CONFIG['block_type']}_{CONFIG['norm_type']}_{CONFIG['optimizer']}" / "checkpoints_pencil" / "best_model_weights.pth"

if not WEIGHTS_PATH.exists():
    raise FileNotFoundError(f"Pencil model weights not found at: {WEIGHTS_PATH}")

xNetwork = SNN_Net(
    tau=CONFIG["tau"],
    final_tau=CONFIG["final_tau"],
    hidden=CONFIG["hidden"],
    norm_type=CONFIG["norm_type"],
    learnable_norm=CONFIG["learnable_norm"],
    init_scale=CONFIG["init_scale"]
)

x_state = torch.load(WEIGHTS_PATH, map_location=DEVICE)
xNetwork.load_state_dict(x_state)
xNetwork.to(DEVICE)
xNetwork.eval()

yNetwork = SNN_Net(
    tau=CONFIG["tau"],
    final_tau=CONFIG["final_tau"],
    hidden=CONFIG["hidden"],
    norm_type=CONFIG["norm_type"],
    learnable_norm=CONFIG["learnable_norm"],
    init_scale=CONFIG["init_scale"]
)

y_state = torch.load(WEIGHTS_PATH, map_location=DEVICE)
yNetwork.load_state_dict(y_state)
yNetwork.to(DEVICE)
yNetwork.eval()

functional.reset_net(xNetwork)
functional.reset_net(yNetwork)


def unpack_prediction(output_tensor):
    """
    Convert a network output shaped like [1, 2] into angle/position scalars.
    """
    prediction = output_tensor.detach().reshape(-1).cpu().numpy()
    if prediction.size != 2:
        raise ValueError(f"Expected 2 outputs [angle, position], got shape {tuple(output_tensor.shape)}")

    angle_rad = float(prediction[0])
    position = float(prediction[1])
    return angle_rad, position

# =============================================================================
# Testing with the event camera (single camera)
# =============================================================================

camera = dv.io.camera.open()

if not camera.isEventStreamAvailable():
    raise RuntimeError("No event camera detected")

resolution = camera.getEventResolution()
W, H = resolution
print("Resolution", resolution)

posFrame = np.zeros((H,W), dtype=np.float32)
negFrame = np.zeros((H,W), dtype=np.float32)

cv.namedWindow("Positive events", cv.WINDOW_NORMAL)
cv.namedWindow("Negative events", cv.WINDOW_NORMAL)

while camera.isRunning():
    start = time.perf_counter()
    events = None
    while True:
        batch = camera.getNextEventBatch()
        if batch is None:
            break
        events = batch
    
    if events is None:
        continue

    events_np = events.numpy()

    xs = events_np['x']
    ys = events_np['y']
    ps = events_np['polarity']

    posMask = ps == 1
    negMask = ps == 0

    posFrame.fill(0)
    negFrame.fill(0)

    posFrame[ys[posMask], xs[posMask]] += 1
    negFrame[ys[negMask], xs[negMask]] += 1

    eventFrame = np.stack([posFrame, negFrame], axis = 0)
    eventFrame = torch.from_numpy(eventFrame).unsqueeze(0).to(DEVICE)

    with torch.no_grad():
        xOutput = xNetwork(eventFrame)
        yOutput = yNetwork(eventFrame)
    
    if torch.isnan(xOutput).any() or torch.isnan(yOutput).any():
        print("NaN detected")
        break

    xAngleRad, xPosition = unpack_prediction(xOutput)
    yAngleRad, yPosition = unpack_prediction(yOutput)

    xOutputDeg = np.rad2deg(xAngleRad)
    yOutputDeg = np.rad2deg(yAngleRad)

    latency = (time.perf_counter() - start) * 1000

    cv.imshow("Positive events", posFrame)
    cv.imshow("Negative events", negFrame)
    cv.waitKey(1)

    print(
        f"X camera -> angle: {xAngleRad:.4f} rad ({xOutputDeg:.2f} deg), position: {xPosition:.4f} | "
        f"Y camera -> angle: {yAngleRad:.4f} rad ({yOutputDeg:.2f} deg), position: {yPosition:.4f} | "
        f"Latency: {latency:.2f} ms"
    )
