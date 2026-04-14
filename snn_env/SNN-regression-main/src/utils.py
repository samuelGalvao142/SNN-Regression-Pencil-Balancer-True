import numpy as np
import cv2 as cv
import matplotlib.pyplot as plt


def normalize_targets(targets, experiment_type):
    """
    Normalize targets to [0, 1] range based on experiment type.

    Supports numpy arrays and torch tensors. For multi-output targets
    of shape [..., 4] we treat channels 0 and 2 as angles and channels
    1 and 3 as horizontal positions (pixels). Positions are normalized
    by image width (default 346).
    """
    import torch

    WIDTH = 346.0

    def _norm_angle(x):
        return (x + np.pi) / (2 * np.pi)

    def _norm_angle_t(x):
        return (x + torch.pi) / (2 * torch.pi)

    if isinstance(targets, torch.Tensor):
        if targets.dim() >= 1 and targets.size(-1) == 4:
            out = targets.clone()
            out[..., 0] = _norm_angle_t(out[..., 0])
            out[..., 2] = _norm_angle_t(out[..., 2])
            out[..., 1] = out[..., 1] / WIDTH
            out[..., 3] = out[..., 3] / WIDTH
            return out
        else:
            if experiment_type.lower() == "pendulum":
                return _norm_angle_t(targets)
            elif experiment_type.lower() == "imu":
                return -targets / torch.pi
            else:
                raise ValueError(f"Unknown experiment type: {experiment_type}")
    else:
        # numpy
        if getattr(targets, 'ndim', 1) and getattr(targets, 'shape', ()) and len(getattr(targets, 'shape', [])) and getattr(targets, 'shape')[-1] == 4:
            out = targets.copy()
            out[..., 0] = _norm_angle(out[..., 0])
            out[..., 2] = _norm_angle(out[..., 2])
            out[..., 1] = out[..., 1] / WIDTH
            out[..., 3] = out[..., 3] / WIDTH
            return out
        else:
            if experiment_type.lower() == "pendulum":
                return (targets + np.pi) / (2 * np.pi)
            elif experiment_type.lower() == "imu":
                return -targets / np.pi
            else:
                raise ValueError(f"Unknown experiment type: {experiment_type}")


def denormalize_targets(normalized_targets, experiment_type):
    """
    Convert normalized targets back to physical units for visualization.

    For pendulum: angles are returned in degrees. For multi-output (4 channels),
    returns array with angle channels converted to degrees and position channels
    converted back to pixels using image width 346.
    """
    WIDTH = 346.0
    import torch

    def _denorm_angle_np(x):
        return (x * 360.0) - 180.0

    def _denorm_angle_t(x):
        return (x * 360.0) - 180.0

    if isinstance(normalized_targets, torch.Tensor):
        if normalized_targets.dim() >= 1 and normalized_targets.size(-1) == 4:
            out = normalized_targets.clone()
            out[..., 0] = _denorm_angle_t(out[..., 0])
            out[..., 2] = _denorm_angle_t(out[..., 2])
            out[..., 1] = out[..., 1] * WIDTH
            out[..., 3] = out[..., 3] * WIDTH
            return out
        else:
            if experiment_type.lower() == "pendulum":
                return _denorm_angle_t(normalized_targets)
            elif experiment_type.lower() == "imu":
                return -normalized_targets * 180.0
            else:
                return _denorm_angle_t(normalized_targets)
    else:
        # numpy
        if getattr(normalized_targets, 'ndim', 1) and getattr(normalized_targets, 'shape', ()) and len(getattr(normalized_targets, 'shape', [])) and getattr(normalized_targets, 'shape')[-1] == 4:
            out = normalized_targets.copy()
            out[..., 0] = _denorm_angle_np(out[..., 0])
            out[..., 2] = _denorm_angle_np(out[..., 2])
            out[..., 1] = out[..., 1] * WIDTH
            out[..., 3] = out[..., 3] * WIDTH
            return out
        else:
            if experiment_type.lower() == "pendulum":
                return (normalized_targets * 360) - 180
            elif experiment_type.lower() == "imu":
                return -normalized_targets * 180
            else:
                return (normalized_targets * 360) - 180


