from pathlib import Path

import dv_processing as dv
import numpy as np
import pandas as pd
import tonic
from ahrs.filters import Madgwick
from scipy.interpolate import interp1d


def load_label_file(label_path, separator=None):
    """Load a TXT or CSV label file and normalize column names."""
    label_path = Path(label_path)

    if separator is not None:
        df = pd.read_csv(label_path, sep=separator, engine="python")
    else:
        try:
            df = pd.read_csv(label_path)
        except pd.errors.ParserError:
            df = pd.read_csv(label_path, sep=r"\s+", engine="python")

    df.columns = [str(column).strip() for column in df.columns]
    return df


def _align_and_slice_events(events, timestamps, data_labels, time_window, START_FRAME, END_FRAME):
    """
    Temporally align events with measurements, slice into frames,
    and assign interpolated labels.
    """
    start_time = max(timestamps[0], events["t"][0])
    end_time = min(timestamps[-1], events["t"][-1])

    mask = (timestamps >= start_time) & (timestamps <= end_time)
    timestamps = timestamps[mask]
    data_labels = data_labels[mask]

    print(f"Total events before filtering: {len(events['t'])}")

    events_per_frame = tonic.slicers.slice_events_by_time(
        events,
        time_window=time_window,
        start_time=start_time,
        end_time=end_time,
    )

    ev_timestamps = start_time + np.arange(1, len(events_per_frame) + 1) * time_window
    interp_func = interp1d(timestamps, data_labels, axis=0, fill_value="extrapolate")
    interpolated_labels = interp_func(ev_timestamps).astype(np.float32)

    print("Labels assigned successfully!")
    print(f"Total grouped events: {len(events_per_frame)}")
    print(f"Total labels assigned: {len(interpolated_labels)}")
    print(f"Frames read: {len(events_per_frame)}")
    print(f"Events read: {len(events)}")

    events_per_frame = events_per_frame[START_FRAME:END_FRAME]
    interpolated_labels = interpolated_labels[START_FRAME:END_FRAME]

    return events_per_frame, interpolated_labels


def read_pendulum_file(
    FILE_PATH,
    CSV_PATH,
    time_window=30000,
    START_FRAME=0,
    END_FRAME=-1,
    separator=None,
    timestamp_column="timestamp_us",
    angle_column="theta_rad",
):
    """Read pendulum event data and a single angular label."""
    print(f"\n{'=' * 70}")
    print("Starting data loading for Pendulum experiment...")
    print(f"{'=' * 70}")

    events = tonic.io.read_aedat4(FILE_PATH)
    df = load_label_file(CSV_PATH, separator)

    required_columns = [timestamp_column, angle_column]
    missing_columns = [column for column in required_columns if column not in df.columns]
    if missing_columns:
        raise ValueError(
            f"Missing required columns {missing_columns} in {CSV_PATH}. "
            f"Available columns: {list(df.columns)}"
        )

    df[timestamp_column] = pd.to_numeric(df[timestamp_column], errors="coerce")
    df[angle_column] = pd.to_numeric(df[angle_column], errors="coerce")
    df = df.dropna(subset=required_columns).sort_values(timestamp_column).reset_index(drop=True)

    timestamps = df[timestamp_column].to_numpy(dtype=np.int64)
    data_labels = df[angle_column].to_numpy(dtype=np.float32)

    return _align_and_slice_events(
        events,
        timestamps,
        data_labels,
        time_window,
        START_FRAME,
        END_FRAME,
    )


