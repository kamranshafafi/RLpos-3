# Installation

## System Details

* Release: **Ubuntu 22.04.1 LTS**
* Codename: *jammy*

## Required Packages for installation

* `sudo apt-get install -y git g++ cmake`

## Downloading Modules and Other Dependencies

1. `mkdir ~/workspace && cd ~/workspace`
1. `git clone https://gitlab.inesctec.pt/ruben.m.queiros/fw-phd.git` # The project git
1. `git clone https://gitlab.inesctec.pt/ruben.m.queiros/ts.git`
1. `git clone https://github.com/rukyr97/rukutils.git`
1. `git clone https://github.com/rukyr97/ns3-ai.git`
1. `cd ~/workspace/ns3-ai && git checkout cmake && cd ..` # Change ns3-ai repo to the cmake branch.
1. `git clone https://github.com/rukyr97/ns3-gym.git`
1. `cd ~/workspace/ns3-gym && git checkout app-ns-3.36+ && cd ..` # Change ns3-gym repo to the app branch.
1. `git clone https://gitlab.com/nsnam/ns-3-dev`
1. `cd ~/workspace/ns-3-dev && git checkout tags/ns-3.38` # Change ns-3-dev repo to the ns-3.38 tag.

1. [Install miniconda](https://docs.conda.io/en/main/miniconda.html). **Restart** the terminal after the installation (you should see `(base)` before the prompt).
1. `conda create -n <env_name> python=3.8.5`
1. `conda activate <env_name>`
1. `cd ~/workspace/fw-phd && pip install -r install/requirements.txt`
1. `bash install/install_script.sh -i`
