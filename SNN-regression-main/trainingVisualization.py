import dv_processing as dv
import cv2 as cv
import numpy as np

FILE_PATH = r"C:\Users\sgalvao\snn_regression\Pendulum-training-data\pendulum_events-001.aedat4"

capture = dv.io.MonoCameraRecording(FILE_PATH)

cv.namedWindow("3ms Events", cv.WINDOW_NORMAL)

resolution = capture.getEventResolution()
W, H = resolution

TIME_WINDOW_US = 3000  # 3 milliseconds
# 333 fps is what 3ms gives us
max_frames = 10 * 333  # seconds

slice_start_time = None

# White background frame (BGR)
frame = np.ones((H, W, 3), dtype=np.uint8) * 255
frame_count = 0

while capture.isRunning():

    events = capture.getNextEventBatch()
    if events is None:
        break

    events_np = events.numpy()

    xs = events_np['x']
    ys = events_np['y']
    ps = events_np['polarity']
    ts = events_np['timestamp']

    for i in range(len(ts)):

        if slice_start_time is None:
            slice_start_time = ts[i]

        # Paint pixel immediately
        if ps[i] == 1:
            frame[ys[i], xs[i]] = (0, 0, 255)   # Red (positive)
        else:
            frame[ys[i], xs[i]] = (255, 0, 0)   # Blue (negative)

        # If 3ms passed → show frame
        if ts[i] - slice_start_time >= TIME_WINDOW_US:

            cv.imshow("3ms Events", frame)

            if cv.waitKey(1) == 27:
                break

            # Reset for next slice
            frame[:] = 255
            slice_start_time = None
            frame_count += 1

    if frame_count > max_frames:
        break

cv.destroyAllWindows()