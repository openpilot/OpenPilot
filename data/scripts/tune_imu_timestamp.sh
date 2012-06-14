#!/bin/zsh

# parameters
demoslam=$JAFAR_DIR/build_release/modules/rtslam/demo_suite/x86_64-linux-gnu/demo_slam
configsetup=/mnt/data/LAAS/data/luma/setup.cfg
configestimation=/mnt/data/LAAS/data/luma/estimation.cfg
datapath=/mnt/data/LAAS/data/luma/run06

samples=100
max_cor=10000 # in us
step_cor=250

threads=4

# other variables, don't modify
#demooptions="-disp-2d=0 --disp-3d=0 --robot=1 --camera=1 --gps=0 --map=1 --render-all=0 --replay=1 --dump=0 --log=0 --heading=0 --rand-seed=0 --pause=0 --simu=0"

# go

cp $configsetup /tmp/setup.cfg
rm -f /tmp/thread_*.lock >& /dev/null

for ((i = 0; i <= $max_cor; i+=$step_cor)); do

	for ((k = 0; k < 2; k++)); do

		cor="0."`printf "%06d" $i`;
		if (($k == 1)); then
			if (($i == 0)); then
				break;
			else
				cor="-$cor";
			fi
		fi

		echo "### Running cor $cor"

		sed -i "s/^IMU_TIMESTAMP_CORRECTION:.*/IMU_TIMESTAMP_CORRECTION: $cor/" /tmp/setup.cfg

		sum=$((0.0))

		if (($threads == 1)); then

			for ((j = 0; j < $samples; j++)); do
				res=`$demoslam \
					--disp-2d=0 --disp-3d=0 --robot=1 --camera=1 --gps=0 --map=1 --render-all=0 --replay=1 --dump=0 --log=0 --heading=0 --rand-seed=0 --pause=0 --simu=0 \
					--data-path=$datapath --config-setup=/tmp/setup.cfg --config-estimation=$configestimation | grep average_robot_innovation | cut -d ' ' -f 2`
				echo "Run finishes : $res"
				echo "$cor $res" >> cor_all.dat
				sum=$(($sum+$res))
			done

		else

			t=0
			for ((j = 0; j < $(($samples+$threads)); j++)); do

				# wait that one has finished
				finished=0
				while (($finished == 0)); do
					t=$(($t+1))
					if (($t>=$threads)); then t=0; sleep 0.2; fi
					lockfile -0 -r1 /tmp/thread_$t.lock >& /dev/null && finished=1
				done

				# process its data
				if (($j >= $threads)); then
					res=`head -n 1 /tmp/thread_$t.dat | cut -d " " -f 1`
					echo "Thread $t finishes : $res"
					echo "$cor $res" >> cor_all.dat
					sum=$(($sum+$res))
				fi

				# start a new one
				if (($j < $samples)); then
					echo "Thread $t starts"
					sh -c "$demoslam \
		                -disp-2d=0 --disp-3d=0 --robot=1 --camera=1 --gps=0 --map=1 --render-all=0 --replay=1 --dump=0 --log=0 --heading=0 --rand-seed=0 --pause=0 --simu=0 \
		                --data-path=$datapath --config-setup=/tmp/setup.cfg --config-estimation=$configestimation | grep average_robot_innovation | cut -d ' ' -f 2 > /tmp/thread_$t.dat ; \
						rm -f /tmp/thread_$t.lock" &
				fi

			done

		fi

		sum=$(($sum / $samples))
		echo "$cor $sum" >> cor_avg.dat
		echo "Average $sum"
		rm -f /tmp/thread_*.lock >& /dev/null
	done

done






