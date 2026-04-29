import random
from pathlib import Path

import numpy as np
import torch
from spikingjelly.activation_based import surrogate

from src import (
    SNN_Net,
    create_dataloaders,
    layer_list_plain,
    layer_list_sew,
    layer_list_spiking,
    plot_all,
    read_IMU_file,
    read_pendulum_file,
    read_pencil_file,
    test,
    train,
)


def main():
    seed = 42
    random.seed(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)
    if torch.cuda.is_available():
        torch.cuda.manual_seed(seed)
        torch.cuda.manual_seed_all(seed)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    train_model = True
    experiment = "Pencil"   # Options: "Pendulum", "Pencil", "IMU"
    monitor_mode = "norm"   # Options: "none", "spikes", "norm", "both"

    test_ratio = 0.05
    val_ratio = 0.07
    sequence_length = 2000
    batch_size = 4

    num_workers = 0
    prefetch_factor = None
    pin_memory = False
    persistent_workers = False

    use_wandb = False
    wandb_name = f"snn-{experiment.lower()}-regression"

    block_type = "SEW"
    reset_type = "soft"
    surrogate_function = surrogate.ATan()
    plif = False
    tau = 2.0
    final_tau = 20.0
    norm_type = "BN"
    learnable_norm = True
    init_scale = 5.0
    K = 10
    num_epochs = 30
    early_stop_patience = 10
    readout_mode = "lif"
    output_bias = True
    optimizer_name = "SGD"
    learning_rate = 1e-2
    momentum = 0.9
    weight_decay = 0.0

    target_specs = None

    if experiment.lower() == "pendulum":
        event_file = "./data/pendulum_events.aedat4"
        label_file = "./data/pendulum_encoder.csv"
        time_window = 30000
        start_frame = 300
        end_frame = -1

        events_per_frame, labels = read_pendulum_file(
            event_file,
            label_file,
            time_window=time_window,
            START_FRAME=start_frame,
            END_FRAME=end_frame,
        )

        hidden = 256
        true_value_initialization = False
        transient = 200
        output_dim = 1
        target_specs = [
            {
                "name": "angle",
                "display_name": "Angle",
                "unit": "deg",
                "normalize": "angle_pm_pi",
            }
        ]

    elif experiment.lower() == "pencil":
        event_file = "./data/hough_camp1_cam1.aedat4"
        label_file = "./data/hough_cam1_cam1_hough.csv"
        time_window = 5000
        start_frame = 300
        end_frame = -1

        events_per_frame, labels, label_metadata = read_pencil_file(
            event_file,
            label_file,
            time_window=time_window,
            START_FRAME=start_frame,
            END_FRAME=end_frame,
            timestamp_column="timestamp_us",
            angle_column="lin_m",
            position_column="lin_b",
            angle_unit="rad",
            position_scale=1.0,
            position_name="x",
            position_unit="mm",
        )

        hidden = 256
        true_value_initialization = False
        transient = 50
        output_dim = 2
        norm_type = "RMS"
        readout_mode = "linear"
        optimizer_name = "Adam"
        learning_rate = 3e-4
        momentum = 0.0
        target_specs = [
            {
                "name": "angle",
                "display_name": "Angle",
                "unit": "deg",
                "normalize": "angle_pm_pi",
            },
            {
                "name": label_metadata["position_name"],
                "display_name": label_metadata["position_name"],
                "unit": label_metadata["position_unit"],
                "normalize": "linear",
                "min": label_metadata["position_min"],
                "max": label_metadata["position_max"],
            },
        ]

    elif experiment.lower() == "imu":
        event_file = "./data/imu_events_large.aedat4"
        time_window = 10000
        start_frame = 0
        end_frame = -2500

        events_per_frame, labels = read_IMU_file(
            event_file,
            time_window=time_window,
            START_FRAME=start_frame,
            END_FRAME=end_frame,
        )

        hidden = 512
        true_value_initialization = True
        transient = 0
        output_dim = 1
        target_specs = [
            {
                "name": "roll",
                "display_name": "Roll",
                "unit": "deg",
                "normalize": "negative_angle_pi",
            }
        ]
    else:
        raise ValueError("Unsupported experiment type. Choose 'Pendulum', 'Pencil', or 'IMU'.")

    trainloader, valloader, testloader = create_dataloaders(
        events_per_frame,
        labels,
        test_ratio=test_ratio,
        val_ratio=val_ratio,
        SEQ_LENGTH=sequence_length,
        BATCH_SIZE=batch_size,
        num_workers=num_workers,
        prefetch_factor=prefetch_factor,
        pin_memory=pin_memory,
        persistent_workers=persistent_workers,
    )

    if block_type.lower() == "sew":
        layer_list = layer_list_sew
    elif block_type.lower() == "plain":
        layer_list = layer_list_plain
    elif block_type.lower() == "spiking":
        layer_list = layer_list_spiking
    else:
        raise ValueError(f"Unsupported block type: {block_type}")

    v_reset = 0.0 if reset_type == "hard" else None

    model = SNN_Net(
        tau=tau,
        final_tau=final_tau,
        layer_list=layer_list,
        hidden=hidden,
        v_reset=v_reset,
        surrogate_function=surrogate_function,
        connect_f="ADD",
        Plif=plif,
        norm_type=norm_type,
        learnable_norm=learnable_norm,
        init_scale=init_scale,
        output_dim=output_dim,
        readout_mode=readout_mode,
        output_bias=output_bias,
    ).to(device)

    CONFIG = {
        "experiment": experiment,
        "event_file": event_file,
        "label_file": label_file if experiment.lower() != "imu" else None,
        "time_window": time_window,
        "output_dim": output_dim,
        "readout_mode": readout_mode,
        "output_bias": output_bias,
        "target_specs": target_specs,
        "block_type": block_type,
        "hidden": hidden,
        "reset_type": reset_type,
        "tau": tau,
        "final_tau": final_tau,
        "surrogate_function": surrogate_function.__class__.__name__,
        "Plif": plif,
        "norm_type": norm_type,
        "learnable_norm": learnable_norm,
        "init_scale": init_scale,
        "K": K,
        "true_value_initialization": true_value_initialization,
        "transient": transient,
        "batch_size": batch_size,
        "sequence_length": sequence_length,
        "optimizer": optimizer_name,
        "learning_rate": learning_rate,
        "momentum": momentum,
        "weight_decay": weight_decay,
        "scheduler": "ReduceLROnPlateau",
        "scheduler_factor": 0.5,
        "scheduler_patience": 1,
        "min_lr": 1e-6,
        "num_epochs": num_epochs,
        "device": str(device),
        "early_stop_patience": early_stop_patience,
    }

    output_dir = Path(
        f"./models/model_{CONFIG['block_type']}_{CONFIG['norm_type']}_{CONFIG['optimizer']}/"
        f"checkpoints_{CONFIG['experiment'].lower()}"
    )

    if train_model:
        print("\n" + "=" * 70)
        print("TRAINING MODE")
        print("=" * 70)
        output_dir.mkdir(parents=True, exist_ok=True)
        train(
            model,
            trainloader,
            valloader,
            CONFIG,
            output_dir,
            loss_fn=torch.nn.MSELoss(),
            use_wandb=use_wandb,
            project_name=wandb_name,
        )
        print("\nModel trained successfully.")

    print("\n" + "=" * 70)
    print("LOADING BEST MODEL WEIGHTS")
    print("=" * 70)
    model_checkpoint_path = output_dir / "best_model_weights.pth"
    if not model_checkpoint_path.exists():
        raise FileNotFoundError(f"Model weights not found: {model_checkpoint_path}")

    print(f"Loading model weights from: {model_checkpoint_path}")
    model.load_state_dict(torch.load(str(model_checkpoint_path), map_location=device, weights_only=True))
    print("Model weights loaded successfully.\n")

    results = test(model, testloader, CONFIG, monitor_mode, loss_fn=torch.nn.MSELoss())
    plot_all(results, window_start=200, window_end=-1, experiment_type=experiment)

    if use_wandb:
        import wandb
        wandb.finish()


if __name__ == "__main__":
    main()
