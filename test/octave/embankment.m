%
% all embankment tests
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
path.source.pos.z = 1.00
path.source.mat = "H"
path.source.source.h = 0.05
path.source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.source.source.Lw.sourceType = "PointSource"
path.source.source.Lw.measurementType = "HemiSpherical"
path.source.source.Lw.frequencyWeighting = "LIN"

% border of hard platform at x = 5m, groundType = H
path.border.pos.x = 5.00
path.border.pos.z = 1.00
path.border.mat = "H"

% border of hard platform at x = 7m, groundType = H
path.ground.pos.x = 7.00
path.border.pos.z = 0.00
path.border.mat = "D"

% receiver, groundType = D
path.receiver.pos.x = 10.00
path.receiver.pos.z = 0.00
path.receiver.mat = "D"
path.receiver.receiver.h = 2.5

%
% Tests
%

% embankment 1m - 10m
path.border.pos.z = 1.00
path.receiver.pos.x = 10.00
embankment1m10m = cnossos_full(method, path, {}, meteo, {});

% embankment 1m - 20m
path.border.pos.z = 1.00
path.receiver.pos.x = 20.00
embankment1m20m = cnossos_full(method, path, {}, meteo, {});

% embankment 1m - 50m
path.border.pos.z = 1.00
path.receiver.pos.x = 50.00
embankment1m50m = cnossos_full(method, path, {}, meteo, {});

% embankment 1m - 100m
path.border.pos.z = 1.00
path.receiver.pos.x = 100.00
embankment1m100m = cnossos_full(method, path, {}, meteo, {});

% embankment 1m - 200m
path.border.pos.z = 1.00
path.receiver.pos.x = 200.00
embankment1m200m = cnossos_full(method, path, {}, meteo, {});

% embankment 1m - 500m
path.border.pos.z = 1.00
path.receiver.pos.x = 500.00
embankment1m500m = cnossos_full(method, path, {}, meteo, {});

% embankment 2m - 10m
path.border.pos.z = 2.00
path.receiver.pos.x = 10.00
embankment2m10m = cnossos_full(method, path, {}, meteo, {});

% embankment 2m - 20m
path.border.pos.z = 2.00
path.receiver.pos.x = 20.00
embankment2m20m = cnossos_full(method, path, {}, meteo, {});

% embankment 2m - 50m
path.border.pos.z = 2.00
path.receiver.pos.x = 50.00
embankment2m50m = cnossos_full(method, path, {}, meteo, {});

% embankment 2m - 100m
path.border.pos.z = 2.00
path.receiver.pos.x = 100.00
embankment2m100m = cnossos_full(method, path, {}, meteo, {});

% embankment 2m - 200m
path.border.pos.z = 2.00
path.receiver.pos.x = 200.00
embankment2m200m = cnossos_full(method, path, {}, meteo, {});

% embankment 2m - 500m
path.border.pos.z = 2.00
path.receiver.pos.x = 500.00
embankment2m500m = cnossos_full(method, path, {}, meteo, {});

% embankment 4m - 10m
path.border.pos.z = 4.00
path.receiver.pos.x = 10.00
embankment4m10m = cnossos_full(method, path, {}, meteo, {});

% embankment 4m - 20m
path.border.pos.z = 4.00
path.receiver.pos.x = 20.00
embankment4m20m = cnossos_full(method, path, {}, meteo, {});

% embankment 4m - 50m
path.border.pos.z = 4.00
path.receiver.pos.x = 50.00
embankment4m50m = cnossos_full(method, path, {}, meteo, {});

% embankment 4m - 100m
path.border.pos.z = 4.00
path.receiver.pos.x = 100.00
embankment4m100m = cnossos_full(method, path, {}, meteo, {});

% embankment 4m - 200m
path.border.pos.z = 4.00
path.receiver.pos.x = 200.00
embankment4m200m = cnossos_full(method, path, {}, meteo, {});

% embankment 4m - 500m
path.border.pos.z = 4.00
path.receiver.pos.x = 500.00
embankment4m500m = cnossos_full(method, path, {}, meteo, {});
