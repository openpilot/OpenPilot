#!/bin/bash

# This script starts montecarlo runs on different machines.
# Modify the machines they should be started on and the number of runs here.
# Modify the montecarlo_ssh_loop.sh script to setup the SLAM options
# (the version which will be used on the distant machines!).
#
# Author: croussil


MACHINES="\
	bugsbunny bugsbunny bugsbunny bugsbunny bugsbunny bugsbunny bugsbunny bugsbunny \
	finot finot finot finot finot finot finot finot \
	sioux sioux sioux sioux sioux sioux sioux sioux \
	valse valse valse valse \
	jive jive jive jive \
	dummy dummy dummy dummy \
	flex flex flex flex \
	topaze topaze topaze topaze \
	germanium germanium germanium germanium \
	arsenic arsenic arsenic arsenic \
	chrome chrome chrome chrome \
	bore bore bore bore \
	calcium calcium calcium calcium \
	gallium gallium gallium gallium \
	silicium silicium silicium silicium \
	azote azote azote azote \
	jaune jaune jaune jaune \
	poussin poussin poussin poussin \
	ocre ocre ocre ocre \
	safran safran safran safran \
	blond blond blond blond\
"
#	cobalt cobalt cobalt cobalt \
#	nickel nickel nickel nickel \
#	antimoine antimoine antimoine antimoine \

RUNPATH=/home/croussil/Libs/jafar/build_release/modules/rtslam
NUM_FILE=nees.log
MAX_NUM=100

if [[ "$1" -ne "0" ]]; then
	# START

	for machine in $MACHINES; do
		ssh -4 -t $machine "cd $RUNPATH; screen -S rtslam -d -m data/scripts/montecarlo_loop.sh $NUM_FILE $MAX_NUM"
		sleep 1
	done;

else
	# STOP

	for machine in $MACHINES; do
		ssh -4 $machine "cd $RUNPATH; lockfile -1 $NUM_FILE.lock; echo $(($MAX_NUM+1)) > $NUM_FILE; rm -f $NUM_FILE.lock"
		break
	done;

	for machine in $(echo $MACHINES | sed "s/ /\n/g" | uniq); do
		echo "Stopping $machine"
		ssh -4 $machine "killall -9 demo_slam; killall -9 screen; screen -wipe"
	done;

	for machine in $MACHINES; do
		ssh -4 $machine "cd $RUNPATH; rm -f $NUM_FILE"
		break
	done;

fi;
