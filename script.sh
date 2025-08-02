#!/bin/bash

for ((i=1; i<=50; i++))
do
    echo "$i"
    python start_script.py --raAlg id --simSeed $i --dir tid$i
    python start_script.py --raAlg min --simSeed $i --dir tmin$i
    python start_script.py --raAlg lupo --simSeed $i --dir tlupo$i

done




