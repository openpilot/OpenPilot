function OPPlots()
    [FileName,PathName,FilterIndex] = uigetfile('*.mat');
    matfile = strcat(PathName,FileName);
    load(matfile);
	 
    %load('specificfilename')

    TimeVA = [VelocityActual.timestamp]/1000;
    VA = [[VelocityActual.North]
          [VelocityActual.East]
          [VelocityActual.Down]]/100;

    TimeGPSPos = [GPSPosition.timestamp]/1000;
    Vgps=[[GPSPosition.Groundspeed].*cos([GPSPosition.Heading]*pi/180)
          [GPSPosition.Groundspeed].*sin([GPSPosition.Heading]*pi/180)];

    figure(1);
    plot(TimeVA,VA(1,:),TimeVA,VA(2,:),TimeGPSPos,Vgps(1,:),TimeGPSPos,Vgps(2,:));
    s1='Velocity Actual North';
    s2='Velocity Actual East';
    s3='GPS Velocity North';
    s4='GPS Velocity East';
    legend(s1,s2,s3,s4);
    xlabel('Time (sec)');
    ylabel('Velocity (m/s)');


    TimePA = [PositionActual.timestamp]/1000;
    PA = [[PositionActual.North]
          [PositionActual.East]
          [PositionActual.Down]]/100;

    TimeGPSPos = [GPSPosition.timestamp]/1000;
    LLA=[[GPSPosition.Latitude]*1e-7;
         [GPSPosition.Longitude]*1e-7;
         [GPSPosition.Altitude]+[GPSPosition.GeoidSeparation]];
    BaseECEF = LLA2ECEF([HomeLocation.Latitude*1e-7, HomeLocation.Longitude*1e-7, HomeLocation.Altitude]');
    Rne = RneFromLLA([HomeLocation.Latitude*1e-7, HomeLocation.Longitude*1e-7, HomeLocation.Altitude]');
    GPSPos=LLA2Base(LLA,BaseECEF,Rne); 

    figure(2);
    plot(TimePA,PA(1,:),TimePA,PA(2,:),TimeGPSPos,GPSPos(1,:),TimeGPSPos,GPSPos(2,:));
    s1='Position Actual North';
    s2='Position Actual East';
    s3='GPS Position North';
    s4='GPS Position East';
    legend(s1,s2,s3,s4);
    xlabel('Time (sec)');
    ylabel('Position (m)');

    figure(3);
    plot3(PA(2,:),PA(1,:),PA(3,:),GPSPos(2,:),GPSPos(1,:),GPSPos(3,:));
    s1='Pos Actual';
    s2='GPS Pos';
    legend(s1,s2);
    xlabel('East (m)');
    ylabel('North(m)');
    zlabel('Up (m)');
    axis equal

end

function NED = LLA2Base(LLA,BaseECEF,Rne)
	n = size(LLA,2);
	
	ECEF = LLA2ECEF(LLA);
	
	diff = ECEF - repmat(BaseECEF,1,n);
	
	NED = Rne*diff;
end

function LLA = Base2LLA(NED,BaseECEF,Rne)
	n = size(NED,2);
	
	diff=Rne'*NED;
	ECEF=diff + repmat(BaseECEF,1,n) ;
	
	LLA=ECEF2LLA(ECEF);
end


% // ****** convert ECEF to Lat,Lon,Alt (ITERATIVE!) *********
function LLA=ECEF2LLA(ECEF, Alt)
% 	/**
% 	 * Altitude parameter is used to prime the iteration.
% 	 * A position within 1 meter of the specified LLA
% 	 * will be calculated within at most 3 iterations.
% 	 * If unknown: Call with any valid altitude
% 	 * will compute within at most 5 iterations.
% 	 * Suggestion: 0
% 	 **/

	if nargin==1
		Alt=0;
	end

	a = 6378137.0;	%// Equatorial Radius
	e = 8.1819190842622e-2;%;	// Eccentricity
	x = ECEF(1);
	y = ECEF(2);
	z = ECEF(3);

	MAX_ITER =10;		%// should not take more than 5 for valid coordinates
	ACCURACY= 1.0e-11;%	// used to be e-14, but we don't need sub micrometer exact calculations
	RAD2DEG=180/pi;
	DEG2RAD=1/RAD2DEG;

	LLA(2) = RAD2DEG * atan2(y, x);
	Lat = DEG2RAD * LLA(1);
	esLat = e * sin(Lat);
	N = a / sqrt(1 - esLat * esLat);
	NplusH = N+Alt;
	delta = 1;
	iter = 0;

	while (((delta > ACCURACY) || (delta < -ACCURACY)) && (iter < MAX_ITER)) 
		delta = Lat - atan(z / (sqrt(x * x + y * y) * (1 - (N * e * e / NplusH))));
		Lat = Lat - delta;
		esLat = e * sin(Lat);
		N = a / sqrt(1 - esLat * esLat);
		NplusH = sqrt(x * x + y * y) / cos(Lat);
		iter = iter+1;
	end

	LLA(1) = RAD2DEG * Lat;
	LLA(3) = NplusH - N;


end

function ECEF = LLA2ECEF(LLA)

	a = 6378137.0;	% Equatorial Radius
	e = 8.1819190842622e-2;	% Eccentricity
	
	n = size(LLA,2);
	ECEF = zeros(3,n);
	
	for i=1:n
		sinLat = sin(pi*LLA(1,i)/180);
		sinLon = sin(pi*LLA(2,i)/180);
		cosLat = cos(pi*LLA(1,i)/180);
		cosLon = cos(pi*LLA(2,i)/180);
		
		N = a / sqrt(1.0 - e * e * sinLat * sinLat); %prime vertical radius of curvature
		
		ECEF(1,i) = (N + LLA(3,i)) * cosLat * cosLon;
		ECEF(2,i) = (N + LLA(3,i)) * cosLat * sinLon;
		ECEF(3,i) = ((1 - e * e) * N + LLA(3,i)) * sinLat;
	end
end

% // ****** find ECEF to NED rotation matrix ********
function  Rne=RneFromLLA(LLA)
	RAD2DEG=180/pi;
	DEG2RAD=1/RAD2DEG;

	sinLat = sin(DEG2RAD * LLA(1));
	sinLon = sin(DEG2RAD * LLA(2));
	cosLat = cos(DEG2RAD * LLA(1));
	cosLon = cos(DEG2RAD * LLA(2));

	Rne(1,1) = -sinLat * cosLon;
	Rne(1,2) = -sinLat * sinLon;
	Rne(1,3) = cosLat;
	Rne(2,1) = -sinLon;
	Rne(2,2) = cosLon;
	Rne(2,3) = 0;
	Rne(3,1) = -cosLat * cosLon;
	Rne(3,2) = -cosLat * sinLon;
	Rne(3,3) = -sinLat;
end

