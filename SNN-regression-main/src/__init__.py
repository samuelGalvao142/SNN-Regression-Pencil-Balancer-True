try:
    from .Dataset import (
        read_pendulum_file,
        read_IMU_file,
        create_dataloaders,
        RotatingBarDataset,
        SequenceDataset,
        ContinuousDataset,
    )
except ModuleNotFoundError:
    read_pendulum_file = None
    read_IMU_file = None
    create_dataloaders = None
    RotatingBarDataset = None
    SequenceDataset = None
    ContinuousDataset = None

try:
    from .Network import SNN_Net, layer_list_sew, layer_list_plain, layer_list_spiking
    from .train import train
    from .test import test
    from .utils import (
        visualize_sequence_from_trainloader,
        plot_prediction,
        plot_spike_activity,
        plot_normalization_stats,
        plot_all,
    )
except ModuleNotFoundError:
    SNN_Net = None
    layer_list_sew = None
    layer_list_plain = None
    layer_list_spiking = None
    train = None
    test = None
    visualize_sequence_from_trainloader = None
    plot_prediction = None
    plot_spike_activity = None
    plot_normalization_stats = None
    plot_all = None
