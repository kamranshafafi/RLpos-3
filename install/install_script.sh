#!/bin/bash

###Variables###
fwphd_path=~/workspace/fw-phd
ns3_path=~/workspace/ns-3-dev
ns3gym_path=~/workspace/ns3-gym
ns3ai_path=~/workspace/ns3-ai
ts_path=~/workspace/ts
rukutils_path=~/workspace/rukutils
ns3_version="ns-3.38" #supporting ns-3.38
ns3gym_branch="app-ns-3.36+" #"app-ns-3.36+" for ns-3.36++
ns3ai_branch="cmake"

#https://stackoverflow.com/questions/5947742/how-to-change-the-output-color-of-echo-in-linux
noc='\033[0m'       # No Colour - Text Reset
red='\033[0;31m' #red colour
yel='\033[0;33m' #yellow colour

###Script Functions###

function help()
{
	echo "Script Usage:"
	echo "  -i   Install: Installs the required dependencies, creates symbolic links, configures and builds ns-3, installs the ns3-gym python library, and installs the rukutils python module."
	echo "  -r   Generate Requirements: Generates a requirements file for the current conda environment"
	echo "  -h   Help documentation: Displays this help documentation"
	echo ""
	echo "Script Variables:"
	echo "  fwphd_path    : Path to fw-phd directory"
	echo "  ns3_path      : Path to ns-3-dev directory"
	echo "  ns3gym_path   : Path to ns3-gym directory"
	echo "  ns3ai_path   : Path to ns3-ai directory"
	echo "  ts_path       : Path to ts directory"
	echo "  rukutils_path : Path to rukutils directory"
	echo "  ns3_version   : Supporting ns-3 version (ns-3.37)"
	echo "  ns3gym_branch : Supporting ns3-gym branch (app-ns-3.36+)"
	echo ""
	echo "Note: The script requires an active conda environment and assumes the existence of the above directories."
}

function install()
{
	echo "Installing..."
	apt_pkg_install
	req_check

	echo "Discarding unstaged changes in ns-3 git repo..."
	cd $ns3_path
	git restore .
	git clean -fdx
	echo "Discarding unstaged changes in ns-3 git repo...Ok!"
	
	cp $fwphd_path/install/missing-builder-file.py $CONDA_PREFIX/lib/python3.8/site-packages/google/protobuf/internal/builder.py	
	#https://stackoverflow.com/questions/71759248/importerror-cannot-import-name-builder-from-google-protobuf-internal
	
	echo "Creating the symbolic links..." 
	ln -s $ns3gym_path $ns3_path/contrib/opengym
	ln -s $ts_path $ns3_path/contrib/ts
	ln -s $ns3ai_path $ns3_path/contrib/ns3-ai
	ln -s $fwphd_path $ns3_path/scratch/fw-phd
	ln -s $fwphd_path/lib/link-adaptation/my-wifi-manager.cc $fwphd_path/lib/link-adaptation/my-wifi-manager.h $ns3_path/src/wifi/model/rate-control/
	ln -s $fwphd_path/lib/link-adaptation/lupo-wifi-manager.cc $fwphd_path/lib/link-adaptation/lupo-wifi-manager.h $ns3_path/src/wifi/model/rate-control/
	echo "Creating the symbolic links...Ok!" 
	
	echo "ns3 configure and build..."
	cp $fwphd_path/lib/link-adaptation/ns-3.38-CMakeLists.txt $ns3_path/src/wifi/CMakeLists.txt
	#$ns3_path/ns3 clean
	$ns3_path/ns3 configure
	$ns3_path/ns3
	echo "ns3 configure and build...Ok!"

	echo "Installing required python lib and modules..."
	pip install $ns3gym_path/model/ns3gym
	pip install $ns3ai_path/py_interface
	pip install $rukutils_path	
	echo "Installing required python lib and modules...Ok!"

	$ns3_path/ns3
	
	echo "Copying missing files..."
	cp -i $fwphd_path/config/examples/baseline_config.json $fwphd_path/config/config.json #-i to prevent overwriting by accident
	#warning python3.8 or python3.9 in path below 
	echo "Copying missing files...Ok!"
	
	echo "Installing SOLVER dependencies..."
	conda install --channel conda-forge pyscipopt
	echo "Installing SOLVER dependencies...Ok!"

	echo "Finished Installation... exiting..."
	exit 0;
}