def visualize_sequence_from_trainloader(trainloader, n_sequences=5, playback_fps=10, scale=1):
    """
    Visualize temporal sequences from the trainloader (output of SequentialRotatingBarDataset).
    
    Args:
        trainloader: DataLoader with frames shaped [T, B, C, H, W] and labels shaped [T, B].
        n_sequences: Number of sequences (batches) to visualize.
        playback_fps: Playback speed for each timestep in the sequence.
        scale: Scale factor for visualization in pixels.
    """
    
    for seq_idx, (frames_batch, labels_batch) in enumerate(trainloader):
        if seq_idx >= n_sequences:
            break
            
        # frames_batch: [T, B, C, H, W]
        # labels_batch: [T, B] or [T, B, 4]
        T, B, C, H, W = frames_batch.shape
        
        print(f"\n=== Sequence {seq_idx+1}/{n_sequences} ===")
        print(f"Batch shape: {frames_batch.shape}, Labels shape: {labels_batch.shape}")
        
        # Visualize only the first element of the batch
        batch_item = 0
        
        for t in range(T):
            # Extract frame at time t for the first batch item
            # frame: [C, H, W] where C==2 (single camera) or C==4 (two cameras concatenated)
            frame = frames_batch[t, batch_item].cpu().numpy()
            # Extract angle from labels: if multi-output, angle is channel 0
            if labels_batch.ndim == 3 and labels_batch.shape[-1] >= 1:
                angle = labels_batch[t, batch_item, 0].item()
            else:
                angle = labels_batch[t, batch_item].item()
            
            # Create RGB visualization
            # Channel 0 = ON events (positive), Channel 1 = OFF events (negative)
            events_img = np.ones((H, W, 3), dtype=np.uint8) * 255
            
            # ON/OFF for camera 1 (first two channels)
            on_events = frame[0] > 0
            events_img[on_events] = [0, 0, 200]

            off_events = frame[1] > 0
            events_img[off_events] = [200, 0, 0]
            
            # Scale for easier viewing
            events_resized = cv.resize(events_img, (W*scale, H*scale), 
                                      interpolation=cv.INTER_NEAREST)
            
            # Add overlay text
            info_text = [
                f"Seq: {seq_idx+1}/{n_sequences}  Time: {t+1}/{T}",
                f"Target angle: {np.rad2deg(angle):.2f} deg",
                f"Batch item: {batch_item+1}/{B}"
            ]
            
            y_offset = 30
            for text in info_text:
                cv.putText(events_resized, text,
                          (10, y_offset), cv.FONT_HERSHEY_SIMPLEX, 
                          0.6, (0, 0, 0), 2)
                y_offset += 25
            
            # Show frame
            cv.imshow("Trainloader Sequence Visualization", events_resized)
            
            key = cv.waitKey(int(1000 / playback_fps))
            if key == 27:  # ESC to exit
                cv.destroyAllWindows()
                return
            elif key == ord('n'):  # 'n' to move to the next sequence
                break
    
    cv.destroyAllWindows()
    print("\nVisualization completed!")


# ============================================================================
# Result Visualization Functions
# ============================================================================

def plot_prediction(results, window_start=0, window_end=-1, experiment_type="pendulum"):
    """
    Plot only model predictions vs targets.
    
    Args:
        results: Dictionary returned from test() function
        window_start: Start index for plotting window
        window_end: End index for plotting window (default: min(2000, total length))
        experiment_type: Type of experiment ("pendulum" or "IMU") for proper denormalization
    """
    
    # If outputs are multi-dimensional (N,4), plot 4 subplots: angle/x for each camera
    out = results.get('test_output')
    targ = results.get('test_target')

    if out is None or targ is None:
        print("No prediction or target data available in results.")
        return

    # Denormalize (handles both single-output and 4-output cases)
    out_dn = denormalize_targets(out, experiment_type)
    targ_dn = denormalize_targets(targ, experiment_type)

    # Ensure slicing is safe
    out_win = out_dn[window_start:window_end]
    targ_win = targ_dn[window_start:window_end]

    if out_win.ndim == 1 or (out_win.ndim == 2 and out_win.shape[1] == 1):
        # Single-output legacy plotting
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 8))
        ax1.plot(out_win, label="Model Output", alpha=0.8, linewidth=1.5)
        ax1.plot(targ_win, label="Target", alpha=0.8, linewidth=1.5)
        ax1.set_xlabel("Frame")
        ax1.set_ylabel("Angle (degrees)")
        ax1.set_title("Model Output vs Target (Continuous Evaluation)")
        ax1.legend()
        ax1.grid(True, alpha=0.3)

        test_error = np.abs(out_win - targ_win)
        ax2.plot(test_error, color='orange', linewidth=1)
        ax2.set_xlabel("Frame")
        ax2.set_ylabel("Absolute Error (degrees)")
        ax2.set_title("Absolute Error over Time")
        ax2.grid(True, alpha=0.3)

        plt.tight_layout()
        plt.show()
    else:
        # Multi-output: expect shape [N,4]
        if out_win.ndim == 1:
            out_win = out_win.reshape(-1, 1)
            targ_win = targ_win.reshape(-1, 1)

        N = out_win.shape[0]
        fig, axes = plt.subplots(2, 2, figsize=(14, 8))
        axes = axes.flatten()

        labels = ["Angle Cam1 (deg)", "X Cam1 (px)", "Angle Cam2 (deg)", "X Cam2 (px)"]

        for i in range(4):
            axes[i].plot(out_win[:, i], label='Pred', alpha=0.8)
            axes[i].plot(targ_win[:, i], label='Target', alpha=0.8)
            axes[i].set_title(labels[i])
            axes[i].set_xlabel('Frame')
            axes[i].legend()
            axes[i].grid(True, alpha=0.3)

        plt.tight_layout()
        plt.show()

        # Print per-channel error statistics
        abs_err = np.abs(out_dn - targ_dn)
        for i, name in enumerate(labels):
            mean_err = np.mean(abs_err[:, i])
            std_err = np.std(abs_err[:, i])
            max_err = np.max(abs_err[:, i])
            print(f"{name}: Mean error={mean_err:.3f}, Std={std_err:.3f}, Max={max_err:.3f}")


