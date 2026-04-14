import pandas as pd
import dv_processing as dv
import tonic
import numpy as np
from scipy.interpolate import interp1d
from ahrs.filters import Madgwick
    
    
def _align_and_slice_events(events, timestamps, data_labels, time_window, START_FRAME, END_FRAME):
    """
    Auxiliary function to temporally align events with measurements,
    slice into frames, and assign interpolated labels.
    """
    # ============================================================================
    # Temporal Alignment: Synchronize events with measurements
    # ============================================================================
    # Event cameras and sensors may not start/stop at exactly the same time.
    # We find the overlapping time range to ensure valid event-label pairs.
    
    # Determine the common time window
    start_time = max(timestamps[0], events['t'][0])
    end_time = min(timestamps[-1], events['t'][-1])
    
    # Filter measurements to valid time range
    mask = (timestamps >= start_time) & (timestamps <= end_time)
    timestamps = timestamps[mask]
    data_labels = data_labels[mask]
    
    print(f"Total events before filtering: {len(events['t'])}")
    
    # ============================================================================
    # Time-based Slicing: Convert continuous event stream into discrete frames
    # ============================================================================
    # Event cameras produce asynchronous events. We group them into fixed time
    # windows to create a sequence of "frames" for the SNN.
    
    # Slice events into time windows
    events_per_frame = tonic.slicers.slice_events_by_time(
        events, 
        time_window=time_window, 
        start_time=start_time, 
        end_time=end_time
    )
    
    # ============================================================================
    # Label interpolation and assignment
    # ============================================================================
    # Measurements are sampled at discrete times, but we need a label for each
    # event frame. We use linear interpolation to estimate values at the END
    # of each time window.
    
    # Compute the end timestamp for each window
    # Window i ends at: start_time + (i+1) * time_window
    ev_timestamps = start_time + np.arange(1, len(events_per_frame) + 1) * time_window
    
    # Interpolate all labels at these timestamps
    interp_func = interp1d(timestamps, data_labels, fill_value="extrapolate")
    interpolated_labels = interp_func(ev_timestamps)
    
    print(f"Labels assigned successfully!")
    print(f"Total grouped events: {len(events_per_frame)}")
    print(f"Total labels assigned: {len(interpolated_labels)}")
    print(f"Frames read: {len(events_per_frame)}")
    print(f"Events read: {len(events)}")
    
    # Slice frames to specified range
    events_per_frame = events_per_frame[START_FRAME:END_FRAME]
    interpolated_labels = interpolated_labels[START_FRAME:END_FRAME]
    
    return events_per_frame, interpolated_labels

def read_pendulum_file(FILE_PATH, CSV_PATH, time_window=30000, START_FRAME=0, END_FRAME=-1):
    """Read pendulum event data and encoder measurements, then align and slice them."""
    
    print(f"\n{'='*70}")
    print("Starting data loading for Pendulum experiment...")
    print(f"{'='*70}")

    # Load CSV measurements
    df = pd.read_csv(CSV_PATH)

    # Detect timestamp and angle columns
    timestamps = df['timestamp_us'].values
    if 'theta_rad' in df.columns:
        angle = df['theta_rad'].values
    elif 'angle_rad' in df.columns:
        angle = df['angle_rad'].values
    else:
        raise ValueError('No angle column found in CSV (expected theta_rad)')

    # Attempt to find horizontal position columns for both cameras
    col_candidates_cam1 = ['x_px_cam1', 'x_cam1', 'x_px1', 'x1', 'x_pos_cam1', 'x_px']
    col_candidates_cam2 = ['x_px_cam2', 'x_cam2', 'x_px2', 'x2', 'x_pos_cam2']

    pos1 = None
    pos2 = None
    for c in col_candidates_cam1:
        if c in df.columns:
            pos1 = df[c].values
            break
    for c in col_candidates_cam2:
        if c in df.columns:
            pos2 = df[c].values
            break

    # If only a single position column exists (e.g., 'x_px'), replicate for both cameras
    if pos1 is None and 'x_px' in df.columns:
        pos1 = df['x_px'].values
    if pos2 is None and 'x_px' in df.columns:
        pos2 = df['x_px'].values

    # Fallback to zeros if positions not present
    if pos1 is None:
        pos1 = np.zeros_like(angle)
    if pos2 is None:
        pos2 = np.zeros_like(angle)

    # Load event data: support single path or list/tuple of two paths
    if isinstance(FILE_PATH, (list, tuple)) and len(FILE_PATH) == 2:
        events1 = tonic.io.read_aedat4(FILE_PATH[0])
        events2 = tonic.io.read_aedat4(FILE_PATH[1])

        # Determine overlapping time range
        start_time = max(timestamps[0], events1['t'][0], events2['t'][0])
        end_time = min(timestamps[-1], events1['t'][-1], events2['t'][-1])

        # Slice both event streams into frames
        frames1 = tonic.slicers.slice_events_by_time(events1, time_window=time_window, start_time=start_time, end_time=end_time)
        frames2 = tonic.slicers.slice_events_by_time(events2, time_window=time_window, start_time=start_time, end_time=end_time)

        # Interpolate labels at window end timestamps (based on start_time)
        ev_timestamps = start_time + np.arange(1, len(frames1) + 1) * time_window
        interp_angle = interp1d(timestamps, angle, fill_value='extrapolate')
        interp_pos1 = interp1d(timestamps, pos1, fill_value='extrapolate')
        interp_pos2 = interp1d(timestamps, pos2, fill_value='extrapolate')

        interpolated_angle = interp_angle(ev_timestamps)
        interpolated_pos1 = interp_pos1(ev_timestamps)
        interpolated_pos2 = interp_pos2(ev_timestamps)

        # Zip frames into pairs and build labels as [angle, x_pos, angle, x_pos]
        min_len = min(len(frames1), len(frames2), len(interpolated_angle))
        frames_paired = [(frames1[i], frames2[i]) for i in range(min_len)]
        labels = np.stack([
            interpolated_angle[:min_len],
            interpolated_pos1[:min_len],
            interpolated_angle[:min_len],
            interpolated_pos2[:min_len]
        ], axis=1)

        # Slice requested frame range
        frames_paired = frames_paired[START_FRAME:END_FRAME]
        labels = labels[START_FRAME:END_FRAME]

        print(f"Labels assigned successfully (two cameras)!")
        print(f"Total grouped event pairs: {len(frames_paired)}")

        return frames_paired, labels

    else:
        # Single camera (legacy behavior)
        events = tonic.io.read_aedat4(FILE_PATH)
        events_per_frame, interpolated_angle = _align_and_slice_events(events, timestamps, angle, time_window, START_FRAME, END_FRAME)

        # Interpolate positions for single-camera case
        interp_pos = interp1d(timestamps, pos1, fill_value='extrapolate')
        ev_timestamps = timestamps[0] + np.arange(1, len(events_per_frame) + 1) * time_window
        interpolated_pos = interp_pos(ev_timestamps)

        # Build labels as [angle, x_pos, angle, x_pos] duplicating angle for both camera slots
        labels = np.stack([
            interpolated_angle,
            interpolated_pos,
            interpolated_angle,
            interpolated_pos
        ], axis=1)

        return events_per_frame, labels

