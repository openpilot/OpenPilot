#!/usr/bin/octave -qf

# This script optimizes accelero biases and gain errors of an IMU
# The input file must contain on each line the values of acceleration measured
# with the IMU static with some orientation, and the approximate value of this
# orientation (better if accelerations are averaged over some time) :
# roll(deg)	pitch(deg)	ax	ay	az
#
# Author: croussil


function R = e2R(e)

	r  = e(1); %roll
	p  = e(2); %pitch
	y  = 0; %yaw

	sr = sin(r);
	cr = cos(r);
	sp = sin(p);
	cp = cos(p);
	sy = sin(y);
	cy = cos(y);

	R= [cp*cy -cr*sy+sr*sp*cy  sr*sy+cr*sp*cy
			cp*sy  cr*cy+sr*sp*sy -sr*cy+cr*sp*sy
			-sp          sr*cp           cr*cp   ];

end

function J = costFunction(theta, y)

	J = 0;
	m = size(y, 1); % number of examples
	g = [0;0;9.806];
	ab = theta(1:3);
	as = theta(4:6);

	for i=1:m
		am = y(i,:)'; % acceleration measured
		oa = theta(5+i*2:6+i*2); % optimized attitude (roll,pitch)

		err = e2R(oa) * ((am ./ as) .- ab) - g;
		J = J + sum(err .* err);
	end

end

%init
if (nargin != 1)
	printf("usage: ./calib_inertial <measures.dat>");
end
args = argv();
measures=load("-ascii", args{1});


initial_theta(1:3) = zeros(3)(:,1);
initial_theta(4:6) = ones(3)(:,1);

m = size(measures, 1); % number of examples
for i=1:m
	initial_theta(5+i*2:6+i*2) = measures(i, 1:2); % initial attitude (roll,pitch)
end

options = optimset('MaxIter', 1000);

[theta, cost] = ...
	fminunc(@(t)(costFunction(t, measures(:,3:5))), initial_theta', options);

theta(1:6)
sqrt(cost/m)


theta(1:3) = zeros(3)(:,1);
theta(4:6) = ones(3)(:,1);
sqrt(costFunction(theta, measures(:,3:5))/m)
