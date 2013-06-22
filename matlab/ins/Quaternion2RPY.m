function [rpy p y] = Quaternion2RPY(q)
% [rpy p y] = Quaternion2RPY(q)
%
% JC 2012-02-26

assert(size(q,2) == 4, 'Quaternion wrong shape');
RAD2DEG = 180 / pi;

q0s = q(:,1) .^ 2;
q1s = q(:,2) .^ 2;
q2s = q(:,3) .^ 2;
q3s = q(:,4) .^ 2;

R13 = 2.0 * (q(:,2) .* q(:,4) - q(:,1) .* q(:,3));
R11 = q0s + q1s - q2s - q3s;
R12 = 2.0 * (q(:,2) .* q(:,3) + q(:,1) .* q(:,4));
R23 = 2.0 * (q(:,3) .* q(:,4) + q(:,1) .* q(:,2));
R33 = q0s - q1s - q2s + q3s;

rpy(:,2) = RAD2DEG * asin(-R13);
rpy(:,3) = RAD2DEG * atan2(R12, R11);
rpy(:,1) = RAD2DEG * atan2(R23, R33);

if nargout > 1
    p = rpy(:,2);
    y = rpy(:,3);
    rpy(:,2:3) = [];
end