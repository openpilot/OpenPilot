baro = fixTime(BaroAltitude);
accel = fixTime(Accels);
attitude = fixTime(AttitudeActual);

%accel.z(end/2:end) = accel.z(end/2:end) + 1;
accel.z = accel.z+2;

Gamma = diag([1e-15 1e-15 1e-3 1e-5]); % state noise
accel_sigma = 10;
baro_sigma = 0.1;
Nu = diag([10 10 10 1000]);
z = zeros(length(accel.z),4);
Nu_n = zeros([4 4 length(accel.z)]);
Nu_n(:,:,1) = Nu;

t = max(accel.timestamp(1), baro.timestamp(1));
last_t = t-1;
last_accel_idx = 1;
last_baro_idx = 1;
i = 1;
timestamp = [];

z(1) = baro.Altitude(1);
z(1:5,4) = 0;
timestamp(1) = t;
log_accel = 0;
while (last_accel_idx + 1) <= length(accel.z) && (last_baro_idx + 1) <= length(baro.Altitude)
    
    update_baro = baro.timestamp(last_baro_idx + 1) < t;
    update_accel = accel.timestamp(last_accel_idx + 1) < t;    

    if 0 && update_accel
        [~,idx] = min(abs(attitude.timestamp - accel.timestamp(last_accel_idx+1)));
        Rbe = Quat2Rbe([attitude.q1(idx), attitude.q2(idx), attitude.q3(idx), attitude.q4(idx)]);
        idx = last_accel_idx + 1;
        accel_ned = Rbe * [accel.x(idx); accel.y(idx); accel.z(idx)];
        accel_ned = accel_ned(3);
%         if(abs(accel_ned) < 1e-1)
%             keyboard
%         end  
    else
        accel_ned = accel.z(last_accel_idx + 1);
    end
    log_accel(i) = accel_ned;
    if update_baro && update_accel;
        x = [baro.Altitude(last_baro_idx + 1); -accel_ned-9.81];
        last_baro_idx = last_baro_idx + 1;
        last_accel_idx = last_accel_idx + 1;
        C = [1 0 0 0; 0 0 1 1];
        Sigma = diag([baro_sigma; accel_sigma]);
    elseif update_accel
        x =  -accel_ned - 9.81;
        last_accel_idx = last_accel_idx + 1;
        C = [0 0 1 1];
        Sigma = [accel_sigma];
    elseif update_baro
        x = [baro.Altitude(last_baro_idx + 1)];
        last_baro_idx = last_baro_idx + 1;
        C = [1 0 0 0];
        Sigma = [baro_sigma];
    else 
        % Take a timestep and look for advance
        t = t + 0.1;
        continue;
   end
    %[last_baro_idx last_accel_idx]
    t = max(baro.timestamp(last_baro_idx), accel.timestamp(last_accel_idx));
    dT = (t - last_t) / 1000;
    if(dT == 0)
        dT = 1.5 / 1000;
    end
    assert(dT ~= 0,'WTF');
    last_t = t;
    
    i = i + 1;
       
    A = [1 dT 0 0; 0 1 dT 0; 0 0 1 0; 0 0 0 1];
 
    P = A * Nu * A' + Gamma;
    K = P*C'*(C*P*C'+Sigma)^-1;      
    
    z(i,:) = A * z(i-1,:)' + K * (x - C * A * z(i-1,:)');
    timestamp(i) = t;
    Nu = (eye(4) - K * C) * P;
    Nu_n(:,:,i) = Nu;
    
    if mod(i,10000) == 0
        subplot(311)
        plot(baro.timestamp, baro.Altitude, '.', timestamp(1:i),z(1:i,1),'r','LineWidth',5)
        subplot(312)
        plot(timestamp(1:i),z(1:i,2),'k')
        subplot(313)
        plot(timestamp(1:i),z(1:i,3),'k');
        xlim(timestamp([1,i]))
        drawnow
    end
end

subplot(311)
plot(baro.timestamp, baro.Altitude, '.', timestamp(1:i),z(1:i,1),'r','LineWidth',5)
subplot(312)
plot(timestamp(1:i),z(1:i,2),'k')
subplot(313)
plot(timestamp(1:i),z(1:i,3),'k');
xlim(timestamp([1,i]))

