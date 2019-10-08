%
% all flat ground tests
%

clear

method = "ISO-9613-2"

options.dummy = true;

meteo.model = "DEFAULT"
meteo.temperature = 15.0
meteo.humidity = 70.0
meteo.pFav = 0.50
meteo.C0 = 3.00

% sourcer
path.p0_source.pos.x = 0
path.p0_source.pos.y = 0
path.p0_source.pos.z = 0
path.p0_source.mat = "H"
path.p0_source.source.h = 0.5
path.p0_source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.p0_source.source.Lw.sourceType = "PointSource"
path.p0_source.source.Lw.measurementType = "HemiSpherical"
path.p0_source.source.Lw.frequencyWeighting = "LIN"

% border of hard platform at x = 5m, groundType = H
path.p1_ground.pos.x = 5.00
path.p1_ground.pos.z = 0.00
path.p1_ground.mat = "H"

% receiver, groundType = D
path.p2_receiver.pos.x = 10.00
path.p2_receiver.pos.z = 0.00
path.p2_receiver.mat = "D"
path.p2_receiver.receiver.h = 2.5

%
% Tests
%

% flat ground 10m
path.p2_receiver.pos.x = 10.00
flatground10m = cnossos_full(method, path, options, meteo);

% flat ground 20m
path.p2_receiver.pos.x = 20.00
flatground20m = cnossos_full(method, path, options, meteo);

% flat ground 50m
path.p2_receiver.pos.x = 50.00
flatground50m = cnossos_full(method, path, options, meteo);

% flat ground 100m
path.p2_receiver.pos.x = 100.00
flatground100m = cnossos_full(method, path, options, meteo);

% flat ground 200m
path.p2_receiver.pos.x = 200.00
flatground200m = cnossos_full(method, path, options, meteo);

% flat ground 500m
path.p2_receiver.pos.x = 500.00
flatground500m = cnossos_full(method, path, options, meteo);
