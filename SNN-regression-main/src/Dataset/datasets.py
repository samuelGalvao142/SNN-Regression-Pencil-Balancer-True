import torch
from torch.utils.data import Dataset
import numpy as np


# ============================================================================
# Base dataset for individual frames
# ============================================================================
class RotatingBarDataset(Dataset):
    """
    Basic dataset that wraps sliced events and their corresponding labels.
    Each item returns a single time window of events and its target angle.
    """
    def __init__(self, sliced_events, labels, transform=None):
        self.sliced_events = sliced_events
        self.labels = labels
        self.transform = transform
        
    def __len__(self):
        return len(self.sliced_events)
    
    def __getitem__(self, idx):
        events = self.sliced_events[idx]
        target = self.labels[idx]
        
        if self.transform:
            events = self.transform(events)
            
        return events, target


# ============================================================================
# Fixed-length sequences for training/validation
# ============================================================================
class SequenceDataset(torch.utils.data.Dataset):
    """
    Splits a long temporal sequence into multiple fixed-length subsequences.
    
    Note:
        Any remaining frames that don't fit into a complete sequence are discarded.
    """
    def __init__(self, base_dataset, seq_length=5000, expected_shape=(2, 346, 260)):
        self.base_dataset = base_dataset
        self.seq_length = seq_length
        self.expected_shape = expected_shape
        self.total_length = len(base_dataset)

        # Compute number of complete sequences (discard remainder)
        self.num_sequences = self.total_length // seq_length

    def __len__(self):
        return self.num_sequences

    def __getitem__(self, idx):
        # Compute start and end indices for this sequence
        start_idx = idx * self.seq_length
        end_idx = start_idx + self.seq_length

        frames = []
        labels = []

        # Collect all frames and labels in this sequence
        for i in range(start_idx, end_idx):
            frame, label = self.base_dataset[i]

            # Convert to tensor if needed
            if isinstance(frame, np.ndarray):
                frame = torch.from_numpy(frame)
            
            frame = frame.squeeze(0)  # Remove n_event_bins dimension [1, C, H, W] -> [C, H, W]

            # Handle potential transpose issues (ensure [C, H, W] format)
            if frame.shape != self.expected_shape:
                frame = frame.permute(0, 2, 1)  # [C, W, H] -> [C, H, W]

            frames.append(frame)
            labels.append(label)

        # Stack into temporal sequences
        frames = torch.stack(frames, dim=0).float()  # [T, C, H, W]
        labels = torch.tensor(labels, dtype=torch.float32)  # [T]

        return frames, labels


# ============================================================================
# Continuous sequence for testing
# ============================================================================
class ContinuousDataset(torch.utils.data.Dataset):
    """
    Loads the entire sequence without splitting, used for testing.
    """
    def __init__(self, base_dataset, expected_shape=(2, 346, 260)):
        self.base_dataset = base_dataset
        self.expected_shape = expected_shape

    def __len__(self):
        return 1  # Single item: the entire sequence

    def __getitem__(self, idx):
        # Load the entire sequence
        frames = []
        labels = []

        for i in range(len(self.base_dataset)):
            frame, label = self.base_dataset[i]

            if isinstance(frame, np.ndarray):
                frame = torch.from_numpy(frame)
            
            frame = frame.squeeze(0)  # Remove n_event_bins dimension

            # Handle potential transpose issues
            if frame.shape != self.expected_shape:
                frame = frame.permute(0, 2, 1)  # [C, W, H] -> [C, H, W]

            frames.append(frame)
            labels.append(label)

        # Stack: [T, C, H, W] and [T]
        frames = torch.stack(frames, dim=0).float()
        labels = torch.tensor(labels, dtype=torch.float32)

        return frames, labels