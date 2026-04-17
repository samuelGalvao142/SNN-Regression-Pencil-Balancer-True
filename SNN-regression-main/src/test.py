import torch
import numpy as np
from tqdm import tqdm
from spikingjelly.activation_based import functional
import matplotlib.pyplot as plt

from .Network.norm import RMSNorm2d, MultiplyBy
from .utils import normalize_targets


# ============================================================================
# Monitoring Mode Configuration
# ============================================================================
def enable_monitoring(model, mode):
    """Enable monitoring hooks based on selected mode."""
    if mode in ["spikes", "both"]:
        model.enable_spike_recording()
    if mode in ["norm", "both"]:
        model.enable_norm_monitoring()


def disable_monitoring(model, mode):
    """Disable monitoring hooks and clean up."""
    if mode in ["spikes", "both"]:
        model.disable_spike_recording()
    if mode in ["norm", "both"]:
        model.disable_norm_monitoring()


def print_norm_layer_stats(model):
    """Print mean weight/bias statistics for normalization layers before testing."""
    import torch.nn as nn
    
    print(f"\n{'='*50}")
    print("Normalization Layer Statistics")
    print(f"{'='*50}")
    
    weight_means = []  # Mean weight per layer
    bias_means = []    # Mean bias per layer (if present)
    
    for layer in model.modules():
        # Check if the layer is BatchNorm2d, RMSNorm2d, or MultiplyBy
        is_norm_layer = isinstance(layer, (nn.BatchNorm2d, RMSNorm2d, MultiplyBy))
        
        if is_norm_layer:
            layer_name = layer.__class__.__name__
            line_parts = [f"Layer: {layer_name}"]
            
            if hasattr(layer, 'weight') and layer.weight is not None:
                if hasattr(layer.weight, 'data'):  # Learnable (tensor)
                    weight_mean = layer.weight.data.mean().item()
                else:  # Fixed (float)
                    weight_mean = float(layer.weight)
                
                weight_means.append(weight_mean)
                line_parts.append(f"Weight mean: {weight_mean:.4f}")
                
                # Bias exists only in BatchNorm2d
                if hasattr(layer, 'bias') and layer.bias is not None:
                    bias_mean = layer.bias.data.mean().item()
                    bias_means.append(bias_mean)
                    line_parts.append(f"Bias mean: {bias_mean:.4f}")
            
            print(" | ".join(line_parts))
    
    # Compute overall network statistics
    if weight_means:
        network_weight_mean = np.mean(weight_means)
        network_weight_std = np.std(weight_means)
        print(f"{'='*50}")
        print(f"Network weight mean: {network_weight_mean:.4f}, std: {network_weight_std:.4f}")
        
    if bias_means:
        network_bias_mean = np.mean(bias_means)
        network_bias_std = np.std(bias_means)
        print(f"Network bias mean: {network_bias_mean:.4f}, std: {network_bias_std:.4f}")
    
    print(f"{'='*50}\n")


