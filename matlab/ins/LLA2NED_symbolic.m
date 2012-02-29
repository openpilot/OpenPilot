function NED = LLA2NED_symbolic(lla, home)

DEG2RAD = pi / 180;
a = 6378137.0;	            % Equatorial Radius
e = 8.1819190842622e-2;	% Eccentricity

lat = home(1) / 10e6 * DEG2RAD;
lon = home(2) / 10e6 * DEG2RAD;
alt = home(3);

delta = (lla - home);
delta(1:2) = delta(1:2) / 10e6 * DEG2RAD;
delta = reshape(delta,[],1);

N = sym('a / sqrt(1.0 - e * e * sin(lat) * sin(lat))');	%prime vertical radius of curvature

ECEF = [sym('(N + alt) * cos(lat) * cos(lon)'); ...
 sym('(N + alt) * cos(lat) * sin(lon)'); ...
 sym('((1 - e * e) * N + alt) * sin(lat)')];

ECEF = subs(ECEF, 'N', N);
ECEF = subs(ECEF, 'e', 0);
ECEF = subs(ECEF, 'a', a);

J = [diff(ECEF,'lat') diff(ECEF,'lon') diff(ECEF,'alt')];

Rne(1,1) = sym('-sin(lat) * cos(lon)');
Rne(1,2) = sym('-sin(lat) * sin(lon)');
Rne(1,3) = sym('cos(lat)');
Rne(2,1) = sym('-sin(lon)');
Rne(2,2) = sym('cos(lon)');
Rne(2,3) = 0;
Rne(3,1) = sym('-cos(lat) * cos(lon)');
Rne(3,2) = sym('-cos(lat) * sin(lon)');
Rne(3,3) = sym('-sin(lat)');

ccode(simplify(Rne * J))

NEDsymb = simplify(Rne * J) * delta;
NED = subs(subs(subs(NEDsymb,'lat',lat),'lon',lon),'alt',alt);
%delta = subs(subs(subs(J * delta,'lat',lat),'lon',lon),'alt',alt)
%Rne = subs(subs(subs(Rne,'lat',lat),'lon',lon),'alt',alt)
