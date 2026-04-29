import cv2 as cv
import matplotlib.pyplot as plt
import numpy as np
import torch


def _resolve_target_specs(config_or_experiment):
    if isinstance(config_or_experiment, dict) and config_or_experiment.get("target_specs"):
        return config_or_experiment["target_specs"]

    experiment_type = (
        config_or_experiment.get("experiment", "pendulum")
        if isinstance(config_or_experiment, dict)
        else str(config_or_experiment)
    )

    experiment_type = experiment_type.lower()
    if experiment_type == "imu":
        return [
            {
                "name": "roll",
                "display_name": "Roll",
                "unit": "deg",
                "normalize": "negative_angle_pi",
            }
        ]

    return [
        {
            "name": "angle",
            "display_name": "Angle",
            "unit": "deg",
            "normalize": "angle_pm_pi",
        }
    ]


def _normalize_component(values, spec):
    normalize_mode = spec["normalize"]
    if normalize_mode == "angle_pm_pi":
        return (values + torch.pi) / (2 * torch.pi)
    if normalize_mode == "negative_angle_pi":
        return -values / torch.pi
    if normalize_mode == "linear":
        value_min = spec["min"]
        value_max = spec["max"]
        value_span = value_max - value_min
        if value_span <= 0:
            raise ValueError(f"Invalid normalization range for {spec['name']}: [{value_min}, {value_max}]")
        return (values - value_min) / value_span
    raise ValueError(f"Unknown normalization mode: {normalize_mode}")


def _denormalize_component(values, spec):
    normalize_mode = spec["normalize"]
    if normalize_mode == "angle_pm_pi":
        return np.rad2deg((values * 2 * np.pi) - np.pi)
    if normalize_mode == "negative_angle_pi":
        return -values * 180.0
    if normalize_mode == "linear":
        value_min = spec["min"]
        value_max = spec["max"]
        value_span = value_max - value_min
        if value_span <= 0:
            raise ValueError(f"Invalid denormalization range for {spec['name']}: [{value_min}, {value_max}]")
        return values * value_span + value_min
    raise ValueError(f"Unknown normalization mode: {normalize_mode}")


def normalize_targets(targets, config_or_experiment):
    """Normalize one or more regression targets to [0, 1]."""
    specs = _resolve_target_specs(config_or_experiment)
    normalized = targets.clone()

    if len(specs) == 1:
        return _normalize_component(normalized, specs[0])

    for index, spec in enumerate(specs):
        normalized[..., index] = _normalize_component(normalized[..., index], spec)
    return normalized


def denormalize_targets(normalized_targets, config_or_experiment):
    """Convert normalized targets back to display units."""
    specs = _resolve_target_specs(config_or_experiment)
    denormalized = np.array(normalized_targets, copy=True)

    if len(specs) == 1:
        return _denormalize_component(denormalized, specs[0])

    for index, spec in enumerate(specs):
        denormalized[..., index] = _denormalize_component(denormalized[..., index], spec)
    return denormalized


def visualize_sequence_from_trainloader(trainloader, n_sequences=5, playback_fps=10, scale=1, target_config=None):
    """
    Visualize temporal sequences from the trainloader.
    """
    specs = _resolve_target_specs(target_config or "pendulum")

    for seq_idx, (frames_batch, labels_batch) in enumerate(trainloader):
        if seq_idx >= n_sequences:
            break

        T, B, C, H, W = frames_batch.shape

        print(f"\n=== Sequence {seq_idx + 1}/{n_sequences} ===")
        print(f"Batch shape: {frames_batch.shape}, Labels shape: {labels_batch.shape}")

        batch_item = 0

        for timestep in range(T):
            frame = frames_batch[timestep, batch_item].cpu().numpy()
            target = labels_batch[timestep, batch_item]

            events_img = np.ones((H, W, 3), dtype=np.uint8) * 255
            events_img[frame[0] > 0] = [0, 0, 200]
            events_img[frame[1] > 0] = [200, 0, 0]

            events_resized = cv.resize(
                events_img,
                (W * scale, H * scale),
                interpolation=cv.INTER_NEAREST,
            )

            info_text = [f"Seq: {seq_idx + 1}/{n_sequences}  Time: {timestep + 1}/{T}"]
            if target.ndim == 0:
                info_text.append(f"{specs[0]['display_name']}: {np.rad2deg(target.item()):.2f} deg")
            else:
                target_values = target.detach().cpu().numpy()
                for index, spec in enumerate(specs):
                    if spec["normalize"].startswith("angle"):
                        value = np.rad2deg(float(target_values[index]))
                        unit = "deg"
                    else:
                        value = float(target_values[index])
                        unit = spec["unit"]
                    info_text.append(f"{spec['display_name']}: {value:.2f} {unit}")
            info_text.append(f"Batch item: {batch_item + 1}/{B}")

            y_offset = 30
            for text in info_text:
                cv.putText(events_resized, text, (10, y_offset), cv.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 2)
                y_offset += 25

            cv.imshow("Trainloader Sequence Visualization", events_resized)

            key = cv.waitKey(int(1000 / playback_fps))
            if key == 27:
                cv.destroyAllWindows()
                return
            if key == ord("n"):
                break

    cv.destroyAllWindows()
    print("\nVisualization completed!")


