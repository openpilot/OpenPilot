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
    BaseECEF = HomeLocation(end).ECEF/100;
    Rne = reshape(HomeLocation(end).RNE,3,3);
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

