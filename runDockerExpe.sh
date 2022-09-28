#!/bin/bash

configFile=${1:-"test.yaml"}
executionMode=${2:-"normal"}
memoryLimit=24G
#resultsPath=$(pwd)/results
resultsPathInContainer=/home/user/results
finalresultsPath=$(pwd)/results
finalresultsPathInContainer=/home/user/finalresults
#imageName=${3:-"localhost:5000/kilobots-mika"}
imageName=${3:-"kilobots-mika"}
uid=$(id -u)
confPath=$(pwd)/conf
confPathInContainer=/home/user/Kilobots_Mika/kilo/conf
priorityParam="-c 128"

if [ ! -d $finalresultsPath ]; then
    mkdir -p $finalresultsPath
fi

inDockerGroup=`id -Gn | grep docker`
if [ -z "$inDockerGroup" ]; then
    sudoCMD="sudo"
else
    sudoCMD=""
fi
dockerCMD="$sudoCMD docker"

if [ -d "$confPath" ]; then
    confVolParam="-v $confPath:$confPathInContainer"
else
    confVolParam=""
fi

if [ "$executionMode" = "normal" ]; then
    exec $dockerCMD run --privileged=true -it -m $memoryLimit --rm $priorityParam --mount type=tmpfs,tmpfs-size=8589934592,target=$resultsPathInContainer --mount type=bind,source=$finalresultsPath,target=$finalresultsPathInContainer  $confVolParam $imageName  "$uid" "normal" "$configFile"
fi

# MODELINE	"{{{1
# vim:expandtab:softtabstop=4:shiftwidth=4:fileencoding=utf-8
# vim:foldmethod=marker
