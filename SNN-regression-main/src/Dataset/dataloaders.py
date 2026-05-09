import torch
from torch.utils.data import DataLoader
from tonic import transforms, MemoryCachedDataset
from .datasets import (
    RotatingBarDataset,
    SequenceDataset,
    ContinuousDataset,
    MultiRunSequenceDataset,
    MultiRunContinuousDataset,
)
from .multi_run import split_loaded_runs

def _numpy_to_float_tensor(x):
    """Convert numpy array to float tensor. Must be a named function for multiprocessing pickling."""
    return torch.from_numpy(x).float()


def collate_time_first(batch):
    """
    Collate function that arranges data as [T, B, ...] instead of [B, T, ...].
    This is more natural for processing temporal sequences with SNNs.
    """
    # batch: list of (frames[T,1,2,H,W], labels[T])
    frames, labels = zip(*batch)
    frames = torch.stack(frames, dim=1)   # [T, B, C, H, W]
    labels = torch.stack(labels, dim=1)   # [T, B]
    return frames, labels


def create_dataloaders(input_data, labels, test_ratio=0.05, val_ratio=0.07, SEQ_LENGTH=2000, BATCH_SIZE=4, 
                       num_workers=0, prefetch_factor=None, pin_memory=False, persistent_workers=False):

    # ============================================================================
    # Train/Validation/Test Split
    # ============================================================================

    total_samples = len(input_data)

    # Define split ratios
    test_split = 0.05   # for final testing
    val_split  = 0.07   # for validation during training
    train_split = 1.0 - test_split - val_split  # for training

    # Calculate split indices (temporal order maintained)
    train_end = int(train_split * total_samples)
    val_end   = int((train_split + val_split) * total_samples)

    # Perform temporal split (NO SHUFFLE!)
    train_events = input_data[:train_end]
    train_labels = labels[:train_end]

    val_events = input_data[train_end:val_end]
    val_labels = labels[train_end:val_end]

    test_events = input_data[val_end:]
    test_labels = labels[val_end:]

    # ============================================================================
    # Create datasets with event-to-frame transformation
    # ============================================================================
    H, W = 260, 346  # DAVIS346 resolution (height, width)

    # Transform events to frames with ON/OFF channels
    frame_transform = transforms.ToFrame(
        sensor_size=(W, H, 2),  # (width, height, polarities)
        n_event_bins=1  # Single bin per time window
    )

    # Create base datasets
    train_dataset = RotatingBarDataset(train_events, train_labels, transform=frame_transform)
    val_dataset   = RotatingBarDataset(val_events, val_labels, transform=frame_transform)
    test_dataset  = RotatingBarDataset(test_events, test_labels, transform=frame_transform)

    print(f"Train dataset size: {len(train_dataset)} samples ({train_split*100:.1f}%)")
    print(f"Val dataset size:   {len(val_dataset)} samples ({val_split*100:.1f}%)")
    print(f"Test dataset size:  {len(test_dataset)} samples ({test_split*100:.1f}%)")

    # ============================================================================
    # Cache training data in memory for faster access
    # ============================================================================
    cached_trainset = MemoryCachedDataset(
        train_dataset, 
        transform=_numpy_to_float_tensor,
    )

    # ============================================================================
    # Create sequence datasets
    # ============================================================================

    # Training and validation use fixed-length sequences
    trainset = SequenceDataset(cached_trainset, seq_length=SEQ_LENGTH, expected_shape=(2, H, W))
    valset = SequenceDataset(val_dataset, seq_length=SEQ_LENGTH, expected_shape=(2, H, W))

    # Test uses continuous sequence
    testset = ContinuousDataset(test_dataset, expected_shape=(2, H, W))

    # ============================================================================
    # Create DataLoaders
    # ============================================================================
    trainloader = DataLoader(
        trainset,
        batch_size=BATCH_SIZE,
        collate_fn=collate_time_first,
        shuffle=True,
        num_workers=num_workers,
        prefetch_factor=prefetch_factor,
        pin_memory=pin_memory,
        persistent_workers=persistent_workers,
    )

    valloader = DataLoader(
        valset,
        batch_size=BATCH_SIZE,
        collate_fn=collate_time_first,
        shuffle=False 
    )

    testloader = DataLoader(
        testset, 
        batch_size=1, 
        collate_fn=collate_time_first, 
        shuffle=False)
    
    """
    # Verify train/val/test split distribution
    import matplotlib.pyplot as plt
    import numpy as np

    fig, axes = plt.subplots(3, 1, figsize=(14, 10), sharex=False)

    # ------------------------
    # Train
    # ------------------------
    axes[0].plot(train_labels * 180 / np.pi, linewidth=0.8, alpha=0.7)
    axes[0].set_title(f'Train Data Distribution ({len(train_labels)} samples)')
    axes[0].set_xlabel('Sample Index')
    axes[0].set_ylabel('Angle (degrees)')
    axes[0].grid(True, alpha=0.3)

    # ------------------------
    # Validation
    # ------------------------
    axes[1].plot(val_labels * 180 / np.pi, linewidth=0.8, alpha=0.7)
    axes[1].set_title(f'Validation Data Distribution ({len(val_labels)} samples)')
    axes[1].set_xlabel('Sample Index')
    axes[1].set_ylabel('Angle (degrees)')
    axes[1].grid(True, alpha=0.3)

    # ------------------------
    # Test
    # ------------------------
    axes[2].plot(test_labels * 180 / np.pi, linewidth=0.8, alpha=0.7)
    axes[2].set_title(f'Test Data Distribution ({len(test_labels)} samples)')
    axes[2].set_xlabel('Sample Index')
    axes[2].set_ylabel('Angle (degrees)')
    axes[2].grid(True, alpha=0.3)

    plt.tight_layout()
    plt.show()

    # ------------------------
    # Ranges (important sanity check)
    # ------------------------
    print(f"\nTrain data range: [{train_labels.min()*180/np.pi:.2f}°, {train_labels.max()*180/np.pi:.2f}°]")
    print(f"Val data range:   [{val_labels.min()*180/np.pi:.2f}°, {val_labels.max()*180/np.pi:.2f}°]")
    print(f"Test data range:  [{test_labels.min()*180/np.pi:.2f}°, {test_labels.max()*180/np.pi:.2f}°]")
    """
    
    return trainloader, valloader, testloader


