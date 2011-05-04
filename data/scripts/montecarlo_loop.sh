#!/bin/bash

# This script executes montecarlo runs on a single machine.
# Modify this file to setup the number of runs you want in total
# and the SLAM options.
# 
# WARNING: Modify the version of this file that is actually used by the
# machines running SLAM!
#
# Author: croussil


NUM_FILE=$1
MAX_NUM=$2
COMMAND="demo_suite/x86_64-linux-gnu/demo_slam"
DATA_PATH="/net/tic/data1/robots/camera/2011-02-16_motion-capture/01/"
ARGS=" \
	--disp-2d=0 --disp-3d=0 --replay=1 --dump=0 --pause=0 --render-all=0 \
	--rand-seed=0 --simu=0 --robot=1 --map=1 \
	--config-estimation=@/estimation.cfg --data-path=$DATA_PATH"
LOG_PREFIX=results

mkdir -p $DATA_PATH/$LOG_PREFIX

num=0

update_num () {
	lockfile -1 $NUM_FILE.lock

	[ -e $NUM_FILE ] && num=`cat $NUM_FILE`
	next_num=$(($num+1))
	echo $next_num > $NUM_FILE

	rm -f $NUM_FILE.lock
}

update_num

while [ "$num" -lt "$MAX_NUM" ]; do
	$COMMAND $ARGS --log=$LOG_PREFIX/rtslam_`printf "%03d" $num`.log
	update_num
done