def _extract_plot_context(results, experiment_type):
    config = results.get("config") or experiment_type
    specs = _resolve_target_specs(config)
    output = denormalize_targets(results["test_output"], config)
    target = denormalize_targets(results["test_target"], config)
    return config, specs, output, target


def _plot_single_output(output_full, target_full, window_start, window_end, label_name, label_unit):
    output_window = output_full[window_start:window_end]
    target_window = target_full[window_start:window_end]

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 8))

    ax1.plot(output_window, label="Model Output", alpha=0.8, linewidth=1.5)
    ax1.plot(target_window, label="Target", alpha=0.8, linewidth=1.5)
    ax1.set_xlabel("Frame")
    ax1.set_ylabel(f"{label_name} ({label_unit})")
    ax1.set_title("Model Output vs Target")
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    error_window = np.abs(output_window - target_window)
    ax2.plot(error_window, color="orange", linewidth=1)
    ax2.set_xlabel("Frame")
    ax2.set_ylabel(f"Absolute Error ({label_unit})")
    ax2.set_title(f"{label_name} Error over Time")
    ax2.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.show()

    print("\nError statistics (full sequence):")
    print(f"  Mean error: {np.mean(np.abs(output_full - target_full)):.3f} {label_unit}")
    print(f"  Std error:  {np.std(np.abs(output_full - target_full)):.3f} {label_unit}")
    print(f"  Max error:  {np.max(np.abs(output_full - target_full)):.3f} {label_unit}")

    print(f"\nError statistics (window [{window_start}:{window_end}]):")
    print(f"  Mean error: {np.mean(error_window):.3f} {label_unit}")
    print(f"  Std error:  {np.std(error_window):.3f} {label_unit}")
    print(f"  Max error:  {np.max(error_window):.3f} {label_unit}")


def _plot_multi_output(output_full, target_full, window_start, window_end, specs):
    output_window = output_full[window_start:window_end]
    target_window = target_full[window_start:window_end]

    angle_name = specs[0]["display_name"]
    angle_unit = specs[0]["unit"]
    position_name = specs[1]["display_name"]
    position_unit = specs[1]["unit"]

    angle_error_window = np.abs(output_window[:, 0] - target_window[:, 0])
    position_error_window = np.abs(output_window[:, 1] - target_window[:, 1])
    angle_error_full = np.abs(output_full[:, 0] - target_full[:, 0])
    position_error_full = np.abs(output_full[:, 1] - target_full[:, 1])

    fig, axes = plt.subplots(4, 1, figsize=(15, 14), sharex=False)

    axes[0].plot(output_window[:, 0], label=f"Predicted {angle_name}", alpha=0.8, linewidth=1.5)
    axes[0].plot(target_window[:, 0], label=f"Target {angle_name}", alpha=0.8, linewidth=1.5)
    axes[0].set_xlabel("Frame")
    axes[0].set_ylabel(f"{angle_name} ({angle_unit})")
    axes[0].set_title(f"Predicted {angle_name} vs Target")
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)

    axes[1].plot(output_window[:, 1], label=f"Predicted {position_name}", alpha=0.8, linewidth=1.5)
    axes[1].plot(target_window[:, 1], label=f"Target {position_name}", alpha=0.8, linewidth=1.5)
    axes[1].set_xlabel("Frame")
    axes[1].set_ylabel(f"{position_name} ({position_unit})")
    axes[1].set_title(f"Predicted {position_name} vs Target")
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)

    axes[2].plot(angle_error_window, color="tab:orange", linewidth=1)
    axes[2].set_xlabel("Frame")
    axes[2].set_ylabel(f"Absolute Error ({angle_unit})")
    axes[2].set_title(f"{angle_name} Error over Time")
    axes[2].grid(True, alpha=0.3)

    axes[3].plot(position_error_window, color="tab:red", linewidth=1)
    axes[3].set_xlabel("Frame")
    axes[3].set_ylabel(f"Absolute Error ({position_unit})")
    axes[3].set_title(f"{position_name} Error over Time")
    axes[3].grid(True, alpha=0.3)

    plt.tight_layout()
    plt.show()

    print("\nError statistics (full sequence):")
    print(f"  {angle_name} mean error: {np.mean(angle_error_full):.3f} {angle_unit}")
    print(f"  {angle_name} std error:  {np.std(angle_error_full):.3f} {angle_unit}")
    print(f"  {angle_name} max error:  {np.max(angle_error_full):.3f} {angle_unit}")
    print(f"  {position_name} mean error: {np.mean(position_error_full):.3f} {position_unit}")
    print(f"  {position_name} std error:  {np.std(position_error_full):.3f} {position_unit}")
    print(f"  {position_name} max error:  {np.max(position_error_full):.3f} {position_unit}")

    print(f"\nError statistics (window [{window_start}:{window_end}]):")
    print(f"  {angle_name} mean error: {np.mean(angle_error_window):.3f} {angle_unit}")
    print(f"  {angle_name} std error:  {np.std(angle_error_window):.3f} {angle_unit}")
    print(f"  {angle_name} max error:  {np.max(angle_error_window):.3f} {angle_unit}")
    print(f"  {position_name} mean error: {np.mean(position_error_window):.3f} {position_unit}")
    print(f"  {position_name} std error:  {np.std(position_error_window):.3f} {position_unit}")
    print(f"  {position_name} max error:  {np.max(position_error_window):.3f} {position_unit}")


