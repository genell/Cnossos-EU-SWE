%
% all high-source tests
%

method = "ISO-9613-2"

meteo.model = "DEFAULT"
meteo.temperature = 15.0
meteo.humidity = 70.0
meteo.pFav = 0.50
meteo.C0 = 3.00

% sourcer
path.source.pos.x = 0
path.source.pos.y = 0
path.source.pos.z = 0
path.source.mat = "H"
path.source.source.h = 4.00
path.source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.source.source.Lw.sourceType = "PointSource"
path.source.source.Lw.measurementType = "HemiSpherical"
path.source.source.Lw.frequencyWeighting = "LIN"

% border of hard platform at x = 9.5m, groundType = H
path.ground.pos.x = 9.50
path.ground.pos.z = 0.00
path.ground.mat = "H"

% receiver, groundType = D
path.receiver.pos.x = 10.00
path.receiver.pos.z = 0.00
path.receiver.mat = "D"
path.receiver.receiver.h = 1.5

%
% Tests
%

% high-source 10m
path.receiver.pos.x = 10.00
highsource10m = cnossos_full(method, path, {}, meteo, {});

% high-source 20m
path.receiver.pos.x = 20.00
highsource20m = cnossos_full(method, path, {}, meteo, {});

% high-source 50m
path.receiver.pos.x = 50.00
highsource50m = cnossos_full(method, path, {}, meteo, {});

% high-source 100m
path.receiver.pos.x = 100.00
highsource100m = cnossos_full(method, path, {}, meteo, {});

% high-source 200m
path.receiver.pos.x = 200.00
highsource200m = cnossos_full(method, path, {}, meteo, {});

% high-source 500m
path.receiver.pos.x = 500.00
highsource500m = cnossos_full(method, path, {}, meteo, {});
