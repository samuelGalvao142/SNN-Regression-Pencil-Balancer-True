# Spiking Neural Network for Event-Based Regression

[![Python](https://img.shields.io/badge/Python-3.8+-blue.svg)](https://www.python.org/)
[![PyTorch](https://img.shields.io/badge/PyTorch-1.12+-red.svg)](https://pytorch.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

Official implementation of **"Spiking Neural Networks for Event-Based Regression: Analysis of Neuron Dynamics and Normalization Strategies"**.

Event-based cameras provide asynchronous visual measurements with microsecond temporal resolution. This repository explores deep Spiking Neural Networks (SNNs) for continuous-valued regression on event streams, analyzing the fundamental design trade-offs between neuron dynamics, normalization mechanisms, and residual architectures.

---

## Dataset Examples

### Rotary Inverted Pendulum
Event camera captures the angular motion of an inverted pendulum from a top-view perspective.

<p align="center">
  <img src="docs/videos/pendulum_example.gif" alt="Pendulum Dataset" width="400"/>
  <br>
  <em>Sample sequence showing ON events (blue) and OFF events (red)</em>
</p>

**Event Statistics:**
- Resolution: 346×260 pixels
- Integration window: Δt = 30ms
- Dataset size: 50,000 timesteps

### Event-Based Ego-Motion (IMU)
DAVIS346 camera moving along roll and yaw axes, with ground truth from onboard IMU.

<p align="center">
  <img src="docs/videos/imu_example.gif" alt="IMU Dataset" width="400"/>
  <br>
  <em>Camera motion with higher temporal resolution (Δt = 10ms)</em>
</p>

**Event Statistics:**
- Resolution: 346×260 pixels
- Integration window: Δt = 10ms
- Dataset size: 65,000 timesteps

---

## Architecture

The SNN consists of:
1. **Convolutional Blocks** with Leaky Integrate-and-Fire (LIF) neurons
2. **Normalization Layers** (BatchNorm / RMSNorm / Fixed Scaling)
3. **Residual Connections** (Plain / Spiking ResNet / SEW-ResNet)
4. **Regression Head** with high-tau LIF neuron (τ=20) for stable output

<p align="center">
  <img src="docs/images/residual_blocks.png" alt="Residual Block Types" width="700"/>
  <br>
  <em>Three architectural configurations: Plain, Spiking ResNet, and SEW-ResNet blocks</em>
</p>

**Note on Output Neurons:** While Integrate-and-Fire (IF) neurons are ideal for event-based regression (preserving state during sparse input), they produce exploding gradients during training. We use high-tau LIF neurons (τ=20) to approximate IF behavior while maintaining training stability. See paper for detailed gradient analysis.

---

## Results

### Experiment 1: Rotary Inverted Pendulum

**Angle Estimation Error (MAE in degrees):**

| Architecture | BatchNorm | RMSNorm | Fixed Scaling |
|--------------|-----------|---------|---------------|
| **SEW**      | 1.698°    | **1.651°** | 1.734°      |
| **Spiking**  | 1.907°    | **1.632°** | 2.153°      |
| **Plain**    | 2.547°    | 1.709°  | 2.341°        |

✨ **Best result:** Spiking ResNet + RMSNorm = **1.632° MAE**

<p align="center">
  <img src="docs/images/pendulum_predictions.png" alt="Pendulum Predictions" width="800"/>
  <br>
  <em>Model predictions vs ground truth on test sequence</em>
</p>

### Impact of Output Layer Time Constant

<p align="center">
  <img src="docs/images/tau_comparison.png" alt="Tau Comparison" width="700"/>
  <br>
  <em>Comparison of τ=2.0 (noisy) vs τ=20.0 (stable) predictions</em>
</p>

### Experiment 2: Event-Based Ego-Motion Estimation

**Roll Angle Estimation:**

<p align="center">
  <img src="docs/images/imu_predictions.png" alt="IMU Predictions" width="800"/>
  <br>
  <em>Visual odometry from event stream (10ms windows)</em>
</p>

---

## Normalization Analysis

Our experiments reveal that **normalization primarily acts as gain control** rather than statistical re-centering:

<p align="center">
  <img src="docs/images/normalization_analysis.png" alt="Normalization Analysis" width="800"/>
</p>

**Key Findings:**
1. Learnable parameters (γ, β, α) remain close to initialization
2. Effective scaling factors converge to similar values across BN/RMS/MUL
3. **Purpose**: Amplify membrane potentials to sustain spiking activity
4. Without sufficient scaling → vanishing spikes → gradient collapse

---

## 🚀 Getting Started

### Installation

```bash
# Clone repository
git clone https://github.com/your-username/snn-event-regression.git
cd snn-event-regression

# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt
```

### Requirements

- Python 3.8+
- PyTorch 1.12+
- SpikingJelly 0.0.0.0.14+
- NumPy, Matplotlib, OpenCV
- Tonic (for event processing)
- Weights & Biases (optional, for logging)

### Quick Start

```bash
python main.py
```

Edit configuration in [main.py](main.py) to select experiment type (`"pendulum"` or `"IMU"`), architecture (`'SEW'`, `'plain'`, `'spiking'`), and normalization strategy (`'BN'`, `'RMS'`, `'MUL'`).

---

## Project Structure

```
snn-event-regression/
├── main.py                      # Main training/testing script
├── data/                        # Dataset directory (not in repo - too large)
├── src/
│   ├── train.py                 # Training loop with TBPTT
│   ├── test.py                  # Testing and evaluation
│   ├── utils.py                 # Visualization and utilities
│   ├── Dataset/                 # Event data loading and preprocessing
│   └── Network/                 # SNN architecture components
├── models/                      # Saved checkpoints (not in repo)
├── notebooks/                   # Jupyter version of main.py for easier visualization
└── docs/                        # Documentation and figures
```

**Note:** The `data/` and `models/` directories are not included in the repository due to their large size. The `notebooks/` folder contains Jupyter notebook versions of the main training/testing pipeline for easier step-by-step execution and visualization.

---

## Training Details

The model is trained using **Truncated Backpropagation Through Time (TBPTT)** with long sequences (2000 timesteps), allowing for near-continuous evaluation. The network successfully maintains stable predictions even on sequences exceeding 2000 timesteps. See [main.py](main.py) for complete hyperparameter configuration.



---

## Citation

```
#TODO Citation
```

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.