import dv_processing as dv
import cv2 as cv
import numpy as np
import time
import os

print("Working directory:", os.getcwd())

capture = dv.io.camera.open()

capture.setEventsRunning(True)
capture.setFramesRunning(True)

if not capture.isEventStreamAvailable():
    raise RuntimeError("No event camera detected")

resolution = capture.getEventResolution()
W, H = resolution
print("Resolution:", resolution)

# -----------------------------
# AEDAT4 WRITER SETUP
# -----------------------------
event_config = dv.io.MonoCameraWriter.EventStream(
    resolution=resolution
)

writer = dv.io.MonoCameraWriter(
    "recording.aedat4",
    event_config
)

while capture.isRunning():

    start = time.perf_counter()

    events = capture.getNextEventBatch()
    if events is None:
        continue

    # Write raw events directly to file
    writer.writeEvents(events)

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

    latency = (time.perf_counter() - start) * 1000

writer.close()