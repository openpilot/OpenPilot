#!/bin/bash

NUM_FILE=nees.log
MAX_NUM=100
COMMAND="demo_suite/x86_64-linux-gnu/demo_slam"
DATA_PATH="/net/tic/data1/robots/camera/2011-02-16_motion-capture/01/"
ARGS=" \
	--disp-2d=0 --disp-3d=0 --replay=1 --dump=0 --pause=0 --render-all=0 \
	--rand-seed=0 --simu=0 --robot=1 --map=1 \
	--config-estimation=@/estimation.cfg --data-path=$DATA_PATH"
LOG_PREFIX=results

#mkdir -p $DATA_PATH/$LOG_PREFIX

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
#	echo "nice -n 15 $COMMAND $ARGS --log=$LOG_PREFIX/rtslam_`printf '%03d' $num`.log"
#	nice -n 15 $COMMAND $ARGS --log=$LOG_PREFIX/rtslam_`printf "%03d" $num`.log
	$COMMAND $ARGS --log=$LOG_PREFIX/rtslam_`printf "%03d" $num`.log
	update_num
done
