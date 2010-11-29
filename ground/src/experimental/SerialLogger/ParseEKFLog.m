function ParseEKFLog(fn)

fid = fopen(fn);
dat = uint8(fread(fid,inf,'uchar'));
fclose(fid);

mag_data_size = 1 + 4*3;
accel_sensor_size = 4*3;
gyro_sensor_size = 4*3;
raw_framing = 15:-1:0;

starts = strfind(char(dat'),char(raw_framing));
starts(end) = [];

counts = typecast(reshape(dat(bsxfun(@plus,starts,(16:19)')),1,[]),'int32');
accel = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(20:31)')),1,[]),'single'),3,[])';
gyro = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(32:43)')),1,[]),'single'),3,[])';
mag_updated = dat(starts+44);
mag = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(45:56)')),1,[]),'single'),3,[])';

%gps = 28 bytes 57:84
NED = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(57:68)')),1,[]),'single'),3,[])';
heading = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(69:72)')),1,[]),'single'),1,[])';
groundspeed = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(73:76)')),1,[]),'single'),1,[])';
quality = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(77:80)')),1,[]),'single'),1,[])';
updated = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(81:84)')),1,[]),'int32'),1,[])';

X = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(85:85+13*4-1)')),1,[]),'single'),13,[])';
P = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(137:137+13*4-1)')),1,[]),'single'),13,[])';

Baro = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(189:192)')),1,[]),'single'),1,[])';
BaroOffset = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(193:196)')),1,[]),'single'),1,[])';

N=length(accel);
rpy = zeros(3,N);
Vne = zeros(2,N);
dt = 0.01;
t=0:dt:(N-1)*dt;
for i = 1:N
    rpy(:,i) = q2rpy(X(i,7:10))*180/pi;
    groundTrack = heading(i)*pi/180;
    Vne(:,i) = groundspeed(i)*[cos(groundTrack); sin(groundTrack)];
end

figure(1); plot(t,rpy(1,:),t,rpy(2,:),t,rpy(3,:));
title('roll-pitch-yaw');
figure(2); plot(t,NED(:,1),t,NED(:,2),t,NED(:,3),t,X(:,1),t,X(:,2),t,X(:,3),t,BaroOffset-Baro);
title('gpsNED');ylim([-500 500]);
figure(3); plot(t,Vne(1,:),t,Vne(2,:),t,X(:,4),t,X(:,5));
title('gpsVne');ylim([-3 3]);
figure(4); plot(t,accel(:,1),t,accel(:,2),t,accel(:,3));
title('accels');ylim([-10 10]);
figure(5); plot(t,gyro(:,1),t,gyro(:,2),t,gyro(:,3));
title('gyros'); ylim([-pi pi]);
figure(6); plot(t,mag(:,1),t,mag(:,2),t,mag(:,3));
title('mags'); ylim([-1200 1200]);
figure(7); plot(t,Baro,t,BaroOffset,t,BaroOffset-NED(:,3));
title('BaroAlt'); ylim([300 410]);
figure(8); plot(t,X(:,11),t,X(:,12),t,X(:,13));
title('GyroBias'); ylim([-1 1]);

Be = [20595  1363  49068]';
ScaledBe = [Be(1)  Be(2) Be(3)]'/Be(3);
ScaledBe = [ScaledBe ScaledBe];
ScaledMag = zeros(3,N);
MagMag = zeros(1,N);
for i = 1:N
    ScaledMag(:,i) = mag(i,:)'/mag(i,3);
    MagMag(i) = norm(mag(i,:));
end
t2 = [t(1) t(end)];
figure(13); plot(t,ScaledMag(1,:),t,ScaledMag(2,:),t,ScaledMag(3,:),t2,ScaledBe(1,:),t2,ScaledBe(2,:),t2,ScaledBe(3,:));
title('scaledMags'); ylim([-1.01 1.01]);
figure(14); plot(t,MagMag);
title('Mag Magnitude'); ylim([500 1500]);


keyboard
