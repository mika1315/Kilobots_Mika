#!/bin/sh

sshKeyFile=${1:-~/.ssh/docker_deploy_id_rsa}
imageName=${2:-kilobots-mika:latest}
if [ ! -f $sshKeyFile ]; then
	echo Please specify the private SSH key file used to access github git repositories
	read sshKeyFile
fi
echo Using SSH key \'$sshKeyFile\'...

inDockerGroup=`id -Gn | grep docker`
if [ -z "$inDockerGroup" ]; then
	sudoCMD="sudo"
else
	sudoCMD=""
fi

$sudoCMD docker build --no-cache --network host -t $imageName . --build-arg SSH_PRIVATE_KEY="`cat $sshKeyFile`"

