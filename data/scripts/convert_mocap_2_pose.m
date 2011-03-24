#!/usr/bin/octave -qf

% This script converts a set of 3D trajectories of 4 markers with the pattern
% Left/Right/Back/Top into a 6D trajectory. The pose of the object is defined
% such that :
% - the axis x of the object is the normalized vector BA with A being on the
%   segment LR, such that AR/LR=alpha, alpha being provided by the user.
% - the axis z of the object is normal to the plane LRB.
% - the axis y is such that (x,y,z) is a direct frame.
% - the origin O is the projection of T on the plane LRB.
% Top should be the closest possible to the real center of your object.
% It returns:
% - time (1-1)
% - position (2-4)
% - rotation matrix (5-13)
% - quaternion (14-17)
% - euler roll/pitch/yaw (18-20)
%
% Author: croussil

% config constants

time_col = 2;
left_col = 3;
right_col = 6;
back_col = 9;
top_col = 12;
alpha=0.462;

% init
if (nargin != 2)
	printf("usage: ./convert <mocap-file.trc> <pose-file.dat>");
endif
args = argv();
trajs=load("-ascii", args{1});

poses=ones(size(trajs)(1), 13);

% some conversion functions from jsola slamToolbox

function q = R2q(R)
	T = trace(R) + 1;
	if isa(T,'sym') || ( T > 0.00000001 )
		S = 2 * sqrt(T);
		a = 0.25 * S;
		b = ( R(2,3) - R(3,2) ) / S;
		c = ( R(3,1) - R(1,3) ) / S;
		d = ( R(1,2) - R(2,1) ) / S;
	else
		if ( R(1,1) > R(2,2) && R(1,1) > R(3,3) ) 
			S  = 2 * sqrt( 1.0 + R(1,1) - R(2,2) - R(3,3) );
			a = (R(2,3) - R(3,2) ) / S;
			b = 0.25 * S;
			c = (R(1,2) + R(2,1) ) / S;
			d = (R(3,1) + R(1,3) ) / S;
		elseif ( R(2,2) > R(3,3) )               
			S  = 2 * sqrt( 1.0 + R(2,2) - R(1,1) - R(3,3) );
			a = (R(3,1) - R(1,3) ) / S;
			b = (R(1,2) + R(2,1) ) / S;
			c = 0.25 * S;
			d = (R(2,3) + R(3,2) ) / S;
		else
			S  = 2 * sqrt( 1.0 + R(3,3) - R(1,1) - R(2,2) );
			a = (R(1,2) - R(2,1) ) / S;
			b = (R(3,1) + R(1,3) ) / S;
			c = (R(2,3) + R(3,2) ) / S;
			d = 0.25 * S;
		end
	end

	q = [a -b -c -d]';
endfunction

function e = R2e(R)
	s = whos('R');
	if (strcmp(s.class,'sym'))
		roll  = atan(R(3,2)/R(3,3));
		pitch = asin(-R(3,1));
		yaw   = atan(R(2,1)/R(1,1));
	else
		roll  = atan2(R(3,2),R(3,3));
		pitch = asin(-R(3,1));
		yaw   = atan2(R(2,1),R(1,1));
	end
	e = [roll;pitch;yaw];
endfunction

% start the conversion

for i = 1:size(trajs)(1)

	# read values
	t=trajs(i,time_col);
	L=[trajs(i,left_col)  trajs(i,left_col+1)  trajs(i,left_col+2)]';
	R=[trajs(i,right_col) trajs(i,right_col+1) trajs(i,right_col+2)]';
	B=[trajs(i,back_col)  trajs(i,back_col+1)  trajs(i,back_col+2)]';
	T=[trajs(i,top_col)   trajs(i,top_col+1)   trajs(i,top_col+2)]';

	# compute A
	A=alpha*L + (1-alpha)*R;

	# compute frame x,y,z
	x = (A-B);           x = x/norm(x);
	z = cross(R-B, L-B); z = z/norm(z);
	y = cross(z,x);

	# compute origin O (k is distance of T to plane)
	# plane equation ax+by+cz+d=0. a=z(0) b=z(1) c=z(2) d=-inner_prod(z,B)
	k = dot(z,B) - dot(z,T);
	O = k*z + T;

	# final transformation matrix, rotation + translation
	poses(i,1 )=t;

	poses(i,2 )=O(1)/1000; poses(i,3)=O(2)/1000; poses(i,4)=O(3)/1000;

	poses(i,5 )=x(1); poses(i,6 )=y(1); poses(i,7 )=z(1); 
	poses(i,8 )=x(2); poses(i,9 )=y(2); poses(i,10)=z(2);
	poses(i,11)=x(3); poses(i,12)=y(3); poses(i,13)=z(3);

	q = R2q([x y z]);
	poses(i,14)=q(1); poses(i,15)=q(2); poses(i,16)=q(3); poses(i,17)=q(4); 
	e = R2e([x y z]);
	poses(i,18)=e(1); poses(i,19)=e(2); poses(i,20)=e(3);

endfor

% save the result

save("-ascii", args{2}, "poses");

