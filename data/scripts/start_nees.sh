#!/bin/bash

MACHINES="\
	bugsbunny bugsbunny bugsbunny bugsbunny bugsbunny bugsbunny bugsbunny bugsbunny \
	finot finot finot finot finot finot finot finot \
	sioux sioux sioux sioux sioux sioux sioux sioux \
	valse valse valse valse \
	jive jive jive jive \
	dummy dummy dummy dummy \
	topaze topaze topaze topaze \
	flex flex flex flex \
	germanium germanium germanium germanium \
	arsenic arsenic arsenic arsenic \
	chrome chrome chrome chrome \
	calcium calcium calcium calcium \
	gallium gallium gallium gallium \
	silicium silicium silicium silicium \
	azote azote azote azote azote \
	antimoine antimoine antimoine antimoine \
	jaune jaune jaune jaune \
	poussin poussin poussin poussin \
	ocre ocre ocre ocre \
	safran safran safran safran \
	blond blond blond blond\
"

RUNPATH=/home/croussil/Libs/jafar/build_release/modules/rtslam

for machine in $MACHINES; do
	ssh -t $machine "cd $RUNPATH; screen -S rtslam -t rtslam -d -m bash data/scripts/run_loop.sh"
	sleep 1
done
