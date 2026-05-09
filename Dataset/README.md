# Dual-Camera Dataset Layout

Use one folder per balancing run. Each run folder should contain the event recording
and Hough-label CSV for both cameras.

Recommended structure:

```text
Dataset/
  raw/
    run_001/
      run_001_cam1.aedat4
      run_001_cam1_hough.csv
      run_001_cam2.aedat4
      run_001_cam2_hough.csv
    run_002/
      run_002_cam1.aedat4
      run_002_cam1_hough.csv
      run_002_cam2.aedat4
      run_002_cam2_hough.csv
```

The loader also accepts simpler names like:

```text
run_001/
  cam1.aedat4
  cam1_hough.csv
  cam2.aedat4
  cam2_hough.csv
```

## Required CSV columns

Each `*_hough.csv` file should include:

- `timestamp_us`
- `slope`
- `intercept`

## Why separate run folders?

Keeping each recording as its own run prevents the training pipeline from
accidentally stitching unrelated balancing sessions into one continuous sequence.

## Loader entry points

The shared helpers live in:

- `SNN-regression-main/src/Dataset/multi_run.py`
- `SNN-regression-main/src/Dataset/read_file.py`

Typical workflow:

1. Put all run folders under `Dataset/raw/`
2. Load one camera at a time
3. Load one target at a time (`slope` or `intercept`)
4. Train the notebook on the resulting multi-run dataset

