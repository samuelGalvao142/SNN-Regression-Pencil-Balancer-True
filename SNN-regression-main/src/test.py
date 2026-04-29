import numpy as np
import torch
from spikingjelly.activation_based import functional
from tqdm import tqdm

from .Network.norm import MultiplyBy, RMSNorm2d
from .train import (
    _align_prediction_shape,
    _mean_relative_error,
    _prepare_initial_state,
    _sum_output_losses,
    _supports_stateful_readout,
)
from .utils import normalize_targets


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

    print(f"\n{'=' * 50}")
    print("Normalization Layer Statistics")
    print(f"{'=' * 50}")

    weight_means = []
    bias_means = []

    for layer in model.modules():
        is_norm_layer = isinstance(layer, (nn.BatchNorm2d, RMSNorm2d, MultiplyBy))

        if is_norm_layer:
            layer_name = layer.__class__.__name__
            line_parts = [f"Layer: {layer_name}"]

            if hasattr(layer, "weight") and layer.weight is not None:
                weight_mean = layer.weight.data.mean().item() if hasattr(layer.weight, "data") else float(layer.weight)
                weight_means.append(weight_mean)
                line_parts.append(f"Weight mean: {weight_mean:.4f}")

                if hasattr(layer, "bias") and layer.bias is not None:
                    bias_mean = layer.bias.data.mean().item()
                    bias_means.append(bias_mean)
                    line_parts.append(f"Bias mean: {bias_mean:.4f}")

            print(" | ".join(line_parts))

    if weight_means:
        network_weight_mean = np.mean(weight_means)
        network_weight_std = np.std(weight_means)
        print(f"{'=' * 50}")
        print(f"Network weight mean: {network_weight_mean:.4f}, std: {network_weight_std:.4f}")

    if bias_means:
        network_bias_mean = np.mean(bias_means)
        network_bias_std = np.std(bias_means)
        print(f"Network bias mean: {network_bias_mean:.4f}, std: {network_bias_std:.4f}")

    print(f"{'=' * 50}\n")


