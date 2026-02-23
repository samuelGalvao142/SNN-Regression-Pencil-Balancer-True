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

model = SNN_Net(
    tau=CONFIG["tau"],
    final_tau=CONFIG["final_tau"],
    hidden=CONFIG["hidden"],
    norm_type=CONFIG["norm_type"],
    learnable_norm=CONFIG["learnable_norm"],
    init_scale=CONFIG["init_scale"]
)

checkpoint = torch.load(r"C:\Users\sgalvao\snn_regression\models\model_SEW_BN\checkpoints_pendulum\checkpoint_best.pth", map_location=DEVICE)
model.load_state_dict(torch.load(r"C:\Users\sgalvao\snn_regression\models\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth"))

model.to(DEVICE)
model.eval()

functional.reset_net(model)

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
capture = dv.io.camera.open()

capture.setEventsRunning(True)
capture.setFramesRunning(True)

if not capture.isEventStreamAvailable():
    raise RuntimeError("No event camera detected")

resolution = capture.getEventResolution()
W, H = resolution
print("Resolution:", resolution)

print("Live inference started")

while capture.isRunning():

    start = time.perf_counter()

    events = capture.getNextEventBatch()
    if events is None:
        continue

    events_np = events.numpy()

    xs = events_np['x']
    ys = events_np['y']
    ps = events_np['polarity']

    pos_mask = ps == 1
    neg_mask = ps == 0

    pos_frame = np.zeros((H, W), dtype=np.float32)
    neg_frame = np.zeros((H, W), dtype=np.float32)

    pos_frame[ys[pos_mask], xs[pos_mask]] += 1
    neg_frame[ys[neg_mask], xs[neg_mask]] += 1

    frame = np.stack([pos_frame, neg_frame], axis=0)
    frame = torch.from_numpy(frame).unsqueeze(0).to(DEVICE)

    with torch.no_grad():
        output = model(frame)

    output_deg = output.item() * 180 / PI

    latency = (time.perf_counter() - start) * 1000

    print(f"Prediction: {output.item():.4f} rad | {output_deg:.1f} degrees | Latency: {latency:.2f} ms")

    out_port.write(f"{output_deg}\n".encode())