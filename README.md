# RLpos-3: Reinforcement Learning for UAV Positioning in Wireless Networks

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![NS-3 Version](https://img.shields.io/badge/NS--3-3.38-blue.svg)](https://www.nsnam.org/)
[![Python Version](https://img.shields.io/badge/Python-3.8.5-green.svg)](https://www.python.org/)
[![TensorFlow](https://img.shields.io/badge/TensorFlow-2.10.1-orange.svg)](https://www.tensorflow.org/)

## Overview

RLpos-3 is a comprehensive framework that integrates NS-3 network simulation with deep reinforcement learning to optimize UAV (Unmanned Aerial Vehicle) positioning in wireless networks. The framework uses Deep Q-Network (DQN) agents to learn optimal UAV positions that maximize network coverage and throughput while maintaining line-of-sight (LoS) constraints with user equipment (UE) nodes.

### Key Features

- **Reinforcement Learning Integration**: Seamless integration of TensorFlow Agents with NS-3 through OpenAI Gym interface
- **3D UAV Positioning**: Support for three-dimensional UAV movement with configurable constraints
- **Building-Aware Propagation**: Realistic signal propagation modeling with building obstacles
- **Flexible Traffic Models**: Support for both TCP and UDP traffic patterns
- **Comprehensive Logging**: Detailed logging of observations, actions, rewards, and throughput metrics
- **Visualization Tools**: Built-in scripts for generating CDF and CCDF plots of performance metrics

## Table of Contents

- [Architecture](#architecture)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Project Structure](#project-structure)
- [Simulation Workflow](#simulation-workflow)
- [Performance Evaluation](#performance-evaluation)
- [Contributing](#contributing)
- [Citation](#citation)
- [Authors](#authors)
- [Acknowledgments](#acknowledgments)
- [License](#license)

## Architecture

The framework consists of three main components:

1. **NS-3 Simulation Environment**: Handles wireless network simulation with UAVs and UEs
2. **OpenAI Gym Interface**: Bridges NS-3 with Python-based RL agents
3. **DQN Agent**: Learns optimal UAV positioning policies through interaction with the environment

```
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│                 │     │                  │     │                 │
│   DQN Agent     │◄────┤  OpenAI Gym      │◄────┤  NS-3 Simulator │
│   (Python)      │     │  Interface       │     │  (C++)          │
│                 │     │                  │     │                 │
└─────────────────┘     └──────────────────┘     └─────────────────┘
        │                                                  │
        └──────────── Actions & Observations ─────────────┘
```

## Requirements

### System Requirements
- **OS**: Ubuntu 22.04.1 LTS (Jammy)
- **RAM**: Minimum 8GB recommended
- **Storage**: At least 5GB free space

### Software Dependencies
- Git
- G++ compiler
- CMake
- Python 3.8.5
- Miniconda/Anaconda
- NS-3.38
- TensorFlow 2.10.1

## Installation

### Quick Install

1. **Clone the repository and dependencies**:
```bash

git clone https://github.com/kamranshafafi/RLpos-3.git 


2. **Install Miniconda** (if not already installed):
Follow instructions at [https://docs.conda.io/en/main/miniconda.html](https://docs.conda.io/en/main/miniconda.html)

3. **Create Python environment**:
```bash
conda create -n rlpos3 python=3.8.5
conda activate rlpos3
```

4. **Install the framework**:
```bash
pip install -r install/requirements.txt
bash install/install_script.sh -i
```

### Manual Installation

For detailed step-by-step installation instructions, see [docs/installation.md](docs/installation.md).

## Usage

### Basic Usage

Run simulation with default configuration:
```bash
python start_script.py
```

### Training Mode

Train a new DQN policy:
```bash
python start_script.py --simType train --dir my_training_run
```

### Evaluation Mode

Evaluate an existing policy:
```bash
python start_script.py --simType eval --dir my_eval_run
```

### Batch Experiments

Run multiple experiments with different seeds:
```bash
bash script.sh
```

### Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--simType` | Simulation type (`train` or `eval`) | `eval` |
| `--dir` | Custom directory name for results | Auto-generated timestamp |
| `--port` | OpenGym port number | 5555 |
| `--seed` | Random seed for simulation | 1 |

## Configuration

The framework uses a JSON configuration file (`config/input.json`) with the following main sections:

### Agent Configuration
```json
{
  "Agent": {
    "newPolicy": false,
    "logSteps": true,
    "simType": "train",
    "hyperParameters": {
      "fullyConnectedLayer": "64,32",
      "numTrainEpisodes": 10,
      "batchSize": 64,
      "learningRate": 0.001,
      "discount": 0.5
    }
  }
}
```

### Network Configuration
- UAV initial positions
- UE positions and SNR requirements
- Building locations
- WiFi parameters (channel, bandwidth, power)
- Traffic patterns

## Project Structure

```
RLpos-3/
├── config/
│   └── input.json         # Main configuration
├── docs/
│   └── installation.md    # Installation guide
├── install/
│   ├── install_script.sh  # Main installation script
│   └── requirements.txt   # Python dependencies
├── lib/
│   ├── json.hpp          # JSON parsing library (860 KB)
│   ├── myutils.cc        # Utility functions implementation
│   ├── myutils.h         # Utility functions header
│   ├── setuplogfile.sh   # Shell script for log setup
│   ├── simconf.cc        # Simulation configuration implementation
│   ├── simconf.h         # Simulation configuration header
│   ├── simlogs.cc        # Logging utilities implementation
│   ├── simlogs.h         # Logging utilities header
│   ├── traffic.cc        # Traffic/coverage calculations
│   └── traffic.h         # Traffic calculations header
├── rl/
│   ├── agent.py          # DQN agent implementation
│   ├── gym.cc           # OpenAI Gym environment implementation
│   ├── gym.h            # OpenAI Gym environment header
│   ├── utilgym.cc       # Gym utilities implementation
│   └── utilgym.h        # Gym utilities header
├── sim.cc              # Main NS-3 simulation
├── start_script.py     # Entry point script
├── logPath.py          # Log directory management
├── CDF.py              # CDF plot generation
├── CCDF.py             # CCDF plot generation
├── spheres.py          # Sphere intersection visualization
├── script.sh           # Batch experiment script
└── README.md           # This file
```

## Simulation Workflow

1. **Environment Initialization**: NS-3 creates the network topology with UAVs, UEs, and buildings
2. **Observation**: UAV position, number of LoS links, and throughput
3. **Action Selection**: DQN agent selects movement direction (up/down/left/right/front/back/same)
4. **Constraint Checking**: Ensures UAV remains within coverage constraints (sphere intersections)
5. **Reward Calculation**: Weighted combination of LoS ratio (80%) and throughput ratio (20%)
6. **State Transition**: Environment updates and provides new observation

## Performance Evaluation

The framework provides several tools for performance analysis:

### Metrics Collected
- Aggregate throughput
- Number of LoS links
- UAV trajectory
- Packet delays
- Reward evolution

### Visualization Scripts
```bash
# Generate CDF plots
python CDF.py

# Generate CCDF plots
python CCDF.py

# Visualize sphere intersections
python spheres.py
```

### Result Files
- `throughput_rx.csv`: Throughput measurements over time
- `observation.csv`: State observations at each step
- `rewards.csv`: Reward values during training
- `action.csv`: Actions taken by the agent
- `logSteps.csv`: Detailed step-by-step logs

## Contributing

We welcome contributions! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

Please read [docs/contributing.md](docs/contributing.md) for detailed guidelines.

## Citation

If you use this framework in your research, please cite:

```bibtex
@misc{shafafi2025frameworkdevelopvalidaterlbased,
      title={A Framework to Develop and Validate RL-Based Obstacle-Aware UAV Positioning Algorithms}, 
      author={Kamran Shafafi and Manuel Ricardo and Rui Campos},
      year={2025},
      eprint={2502.08787},
      archivePrefix={arXiv},
      primaryClass={cs.NI},
      url={https://arxiv.org/abs/2502.08787}, 
}
```

## Authors

**Kamran Shafafi**  
- Email: kamranshafafi@gmail.com
- Affiliation: INESC TEC


### Third-Party Libraries

- [NS-3 Network Simulator](https://www.nsnam.org/)
- [TensorFlow](https://www.tensorflow.org/)
- [OpenAI Gym](https://gym.openai.com/)
- [nlohmann/json](https://github.com/nlohmann/json)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2024 Kamran Shafafi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

<p align="center">
  Made with ❤️ at INESC TEC
</p>
