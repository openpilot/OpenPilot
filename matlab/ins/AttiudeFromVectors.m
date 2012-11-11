function q = AttiudeFromVectors(mag,north,accel)
% q = AttiudeFromVectors(mag,north,accel)

ge = [0 0 -9.81]';
ge = ge / sqrt(ge' * ge);
mag = mag / sqrt(mag' * mag);
north = north / sqrt(north' * north);
accel = accel / sqrt(accel' * accel);

Rib(1,:) = mag;
Rib(2,:) = cross(mag,accel);
Rib(3,:) = cross(Rib(1,:),Rib(2,:));

Rie(1,:) = north;
Rie(2,:) = cross(north,ge);
Rie(3,:) = cross(Rie(1,:),Rie(2,:));

R = (Rib' * Rie)';

m = [1 + R(1,1) + R(2,2) + R(3,3); ...
    1 + R(1,1) - R(2,2) - R(3,3); ...
    1 - R(1,1) + R(2,2) - R(3,3); ...
    1 - R(1,1) - R(2,2) + R(3,3)];

[mag,idx] = max(m);
mag = 2 * sqrt(mag);

if (idx == 1)
    q = [mag/4; ...
        (R(2,3)-R(3,2))/mag; ...
    	(R(3,1)-R(1,3))/mag; ...
        (R(1,2)-R(2,1))/mag];
elseif (idx == 2)
    q(2) = mag/4;
    q(1) = (R(2,3)-R(3,2))/mag;
    q(3) = (R(1,2)+R(2,1))/mag;
    q(4) = (R(1,3)+R(3,1))/mag;
elseif (idx == 3)
    q(3) = mag/4;
    q(1) = (R(3,1)-R(1,3))/mag;
    q(2) = (R(1,2)+R(2,1))/mag;
    q(4) = (R(2,3)+R(3,2))/mag;
else
    q(4) = mag/4;
    q(1) = (R(1,2)-R(2,1))/mag;
    q(2) = (R(1,3)+R(3,1))/mag;
    q(3) = (R(2,3)+R(3,2))/mag;
end
    
if (q(1) < 0)
    q = -q;
end

q = reshape(q,[],1);