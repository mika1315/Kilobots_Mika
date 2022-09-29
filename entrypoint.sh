#!/bin/bash
set -e

uid=$1 #${1:-1000}
executionMode=$2 #${2:-"normal"}
configFile=$3
shift; shift; shift;

useradd -d /home/user -Ms /bin/bash -u $uid user
chown -R $uid /home/user

# Launch openssh server
#/etc/init.d/ssh start

# Launch xorg with XDummy driver
/usr/bin/Xorg -noreset +extension GLX +extension RANDR +extension RENDER -logfile ./xdummy.log -config /etc/X11/xorg.conf :1 &

sleep 3

# Launch illumination
if [ "$executionMode" = "normal" ]; then
    gosu user bash -c "cd /home/user/Kilobots_Mika; ln -s /home/user/results results; ./runKilombo.py -o results -c conf/$configFile; rsync -avz /home/user/results/ /home/user/finalresults/"
fi

kill %1

# MODELINE	"{{{1
# vim:expandtab:softtabstop=4:shiftwidth=4:fileencoding=utf-8
# vim:foldmethod=marker
