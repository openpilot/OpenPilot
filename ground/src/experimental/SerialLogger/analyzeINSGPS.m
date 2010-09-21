function [q gyro accel rpy time] = analyzeINSGPS(fn)
% Analyzes data collected from SerialLogger while DUMP_FRIENDLY
% enabled in AHRS
%
% [q gyro accel time] = analyzeINSGPS(fn)

fid = fopen(fn);

i = 1;
data(i).block = -1;
tline = fgetl(fid);
while ischar(tline) && ~isempty(tline)
    switch(tline(1))
        case 'q'
            c = textscan(tline,'q: %f %f %f %f');
            data(i).q = [c{:}] / 1000;
        case 'b'
            c = textscan(tline,'b: %f');
            i = i+1;
            data(i).block = c{1};
        case 'm'
            c = textscan(tline,'m: %f %f %f');
            data(i).mag = [c{:}];
        case 'a'
            c = textscan(tline,'a: %f %f %f %f');
            data(i).accel = [c{:}] / 1000;
        case 'g'
            c = textscan(tline,'g: %f %f %f %f');
            data(i).gyro = [c{:}] / 1000;
    end
    tline = fgetl(fid);
end

fclose(fid);

b = [data.block]; % get block counts
gaps = find(diff(b) ~= 1);
if(gaps) % get biggest contiguous chunk
    gaps = [1 gaps length(b)];
    lengths = diff(gaps);
    [foo idx] = max(lengths);
    idx = gaps(idx):gaps(idx+1);
    data = data(idx);
end

data(end) = []; % delete in case partial update

q = cat(1,data.q);
accel = cat(1,data.accel);
gyro = cat(1,data.gyro);
time = (1:size(q,1)) / 50;

rpy = Quaternion2RPY(q);

h(1) = subplot(411);
plot(time,q)
ylabel('Quaternion');
h(2) = subplot(412);
plot(time,rpy);
ylabel('RPY'); legend('Roll','Pitch','Yaw');
h(3) = subplot(413);
plot(time,accel);
ylabel('m/s')
h(4) = subplot(414);
plot(time,gyro);
ylabel('rad/sec');
xlabel('Time (s)')

linkaxes(h,'x');

function rpy = Quaternion2RPY(q)

RAD2DEG = 180/pi;

q = bsxfun(@rdivide,q,sqrt(sum(q.^2,2)));
qs = q .* q;

R13 = 2*(q(:,2).*q(:,4) -q(:,1).*q(:,3));     %2*(q[1]*q[3]-q[0]*q[2]);
R11 = qs(:,1) + qs(:,2) - qs(:,3) - qs(:,4);  %q0s+q1s-q2s-q3s;
R12 = 2*(q(:,2).*q(:,3) + q(:,1).*q(:,4));    %2*(q[1]*q[2]+q[0]*q[3]);
R23 = 2*(q(:,3).*q(:,4) + q(:,1) .* q(:,2));  %2*(q[2]*q[3]+q[0]*q[1]);
R33 = qs(:,1)-qs(:,2)-qs(:,3)+qs(:,4);        %q0s-q1s-q2s+q3s;

rpy(:,2)=RAD2DEG*asin(-R13);   % pitch always between -pi/2 to pi/2
rpy(:,3)=RAD2DEG*atan2(R12,R11);
rpy(:,1)=RAD2DEG*atan2(R23,R33);

