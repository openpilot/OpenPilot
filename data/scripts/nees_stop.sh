#!/bin/bash

MACHINES="sioux finot bugsbunny valse jive dummy topaze flex germanium arsenic chrome calcium gallium silicium azote antimoine jaune poussin ocre safran blond"

NUM_FILE=nees.log
RUNPATH=/home/croussil/Libs/jafar/build_release/modules/rtslam

ssh -4 verdaguer "cd $RUNPATH; echo 1000 > $NUM_FILE"

for machine in $MACHINES; do
	echo $machine
	ssh $machine "killall -9 demo_slam; screen -wipe"
done

ssh -4 verdaguer "cd $RUNPATH; rm -f $NUM_FILE"
