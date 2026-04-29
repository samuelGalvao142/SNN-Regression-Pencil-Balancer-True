import torch
import time
import math
#import serial
import numpy as np
import dv_processing as dv
from spikingjelly.activation_based import functional
from model_definition import SNN_Net, CONFIG
import cv2 as cv
from pathlib import Path

#out_port = serial.Serial('COM3', 115200)
PI = math.pi

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")
PROJECT_ROOT = Path(__file__).resolve().parents[1]
WEIGHTS_PATH = PROJECT_ROOT / "models" / f"model_{CONFIG['block_type']}_{CONFIG['norm_type']}_{CONFIG['optimizer']}" / "checkpoints_pencil" / "best_model_weights.pth"

xNetwork = SNN_Net(
    tau=CONFIG["tau"],
    final_tau=CONFIG["final_tau"],
    hidden=CONFIG["hidden"],
    norm_type=CONFIG["norm_type"],
    learnable_norm=CONFIG["learnable_norm"],
    init_scale=CONFIG["init_scale"]
)

if not WEIGHTS_PATH.exists():
    raise FileNotFoundError(f"Pencil model weights not found at: {WEIGHTS_PATH}")

x_state = torch.load(WEIGHTS_PATH, map_location=DEVICE)
xNetwork.load_state_dict(x_state)
xNetwork.to(DEVICE)
xNetwork.eval()

functional.reset_net(xNetwork)


def unpack_prediction(output_tensor):
    prediction = output_tensor.detach().reshape(-1).cpu().numpy()
    if prediction.size != 2:
        raise ValueError(f"Expected 2 outputs [angle, position], got shape {tuple(output_tensor.shape)}")
    return float(prediction[0]), float(prediction[1])

#yNetwork = SNN_Net(
#    tau=CONFIG["tau"],
#    final_tau=CONFIG["final_tau"],
#    hidden=CONFIG["hidden"],
#    norm_type=CONFIG["norm_type"],
#    learnable_norm=CONFIG["learnable_norm"],
#    init_scale=CONFIG["init_scale"]
#)

#y_state = torch.load(r"C:\Users\sgalvao\snn_regression\models\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth", map_location=DEVICE)
#yNetwork.load_state_dict(y_state)
#yNetwork.to(DEVICE)
#yNetwork.eval()

#functional.reset_net(yNetwork)

capture = dv.io.camera.open()

#capture.setEventsRunning(True)
#capture.setFramesRunning(True)

if not capture.isEventStreamAvailable():
    raise RuntimeError("No event camera detected")

resolution = capture.getEventResolution()
W, H = resolution
print("Resolution:", resolution)

print("Live inference started")

x_pos_frame = np.zeros((H, W), dtype=np.float32)
x_neg_frame = np.zeros((H, W), dtype=np.float32)

cv.namedWindow("Positive events", cv.WINDOW_NORMAL)
cv.namedWindow("Negative events", cv.WINDOW_NORMAL)

while capture.isRunning():
    start = time.perf_counter()

    xEvents = capture.getNextEventBatch()

    if xEvents is None:
        print("No events")
        continue
    
    #frame = capture.getNextFrame()
    print("Events received")

    xEvents_np = xEvents.numpy()
    #yEvents_np = yEvents.numpy()

    x_xs = xEvents_np['x']
    x_ys = xEvents_np['y']
    x_ps = xEvents_np['polarity']

    #y_xs = yEvents_np['x']
    #y_ys = yEvents_np['y']
    #y_ps = yEvents_np['polarity']

    x_pos_mask = x_ps == 1
    x_neg_mask = x_ps == 0

    #y_pos_mask = y_ps == 1
    #y_neg_mask = y_ps == 0

    x_pos_frame.fill(0)
    x_neg_frame.fill(0)
    
    #y_pos_frame = np.zeros((H, W), dtype=np.float32)
    #y_neg_frame = np.zeros((H, W), dtype=np.float32)

    x_pos_frame[x_ys[x_pos_mask], x_xs[x_pos_mask]] += 1
    x_neg_frame[x_ys[x_neg_mask], x_xs[x_neg_mask]] += 1

    #y_pos_frame[y_ys[y_pos_mask], y_xs[y_pos_mask]] += 1
    #y_neg_frame[y_ys[y_neg_mask], y_xs[y_neg_mask]] += 1

    xFrame = np.stack([x_pos_frame, x_neg_frame], axis=0)
    xFrame = torch.from_numpy(xFrame).unsqueeze(0).to(DEVICE)

    #yFrame = np.stack([y_pos_frame, y_neg_frame], axis=0)
    #yFrame = torch.from_numpy(yFrame).unsqueeze(0).to(DEVICE)

    with torch.no_grad():
        x_output = xNetwork(xFrame)
    #    y_output = yNetwork(yFrame)

    if torch.isnan(x_output).any():
        print("NaN detected")
        break

    x_angle_rad, x_position = unpack_prediction(x_output)
    x_output_deg = np.rad2deg(x_angle_rad)

    latency = (time.perf_counter() - start) * 1000

    #if frame is not None:
    #    cv.imshow("Preview", frame.image)

    cv.imshow("Positive events", x_pos_frame)
    cv.imshow("Negative events", x_neg_frame)
    cv.waitKey(1)

    #print(f"x Axis Prediction: {x_output.item():.4f} rad | y Axis Prediction: {y_output.item():.4f} rad | Latency: {latency:.2f} ms")
    print(f"x Axis Prediction: angle={x_angle_rad:.4f} rad ({x_output_deg:.2f} deg) | position={x_position:.4f} | Latency: {latency:.2f} ms")
