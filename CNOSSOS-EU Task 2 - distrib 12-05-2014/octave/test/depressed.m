% all barrier tests

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
path.p0_source.source.h = 0.05
path.p0_source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.p0_source.source.Lw.sourceType = "PointSource"
path.p0_source.source.Lw.measurementType = "HemiSpherical"
path.p0_source.source.Lw.frequencyWeighting = "LIN"

% border of hard platform at x = 5m, groundType = H
path.p1_border.pos.x = 5.00
path.p1_border.pos.z	= 0.00
path.p1_border.mat = "H"

% border of hard platform at x = 7m, groundType = H
path.p2_ground.pos.x = 7.00
path.p2_ground.pos.z = 2.00
path.p2_ground.mat = "D"

% receiver, groundType = D
path.p3_receiver.pos.x = 10.00
path.p3_receiver.pos.z = 2.00
path.p3_receiver.mat = "D"
path.p3_receiver.receiver.h = 2.5

%
% Tests
%

% depressed 10m
path.p3_receiver.pos.x = 10.00
depressed10m = cnossos_full(method, path, options, meteo);

% depressed 20m
path.p3_receiver.pos.x = 20.00
depressed20m = cnossos_full(method, path, options, meteo);

% depressed 50m
path.p3_receiver.pos.x = 50.00
depressed50m = cnossos_full(method, path, options, meteo);

% depressed 100m
path.p3_receiver.pos.x = 100.00
depressed100m = cnossos_full(method, path, options, meteo);

% depressed 200m
path.p3_receiver.pos.x = 200.00
depressed200m = cnossos_full(method, path, options, meteo);

% depressed 500m
path.p3_receiver.pos.x = 500.00
depressed500m = cnossos_full(method, path, options, meteo);
