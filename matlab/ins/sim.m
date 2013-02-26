t0 = 60000;

bad = find(Gyros.timestamp > max(medfilt1(Gyros.timestamp,5)) | Gyros.timestamp < t0);
Gyros.timestamp(bad) = [];
Gyros.x(bad) = [];
Gyros.y(bad) = [];
Gyros.z(bad) = [];
Gyros.timestamp = medfilt1(Gyros.timestamp,5);

bad = find(Accels.timestamp > max(medfilt1(Accels.timestamp,5)));
Accels.timestamp(bad) = [];
Accels.x(bad) = [];
Accels.y(bad) = [];
Accels.z(bad) = [];
Accels.timestamp = medfilt1(Accels.timestamp,5);

bad = find(Magnetometer.timestamp > max(medfilt1(Magnetometer.timestamp,5)));
Magnetometer.timestamp(bad) = [];
Magnetometer.x(bad) = [];
Magnetometer.y(bad) = [];
Magnetometer.z(bad) = [];
Magnetometer.timestamp = medfilt1(Magnetometer.timestamp,5);

bad = find(BaroAltitude.timestamp > max(medfilt1(BaroAltitude.timestamp,5)));
BaroAltitude.timestamp(bad) = [];
BaroAltitude.Altitude(bad) = [];


bad = find(GPSPosition.timestamp > max(medfilt1(GPSPosition.timestamp,5)));
GPSPosition.timestamp(bad) = [];
GPSPosition.Latitude(bad) = [];
GPSPosition.Longitude(bad) = [];
GPSPosition.Altitude(bad) = [];
GPSPosition.Heading(bad) = [];
GPSPosition.Satellites(bad) = [];
GPSPosition.Groundspeed(bad) = [];
GPSPosition.timestamp = medfilt1(GPSPosition.timestamp,5);

outdoor = any(GPSPosition.Satellites > 7);

north = [24000 1700 43000];
baro_offset = 0;
if outdoor
    idx = find(GPSPosition.Satellites > 7, 1, 'first');
    home = [GPSPosition.Latitude(idx) GPSPosition.Longitude(idx) GPSPosition.Altitude(idx)];
    home(1:2) = home(1:2) / 10e6;
    baro_offset = -125;
end

insgps_ml('INSGPSInit')
insgps_ml('INSSetMagNorth',north);
insgps_ml('INSSetMagVar',[5 5 5]/1000);
insgps_ml('INSSetAccelVar',1.5e-5 * ones(1,3));
insgps_ml('INSSetGyroVar',2e-4*ones(1,3));

accel_idx = 1;
mag_idx = 1;
baro_idx = 1;
gps_idx = 1;

update_mag = false;
update_baro = false;
update_pos = false;

inited = false;

