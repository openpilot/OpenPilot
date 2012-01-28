fn = '~/Documents/Programming/serial_logger/bma180_l3gd20_desk_20120228.dat';
fn = '~/Documents/Programming/serial_logger/mpu6000_desk_20120228.dat';
fn = '~/Documents/Programming/serial_logger/mpu6000_desk2_20110228.dat';
fn = '~/Documents/Programming/serial_logger/output.dat';
fid = fopen(fn);
dat = uint8(fread(fid,'uint8'));
fclose(fid);

accel = zeros(4096,1);
accel_idx = 0;

gyro = zeros(4096,1);
gyro_idx = 0;

mag = zeros(4096,1);
mag_idx = 0;

latitude = zeros(4096,1);
longitude = zeros(4096,1);
altitude = zeros(4096,1);
heading = zeros(4096,1);
groundspeed = zeros(4096,1);
gps_satellites = zeros(4096,1);
gps_time = zeros(4096,1);
gps_idx = 0;

baro = zeros(4096,1);
baro_idx = 0;

total = length(dat);
count = 0;
head = 0;
last_time = 0;
good_samples = 0;
bad_samples = 0;
while head < (length(dat) - 200)
    
    last_head = head;
    
    count = count + 1;
    if count >= 5000
        disp(sprintf('Processed: %0.3g%% Bad: %0.3g%%',(head/total) * 100,bad_samples * 100 / (bad_samples + good_samples)));
        count = 0;
    end
    idx = find(dat(head+1:head+100)==255,1,'first');
    if isempty(idx)
        head = head + 100;
        continue;
    end
    head = head + idx;
    
    
    % Get the time
    time = double(dat(head+1))* 256 + double(dat(head+2));%typecast(flipud(dat(head+(1:2))),'uint16');
    if min([(time - last_time) (last_time - time)]) > 2
        disp(['Err' num2str(time-last_time)]);
        last_time = time;
        bad_samples = bad_samples + (head - last_head);
        continue
    end
    last_time = time;

    head = head + 2;
    
    % Get the accels
    accel_idx = accel_idx + 1;
    if accel_idx > size(accel,1)
        accel(accel_idx * 2,:) = 0;
    end
    accel(accel_idx,1:3) = typecast(dat(head+(1:12)),'single');
    head = head + 12;
    accel(accel_idx,4) = time;
    
    % Get the gyros
    gyro_idx = gyro_idx + 1;
    if gyro_idx > size(gyro,1);
        gyro(gyro_idx * 2,:) = 0;
    end
    gyro(gyro_idx,1:4) = typecast(dat(head+(1:16)),'single');
    head = head + 16;
    gyro(gyro_idx,5) = time;
    
    if dat(head+1) == 1 % Process the mag data
        head = head+1;
        mag_idx = mag_idx + 1;
        if mag_idx > size(mag,1)
            mag(mag_idx * 2, :) = 0;
        end
        mag(mag_idx,1:3) = typecast(dat(head + (1:12)),'single');
        head = head+12;
        mag(mag_idx,4) = time;
    end
    
    if dat(head+1) == 2 % Process the GPS data
        % typedef struct {
        %     int32_t Latitude;
        %     int32_t Longitude;
        %     float Altitude;
        %     float GeoidSeparation;
        %     float Heading;
        %     float Groundspeed;
        %     float PDOP;
        %     float HDOP;
        %     float VDOP;
        %     uint8_t Status;
        %     int8_t Satellites;
        % } __attribute__((packed)) GPSPositionData;
        
        head = head+1;
        
        gps_idx = gps_idx + 1;
        if gps_idx > length(latitude)
            latitude(gps_idx * 2) = 0;
            longitude(gps_idx * 2) = 0;
            altitude(gps_idx * 2) = 0;
            heading(gps_idx * 2) = 0;
            groundspeed(gps_idx * 2) = 0;
            gps_satellites(gps_idx * 2) = 0;
            gps_time(gps_idx * 2) = 0;
        end
        
        latitude(gps_idx) = double(typecast(dat(head+(1:4)),'int32')) / 1e7;
        longitude(gps_idx) = double(typecast(dat(head+(5:8)),'int32')) / 1e7;
        altitude(gps_idx) = typecast(dat(head+(9:12)),'single');
        heading(gps_idx) = typecast(dat(head+(17:20)),'single');
        groundspeed(gps_idx) = typecast(dat(head+(21:24)),'single');
        gps_satelites(gps_idx) = dat(head+38);
        gps_time(gps_idx) = time;
        head = head + 9 * 4 + 2;
    end
    
    if dat(head+1) == 3 % Process the baro data
        head = head + 1;
        baro_idx = baro_idx + 1;
        if baro_idx  > size(baro,1)
            baro(baro_idx * 2,:) = 0;
        end
        baro(baro_idx,1) = typecast(dat(head+(1:4)),'single');
        baro(baro_idx,2) = time;
    end
    
    good_samples = good_samples + (head - last_head);
end

accel(accel_idx+1:end,:) = [];
gyro(gyro_idx+1:end,:) = [];
mag(mag_idx+1:end) = [];
latitude(gps_idx+1:end) = [];
longitude(gps_idx+1:end) = [];
altitude(gps_idx+1:end) = [];
groundspeed(gps_idx+1:end) = [];
gps_satellites(gps_idx+1:end) = [];
gps_time(gps_idx+1:end) = [];
baro(baro_idx+1:end,:) = [];
