import csv
import torch
import time
import math
import numpy as np
import dv_processing as dv
from spikingjelly.activation_based import functional
from model_definition import SNN_Net, CONFIG

# out_port = serial.Serial('COM3', 115200)

csv_file_path = r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True\tests\snntest.csv"

with open(csv_file_path, 'w', newline='') as file:
    writer = csv.writer(file)
    field = ["timestep", "lin_m", "lin_b"]
    writer.writerow(field)

capture = dv.io.camera.open()

capture.setEventsRunning(True)
capture.setFramesRunning(True)

if not capture.isEventStreamAvailable():
    raise RuntimeError("No event camera detected")

resolution = capture.getEventResolution()
W, H = resolution

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")

bModel = SNN_Net(
    tau=CONFIG["tau"],
    final_tau=CONFIG["final_tau"],
    hidden=CONFIG["hidden"],
    norm_type=CONFIG["norm_type"],
    learnable_norm=CONFIG["learnable_norm"],
    init_scale=CONFIG["init_scale"]
)

mModel = SNN_Net(
    tau=CONFIG["tau"],
    final_tau=CONFIG["final_tau"],
    hidden=CONFIG["hidden"],
    norm_type=CONFIG["norm_type"],
    learnable_norm=CONFIG["learnable_norm"],
    init_scale=CONFIG["init_scale"]
)

bModel.load_state_dict(torch.load(r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True\models\lin_b\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth"))
mModel.load_state_dict(torch.load(r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True\models\lin_m\model_SEW_BN\checkpoints_pendulum\best_model_weights.pth"))

bModel.to(DEVICE)
mModel.to(DEVICE)
bModel.eval()
mModel.eval()

functional.reset_net(bModel)
functional.reset_net(mModel)

pos_frame = np.zeros((H, W), dtype=np.float32)
neg_frame = np.zeros((H, W), dtype=np.float32)

reader = dv.io.MonoCameraRecording(r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True\Dataset\hough_cam1_cam1.aedat4")

timestep = 0

while reader.isRunning():
    events = reader.getNextEventBatch()
    if events is not None:
        start = time.perf_counter()
        
        pos_frame.fill(0)
        neg_frame.fill(0)
        
        events_np = events.numpy()

        xs = events_np['x']
        ys = events_np['y']
        ps = events_np['polarity']

        pos_mask = ps == 1
        neg_mask = ps == 0

        pos_frame[ys[pos_mask], xs[pos_mask]] += 1
        neg_frame[ys[neg_mask], xs[neg_mask]] += 1

        frame = np.stack([pos_frame, neg_frame], axis=0)
        frame = torch.from_numpy(frame).unsqueeze(0).to(DEVICE)

        with torch.no_grad():
            bOut = bModel(frame)
            mOut = mModel(frame)

        

        latency = (time.perf_counter() - start) * 1000

        print(f"Prediction: m = {mOut.item():.4f} | b = {bOut.item():.4f} | x = {mOut.item():.4f}y + {bOut.item():.4f} | Latency: {latency:.2f} ms")

        with open(csv_file_path, 'a', newline='') as file:
            writer = csv.writer(file)
            field = [str(timestep), str(mOut.item()), str(bOut.item())]
            writer.writerow(field)

        timestep += 1