def read_pencil_file(
    EVENT_FILE,
    LABEL_FILE,
    time_window=3000,
    START_FRAME=300,
    END_FRAME=-1,
    separator=None,
    timestamp_column="timestamp_us",
    angle_column="lin_m",
    position_column="lin_b",
    angle_unit="rad",
    timestamp_scale_to_us=1.0,
    position_scale=1.0,
    position_min=None,
    position_max=None,
    position_name=None,
    position_unit="mm",
):
    """Read event data and two synchronized regression labels: angle and position."""
    print(f"\n{'=' * 70}")
    print("Starting data loading for Pencil experiment...")
    print(f"{'=' * 70}")

    events = tonic.io.read_aedat4(EVENT_FILE)
    df = load_label_file(LABEL_FILE, separator)

    required_columns = [timestamp_column, angle_column, position_column]
    missing_columns = [column for column in required_columns if column not in df.columns]
    if missing_columns:
        raise ValueError(
            f"Missing required columns {missing_columns} in {LABEL_FILE}. "
            f"Available columns: {list(df.columns)}"
        )

    df[timestamp_column] = pd.to_numeric(df[timestamp_column], errors="coerce") * timestamp_scale_to_us
    df[angle_column] = pd.to_numeric(df[angle_column], errors="coerce")
    df[position_column] = pd.to_numeric(df[position_column], errors="coerce") * position_scale
    df = df.dropna(subset=required_columns).sort_values(timestamp_column).reset_index(drop=True)

    if angle_unit.lower() == "deg":
        df[angle_column] = np.deg2rad(df[angle_column])
    elif angle_unit.lower() != "rad":
        raise ValueError("angle_unit must be either 'rad' or 'deg'.")

    timestamps = df[timestamp_column].to_numpy(dtype=np.int64)
    labels = np.stack(
        [
            df[angle_column].to_numpy(dtype=np.float32),
            df[position_column].to_numpy(dtype=np.float32),
        ],
        axis=1,
    )

    events_per_frame, labels = _align_and_slice_events(
        events,
        timestamps,
        labels,
        time_window,
        START_FRAME,
        END_FRAME,
    )

    observed_position_min = float(labels[:, 1].min())
    observed_position_max = float(labels[:, 1].max())
    effective_position_min = observed_position_min if position_min is None else float(position_min)
    effective_position_max = observed_position_max if position_max is None else float(position_max)

    if effective_position_max <= effective_position_min:
        raise ValueError("position_max must be greater than position_min.")

    metadata = {
        "timestamp_column": timestamp_column,
        "angle_column": angle_column,
        "position_column": position_column,
        "position_name": position_name or position_column,
        "position_unit": position_unit,
        "position_min": effective_position_min,
        "position_max": effective_position_max,
        "observed_position_min": observed_position_min,
        "observed_position_max": observed_position_max,
        "time_window_us": time_window,
    }

    print(f"Using columns -> timestamp: {timestamp_column}, angle: {angle_column}, position: {position_column}")
    print(
        "Position range for normalization: "
        f"[{effective_position_min:.3f}, {effective_position_max:.3f}] {position_unit}"
    )

    return events_per_frame, labels, metadata


def read_IMU_file(FILE_PATH, time_window=10000, START_FRAME=0, END_FRAME=-1):
    """Read IMU event data and compute orientation (roll) using Madgwick."""
    print(f"\n{'=' * 70}")
    print("Starting data loading for IMU experiment...")
    print(f"{'=' * 70}")

    events = tonic.io.read_aedat4(FILE_PATH)
    reader = dv.io.MonoCameraRecording(FILE_PATH)

    timestamps = []
    acc = []
    gyro = []

    while reader.isRunning():
        imu_batch = reader.getNextImuBatch()
        if imu_batch is not None and len(imu_batch) > 0:
            for measurement in imu_batch:
                timestamps.append(measurement.timestamp)
                acc.append([measurement.accelerometerX, measurement.accelerometerY, measurement.accelerometerZ])
                gyro.append([measurement.gyroscopeX, measurement.gyroscopeY, measurement.gyroscopeZ])

    timestamps = np.array(timestamps)
    acc = np.array(acc)
    gyro = np.deg2rad(np.array(gyro))

    init_madgwick = Madgwick(acc=acc[:1], gyr=gyro[:1], frequency=1000)
    q = init_madgwick.Q[0]

    madgwick = Madgwick(gain=0.033)
    quaternions = [q]

    for index in range(1, len(acc)):
        madgwick.Dt = (timestamps[index] - timestamps[index - 1]) * 1e-6
        q = madgwick.updateIMU(q, gyr=gyro[index], acc=acc[index])
        quaternions.append(np.array(q))

    quaternions = np.array(quaternions)
    w = quaternions[:, 0]
    x = quaternions[:, 1]
    y = quaternions[:, 2]
    z = quaternions[:, 3]

    sinr_cosp = 2 * (w * x + y * z)
    cosr_cosp = 1 - 2 * (x * x + y * y)
    roll = np.arctan2(sinr_cosp, cosr_cosp).astype(np.float32)

    return _align_and_slice_events(
        events,
        timestamps,
        roll,
        time_window,
        START_FRAME,
        END_FRAME,
    )
