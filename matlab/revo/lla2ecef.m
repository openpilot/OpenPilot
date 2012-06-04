function ECEF = lla2ecef(latitude,longitude,altitude)

a = 6378137.0;
e = 8.1819190842622e-2;
DEG2RAD = pi / 180;
sinLat = sin(DEG2RAD * latitude);
sinLon = sin(DEG2RAD * longitude);
cosLat = cos(DEG2RAD * latitude);
cosLon = cos(DEG2RAD * longitude);

N = a ./ sqrt(1.0 - e * e .* sinLat .* sinLat);

ECEF(:,1) = (N + altitude) .* cosLat .* cosLon;
ECEF(:,2) = (N + altitude) .* cosLat .* sinLon;
ECEF(:,3) = ((1 - e * e) * N + altitude) .* sinLat;
