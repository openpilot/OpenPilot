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