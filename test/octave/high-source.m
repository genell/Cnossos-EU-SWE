%
% all high-source tests
%

clear

method = "ISO-9613-2"

options.dummy = true;

meteo.model = "DEFAULT"
meteo.temperature = 15.0
meteo.humidity = 70.0
meteo.pFav = 0.50
meteo.C0 = 3.00

materials.dummy.G = 0;

% sourcer
path.p0_source.pos.x = 0
path.p0_source.pos.y = 0
path.p0_source.pos.z = 0
path.p0_source.mat = "H"
path.p0_source.source.h = 4.00
path.p0_source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.p0_source.source.Lw.sourceType = "PointSource"
path.p0_source.source.Lw.measurementType = "HemiSpherical"
path.p0_source.source.Lw.frequencyWeighting = "LIN"

% border of hard platform at x = 9.5m, groundType = H
path.p1_ground.pos.x = 9.50
path.p1_ground.pos.z = 0.00
path.p1_ground.mat = "H"

% receiver, groundType = D
path.p2_receiver.pos.x = 10.00
path.p2_receiver.pos.z = 0.00
path.p2_receiver.mat = "D"
path.p2_receiver.receiver.h = 1.5

%
% Tests
%

% high-source 10m
path.p2_receiver.pos.x = 10.00
highsource10m = cnossos_full(method, path, options, meteo, materials);

% high-source 20m
path.p2_receiver.pos.x = 20.00
highsource20m = cnossos_full(method, path, options, meteo, materials);

% high-source 50m
path.p2_receiver.pos.x = 50.00
highsource50m = cnossos_full(method, path, options, meteo, materials);

% high-source 100m
path.p2_receiver.pos.x = 100.00
highsource100m = cnossos_full(method, path, options, meteo, materials);

% high-source 200m
path.p2_receiver.pos.x = 200.00
highsource200m = cnossos_full(method, path, options, meteo, materials);

% high-source 500m
path.p2_receiver.pos.x = 500.00
highsource500m = cnossos_full(method, path, options, meteo, materials);
