%
% all flat ground tests
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
path.source.source.h = 0.5
path.source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.source.source.Lw.sourceType = "PointSource"
path.source.source.Lw.measurementType = "HemiSpherical"
path.source.source.Lw.frequencyWeighting = "LIN"

% border of hard platform at x = 5m, groundType = H
path.ground.pos.x = 5.00
path.ground.pos.z = 0.00
path.ground.mat = "H"

% receiver, groundType = D
path.receiver.pos.x = 10.00
path.receiver.pos.z = 0.00
path.receiver.mat = "D"
path.receiver.receiver.h = 2.5

%
% Tests
%

% flat ground 10m
path.receiver.pos.x = 10.00
flatground10m = cnossos_full(method, path, {}, meteo, {});

% flat ground 20m
path.receiver.pos.x = 20.00
flatground20m = cnossos_full(method, path, {}, meteo, {});

% flat ground 50m
path.receiver.pos.x = 50.00
flatground50m = cnossos_full(method, path, {}, meteo, {});

% flat ground 100m
path.receiver.pos.x = 100.00
flatground100m = cnossos_full(method, path, {}, meteo, {});

% flat ground 200m
path.receiver.pos.x = 200.00
flatground200m = cnossos_full(method, path, {}, meteo, {});

% flat ground 500m
path.receiver.pos.x = 500.00
flatground500m = cnossos_full(method, path, {}, meteo, {});
