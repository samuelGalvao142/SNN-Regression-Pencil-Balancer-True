from torch import nn
import torch
from spikingjelly.activation_based import neuron, surrogate
from .norm import get_norm_layer

# ============================================================================
# Convolution Helpers
# ============================================================================
def conv3x3(in_ch, out_ch, stride=1, norm_type="BN", learnable=True, init_scale=5.0):
    """
    3x3 convolution with normalization.
    """
    return nn.Sequential(
        nn.Conv2d(in_ch, out_ch, kernel_size=3, stride=stride, padding=1, bias=False),
        get_norm_layer(norm_type, out_ch, learnable=learnable, init_scale=init_scale)
    )


def conv1x1(in_ch, out_ch, stride=1, norm_type="BN", learnable=True, init_scale=5.0):
    """
    1x1 convolution with normalization.
    """
    return nn.Sequential(
        nn.Conv2d(in_ch, out_ch, kernel_size=1, stride=stride, bias=False),
        get_norm_layer(norm_type, out_ch, learnable=learnable, init_scale=init_scale)
    )


# ============================================================================
# Plain Block (No Residual Connection)
# ============================================================================
class PlainBlock(nn.Module):
    """
    Plain convolutional block without residual connections.
    
    Architecture:
        Conv3x3 → Norm → LIF → Conv3x3 → Norm → LIF
    """
    def __init__(self, in_channels, mid_channels, tau=2.0, Plif=False, v_reset=0.0,
                 surrogate_function=surrogate.ATan(), stride1=1, stride2=1,
                 norm_type="BN", learnable_norm=True, init_scale=5.0):
        super(PlainBlock, self).__init__()

        self.conv = nn.Sequential(
            # First conv-norm-LIF
            conv3x3(in_channels, mid_channels, stride=stride1, norm_type=norm_type, learnable=learnable_norm, init_scale=init_scale),
            neuron.ParametricLIFNode(init_tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True)
            if Plif else neuron.LIFNode(tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True),

            # Second conv-norm-LIF
            conv3x3(mid_channels, in_channels, stride=stride2, norm_type=norm_type, learnable=learnable_norm, init_scale=init_scale),
            neuron.ParametricLIFNode(init_tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True)
            if Plif else neuron.LIFNode(tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True)
        )

    def forward(self, x: torch.Tensor):
        return self.conv(x)


# ============================================================================
# Spiking ResNet Block
# ============================================================================
class SpikingBlock(nn.Module):
    """
    Spiking ResNet block with residual connection.
    
    Architecture:
        x → Conv3x3 → Norm → LIF → Conv3x3 → Norm → (+) → LIF → out
        └──────────────── downsample ────────────────┘
    """
    def __init__(self, in_channels, mid_channels, tau=2.0, Plif=False, v_reset=0.0,
                 surrogate_function=surrogate.ATan(), stride1=1, stride2=1,
                 norm_type="BN", learnable_norm=True, init_scale=5.0):
        super(SpikingBlock, self).__init__()

        # Main path
        self.conv = nn.Sequential(
            conv3x3(in_channels, mid_channels, stride=stride1, norm_type=norm_type, learnable=learnable_norm, init_scale=init_scale),
            neuron.ParametricLIFNode(init_tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True)
            if Plif else neuron.LIFNode(tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True),

            nn.Conv2d(mid_channels, in_channels, kernel_size=3, padding=1, stride=stride2, bias=False),
            get_norm_layer(norm_type=norm_type, num_features=in_channels, learnable=learnable_norm, init_scale=init_scale)
        )

        # Output LIF (after addition)
        self.sn = neuron.ParametricLIFNode(init_tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True) \
                  if Plif else neuron.LIFNode(tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True)

        # Shortcut connection (downsample if needed)
        if stride1 == 2:
            self.downsample = conv1x1(in_channels, in_channels, stride=2, norm_type=norm_type, learnable=learnable_norm, init_scale=init_scale)
        else:
            self.downsample = nn.Identity()

    def forward(self, x: torch.Tensor):
        # Residual connection: x + conv(x)
        return self.sn(self.downsample(x) + self.conv(x))


# ============================================================================
# SEW (Spiking Element-Wise) Block
# ============================================================================
class SEWBlock(nn.Module):
    """
    Spiking Element-Wise residual block with flexible connection functions.
    
    Architecture:
        x → Conv3x3 → Norm → LIF → Conv3x3 → Norm → LIF → (+) → out
        └──────────────── downsample → LIF ────────────────┘
    """
    def __init__(self, in_channels, mid_channels, tau=2.0, Plif=False, v_reset=0.0,
                 surrogate_function=surrogate.ATan(), stride1=1, stride2=1,
                 connect_f="ADD", norm_type="BN", learnable_norm=True, init_scale=5.0):
        super().__init__()

        self.connect_f = connect_f

        # First conv-norm-LIF
        self.conv1 = conv3x3(in_channels, mid_channels, stride=stride1, norm_type=norm_type, learnable=learnable_norm, init_scale=init_scale)
        self.lif1 = neuron.ParametricLIFNode(init_tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True) \
                    if Plif else neuron.LIFNode(tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True)

        # Second conv-norm-LIF
        self.conv2 = conv3x3(mid_channels, in_channels, stride=stride2, norm_type=norm_type, learnable=learnable_norm, init_scale=init_scale)
        self.lif2 = neuron.ParametricLIFNode(init_tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True) \
                    if Plif else neuron.LIFNode(tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True)

        # Shortcut (with LIF)
        if stride1 == 2:
            self.downsample = nn.Sequential(
                conv1x1(in_channels, in_channels, stride=2, norm_type=norm_type, learnable=learnable_norm, init_scale=init_scale),
                neuron.ParametricLIFNode(init_tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True)
                if Plif else neuron.LIFNode(tau=tau, v_reset=v_reset, surrogate_function=surrogate_function, detach_reset=True)
            )
        else:
            self.downsample = nn.Identity()

    def forward(self, x):
        # Shortcut path (produces spikes)
        identity = self.downsample(x)

        # Main path
        spk1 = self.lif1(self.conv1(x))
        spk2 = self.lif2(self.conv2(spk1))

        # Apply connection function
        if self.connect_f == "ADD":
            return spk2 + identity
        elif self.connect_f == "AND":
            return spk2 * identity 
        elif self.connect_f == "IAND":
            return identity * (1 - spk2) 
        else:
            raise NotImplementedError