#!/bin/bash

simType=$1
argdir=$2
now=$(date +%Y%m%d%H%M%S)
wd=$(pwd)

if [[ $simType == "eval" ]];
then
	dirprefix="e";
elif [[ $simType == "train" ]];
then
	dirprefix="t";
else
	echo "Error: Valid simTypes are <eval> and <train>. Not <$simType>.";
	exit 1;
fi

if [[ -z "$argdir" ]];
then
	rundir="$wd/results/${dirprefix}${now}"
else
	rundir="$wd/results/$argdir"
fi

if [[ ! -d "$rundir" ]];
then
	mkdir $rundir
fi

cp $wd/config/config.json $rundir

echo "$rundir" | tr -d '\n'
exit 0;