def plot_spike_activity(results, window_start=0, window_end=-1):
    """Plot spike activity and input events."""
    if results['spike_activity'] is None:
        print("No spike activity data available. Run test() with monitor_mode='spikes' or 'both'")
        return
    
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 8))
    
    # Subplot 1: Average spike activity
    ax1.plot(results['spike_activity'][window_start:window_end], 
             color='green', linewidth=1.5)
    ax1.set_xlabel("Frame")
    ax1.set_ylabel("Average Spike Activity")
    ax1.set_title("Average Spike Activity Across the Network Over Time")
    ax1.grid(True, alpha=0.3)

    # Subplot 2: Number of input events
    ax2.plot(results['num_events'][window_start:window_end], 
             color='purple', linewidth=1.5)
    ax2.set_xlabel("Frame")
    ax2.set_ylabel("Number of Events")
    ax2.set_title("Number of Input Events per Timestep")
    ax2.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.show()
    
    # Print spike statistics
    print(f"\nSpike activity statistics:")
    print(f"  Mean activity: {np.mean(results['spike_activity']):.6f}")
    print(f"  Std activity: {np.std(results['spike_activity']):.6f}")
    print(f"  Max activity: {np.max(results['spike_activity']):.6f}")
    print(f"  Min activity: {np.min(results['spike_activity']):.6f}")
    
    print(f"\nInput event statistics:")
    print(f"  Mean events per timestep: {np.mean(results['num_events']):.2f}")
    print(f"  Max events per timestep: {np.max(results['num_events']):.2f}")
    print(f"  Min events per timestep: {np.min(results['num_events']):.2f}")


def plot_normalization_stats(results, window_start=0, window_end=-1):
    """Plot normalization summary using amplification factors."""
    if results['norm_stats'] is None:
        print("No normalization statistics data available. Run test() with monitor_mode='norm' or 'both'")
        return

    layer_names, mean_amplifications = compute_mean_amplification_per_layer(
        results['norm_stats'], window_start, window_end
    )

    if len(layer_names) == 0:
        print("No normalization amplification data available to plot.")
        return

    fig = plot_network_amplification(layer_names, mean_amplifications)
    plt.show()


def compute_mean_amplification_per_layer(norm_activity_over_time, window_start=0, window_end=-1):
    """Compute average amplification factor (std_out / std_in) over time for each layer.
    
    For BatchNorm/RMSNorm: computes std_out / std_in from activity stats.
    """
    layer_names = []
    mean_amplifications = []

    # Process layers with temporal activity stats (BatchNorm, RMSNorm)
    if norm_activity_over_time:
        for layer_name, activity in norm_activity_over_time.items():
            input_std = np.array(activity['input_std'][window_start:window_end])
            output_std = np.array(activity['output_std'][window_start:window_end])

            if input_std.size == 0 or output_std.size == 0:
                continue

            std_scaling = output_std / (input_std + 1e-8)
            mean_scaling = std_scaling.mean()

            layer_names.append(layer_name)
            mean_amplifications.append(mean_scaling)

    return layer_names, np.array(mean_amplifications)


def plot_network_amplification(layer_names, mean_amplifications):
    """Bar plot showing average amplification factor per layer."""
    layers = np.arange(len(layer_names))

    fig, ax = plt.subplots(figsize=(10, 4))

    ax.bar(layers, mean_amplifications, alpha=0.8, color='purple')
    ax.axhline(1.0, color='red', linestyle='--', linewidth=2, label='No scaling (1x)')

    avg_network = mean_amplifications.mean()
    ax.axhline(avg_network, color='black', linestyle=':', linewidth=2,
               label=f'Network avg = {avg_network:.1f}x')

    ax.set_xlabel('Layer')
    ax.set_ylabel('Mean Amplification (Output / Input)')
    ax.set_title(f'Mean Amplification Factor per Layer (avg={avg_network:.1f}x)')
    ax.legend()
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    return fig


def plot_all(results, window_start=0, window_end=-1, experiment_type="pendulum"):
    """Plot all available data (predictions, spikes, normalization)."""
    plot_prediction(results, window_start, window_end, experiment_type)
    
    if results['spike_activity'] is not None:
        plot_spike_activity(results, window_start, window_end)
    
    if results['norm_stats'] is not None:
        plot_normalization_stats(results, window_start, window_end)