def test(model, testloader, CONFIG, monitor_mode="both", loss_fn=None):
    """Evaluate model on test set with comprehensive monitoring capabilities.
    
    Args:
        model: The trained neural network model
        testloader: DataLoader for test data
        CONFIG: Configuration dictionary containing device and other settings
        loss_fn: Loss function to use (default: MSELoss). Can be MSELoss or L1Loss
        monitor_mode: "none", "spikes", "norm", or "both" for monitoring during evaluation
    """
    true_value_initialization = CONFIG["true_value_initialization"]
    device = torch.device(CONFIG["device"])
    experiment_type = CONFIG["experiment"]  # Get experiment type
    
    # Set default loss function if not provided
    if loss_fn is None:
        loss_fn = torch.nn.MSELoss()
    
    # Determine loss type for reporting
    loss_type = "MSE" if isinstance(loss_fn, torch.nn.MSELoss) else "L1"
    
    # Print normalization layer statistics before testing
    print_norm_layer_stats(model)
    
    # Initialize monitoring structures based on mode
    spike_activity_over_time = {} if monitor_mode in ["spikes", "both"] else None
    norm_activity_over_time = {} if monitor_mode in ["norm", "both"] else None

    with torch.no_grad():
        model.eval()
        
        enable_monitoring(model, monitor_mode)


        print(f"\nEvaluating model on test set (Monitor mode: {monitor_mode})...")
        
        test_loss_total = 0.0
        test_rel_err_total = 0.0
        iter_count = 0
        
        # Store all predictions for visualization
        all_predictions = []
        all_targets = []
        
        # Progress bar for test batches
        pbar_test = tqdm(testloader, desc="  Test batches", leave=True)
        
        for data, targets in pbar_test:
            iter_count += 1
            data = data.to(device)           # [T, B, C, H, W]
            targets = targets.to(device)     # [T, B]
            
            # Normalize targets based on experiment type
            targets = normalize_targets(targets, experiment_type)
            
            num_steps = data.size(0)  # T
            
            # Reset hidden states at start of each sequence batch
            functional.reset_net(model)
            
            if true_value_initialization:
                model.lif_out.v = targets[0].unsqueeze(-1)  # Initialize output neuron state
            
            test_mem_list = []
            
            # Process through entire test sequence
            start_step = 1 if true_value_initialization else 0
            for step in range(start_step, num_steps):
                
                # Forward pass
                mem_out = model(data[step])  # [B, 1]
                test_mem_list.append(mem_out)
                
                # Initialize tracking lists on the first timestep
                if step == start_step:
                    if monitor_mode in ["spikes", "both"]:
                        for k in model.spike_record.keys():
                            spike_activity_over_time[k] = []
                    
                    if monitor_mode in ["norm", "both"]:
                        for k in model.norm_stats.keys():
                            norm_activity_over_time[k] = {
                                'input_mean': [],
                                'input_std': [],
                                'output_mean': [],
                                'output_std': [],
                                'input_range': [],
                                'output_range': []
                            }
                
                # Record spike activity if enabled
                if monitor_mode in ["spikes", "both"]:
                    # For each LIF layer, compute spike activity for this timestep
                    for k, v in model.spike_record.items():
                        # v: [B, C, H, W] or [B, N]
                        # Average spikes per neuron in this layer at this timestep
                        activity = v.detach().cpu().mean().item()
                        spike_activity_over_time[k].append(activity)
                
                # Record normalization statistics if enabled
                if monitor_mode in ["norm", "both"]:
                    # For each BatchNorm layer, extract statistics at this timestep
                    for layer_name, stats in model.norm_stats.items():
                        # Average across all channels for this timestep
                        norm_activity_over_time[layer_name]['input_mean'].append(
                            np.mean(stats['input_mean_per_channel'])
                        )
                        norm_activity_over_time[layer_name]['input_std'].append(
                            np.mean(stats['input_std_per_channel'])
                        )
                        norm_activity_over_time[layer_name]['output_mean'].append(
                            np.mean(stats['output_mean_per_channel'])
                        )
                        norm_activity_over_time[layer_name]['output_std'].append(
                            np.mean(stats['output_std_per_channel'])
                        )
                        
                        # Ranges (max - min) averaged
                        input_range = np.mean(stats['input_max_per_channel'] - stats['input_min_per_channel'])
                        output_range = np.mean(stats['output_max_per_channel'] - stats['output_min_per_channel'])
                        
                        norm_activity_over_time[layer_name]['input_range'].append(input_range)
                        norm_activity_over_time[layer_name]['output_range'].append(output_range)
            
            # Stack all predictions for this batch
            batch_predictions = torch.stack(test_mem_list, dim=0)  # [T, B, 1]
            batch_predictions = batch_predictions.squeeze(-1)  # [T, B]

            # Align targets when using true value initialization
            targets_aligned = targets[start_step:]  # [T, B]

            # Calculate metrics for this batch over full aligned sequence
            batch_loss = loss_fn(batch_predictions, targets_aligned)
            batch_rel_err = torch.linalg.norm(batch_predictions - targets_aligned) / torch.linalg.norm(targets_aligned)
            
            test_loss_total += batch_loss.item()
            test_rel_err_total += batch_rel_err.item()
            
            # Update progress bar
            pbar_test.set_postfix({'loss': f'{batch_loss.item():.6f}'})
            
            # Store for visualization (flatten batch dimension) over full aligned sequence
            all_predictions.append(batch_predictions.detach().cpu().numpy())
            all_targets.append(targets_aligned.detach().cpu().numpy())
        
        pbar_test.close()
        
        # Average metrics
        avg_test_loss = test_loss_total / iter_count
        avg_test_rel_err = test_rel_err_total / iter_count
        
        # Concatenate all predictions (flatten across batches and batch dimension)
        test_mem_continuous = np.concatenate([p.reshape(-1) for p in all_predictions])
        test_target_continuous = np.concatenate([t.reshape(-1) for t in all_targets])
    
    # Disable monitoring after evaluation
    disable_monitoring(model, monitor_mode)

    # ========================================================================
    # Print evaluation results
    # ========================================================================
    print(f"\n{'='*50}")
    print(f"{'Test ' + loss_type + ' Loss:':<{20}}{avg_test_loss:1.2e}")
    print(f"{'Test Rel. Error:':<{20}}{avg_test_rel_err:1.2e}")
    print(f"{'Total iterations:':<{20}}{iter_count}")
    print(f"{'Total timesteps:':<{20}}{len(test_mem_continuous)}")
    print(f"{'Monitoring mode:':<{20}}{monitor_mode}")
    print(f"{'='*50}")

    # ========================================================================
    # Extract spike and normalization statistics
    # ========================================================================
    
    # Process spike activity if monitoring was enabled
    if monitor_mode in ["spikes", "both"]:
        # Convert to arrays to calculate averages across the network
        all_layers = list(spike_activity_over_time.keys())
        T = len(next(iter(spike_activity_over_time.values())))  # number of timesteps

        # Average spike activity per timestep across the network
        spike_activity_total = np.zeros(T)
        for k in all_layers:
            spike_activity_total += np.array(spike_activity_over_time[k])
        spike_activity_total /= len(all_layers)
        
        # data: [T, B, C, H, W]
        num_timesteps = data.size(0)
        num_events_per_timestep = []

        # Align event counting with processed timesteps
        start_step = 1 if true_value_initialization else 0
        for t in range(start_step, num_timesteps):
            # Count non-zero values at this timestep (sum over batch, channels, height, width)
            events = (data[t] != 0).sum().item()
            num_events_per_timestep.append(events)

        num_events_per_timestep = np.array(num_events_per_timestep)
            
    else:
        spike_activity_total = None
        num_events_per_timestep = None

    # Return results dictionary for further analysis
    results = {
        'avg_loss': avg_test_loss,
        'avg_rel_err': avg_test_rel_err,
        'test_output': test_mem_continuous,
        'test_target': test_target_continuous,
        'spike_activity': spike_activity_total,
        'num_events': num_events_per_timestep,
        'norm_stats': norm_activity_over_time,
        'spike_stats': spike_activity_over_time,
    }
    
    return results