def read_IMU_file(FILE_PATH, time_window=10000, START_FRAME=0, END_FRAME=-1):
    """Read IMU event data and compute orientation (pitch/roll) using Madgwick filter."""
    
    print(f"\n{'='*70}")
    print("Starting data loading for IMU experiment...")
    print(f"{'='*70}")
    
    # Load event data using Tonic library
    events = tonic.io.read_aedat4(FILE_PATH)
    
    # Load IMU data using dv_processing library
    reader = dv.io.MonoCameraRecording(FILE_PATH)
    
    timestamps = []
    acc = []
    gyro = []
    
    # Read all IMU measurements
    while reader.isRunning():
        imu_batch = reader.getNextImuBatch()
        if imu_batch is not None and len(imu_batch) > 0:
            for m in imu_batch:
                timestamps.append(m.timestamp)
                acc.append([m.accelerometerX, m.accelerometerY, m.accelerometerZ])
                gyro.append([m.gyroscopeX, m.gyroscopeY, m.gyroscopeZ])
    
    timestamps = np.array(timestamps)
    acc = np.array(acc)
    gyro = np.array(gyro)
    
    # Convert gyroscope from degrees to radians
    gyro = np.deg2rad(gyro)
    
    # ============================================================================
    # Madgwick Filter - Orientation Estimation from Inertial Measurement Unit (IMU)
    # ============================================================================
    # Automatic initialization (the smart way)
    # Initialize Madgwick filter with first IMU sample to compute initial quaternion
    
    init_madgwick = Madgwick(acc=acc[:1], gyr=gyro[:1], frequency=1000)
    
    # Extract the computed initial quaternion
    q = init_madgwick.Q[0]
    
    madgwick = Madgwick(gain=0.033)  # IMU-only, recommended value
    
    Q = [q]
    
    # Process all IMU measurements through Madgwick filter
    for k in range(1, len(acc)):
        # Real timestep in seconds
        dt = (timestamps[k] - timestamps[k-1]) * 1e-6
        madgwick.Dt = dt
        
        q = madgwick.updateIMU(q, gyr=gyro[k], acc=acc[k])
        Q.append(np.array(q))
    
    Q = np.array(Q)
    
    # Extract quaternion components (w, x, y, z)
    w = Q[:, 0]
    x = Q[:, 1]
    y = Q[:, 2]
    z = Q[:, 3]
    
    # Calculate Roll (Rotation around X-axis)
    sinr_cosp = 2 * (w * x + y * z)
    cosr_cosp = 1 - 2 * (x * x + y * y)
    roll = np.arctan2(sinr_cosp, cosr_cosp)
    
    # Use auxiliary function for alignment and slicing
    events_per_frame, labels = _align_and_slice_events(
        events, timestamps, roll, time_window, START_FRAME, END_FRAME
    )
        
    return events_per_frame, labels