function gen_reqs()
{
	echo "Generating requirements file for the $CONDA_DEFAULT_ENV conda environment..."
	echo "Reminder: excluding modules that need to be installed after ns3 configure."
	pip freeze --exclude ns3gym --exclude certifi --exclude rukutils --exclude ns3-ai > install/requirements.txt
}
function usage()
{
  echo " Usage: $0 [-i Install][-r Generate Requirements][-h Help documentation] "
}
function apt_pkg_install()
{
	echo "Installing required dependencies..."
	sudo apt-get update && sudo apt-get install -y gcc g++ gdb libzmq5 libzmq3-dev pkg-config protobuf-compiler cmake jq zlib1g-dev
	echo "Installing required dependencies...Ok!"
}
function req_check()
{
	echo "Checking if active conda environment has properly installed every dependency..."
	result=$(diff -u $fwphd_path/install/requirements.txt <(pip freeze --exclude ns3gym --exclude certifi --exclude PySCIPOpt --exclude rukutils --exclude ns3-ai ))
	if [ -n "$result" ];
	then
		echo -e "${red}ERROR: Active conda environment does not satisfy every dependency.${noc}"
		echo "Differences to the requirements.txt file:"
		echo "$result"
		echo -e "${yel}Delete extra dependencies (+ <dep_name>): pip uninstall <dep_name>${noc}"
		echo -e "${yel}Do (with conda environment activated): pip install -r install/requirements.txt${noc}"
		exit 1; 	
	fi
	echo "Checking if active conda environment has properly installed every dependency...Ok!"

	echo "Checking if Directory modules are properly cloned..."
	if [ ! -d $ns3gym_path ] || [ ! -d $ts_path ] || [ ! -d $rukutils_path ] || [ ! -d $ns3ai_path ];
	then
		echo -e "${red}ERROR: One or more modules not found.${noc}"
		echo -e "${yel}Expected paths: $ns3gym_path , $ts_path, $rukutils_path, $ns3ai_path. exiting...${noc}"
	   	exit 1; 
	fi
	echo "Checking if Directory modules are properly cloned...Ok!"
	
	echo "Checking if the ns3-gym git repo is at the correct branch..."
	if [ "$(git --git-dir $ns3gym_path/.git branch --show-current)" != "$ns3gym_branch" ];
	then
		echo -e "${red}ERROR: The ns3-gym git repo is not at $ns3gym_branch branch.${noc}"	
		echo -e "${yel}Do (inside ns3-gym git folder): git checkout $ns3gym_branch.${noc}"
		exit 1;
	fi
	echo "Checking if the ns3-gym git repo is at the correct branch...Ok!"
	
	echo "Checking if the ns3-ai git repo is at the correct branch..."
	if [ "$(git --git-dir $ns3ai_path/.git branch --show-current)" != "$ns3ai_branch" ];
	then
		echo -e "${red}ERROR: The ns3-ai git repo is not at $ns3ai_branch branch.${noc}"	
		echo -e "${yel}Do (inside ns3-ai git folder): git checkout $ns3ai_branch.${noc}"
		exit 1;
	fi
	echo "Checking if the ns3-ai git repo is at the correct branch...Ok!"
	
	echo "Checking if the ns-3 git repo is at the supported tag version..."
	if [ "$(git --git-dir $ns3_path/.git describe --tags)" != "$ns3_version" ];
	then
		echo -e "${red}ERROR: The ns-3 git repo is not at $ns3_version version.${noc}"	
		echo -e "${yel}Do (inside ns-3 git folder): git checkout tags/$ns3_version.${noc}"
		exit 1;
	fi
	echo "Checking if the ns-3 git repo is at the supported tag version...Ok!"
}
### Command Line Interface ###
while getopts hir flag ;
do
	case "$flag" in
		h) help && exit 0;;	
		i) install && exit 0;; 
		r) gen_reqs && exit 0;;
		*) usage && exit 1;;	
	esac
done
echo "Nothing happened..." && usage
exit 0;

