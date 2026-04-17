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
from spikingjelly.activation_based import functional
from model_definition import SNN_Net, CONFIG

PI = math.pi

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")

xNetwork = SNN_Net(
    tau=CONFIG["tau"],
    final_tau=CONFIG["final_tau"],
    hidden=CONFIG["hidden"],
    norm_type=CONFIG["norm_type"],
    learnable_norm=CONFIG["learnable_norm"],
    init_scale=CONFIG["init_scale"]
)

x_state = torch.load(r"C:\Users\sgalvao\snn_regression\models\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth", map_location=DEVICE)
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

y_state = torch.load(r"C:\Users\sgalvao\snn_regression\models\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth", map_location=DEVICE)
yNetwork.load_state_dict(y_state)
yNetwork.to(DEVICE)
yNetwork.eval()

functional.reset_net(xNetwork)
functional.reset_net(yNetwork)

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
    
    if torch.isnan(xOutput) or torch.isnan(yOutput):
        print("NaN detected")
        break

    xOutputDeg = xOutput.item() * 180
    yOutputDeg = yOutput.item() * 180

    latency = (time.perf_counter() - start) * 1000

    cv.imshow("Positive events", posFrame)
    cv.imshow("Negative events", negFrame)
    cv.waitKey(1)

    print(f"x Axis Prediction: {xOutput.item():.4f} rad | y Axis Prediction: {yOutput.item():.4f} rad | Latency: {latency:.2f} ms")
