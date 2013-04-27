function test

insgps_ml('INSGPSInit');
insgps_ml('INSSetMagNorth',[24000 1700 43000]);
insgps_ml('INSSetMagVar',[5 5 5]/1000/1000);
insgps_ml('INSSetAccelVar',1.5e-4 * ones(1,3));
insgps_ml('INSSetGyroVar',2e-5*ones(1,3));

accel = [.15 -.15 -9.9];
gyro = [0.01 0.007 0.012];
mag = [-364 -64 603];
%mag = [-1700 24000 43000];

N = 20000;
a = zeros(N,13);
c = zeros(N,13);
dt = 0.01;
time = (1:N)*dt;
for i = 1:20000
	[a(i,:) b K] = insgps_ml('INSStatePrediction',gyro, accel,0.01);
	if(mod(i,10) == 0)
		[c(i,:) d K] = insgps_ml('INSMagVelBaroCorrection',mag,[0 0 0], 0);
	else
		[c(i,:) d K] = insgps_ml('INSVelBaroCorrection',[0 0 0], 0);
	end
	if(mod(i,50) == 0) 
		subplot(221);
		imagesc(b); %//,[-10 0])
		subplot(222);
		imagesc(K)
		subplot(212);
		imagesc(a)
		drawnow
	end
end

keyboard

