import torch
import torch.nn as nn
from torch import Tensor
from torch.nn.parameter import Parameter

# ============================================================================
# RMS Normalization
# ============================================================================
class RMSNorm2d(nn.Module):
    """
    Root Mean Square Normalization for 2D feature maps.
    
    Unlike BatchNorm, RMSNorm:
    - Does not center the data (no mean subtraction)
    - Only normalizes by the RMS (scale only)
    
    Formula: x_norm = x / sqrt(mean(x^2) + eps)
    """
    def __init__(self, num_features, eps=1e-5, affine=True):
        super().__init__()
        self.num_features = num_features
        self.eps = eps
        self.affine = affine

        if self.affine:
            # Learnable per-channel scaling parameter
            self.weight = nn.Parameter(torch.ones(1, num_features, 1, 1))
        else:
            self.register_parameter('weight', None)

    def forward(self, x):
        # x: [B, C, H, W]
        # Compute mean squared value over spatial dimensions (H, W)
        ms = x.pow(2).mean(dim=(2, 3), keepdim=True)  # [B, C, 1, 1]
        
        # Compute RMS
        rms = torch.sqrt(ms + self.eps)

        # Normalize
        x_norm = x / rms

        # Apply learnable scaling if enabled
        if self.weight is not None:
            return x_norm * self.weight
        return x_norm


# ============================================================================
# Simple Scaling (Multiply)
# ============================================================================
class MultiplyBy(nn.Module):
    """
    Simple multiplication by a learnable or fixed scalar.
    
    From: https://github.com/urancon/StereoSpike/blob/main/network/blocks.py
    """
    def __init__(self, weight: float = 5., learnable: bool = True) -> None:
        super(MultiplyBy, self).__init__()

        if learnable:
            # Learnable parameter
            self.weight = Parameter(Tensor([weight]))
        else:
            # Fixed value
            self.weight = weight

    def forward(self, input: Tensor) -> Tensor:
        return torch.mul(input, self.weight)


# ============================================================================
# Normalization Factory Function
# ============================================================================
def get_norm_layer(norm_type: str, num_features: int, learnable: bool = True, eps: float = 1e-5, init_scale: float = 5.0
):
    """
    Factory function to create normalization layers.
    """
    if norm_type == "BN":
        # Standard BatchNorm2d
        return nn.BatchNorm2d(num_features, affine=learnable, eps=eps)

    elif norm_type == "RMS":
        # RMS Normalization
        return RMSNorm2d(num_features, eps=eps, affine=learnable)

    elif norm_type == "MUL":
        # Simple scaling
        return MultiplyBy(init_scale, learnable=learnable)

    elif norm_type is None:
        # No normalization
        return nn.Identity()

    else:
        raise ValueError(f"Unknown normalization type: {norm_type}")