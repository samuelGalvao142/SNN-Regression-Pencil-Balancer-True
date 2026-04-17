# =============================================================================
# Event camera viualization with opencv library
# =============================================================================
# Disclaimer: The architecture is based on code provided by Geronimo Marin 
# Hurtado, available at:
# https://github.com/Geronimo9177/snn-event-regression
# Most recent access: Feb 17, 2026
# =============================================================================
# This is a demo for the 2026 Lab Tour
# =============================================================================

import dv_processing as dv
import cv2 as cv
import numpy as np

capture = dv.io.camera.open()

cv.namedWindow("Preview", cv.WINDOW_NORMAL)

cv.namedWindow("Positive events", cv.WINDOW_NORMAL)
cv.namedWindow("Negative events", cv.WINDOW_NORMAL)

print(f"Opened [{capture.getCameraName()}] camera, it provides:")

if capture.isEventStreamAvailable():
    resolution = capture.getEventResolution()
    W, H = resolution
    print(f"* Frames at ({resolution[0]}x{resolution[1]}) resolution")

if capture.isImuStreamAvailable():
    print("* IMU measurements")

if capture.isTriggerStreamAvailable():
    print("* Triggers")

while capture.isRunning():
    frame = capture.getNextFrame()
    events = None
    while True:
        batch = capture.getNextEventBatch()
        if batch is None:
            break
        events = batch

    if frame is not None:
        print(f"Received a frame at time [{frame.timestamp}]")

        cv.imshow("Preview", frame.image)

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

    cv.imshow("Positive events", pos_frame)
    cv.imshow("Negative events", neg_frame)

    cv.waitKey(1)