source = gyro;
clear data
data.freq = source(:,3)';
data.time = unwrap_time(source(:,end))'/1000;
data.rate = 1/mean(diff(data.time))
tau = 1/data.rate * round(logspace(0,7,200))
[retval, s, errorb, tau] = allan(data,tau,'gyro')

% [~,~,gyro_residual] = regress(gyro(:,1)-mean(gyro(:,1)),[gyro(:,4)-mean(gyro(:,4)), (1:size(gyro,1))']);
% data.freq = gyro_residual;
% data.time = unwrap_time(source(:,end))'/1000;
% data.rate = 1/mean(diff(data.time))
% tau = 1/data.rate * round(logspace(0,7,200))
% [retval, s, errorb, tau] = allan(data,tau,'gyro')