def create_multi_run_dataloaders(
    loaded_runs,
    *,
    test_ratio=0.05,
    val_ratio=0.07,
    SEQ_LENGTH=2000,
    BATCH_SIZE=4,
    shuffle_runs=False,
    run_split_seed=42,
    num_workers=0,
    prefetch_factor=None,
    pin_memory=False,
    persistent_workers=False,
):
    """
    Create dataloaders from multiple independent runs while preserving run boundaries.
    """

    train_runs, val_runs, test_runs = split_loaded_runs(
        list(loaded_runs),
        test_ratio=test_ratio,
        val_ratio=val_ratio,
        shuffle=shuffle_runs,
        seed=run_split_seed,
    )

    H, W = 260, 346
    frame_transform = transforms.ToFrame(
        sensor_size=(W, H, 2),
        n_event_bins=1,
    )

    def _make_base_dataset(run):
        return RotatingBarDataset(run.sliced_events, run.labels, transform=frame_transform)

    train_base = [_make_base_dataset(run) for run in train_runs]
    val_base = [_make_base_dataset(run) for run in val_runs]
    test_base = [_make_base_dataset(run) for run in test_runs]

    cached_train_base = [
        MemoryCachedDataset(dataset, transform=_numpy_to_float_tensor)
        for dataset in train_base
    ]

    trainset = MultiRunSequenceDataset(cached_train_base, seq_length=SEQ_LENGTH, expected_shape=(2, H, W))
    valset = MultiRunSequenceDataset(val_base, seq_length=SEQ_LENGTH, expected_shape=(2, H, W))
    testset = MultiRunContinuousDataset(test_base, expected_shape=(2, H, W))

    print(f"Train runs: {len(train_runs)}")
    print(f"Validation runs: {len(val_runs)}")
    print(f"Test runs: {len(test_runs)}")
    print(f"Train sequences: {len(trainset)}")
    print(f"Validation sequences: {len(valset)}")
    print(f"Test sequences: {len(testset)}")

    trainloader = DataLoader(
        trainset,
        batch_size=BATCH_SIZE,
        collate_fn=collate_time_first,
        shuffle=True,
        num_workers=num_workers,
        prefetch_factor=prefetch_factor,
        pin_memory=pin_memory,
        persistent_workers=persistent_workers,
    )

    valloader = DataLoader(
        valset,
        batch_size=BATCH_SIZE,
        collate_fn=collate_time_first,
        shuffle=False,
    )

    testloader = DataLoader(
        testset,
        batch_size=1,
        collate_fn=collate_time_first,
        shuffle=False,
    )

    return trainloader, valloader, testloader
