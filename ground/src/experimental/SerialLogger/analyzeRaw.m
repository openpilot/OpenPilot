function analyzeRaw(device)

downsample = 12;   % relevant for knowing block size
Fs = 512;          % need to verify, close

if ischar(device)
    s = serial(device,'Baud',115200);
    set(s,'InputBufferSize',10000);
    fopen(s);
    
    dat = [];
    for i = 1:20
        i
        if(i > 5)  % must flush buffer
            dat = [dat; uint8(fread(s))];
        end
    end
else 
    dat = uint8(device);
end

raw_framing = 0:15;
starts = strfind(char(dat'),char(raw_framing));
if(starts) % found raw data, process
    % warning - could occasionally crash if at very last 3 or less bytes
    counts = typecast(reshape(dat(bsxfun(@plus,starts,(16:19)')),1,[]),'int32');
    gaps = find(diff(counts) > 1) % exclude any discontiuous sections
    if(gaps)
        starts(1:gaps(end)) = [];
    end
    blocks = typecast(reshape(dat(bsxfun(@plus,starts(1:end-1),(20:20+downsample*8*2-1)')),1,[]),'int16');
    blocks = double(reshape(blocks,8,[])); 
    
    accel_y = 0.012*(blocks(1,:)-2048);
    accel_x = 0.012*(blocks(3,:)-2048);
    accel_z = 0.012*(blocks(5,:)-2048);
    gyro_x = 0.007*(blocks(2,:)-1675);
    gyro_y = 0.007*(blocks(4,:)-1675);
    gyro_z = 0.007*(blocks(6,:)-1675);

    time = (1:length(accel_x))/512;
    
    % display accels
    figure(1)
    subplot(321);
    plot(time,accel_x);
    xlim([0 1])
    ylim([mean(accel_x)-2*std(accel_x) mean(accel_x)+4*std(accel_x)]);
    title('Accel X');
    subplot(322);
    pwelch(accel_x-mean(accel_x),[],[],[],Fs);
    
    subplot(323);
    plot(time,accel_y); 
    xlim([0 1])
    ylim([mean(accel_y)-2*std(accel_y) mean(accel_y)+4*std(accel_y)]);
    title('Accel Y');
    subplot(324);
    pwelch(accel_y-mean(accel_y),[],[],[],Fs);

    subplot(325);
    plot(time,accel_z); 
    xlim([0 1])
    ylim([mean(accel_z)-2*std(accel_z) mean(accel_z)+4*std(accel_z)]);
    title('Accel Z');
    subplot(326);
    pwelch(accel_z-mean(accel_z),[],[],[],Fs);
    
    % display gyros
    figure(2)
    subplot(321);
    plot(time,gyro_x);
    xlim([0 1])
    ylim([mean(gyro_x)-2*std(gyro_x) mean(gyro_x)+4*std(gyro_x)]);
    title('Gyro X');
    subplot(322);
    pwelch(gyro_x-mean(gyro_x),[],[],[],Fs);
    
    subplot(323);
    plot(time,gyro_y); 
    xlim([0 1])
    ylim([mean(gyro_y)-2*std(gyro_y) mean(gyro_y)+4*std(gyro_y)]);
    title('Gyro Y');
    subplot(324);
    pwelch(gyro_y-mean(gyro_y),[],[],[],Fs);
    
    subplot(325);
    plot(time,gyro_z); 
    xlim([0 1])
    ylim([mean(gyro_z)-2*std(gyro_z) mean(gyro_z)+4*std(gyro_z)]);
    title('Gyro Z');
    subplot(326);
    pwelch(gyro_z-mean(gyro_z),[],[],[],Fs);
end
    
    
    