ned = zeros(3,length(Gyros.timestamp));
gps_vel = zeros(3,length(Gyros.timestamp));
a = zeros(13,length(Gyros.timestamp));
for i = 1:length(Gyros.timestamp)-1000
    t = Gyros.timestamp(i);
    gyro = [Gyros.x(i) Gyros.y(i) Gyros.z(i)] * pi / 180;
    
    accel_idx = accel_idx - 1 + find(Accels.timestamp(accel_idx:end) >= t,1,'first');
    accel = [Accels.x(accel_idx) Accels.y(accel_idx) Accels.z(accel_idx)-0.4];

    dT = 0.0013;
    [a(:,i) b K] = insgps_ml('INSStatePrediction',gyro,accel,dT);

    if Magnetometer.timestamp(mag_idx) <= t
        update_mag = true;
        mag_idx = mag_idx + 1;
        mag = [Magnetometer.x(mag_idx) Magnetometer.y(mag_idx) Magnetometer.z(mag_idx)];
    end
    
    if BaroAltitude.timestamp(baro_idx) <= t
        baro_idx = baro_idx + 1;
        update_baro = true;
        baro = -BaroAltitude.Altitude(baro_idx) + baro_offset;
    end

    if outdoor && GPSPosition.timestamp(gps_idx) <= t
        gps_idx = gps_idx + 1;
        lla = [GPSPosition.Latitude(gps_idx) GPSPosition.Longitude(gps_idx) GPSPosition.Altitude(gps_idx)];
        lla(1:2) = lla(1:2) / 10e6;
        pos = LLA2NED(lla, home);
        vel = [cos(GPSPosition.Heading(gps_idx) * pi / 180), sin(GPSPosition.Heading(gps_idx) * pi / 180) 0];
        vel = vel * GPSPosition.Groundspeed(gps_idx);
        update_pos = true;
    elseif ~outdoor
        pos = [0 0 baro];
        vel = [0 0 0];
        update_pos = true;
    end
    
    if t > 4.1e5 & t < 4.2e5
        update_pos = false;
    end
    
    ned(:,i) = pos;
    gps_vel(:,i) = vel;
    if ~inited && update_pos && update_baro && update_mag
        update_pos = false;
        update_baro = false;
        update_mag = false;
        
        inited = true;
        
        rpy(1) = atan2(accel(1), accel(3)) * 0;
        rpy(2) = atan2(accel(2), accel(3)) * 0;
        rpy(3) = atan2(mag(2),-mag(1));
        
        %q1 = AttiudeFromVectors(mag',north',accel');
        q1 = RPY2Quaternion(rpy * 180 / pi)';
        insgps_ml('INSSetState',[pos(1) pos(2) baro vel q1' -mean(Gyros.x(1:100) * pi / 180) -mean(Gyros.y(1:100) * pi / 180) 0]);
    elseif inited && i < 100
        insgps_ml('INSSetMagVar',[5 5 5]);
        insgps_ml('INSSetPosVelVar',1 / 1000);
        % zero bias
        %insgps_ml('INSSetState',[a(1:10,i)' 0 0 0]);
        %state = insgps_ml('INSFullCorrection',mag,pos,vel,baro);
    elseif inited
        insgps_ml('INSSetMagVar',[5 5 5]*100);

        if ~outdoor
            if update_mag
                insgps_ml('INSMagCorrection',mag);
                update_mag = false;
            elseif update_baro
                insgps_ml('INSVelBaroCorrection',vel,baro);
                update_baro = false;
            elseif update_pos
                insgps_ml('INSGpsCorrection',[0 0 baro],[0 0 0]);
                update_pos = false;
            end
        else % outdoor
            insgps_ml('INSSetPosVelVar',0.0001);

            if update_mag
                insgps_ml('INSMagCorrection',mag);
                update_mag = false;
            elseif update_baro
                insgps_ml('INSBaroCorrection',baro);
                update_baro = false;
            elseif update_pos
                insgps_ml('INSGpsCorrection',pos,vel);
                update_pos = false;
            end
        end    
    end
        
    %state = insgps_ml('INSFullCorrection',mag,pos,vel,baro);
    if(mod(i,100) == 0) 
        q = a(7:10,1:i);
        rpy = Quaternion2RPY(q');
        subplot(311);
        idx = find((AttitudeActual.timestamp < t) & (AttitudeActual.timestamp > Gyros.timestamp(1)));
        plot(Gyros.timestamp(1:i),rpy(:,1:2),AttitudeActual.timestamp(idx),AttitudeActual.Roll(idx),'k',AttitudeActual.timestamp(idx),AttitudeActual.Pitch(idx),'k');
        subplot(312);
        plot(Gyros.timestamp(1:i), a(1:2,1:i)',Gyros.timestamp(1:i), ned(1:2,1:i)','k');
        subplot(313);
        plot(Gyros.timestamp(1:i), a(4:5,1:i)',Gyros.timestamp(1:i), gps_vel(1:2,1:i)','k');
        drawnow
    end    
end