def test(model, testloader, CONFIG, monitor_mode="both", loss_fn=None):
    """Evaluate model on test set with optional spike and normalization monitoring."""
    true_value_initialization = CONFIG["true_value_initialization"]
    device = torch.device(CONFIG["device"])

    if loss_fn is None:
        loss_fn = torch.nn.MSELoss()

    loss_type = "MSE" if isinstance(loss_fn, torch.nn.MSELoss) else "L1"
    print_norm_layer_stats(model)

    spike_activity_over_time = {} if monitor_mode in ["spikes", "both"] else None
    norm_activity_over_time = {} if monitor_mode in ["norm", "both"] else None

    with torch.no_grad():
        model.eval()
        enable_monitoring(model, monitor_mode)

        print(f"\nEvaluating model on test set (Monitor mode: {monitor_mode})...")

        test_loss_total = 0.0
        test_rel_err_total = 0.0
        iter_count = 0

        all_predictions = []
        all_targets = []

        pbar_test = tqdm(testloader, desc="  Test batches", leave=True)

        for data, targets in pbar_test:
            iter_count += 1
            data = data.to(device)
            targets = normalize_targets(targets.to(device), CONFIG)

            num_steps = data.size(0)

            functional.reset_net(model)

            if true_value_initialization and _supports_stateful_readout(model):
                model.lif_out.v = _prepare_initial_state(targets)

            test_mem_list = []
            start_step = 1 if true_value_initialization else 0

            for step in range(start_step, num_steps):
                mem_out = model(data[step])
                test_mem_list.append(mem_out)

                if step == start_step:
                    if monitor_mode in ["spikes", "both"]:
                        for key in model.spike_record.keys():
                            spike_activity_over_time[key] = []

                    if monitor_mode in ["norm", "both"]:
                        for key in model.norm_stats.keys():
                            norm_activity_over_time[key] = {
                                "input_mean": [],
                                "input_std": [],
                                "output_mean": [],
                                "output_std": [],
                                "input_range": [],
                                "output_range": [],
                            }

                if monitor_mode in ["spikes", "both"]:
                    for key, value in model.spike_record.items():
                        spike_activity_over_time[key].append(value.detach().cpu().mean().item())

                if monitor_mode in ["norm", "both"]:
                    for layer_name, stats in model.norm_stats.items():
                        norm_activity_over_time[layer_name]["input_mean"].append(np.mean(stats["input_mean_per_channel"]))
                        norm_activity_over_time[layer_name]["input_std"].append(np.mean(stats["input_std_per_channel"]))
                        norm_activity_over_time[layer_name]["output_mean"].append(np.mean(stats["output_mean_per_channel"]))
                        norm_activity_over_time[layer_name]["output_std"].append(np.mean(stats["output_std_per_channel"]))

                        input_range = np.mean(stats["input_max_per_channel"] - stats["input_min_per_channel"])
                        output_range = np.mean(stats["output_max_per_channel"] - stats["output_min_per_channel"])
                        norm_activity_over_time[layer_name]["input_range"].append(input_range)
                        norm_activity_over_time[layer_name]["output_range"].append(output_range)

            batch_predictions = torch.stack(test_mem_list, dim=0)
            targets_aligned = targets[start_step:]
            batch_predictions, targets_aligned = _align_prediction_shape(batch_predictions, targets_aligned)

            batch_loss = _sum_output_losses(loss_fn, batch_predictions, targets_aligned)
            batch_rel_err = _mean_relative_error(batch_predictions, targets_aligned)

            test_loss_total += batch_loss.item()
            test_rel_err_total += batch_rel_err.item()

            pbar_test.set_postfix({"loss": f"{batch_loss.item():.6f}"})

            all_predictions.append(batch_predictions.detach().cpu().numpy())
            all_targets.append(targets_aligned.detach().cpu().numpy())

        pbar_test.close()

        avg_test_loss = test_loss_total / iter_count
        avg_test_rel_err = test_rel_err_total / iter_count

        if all_predictions and all_predictions[0].ndim == 3:
            test_mem_continuous = np.concatenate([prediction.reshape(-1, prediction.shape[-1]) for prediction in all_predictions], axis=0)
            test_target_continuous = np.concatenate([target.reshape(-1, target.shape[-1]) for target in all_targets], axis=0)
        else:
            test_mem_continuous = np.concatenate([prediction.reshape(-1) for prediction in all_predictions])
            test_target_continuous = np.concatenate([target.reshape(-1) for target in all_targets])

    disable_monitoring(model, monitor_mode)

    print(f"\n{'=' * 50}")
    print(f"{'Test ' + loss_type + ' Loss:':<{20}}{avg_test_loss:1.2e}")
    print(f"{'Test Rel. Error:':<{20}}{avg_test_rel_err:1.2e}")
    print(f"{'Total iterations:':<{20}}{iter_count}")
    print(f"{'Total timesteps:':<{20}}{len(test_mem_continuous)}")
    print(f"{'Monitoring mode:':<{20}}{monitor_mode}")
    print(f"{'=' * 50}")

    if monitor_mode in ["spikes", "both"]:
        all_layers = list(spike_activity_over_time.keys())
        num_timesteps = len(next(iter(spike_activity_over_time.values())))

        spike_activity_total = np.zeros(num_timesteps)
        for layer_name in all_layers:
            spike_activity_total += np.array(spike_activity_over_time[layer_name])
        spike_activity_total /= len(all_layers)

        num_events_per_timestep = []
        start_step = 1 if true_value_initialization else 0
        for timestep in range(start_step, data.size(0)):
            num_events_per_timestep.append((data[timestep] != 0).sum().item())
        num_events_per_timestep = np.array(num_events_per_timestep)
    else:
        spike_activity_total = None
        num_events_per_timestep = None

    return {
        "avg_loss": avg_test_loss,
        "avg_rel_err": avg_test_rel_err,
        "test_output": test_mem_continuous,
        "test_target": test_target_continuous,
        "spike_activity": spike_activity_total,
        "num_events": num_events_per_timestep,
        "norm_stats": norm_activity_over_time,
        "spike_stats": spike_activity_over_time,
        "config": CONFIG,
    }
