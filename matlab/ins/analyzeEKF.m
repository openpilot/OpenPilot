function analyzeEKF(fn)

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
X = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(85:85+13*4-1)')),1,[]),'single'),13,[])';
P = reshape(typecast(reshape(dat(bsxfun(@plus,starts,(137:137+13*4-1)')),1,[]),'single'),13,[])';

insgps_ml('INSGPSInit');
insgps_ml('INSSetMagNorth',[24000 1700 43000]);
insgps_ml('INSSetMagVar',[50 50 50]/1000/1000);
insgps_ml('INSSetAccelVar',1.5e-4 * ones(1,3));
insgps_ml('INSSetGyroVar',2e-5*ones(1,3));

for i = 1:size(accel,1)
	[a(:,i) b K] = insgps_ml('INSStatePrediction',gyro(i,:),accel(i,:), 0.01);
	if(mag_updated(i))
		[c(:,i) d] = insgps_ml('INSMagVelBaroCorrection',mag(i,:),[0 0 0], 0);
	else
		[c(:,i) d] = insgps_ml('INSVelBaroCorrection',[0 0 0], 0);
	end
	if(mod(i,10) == 0) 
		subplot(211);
		imagesc(b < 0); %//,[-10 0])
		subplot(212);
		imagesc(a)
		drawnow
	end
end

imagesc(a)
keyboard
