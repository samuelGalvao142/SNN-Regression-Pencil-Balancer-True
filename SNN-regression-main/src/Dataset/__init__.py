from .datasets import (
    RotatingBarDataset,
    SequenceDataset,
    ContinuousDataset,
    MultiRunSequenceDataset,
    MultiRunContinuousDataset,
)
from .multi_run import (
    CameraRunRecord,
    LoadedCameraRun,
    discover_camera_runs,
    summarize_camera_runs,
    load_camera_runs,
    split_loaded_runs,
)

try:
    from .dataloaders import create_dataloaders, create_multi_run_dataloaders
except ModuleNotFoundError:
    create_dataloaders = None
    create_multi_run_dataloaders = None

try:
    from .read_file import read_pendulum_file, read_IMU_file, read_line_file
except ModuleNotFoundError:
    read_pendulum_file = None
    read_IMU_file = None
    read_line_file = None
