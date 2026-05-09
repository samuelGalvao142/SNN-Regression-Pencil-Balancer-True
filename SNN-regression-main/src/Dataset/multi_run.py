from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import random


_EVENT_EXTENSIONS = (".aedat4", ".aedat")


@dataclass(frozen=True)
class CameraRunRecord:
    run_name: str
    run_dir: Path
    camera_id: int
    events_path: Path
    labels_path: Path


@dataclass(frozen=True)
class LoadedCameraRun:
    record: CameraRunRecord
    sliced_events: object
    labels: object


def _normalize_camera_id(camera: int | str) -> int:
    if isinstance(camera, int):
        if camera in (1, 2):
            return camera
        raise ValueError(f"Unsupported camera id: {camera}")

    token = str(camera).strip().lower().replace("_", "").replace("-", "")
    if token in {"1", "cam1", "camera1"}:
        return 1
    if token in {"2", "cam2", "camera2"}:
        return 2
    raise ValueError(f"Unsupported camera id: {camera}")


def _camera_tokens(camera_id: int) -> tuple[str, ...]:
    return (
        f"cam{camera_id}",
        f"camera{camera_id}",
    )


def _is_event_file(path: Path) -> bool:
    return path.suffix.lower() in _EVENT_EXTENSIONS


def _is_label_file(path: Path) -> bool:
    return path.suffix.lower() == ".csv"


def _match_camera_file(files: Iterable[Path], camera_id: int, *, kind: str) -> Path | None:
    tokens = _camera_tokens(camera_id)
    matches: list[Path] = []

    for path in files:
        lower_name = path.name.lower()
        if not any(token in lower_name for token in tokens):
            continue
        if kind == "event" and _is_event_file(path):
            matches.append(path)
        elif kind == "label" and _is_label_file(path) and "hough" in lower_name:
            matches.append(path)

    if not matches:
        return None
    if len(matches) > 1:
        formatted = ", ".join(str(path.name) for path in matches)
        raise ValueError(
            f"Expected one {kind} file for camera {camera_id}, but found multiple matches: {formatted}"
        )
    return matches[0]


def _iter_run_directories(dataset_root: Path) -> list[Path]:
    run_dirs: list[Path] = []
    for directory in sorted(path for path in dataset_root.rglob("*") if path.is_dir()):
        files = [path for path in directory.iterdir() if path.is_file()]
        if any(_is_event_file(path) for path in files) or any(_is_label_file(path) for path in files):
            run_dirs.append(directory)
    if not run_dirs and dataset_root.exists():
        files = [path for path in dataset_root.iterdir() if path.is_file()]
        if files:
            run_dirs.append(dataset_root)
    return run_dirs


def discover_camera_runs(
    dataset_root: str | Path,
    *,
    cameras: Iterable[int | str] = (1, 2),
    strict: bool = True,
) -> list[CameraRunRecord]:
    dataset_root = Path(dataset_root).expanduser().resolve()
    if not dataset_root.exists():
        raise FileNotFoundError(f"Dataset root does not exist: {dataset_root}")

    camera_ids = [_normalize_camera_id(camera) for camera in cameras]
    records: list[CameraRunRecord] = []

    for run_dir in _iter_run_directories(dataset_root):
        files = [path for path in run_dir.iterdir() if path.is_file()]
        for camera_id in camera_ids:
            events_path = _match_camera_file(files, camera_id, kind="event")
            labels_path = _match_camera_file(files, camera_id, kind="label")

            if events_path is None and labels_path is None:
                continue

            if strict and (events_path is None or labels_path is None):
                raise FileNotFoundError(
                    f"Run '{run_dir.name}' is incomplete for camera {camera_id}. "
                    f"Found event={events_path is not None}, label={labels_path is not None}."
                )

            if events_path is None or labels_path is None:
                continue

            records.append(
                CameraRunRecord(
                    run_name=run_dir.name,
                    run_dir=run_dir,
                    camera_id=camera_id,
                    events_path=events_path,
                    labels_path=labels_path,
                )
            )

    return records


def summarize_camera_runs(records: Iterable[CameraRunRecord]) -> dict[int, list[str]]:
    summary: dict[int, list[str]] = {1: [], 2: []}
    for record in records:
        summary.setdefault(record.camera_id, []).append(record.run_name)
    return summary


def load_camera_runs(
    dataset_root: str | Path,
    *,
    camera: int | str,
    target_column: str,
    time_window: int = 30000,
    start_frame: int = 0,
    end_frame: int = -1,
    timestamp_column: str = "timestamp_us",
    strict: bool = True,
) -> list[LoadedCameraRun]:
    from .read_file import read_line_file

    camera_id = _normalize_camera_id(camera)
    records = [
        record
        for record in discover_camera_runs(dataset_root, cameras=(camera_id,), strict=strict)
        if record.camera_id == camera_id
    ]

    loaded_runs: list[LoadedCameraRun] = []
    for record in records:
        sliced_events, labels = read_line_file(
            FILE_PATH=record.events_path,
            CSV_PATH=record.labels_path,
            label_column=target_column,
            time_window=time_window,
            START_FRAME=start_frame,
            END_FRAME=end_frame,
            timestamp_column=timestamp_column,
        )
        loaded_runs.append(
            LoadedCameraRun(
                record=record,
                sliced_events=sliced_events,
                labels=labels,
            )
        )

    return loaded_runs


def split_loaded_runs(
    loaded_runs: list[LoadedCameraRun],
    *,
    test_ratio: float = 0.05,
    val_ratio: float = 0.07,
    shuffle: bool = False,
    seed: int = 42,
) -> tuple[list[LoadedCameraRun], list[LoadedCameraRun], list[LoadedCameraRun]]:
    if not loaded_runs:
        return [], [], []

    if test_ratio < 0 or val_ratio < 0 or test_ratio + val_ratio >= 1.0:
        raise ValueError("test_ratio and val_ratio must be non-negative and sum to less than 1.")

    runs = list(loaded_runs)
    if shuffle:
        rng = random.Random(seed)
        rng.shuffle(runs)

    total_runs = len(runs)
    test_count = int(round(total_runs * test_ratio))
    val_count = int(round(total_runs * val_ratio))

    if total_runs >= 3 and test_count == 0 and test_ratio > 0:
        test_count = 1
    if total_runs >= 3 and val_count == 0 and val_ratio > 0:
        val_count = 1

    while test_count + val_count >= total_runs and total_runs > 1:
        if test_count >= val_count and test_count > 0:
            test_count -= 1
        elif val_count > 0:
            val_count -= 1
        else:
            break

    train_count = total_runs - test_count - val_count
    if train_count <= 0:
        raise ValueError("Not enough runs to create a non-empty training split.")

    train_runs = runs[:train_count]
    val_runs = runs[train_count : train_count + val_count]
    test_runs = runs[train_count + val_count :]
    return train_runs, val_runs, test_runs
