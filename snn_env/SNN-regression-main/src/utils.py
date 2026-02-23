import numpy as np
import cv2 as cv
import matplotlib.pyplot as plt


def normalize_targets(targets, experiment_type):
    """
    Normalize targets to [0, 1] range based on experiment type.
    """
    if experiment_type.lower() == "pendulum":
        return (targets + np.pi)/(2*np.pi)
    elif experiment_type.lower() == "imu":
        return -targets / np.pi
    else:
        raise ValueError(f"Unknown experiment type: {experiment_type}")


def denormalize_targets(normalized_targets, experiment_type):
    """
    Convert normalized targets back to degrees for visualization.
    """
    if experiment_type.lower() == "pendulum":
        # Pendulum: [0, 1] normalized -> [-180, 180] degrees
        return (normalized_targets * 360) - 180
    elif experiment_type.lower() == "imu":
        # IMU: [0, 1] normalized -> [0, -pi] radians -> [0, -180] degrees
        return -normalized_targets * 180
    else:
        # Default to pendulum behavior
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
        # labels_batch: [T, B]
        T, B, C, H, W = frames_batch.shape
        
        print(f"\n=== Sequence {seq_idx+1}/{n_sequences} ===")
        print(f"Batch shape: {frames_batch.shape}, Labels shape: {labels_batch.shape}")
        
        # Visualize only the first element of the batch
        batch_item = 0
        
        for t in range(T):
            # Extract frame at time t for the first batch item
            # frame: [C, H, W] where C=2 (ON/OFF polarities)
            frame = frames_batch[t, batch_item].cpu().numpy()  # [2, H, W]
            angle = labels_batch[t, batch_item].item()
            
            # Create RGB visualization
            # Channel 0 = ON events (positive), Channel 1 = OFF events (negative)
            events_img = np.ones((H, W, 3), dtype=np.uint8) * 255
            
            # ON events in dark blue
            on_events = frame[0] > 0
            events_img[on_events] = [0, 0, 200]
            
            # OFF events in dark red
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
    
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 8))
    
    # Denormalize predictions and targets based on experiment type
    output_degrees = denormalize_targets(results['test_output'][window_start:window_end], experiment_type)
    target_degrees = denormalize_targets(results['test_target'][window_start:window_end], experiment_type)
    
    # Subplot 1: Model output vs target
    ax1.plot(output_degrees, label="Model Output", alpha=0.8, linewidth=1.5)
    ax1.plot(target_degrees, label="Target", alpha=0.8, linewidth=1.5)
    ax1.set_xlabel("Frame")
    ax1.set_ylabel("Angle (degrees)")
    ax1.set_title("Model Output vs Target (Continuous Evaluation)")
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    # Subplot 2: Absolute error
    test_error = np.abs(output_degrees - target_degrees)
    ax2.plot(test_error, color='orange', linewidth=1)
    ax2.set_xlabel("Frame")
    ax2.set_ylabel("Absolute Error (degrees)")
    ax2.set_title("Absolute Error over Time")
    ax2.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.show()
    
    # Print error statistics
    output_full = denormalize_targets(results['test_output'], experiment_type)
    target_full = denormalize_targets(results['test_target'], experiment_type)
    
    print(f"\nError statistics (Full sequence):")
    print(f"  Mean error: {np.mean(np.abs(output_full - target_full)):.3f}°")
    print(f"  Std error: {np.std(np.abs(output_full - target_full)):.3f}°")
    print(f"  Max error: {np.max(np.abs(output_full - target_full)):.3f}°")

    print(f"\nError statistics (Window [{window_start}:{window_end}]):")
    print(f"  Mean error: {np.mean(np.abs(output_degrees - target_degrees)):.3f}°")
    print(f"  Std error: {np.std(np.abs(output_degrees - target_degrees)):.3f}°")
    print(f"  Max error: {np.max(np.abs(output_degrees - target_degrees)):.3f}°")


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