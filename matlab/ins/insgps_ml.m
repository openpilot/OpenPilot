function insgps_ml(funName, ...)
% Calls the C linked insgps algorithm
%
% insgps_ml('INSGPSInit') 
%   initialize the algorithm
% insgps_ml('INSStatePrediction',gyro,accel,dT);
% insgps_ml('INSFullCorrection',mag,pos,vel,baro);
% JC 2010-11-31

