% all barrier tests
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
path.source.source.h = 0.05
path.source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.source.source.Lw.sourceType = "PointSource"
path.source.source.Lw.measurementType = "HemiSpherical"
path.source.source.Lw.frequencyWeighting = "LIN"

% border of hard platform at x = 5m, groundType = H
path.border.pos.x = 5.00
path.border.pos.z	= 0.00
path.border.mat = "H"

% border of hard platform at x = 7m, groundType = H
path.ground.pos.x = 7.00
path.ground.pos.z = 2.00
path.ground.mat = "D"

% receiver, groundType = D
path.receiver.pos.x = 10.00
path.receiver.pos.z = 2.00
path.receiver.mat = "D"
path.receiver.receiver.h = 2.5

%
% Tests
%

% depressed 10m
path.receiver.pos.x = 10.00
depressed10m = cnossos_full(method, path, {}, meteo, {});

% depressed 20m
path.receiver.pos.x = 20.00
depressed20m = cnossos_full(method, path, {}, meteo, {});

% depressed 50m
path.receiver.pos.x = 50.00
depressed50m = cnossos_full(method, path, {}, meteo, {});

% depressed 100m
path.receiver.pos.x = 100.00
depressed100m = cnossos_full(method, path, {}, meteo, {});

% depressed 200m
path.receiver.pos.x = 200.00
depressed200m = cnossos_full(method, path, {}, meteo, {});

% depressed 500m
path.receiver.pos.x = 500.00
depressed500m = cnossos_full(method, path, {}, meteo, {});
