# =============================================================================
# Event camera viualization and recording of Hough Transform output
# =============================================================================
# Disclaimer: The architecture is based on code provided by Geronimo Marin 
# Hurtado, available at:
# https://github.com/Geronimo9177/snn-event-regression
# Most recent access: Apr 21, 2026
# =============================================================================
# Test Test
# Library imports
import dv_processing as dv
import cv2 as cv
import numpy as np
import time
import os
import csv
from pathlib import Path

# Setting output path
output_path = Path(r"C:\Users\samue\PURDUE-2026\Datasets")
output_path.parent.mkdir(parents=True, exist_ok=True)
csv_path = output_path / "hough_lines_csv"
csv_file = open(csv_path, mode="w", newline="")
csv_writer = csv.writer(csv_file)
csv_writer.writerow(["timestamp_us", "theta", "x1"])

# DV-Processing: accessing event camera and opening visualization windows
capture = dv.io.camera.open()
cv.namedWindow("Preview", cv.WINDOW_NORMAL)
cv.namedWindow("Events", cv.WINDOW_NORMAL)

resolution = capture.getEventResolution()
W, H = resolution

# DVS Writer setup
config = dv.io.MonoCameraWriter.DAVISConfig("DAVIS346_sample", resolution)
writer = dv.io.MonoCameraWriter(str(output_path / "output_davis346.aedat4"), config)

img = np.zeros((H, W), dtype=np.uint8)

# Main loop: Visualization and recording data from Hough Space Transform
while capture.isRunning():
    img.fill(0)

    frame = capture.getNextFrame()
    events = None
    while True:
        batch = capture.getNextEventBatch()
        if batch is None:
            break
        events = batch

    if frame is not None:
        writer.writeFrame(frame)
        cv.imshow("Preview", frame.image)

    if events is None:
        continue

    writer.writeEvents(events)

    # Treating the data: events are received as positive and negative
    # Here we build the positive and negative frames
    events_np = events.numpy()
    xs = events_np['x']
    ys = events_np['y']
    ps = events_np['polarity']
    ts = events_np['timestamp']

    np.add.at(img, (ys, xs), 1)

    img = cv.GaussianBlur(img, (3, 3), 0)
    img = cv.normalize(img, None, 0, 255, cv.NORM_MINMAX).astype(np.uint8)

    edges = cv.Canny(img, 30, 90)

    lines = cv.HoughLinesP(
        edges,
        rho=1,
        theta=np.pi / 180,
        threshold=40,
        minLineLength=25,
        maxLineGap=10
    )

    vis = cv.cvtColor(img, cv.COLOR_GRAY2BGR)

    if lines is not None:
        for l in lines:
            x1, y1, x2, y2 = l[0]
        
        cv.line(vis, (x1, y1), (x2, y2), (0, 255, 0), 2)

        dx = x2 - x1
        dy = y2 - y1
        theta = np.arctan2(dy,dx)
        rho = x1 * np.cos(theta) + y1 * np.sin(theta)

        timestamp = int(np.mean(ts))
        
        csv_writer.writerow([timestamp, theta, x1])

    cv.imshow("Events", vis)

    if cv.waitKey(1) == 27:
        break

csv_file.close()
capture.stop()