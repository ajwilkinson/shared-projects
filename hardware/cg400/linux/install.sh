#!/bin/sh
module="siplx"
card_num=0

# DRIVER
cd siplx
make clean
make

# Get the VID
SI_VID=0x$(sudo /usr/bin/lspci -nn | sed '/PLX/!d;   s/.*\[//;   s/:.*//')
echo SI_VID = $SI_VID

# Get the DID
SI_DID=0x$(sudo /usr/bin/lspci -nn | sed '/PLX/!d;s/.*://;s/].*//')
echo SI_DID = $SI_DID

sudo /sbin/insmod siplx.ko SI_DID=$SI_DID SI_VID=$SI_VID
major=`(awk "\\$2==\"$module\" {print \\$1}" /proc/devices)`
sudo mknod /dev/siplx0 c $major 0

sudo chmod 666 /dev/siplx*

cd ../setfrequency/
make clean
make
./setfrequency 200.0

# END

