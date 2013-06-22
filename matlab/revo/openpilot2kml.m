function openpilot2kml(matfile)

if ~exist('ge_colorbar', 'file')
	msg=['Google Earth Toolbox must be present and included in the Matlab path. For example:'...
		10 ' path(path, ''/Users/ponthy/googleearth'''];
	error(msg)
end

if nargin==0
	[FileName,PathName] = uigetfile('*.mat');
	matfile = strcat(PathName,FileName);
end

if ~exist(matfile, 'file')
	error('Not a valid matlab file')
end
load(matfile);

%%

t=PositionActual.timestamp/1000;

NED=[PositionActual.North', PositionActual.East', PositionActual.Down'];
BaseECEF = LLA2ECEF([HomeLocation.Latitude*1e-7, HomeLocation.Longitude*1e-7, HomeLocation.Altitude]');
Rne = RneFromLLA([HomeLocation.Latitude*1e-7, HomeLocation.Longitude*1e-7, HomeLocation.Altitude]');

LLA=Base2LLA(NED',BaseECEF,Rne);


gpsPath=LLA(1:2,:)';
gpsAlt=LLA(3,:)';
gpsSpeed=interp1(BaroAirspeed.timestamp/1000, BaroAirspeed.Airspeed, t);

%%
[pathstr, opName] = fileparts(matfile);

kmlFileName=[opName '.kml'];


kmlFolderName=pathstr;

line_seg={}; %#ok<NASGU> %Clear line_seg
gpsTime=t;

maxSpeed=200/3.6;
midSpeed=100/3.6;

%Define colormap
M=jet(round(maxSpeed));

line_seg=cell(round(length(gpsTime)*2),1);

%Create style for waypoints
line_seg{1}= ['<Style id="track"> <IconStyle> <scale>1.2</scale> <Icon> <href>http://earth.google.com/images/kml-icons/track-directional/track-none.png</href> </Icon> </IconStyle> </Style>' 10];

line_seg{2} = ge_colorbar(gpsPath(1,2), gpsPath(1,1), maxSpeed*(1-exp(log(1/2)/midSpeed*(0:maxSpeed))),...
	'cBarBorderWidth',1,...
	'cBarFormatStr','%+3.0f',...
	'altitudeMode', 'clampToGround',...
	'numUnits',10,...
	'name','Speed colorbar');

%Add start and stop points
line_seg{3}=ge_point(gpsPath(1,2), gpsPath(1,1), gpsAlt(1), 'altitudeMode', 'clampToGround', 'name', 'Start');
line_seg{4}=ge_point(gpsPath(end,2), gpsPath(end,1), gpsAlt(end), 'altitudeMode', 'clampToGround', 'name', 'Stop');
k=4; m=0; n=0;

%Add text progress output
fprintf('\n\n\n');
str2=sprintf('Completed %3.0f%%', 0);
fprintf([str2 '\n'])

%Create line segments and color based on speed
for i=1:length(gpsTime)-1
	m=m+1;
	Y=gpsPath(i:i+1,1);
	X=gpsPath(i:i+1,2);
	Z=[gpsAlt(i) gpsAlt(i)];
	logSpeed=maxSpeed*(1-exp(log(1/2)/midSpeed*gpsSpeed(i)));
	
	red=round(M(floor(min(logSpeed+1, length(M))),1)*255);
	green=round(M(floor(min(logSpeed+1, length(M))),2)*255);
	blue=round(M(floor(min(logSpeed+1, length(M))),3)*255);
	
	tmpHour=floor(t(i)/3600);
	tmpMinute=floor((t(i)-tmpHour*3600)/60);
	tmpSec=t(i)-tmpHour*3600-tmpMinute*60;
	tGPS=[2012 06 23 tmpHour tmpMinute tmpSec];
	tStart = datestr( tGPS, 'yyyy-mm-ddTHH:MM:SSZ');
	tStop  = datestr( tGPS+[0 0 0 0 0 2], 'yyyy-mm-ddTHH:MM:SSZ'); %Visible for two seconds
	
	[line_seg{i+k}]= ge_plot3(X,Y,Z,...
		'altitudeMode', 'absolute',...
		'timeSpanStart',tStart,...
		'timeSpanStop',tStop,...
		'lineWidth', 6, ...
		'lineColor', [ 'ff' dec2hex(red, 2) dec2hex(green, 2) dec2hex(blue, 2)], ...
		'forceAsLine', false, ...
		'msgToScreen', false);
	
	if mod(i,10) == 0
		%%
		k=k+1;
		n=n+1;
		
		descriptionCell={'North coords., in [m]', num2str(NED(i,1), '%10.2f');
			'East coords., in [m]', num2str(NED(i,2), '%10.2f');
			'Height AGL, in [m]', num2str(Z(1), '%10.2f') ;
			'Airspeed, in [km/hr]', num2str(gpsSpeed(i)*3.6, '%3.1f')};
		
		[line_seg{i+k}]=ge_point(X(1), Y(1), gpsAlt(i),...
			'altitudeMode', 'absolute',...
			'extrude', 1, ...
			'name', num2str(t(i)-t(1)),...
			'timeSpanStart',tStart,...
			'timeSpanStop',tStop,...
			'styleURL', '#track',...
			'iconURL', 'http://earth.google.com/images/kml-icons/track-directional/track-none.png',...
			'pointDataCell', descriptionCell);
		
		%%
		str1=[];
		for j=1:length(str2)+1;
			str1=[str1 sprintf('\b')]; %#ok<AGROW>
		end
		str2=['Completed ' num2str(i/length(t)*100, '%3.0f')];
		
		fprintf([str1 str2  '%%']);
		
	end
end
%Pare down line_seg
%%
line_seg=line_seg(1:i+k);

if 0
	%Properly formats XML part of kml file. IS TOO SLOW!
	Pref=[]; %#ok<UNRCH>
	Pref.StructItem = false;
	Pref.XmlEngine = 'String';  %Forces xml_write to use Apache XML engine instead of Matlab XML.
	
	[a,r]=xml_write([], plot_xml, 'b', Pref);
	indexFirst=find(r==10, 2);
	indexLast=find(r==10, 1, 'last');
	
	r=r(indexFirst(2)+1:indexLast);
	
	[a,s]=xml_write([], point_xml, 'b', Pref);
	indexFirst=find(s==10, 2);
	indexLast=find(s==10, 1, 'last');
	
	s=s(indexFirst(2)+1:indexLast);
	
	ge_output(kmlFileName, [line_seg{1} r s], 'name', kmlFolderName, 'msgToScreen', true)
else
	%%
	%Output segments to kml files
	ge_output(fullfile(kmlFolderName, kmlFileName), cat(2,line_seg{:}), 'name', opName, 'msgToScreen', true);
end

end

% ===========================


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

LLA=zeros(size(NED));
for i=1:n
	LLA(:,i)=ECEF2LLA(ECEF(:,i))';
end
end


% ****** convert ECEF to Lat,Lon,Alt (ITERATIVE!) *********
function LLA=ECEF2LLA(ECEF, Alt)
% Altitude parameter is used to prime the iteration.
%   A position within 1 meter of the specified LLA
% 	 will be calculated within at most 3 iterations.
% 	 If unknown: Call with any valid altitude
% 	 will compute within at most 5 iterations.
% 	 Suggestion: 0
% 	 

if nargin==1
	Alt=0;
end

a = 6378137.0;	          % Equatorial Radius
e = 8.1819190842622e-2;  % Eccentricity
x = ECEF(1);
y = ECEF(2);
z = ECEF(3);

MAX_ITER =10;		 % should not take more than 5 for valid coordinates
ACCURACY= 1.0e-11; % used to be e-14, but we don't need sub micrometer exact calculations
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

% ****** find ECEF to NED rotation matrix ********
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

