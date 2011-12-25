fn = '~/Desktop/kenn_stationary (2011-12-26 14:51:11 +0000)';
fid = fopen(fn);
dat = uint8(fread(fid,'uint8'));
fclose(fid);

accel = [];
accel_idx = 0;

gyro = [];
gyro_idx = 0;

mag = [];
mag_idx = 0;

latitude = [];
longitude = [];
altitude = [];
heading = [];
groundspeed = [];
gps_satellites = [];
gps_time = [];
gps_idx = 0;

baro = [];
baro_idx = 0;

total = length(dat);
count = 0;
head = 0;
last_time = 0;
while head < (length(dat) - 200)
    
    count = count + 1;
    if count >= 100
        disp(sprintf('%0.3g%%',(head/total) * 100));
        count = 0;
    end
    head = head + find(dat(head+1:end)==255,1,'first');
    
    % Get the time
    time = typecast(flipud(dat(head+(1:2))),'uint16');
    if (time - last_time) ~= 2 && (time-last_time) ~= (hex2dec('10000')-2)
        last_time = time;
        disp('Err');
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
end

accel(accel_idx+1:end,:) = [];
gyro(gyro_idx+1:end,:) = [];
mag(mag_idx+1:end) = [];
latitude(gps_idx+1:end) = [];
longitude(gps_idx+1:end) = [];
altitude(gps_idx+1:end) = [];
groundspeed(gps_idx+1:end) = [];
gps_satelites(gps_idx+1:end) = [];
gps_time(gps_idx+1:end) = [];
baro(baro_idx+1:end,:) = [];
