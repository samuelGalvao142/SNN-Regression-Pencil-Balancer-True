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
import numpy as np
import dv_processing as dv
from spikingjelly.activation_based import functional
from model_definition import SNN_Net, CONFIG

out_port = serial.Serial('COM3', 115200)
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

checkpoint = torch.load(r"C:\Users\sgalvao\snn_regression\models\model_SEW_BN\checkpoints_pendulum\checkpoint_best.pth", map_location=DEVICE)
xNetwork.load_state_dict(torch.load(r"C:\Users\sgalvao\snn_regression\models\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth"))

xNetwork.to(DEVICE)
xNetwork.eval()

functional.reset_net(xNetwork)

yNetwork = SNN_Net(
    tau=CONFIG["tau"],
    final_tau=CONFIG["final_tau"],
    hidden=CONFIG["hidden"],
    norm_type=CONFIG["norm_type"],
    learnable_norm=CONFIG["learnable_norm"],
    init_scale=CONFIG["init_scale"]
)

yNetwork.load_state_dict(torch.load(r"C:\Users\sgalvao\snn_regression\models\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth"))

yNetwork.to(DEVICE)
yNetwork.eval()

functional.reset_net(yNetwork)

# =============================================================================
# Testing with prerecorded footage
# =============================================================================
#reader = io.MonoCameraRecording(r"C:\Users\sgalvao\snn_regression\Pendulum-training-data\pendulum_events-001.aedat4")

#resolution = reader.getEventResolution()
#H, W = resolution[1], resolution[0]

#accumulator = dv.Accumulator(resolution)
#accumulator.setDecayFunction(dv.Accumulator.Decay.EXPONENTIAL)
#accumulator.setDecayParam(1e6)

#print("Starting replay...")

#while reader.isRunning():
#    events = reader.getNextEventBatch()
#    if events is None:
#        break

#    events_np = events.numpy()

#    pos_frame = np.zeros((H, W), dtype=np.float32)
#    neg_frame = np.zeros((H, W), dtype=np.float32)

#    xs = events_np['x']
#    ys = events_np['y']
#    ps = events_np['polarity']

#    pos_mask = ps == 1
#    neg_mask = ps == 0
    
#    pos_frame[ys[pos_mask], xs[pos_mask]] += 1
#    neg_frame[ys[neg_mask], xs[neg_mask]] += 1

#    frame = np.stack([pos_frame, neg_frame], axis=0)
#    frame = torch.from_numpy(frame).unsqueeze(0).to(DEVICE)

#    with torch.no_grad():
#        output = model(frame)

#    print(output.item())

# =============================================================================
# Testing with random frame generation
# =============================================================================
"""
try:
    while True:

        frame = torch.randn(1, 2, 346, 260).to(DEVICE)
        
        start = time.perf_counter()

        with torch.no_grad():
            output = model(frame)

        latency = (time.perf_counter() - start) * 1000

        print(f"Prediction: {output.item():.4f} | Latency: {latency:.2f} ms")
except KeyboardInterrupt:
    print("Stopping real time inference")
"""


# =============================================================================
# Testing with the event camera
# =============================================================================
cameras = dv.io.camera.discover()

xCapture = dv.io.camera.open(cameras[0])
yCapture = dv.io.camera.open(cameras[1])

xCapture.setEventsRunning(True)
xCapture.setFramesRunning(True)

yCapture.setEventsRunning(True)
yCapture.setFramesRunning(True)

if not xCapture.isEventStreamAvailable():
    raise RuntimeError("No event camera detected (X axis)")

if not yCapture.isEventStreamAvailable():
    raise RuntimeError("No event camera detected (Y axis)")

resolution = xCapture.getEventResolution()
W, H = resolution
print("Resolution:", resolution)

print("Live inference started")

while xCapture.isRunning() and yCapture.isRunning():

    start = time.perf_counter()

    xEvents = xCapture.getNextEventBatch()
    yEvents = yCapture.getNextEventBatch()
    if xEvents is None or yEvents is None:
        continue

    xEvents_np = xEvents.numpy()
    yEvents_np = yEvents.numpy()

    x_xs = xEvents_np['x']
    x_ys = xEvents_np['y']
    x_ps = xEvents_np['polarity']

    y_xs = yEvents_np['x']
    y_ys = yEvents_np['y']
    y_ps = yEvents_np['polarity']

    x_pos_mask = x_ps == 1
    x_neg_mask = x_ps == 0

    y_pos_mask = y_ps == 1
    y_neg_mask = y_ps == 0

    x_pos_frame = np.zeros((H, W), dtype=np.float32)
    x_neg_frame = np.zeros((H, W), dtype=np.float32)
    
    y_pos_frame = np.zeros((H, W), dtype=np.float32)
    y_neg_frame = np.zeros((H, W), dtype=np.float32)

    x_pos_frame[x_ys[x_pos_mask], x_xs[x_pos_mask]] += 1
    x_neg_frame[x_ys[x_neg_mask], x_xs[x_neg_mask]] += 1

    y_pos_frame[y_ys[y_pos_mask], y_xs[y_pos_mask]] += 1
    y_neg_frame[y_ys[y_neg_mask], y_xs[y_neg_mask]] += 1

    xFrame = np.stack([x_pos_frame, x_neg_frame], axis=0)
    xFrame = torch.from_numpy(xFrame).unsqueeze(0).to(DEVICE)

    yFrame = np.stack([y_pos_frame, y_neg_frame], axis=0)
    yFrame = torch.from_numpy(yFrame).unsqueeze(0).to(DEVICE)

    with torch.no_grad():
        x_output = xNetwork(xFrame)
        y_output = yNetwork(yFrame)

    x_output_deg = x_output.item() * 180
    y_output_deg = y_output.item() * 180

    latency = (time.perf_counter() - start) * 1000

    print(f"x Axis Prediction: {x_output.item():.4f} rad | y Axis Prediction: {y_output.item():.4f} rad | Latency: {latency:.2f} ms")


    out_port.write(f"{x_output_deg}\n".encode())
    out_port.write(f"{y_output_deg}\n".encode())