def plot_prediction(results, window_start=0, window_end=-1, experiment_type="pendulum"):
    """Plot model predictions versus targets for one or multiple outputs."""
    _, specs, output_full, target_full = _extract_plot_context(results, experiment_type)

    if output_full.ndim == 1:
        _plot_single_output(output_full, target_full, window_start, window_end, specs[0]["display_name"], specs[0]["unit"])
        return

    if output_full.shape[-1] == 1:
        _plot_single_output(
            output_full[:, 0],
            target_full[:, 0],
            window_start,
            window_end,
            specs[0]["display_name"],
            specs[0]["unit"],
        )
        return

    _plot_multi_output(output_full, target_full, window_start, window_end, specs)


def plot_spike_activity(results, window_start=0, window_end=-1):
    """Plot spike activity and input events."""
    if results["spike_activity"] is None:
        print("No spike activity data available. Run test() with monitor_mode='spikes' or 'both'")
        return

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 8))

    ax1.plot(results["spike_activity"][window_start:window_end], color="green", linewidth=1.5)
    ax1.set_xlabel("Frame")
    ax1.set_ylabel("Average Spike Activity")
    ax1.set_title("Average Spike Activity Across the Network Over Time")
    ax1.grid(True, alpha=0.3)

    ax2.plot(results["num_events"][window_start:window_end], color="purple", linewidth=1.5)
    ax2.set_xlabel("Frame")
    ax2.set_ylabel("Number of Events")
    ax2.set_title("Number of Input Events per Timestep")
    ax2.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.show()

    print("\nSpike activity statistics:")
    print(f"  Mean activity: {np.mean(results['spike_activity']):.6f}")
    print(f"  Std activity: {np.std(results['spike_activity']):.6f}")
    print(f"  Max activity: {np.max(results['spike_activity']):.6f}")
    print(f"  Min activity: {np.min(results['spike_activity']):.6f}")

    print("\nInput event statistics:")
    print(f"  Mean events per timestep: {np.mean(results['num_events']):.2f}")
    print(f"  Max events per timestep: {np.max(results['num_events']):.2f}")
    print(f"  Min events per timestep: {np.min(results['num_events']):.2f}")


def plot_normalization_stats(results, window_start=0, window_end=-1):
    """Plot normalization summary using amplification factors."""
    if results["norm_stats"] is None:
        print("No normalization statistics data available. Run test() with monitor_mode='norm' or 'both'")
        return

    layer_names, mean_amplifications = compute_mean_amplification_per_layer(
        results["norm_stats"],
        window_start,
        window_end,
    )

    if len(layer_names) == 0:
        print("No normalization amplification data available to plot.")
        return

    fig = plot_network_amplification(layer_names, mean_amplifications)
    plt.show()


def compute_mean_amplification_per_layer(norm_activity_over_time, window_start=0, window_end=-1):
    """Compute average amplification factor (std_out / std_in) over time for each layer."""
    layer_names = []
    mean_amplifications = []

    if norm_activity_over_time:
        for layer_name, activity in norm_activity_over_time.items():
            input_std = np.array(activity["input_std"][window_start:window_end])
            output_std = np.array(activity["output_std"][window_start:window_end])

            if input_std.size == 0 or output_std.size == 0:
                continue

            std_scaling = output_std / (input_std + 1e-8)
            layer_names.append(layer_name)
            mean_amplifications.append(std_scaling.mean())

    return layer_names, np.array(mean_amplifications)


def plot_network_amplification(layer_names, mean_amplifications):
    """Bar plot showing average amplification factor per layer."""
    layers = np.arange(len(layer_names))
    fig, ax = plt.subplots(figsize=(10, 4))

    ax.bar(layers, mean_amplifications, alpha=0.8, color="purple")
    ax.axhline(1.0, color="red", linestyle="--", linewidth=2, label="No scaling (1x)")

    avg_network = mean_amplifications.mean()
    ax.axhline(avg_network, color="black", linestyle=":", linewidth=2, label=f"Network avg = {avg_network:.1f}x")

    ax.set_xlabel("Layer")
    ax.set_ylabel("Mean Amplification (Output / Input)")
    ax.set_title(f"Mean Amplification Factor per Layer (avg={avg_network:.1f}x)")
    ax.legend()
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    return fig


def plot_all(results, window_start=0, window_end=-1, experiment_type="pendulum"):
    """Plot predictions and any enabled monitoring summaries."""
    plot_prediction(results, window_start, window_end, experiment_type)

    if results["spike_activity"] is not None:
        plot_spike_activity(results, window_start, window_end)

    if results["norm_stats"] is not None:
        plot_normalization_stats(results, window_start, window_end)
