function q = RPY2Quaternion(rpy)

DEG2RAD = pi / 180;
phi = DEG2RAD * rpy(1) / 2;
theta = DEG2RAD * rpy(2) / 2;
psi = DEG2RAD * rpy(3) / 2;
cphi = cos(phi);
sphi = sin(phi);
ctheta = cos(theta);
stheta = sin(theta);
cpsi = cos(psi);
spsi = sin(psi);

q(1) = cphi * ctheta * cpsi + sphi * stheta * spsi;
q(2) = sphi * ctheta * cpsi - cphi * stheta * spsi;
q(3) = cphi * stheta * cpsi + sphi * ctheta * spsi;
q(4) = cphi * ctheta * spsi - sphi * stheta * cpsi;

if q(1) < 0
    q = -q;
end
