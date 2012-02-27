Gyros.timestamp = medfilt1(Gyros.timestamp,3);
Accels.timestamp = medfilt1(Accels.timestamp,3);
Magnetometer.timestamp = medfilt1(Magnetometer.timestamp,3);
BaroAltitude.timestamp = medfilt1(BaroAltitude.timestamp,3);

north = [24000 1700 43000];
insgps_ml('INSGPSInit')
insgps_ml('INSSetMagNorth',north);
insgps_ml('INSSetMagVar',[5 5 5]/1000);
insgps_ml('INSSetAccelVar',1.5e-4 * ones(1,3));
insgps_ml('INSSetGyroVar',2e-5*ones(1,3));
accel_idx = 1;
mag_idx = 1;
baro_idx = 1;
for i = 1:length(Gyros.timestamp)-1000
    t = Gyros.timestamp(i);
    gyro = [Gyros.x(i) Gyros.y(i) Gyros.z(i)] * pi / 180;
    
    accel_idx = accel_idx - 1 + find(Accels.timestamp(accel_idx:end) >= t,1,'first');
    accel = [Accels.x(accel_idx) Accels.y(accel_idx) Accels.z(accel_idx)-0.4];

    dT = 0.0015;
    [a(:,i) b K] = insgps_ml('INSStatePrediction',gyro,accel,dT);

    mag_idx = mag_idx - 1 + find(Magnetometer.timestamp(mag_idx:end) >= t,1,'first');
    mag = [Magnetometer.x(mag_idx) Magnetometer.y(mag_idx) Magnetometer.z(mag_idx)];

    baro_idx = baro_idx - 1 + find(BaroAltitude.timestamp(mag_idx:end) >= t,1,'first');
    baro = -BaroAltitude.Altitude(baro_idx);

    pos = [0 0 -baro];
    vel = [0 0 0];
    if i == 1
        rpy(1) = atan2(accel(1), accel(3));
        rpy(2) = atan2(accel(2), accel(3));
        rpy(3) = atan2(mag(2),-mag(1));
        
        %q1 = AttiudeFromVectors(mag',north',accel');
        q1 = RPY2Quaternion(rpy * 180 / pi)';
        insgps_ml('INSSetState',[0 0 -baro 0 0 0 q1' 0 0 0]);
    elseif i < 1000
        insgps_ml('INSSetMagVar',[5 5 5]);
        insgps_ml('INSSetPosVelVar',1 / 1000);
        % zero bias
        insgps_ml('INSSetState',[a(1:10,i)' 0 0 0]);
        state = insgps_ml('INSFullCorrection',mag,pos,vel,baro);
    else 
        insgps_ml('INSSetMagVar',[5 5 5]);
        insgps_ml('INSSetPosVelVar',1);

        insgps_ml('INSMagVelBaroCorrection',mag,vel,baro);
    end
        
    %state = insgps_ml('INSFullCorrection',mag,pos,vel,baro);
    if(mod(i,10) == 0) 
        q = a(7:10,1:i);
        rpy = Quaternion2RPY(q');
        subplot(211);
        plot(rpy(:,1:2))
        subplot(212);
        plot(a(1:3,1:i)');
        drawnow
    end    
end
