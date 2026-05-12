import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


# === CONFIG ===
# Point this at either a folder containing CSVs or a glob pattern.
# Examples:
#   csv_path = r"C:\path\to\logs"
#   csv_path = r"C:\path\to\logs\*_hough.csv"
DATASET_RAW_DIR = r"C:\Users\pcadm\Documents\SNN-Lucca-Meg\SNN-Regression-Pencil-Balancer-True\Dataset\raw"
csv_path = DATASET_RAW_DIR
bins = 200
recursive = True
make_per_file_histograms = False
save_figures = False
output_dir = "batch_statistics_output"
lower_percentile = 1.0
upper_percentile = 99.0


EXPECTED_COLS = {"timestamp_us", "slope", "intercept"}


def discover_csv_files(path_text, recursive_search=True):
    path_text = str(path_text)
    path = Path(path_text).expanduser()

    if any(char in path_text for char in "*?[]"):
        parent = path.parent if str(path.parent) else Path(".")
        files = sorted(parent.glob(path.name))
    elif path.is_dir():
        pattern = "**/*.csv" if recursive_search else "*.csv"
        files = sorted(path.glob(pattern))
    else:
        files = [path]

    return [file for file in files if file.is_file() and file.suffix.lower() == ".csv"]


def read_hough_csv(csv_file):
    df = pd.read_csv(csv_file)

    if not EXPECTED_COLS.issubset(df.columns):
        missing = EXPECTED_COLS.difference(df.columns)
        raise ValueError(f"{csv_file} is missing columns: {sorted(missing)}")

    return df


def percentile_bounds(values, lower_pct, upper_pct):
    return values.quantile(lower_pct / 100.0), values.quantile(upper_pct / 100.0)


def plot_histogram(values, title, xlabel, bins_count, lower_pct, upper_pct, save_path=None):
    min_value = values.min()
    max_value = values.max()
    lower_value, upper_value = percentile_bounds(values, lower_pct, upper_pct)

    plt.figure()
    plt.hist(values, bins=bins_count)
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel("Frequência")
    plt.grid(True)
    plt.axvline(min_value, color="red", linestyle=":", linewidth=1.2, label=f"True min = {min_value:.6f}")
    plt.axvline(max_value, color="green", linestyle=":", linewidth=1.2, label=f"True max = {max_value:.6f}")
    plt.axvline(
        lower_value,
        color="orange",
        linestyle="--",
        linewidth=1.8,
        label=f"P{lower_pct:g} = {lower_value:.6f}",
    )
    plt.axvline(
        upper_value,
        color="purple",
        linestyle="--",
        linewidth=1.8,
        label=f"P{upper_pct:g} = {upper_value:.6f}",
    )
    plt.legend()

    if save_path is not None:
        plt.savefig(save_path, dpi=150, bbox_inches="tight")


def print_summary(name, values, lower_pct, upper_pct):
    lower_value, upper_value = percentile_bounds(values, lower_pct, upper_pct)
    print(
        f"{name}: count={len(values)} | "
        f"mean={values.mean():.6f} | "
        f"std={values.std():.6f} | "
        f"min={values.min():.6f} | "
        f"max={values.max():.6f} | "
        f"p{lower_pct:g}={lower_value:.6f} | "
        f"p{upper_pct:g}={upper_value:.6f}"
    )


def print_normalization_formula(name, values, lower_pct, upper_pct):
    lower_value, upper_value = percentile_bounds(values, lower_pct, upper_pct)
    scale = upper_value - lower_value

    print(f"\n{name} robust normalization using P{lower_pct:g}/P{upper_pct:g}:")
    print(f"  {name.lower()}_low = {lower_value:.12f}")
    print(f"  {name.lower()}_high = {upper_value:.12f}")
    print(f"  {name.lower()}_norm = clip(({name.lower()} - {lower_value:.12f}) / {scale:.12f}, 0, 1)")
    print(f"  {name.lower()} = {name.lower()}_norm * {scale:.12f} + {lower_value:.12f}")


