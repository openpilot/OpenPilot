function [q gyro accel time] = analyzeINSGPS(fn)
% Analyzes data collected from SerialLogger while DUMP_FRIENDLY
% enabled in AHRS
%
% [q gyro accel time] = analyzeINSGPS(fn)

fid = fopen(fn);

i = 1;
data(i).block = -1;
tline = fgetl(fid);
while ischar(tline)
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

h(1) = subplot(311);
plot(time,q)
ylabel('Quaternion');
h(2) = subplot(312);
plot(time,accel);
ylabel('m/s')
h(3) = subplot(313);
plot(time,gyro);
ylabel('rad/sec');
xlabel('Time (s)')

linkaxes(h,'x');