def main():
    parser = argparse.ArgumentParser(
        description="Generate slope/intercept histograms for a batch of Hough CSV files."
    )
    parser.add_argument(
        "path",
        nargs="?",
        default=csv_path,
        help="CSV file, directory, or glob pattern. Defaults to csv_path in the script.",
    )
    parser.add_argument("--bins", type=int, default=bins, help="Number of histogram bins.")
    parser.add_argument(
        "--no-recursive",
        action="store_true",
        help="When path is a directory, only read CSVs directly inside it.",
    )
    parser.add_argument(
        "--per-file",
        action="store_true",
        default=make_per_file_histograms,
        help="Also generate slope/intercept histograms for each CSV file.",
    )
    parser.add_argument(
        "--save",
        action="store_true",
        default=save_figures,
        help="Save figures to output_dir instead of only showing them.",
    )
    parser.add_argument(
        "--output-dir",
        default=output_dir,
        help="Directory used when --save is enabled.",
    )
    parser.add_argument(
        "--lower-percentile",
        type=float,
        default=lower_percentile,
        help="Lower percentile used for robust normalization bounds.",
    )
    parser.add_argument(
        "--upper-percentile",
        type=float,
        default=upper_percentile,
        help="Upper percentile used for robust normalization bounds.",
    )
    args = parser.parse_args()

    if not 0 <= args.lower_percentile < args.upper_percentile <= 100:
        raise ValueError("Percentiles must satisfy 0 <= lower < upper <= 100.")

    csv_files = discover_csv_files(args.path, recursive_search=not args.no_recursive)

    if not csv_files:
        raise FileNotFoundError(f"No CSV files found for: {args.path}")

    save_dir = Path(args.output_dir)
    if args.save:
        save_dir.mkdir(parents=True, exist_ok=True)

    all_frames = []

    print(f"Found {len(csv_files)} CSV file(s):")
    for csv_file in csv_files:
        print(f"  {csv_file}")
        df = read_hough_csv(csv_file)
        all_frames.append(df.assign(source_file=str(csv_file)))

        if args.per_file:
            slope = df["slope"].dropna()
            intercept = df["intercept"].dropna()
            safe_stem = csv_file.stem.replace(" ", "_")

            plot_histogram(
                slope,
                f"Histograma de Slope - {csv_file.name}",
                "Slope",
                args.bins,
                args.lower_percentile,
                args.upper_percentile,
                save_dir / f"{safe_stem}_slope_hist.png" if args.save else None,
            )
            plot_histogram(
                intercept,
                f"Histograma de Intercept - {csv_file.name}",
                "Intercept",
                args.bins,
                args.lower_percentile,
                args.upper_percentile,
                save_dir / f"{safe_stem}_intercept_hist.png" if args.save else None,
            )

    combined = pd.concat(all_frames, ignore_index=True)
    slope_all = combined["slope"].dropna()
    intercept_all = combined["intercept"].dropna()

    print("\nCombined statistics:")
    print_summary("Slope", slope_all, args.lower_percentile, args.upper_percentile)
    print_summary("Intercept", intercept_all, args.lower_percentile, args.upper_percentile)
    print_normalization_formula("Slope", slope_all, args.lower_percentile, args.upper_percentile)
    print_normalization_formula("Intercept", intercept_all, args.lower_percentile, args.upper_percentile)

    plot_histogram(
        slope_all,
        f"Histograma de Slope - Batch ({len(csv_files)} arquivos)",
        "Slope",
        args.bins,
        args.lower_percentile,
        args.upper_percentile,
        save_dir / "batch_slope_hist.png" if args.save else None,
    )
    plot_histogram(
        intercept_all,
        f"Histograma de Intercept - Batch ({len(csv_files)} arquivos)",
        "Intercept",
        args.bins,
        args.lower_percentile,
        args.upper_percentile,
        save_dir / "batch_intercept_hist.png" if args.save else None,
    )

    if args.save:
        print(f"\nSaved figures to: {save_dir.resolve()}")

    plt.show()


if __name__ == "__main__":
    main()
