%
% all barrier tests
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
path.source.source.h = 0.05
path.source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.source.source.Lw.sourceType = "PointSource"
path.source.source.Lw.measurementType = "HemiSpherical"
path.source.source.Lw.frequencyWeighting = "LIN"

% barrier at the border of hard platform, groundType = H
path.barrier.pos.x = 10.0
path.barrier.pos.z = 0.0
path.barrier.mat = "H"
path.barrier.barrier.h = 1.00
path.barrier.barrier.mat = "A0"

% receiver, groundType = D
path.receiver.pos.x = 20.00
path.receiver.pos.z = 0.00
path.receiver.mat = "D"
path.receiver.receiver.h = 2.5

%
% Tests
%

% barrier 1m - 20m
path.barrier.barrier.h = 1.00
path.receiver.pos.x = 20.00
barrier1m20m = cnossos_full(method, path, {}, meteo, {});

% barrier 1m - 50m
path.barrier.barrier.h = 1.00
path.receiver.pos.x = 50.00
barrier1m50m = cnossos_full(method, path, {}, meteo, {});

% barrier 1m - 100m
path.barrier.barrier.h = 1.00
path.receiver.pos.x = 100.00
barrier1m100m = cnossos_full(method, path, {}, meteo, {});

% barrier 1m - 200m
path.barrier.barrier.h = 1.00
path.receiver.pos.x = 200.00
barrier1m200m = cnossos_full(method, path, {}, meteo, {});

% barrier 1m - 500m
path.barrier.barrier.h = 1.00
path.receiver.pos.x = 500.00
barrier1m500m = cnossos_full(method, path, {}, meteo, {});

% barrier 2m - 20m
path.barrier.barrier.h = 2.00
path.receiver.pos.x = 20.00
barrier2m20m = cnossos_full(method, path, {}, meteo, {});

% barrier 2m - 50m
path.barrier.barrier.h = 2.00
path.receiver.pos.x = 50.00
barrier2m50m = cnossos_full(method, path, {}, meteo, {});

% barrier 2m - 100m
path.barrier.barrier.h = 2.00
path.receiver.pos.x = 100.00
barrier2m100m = cnossos_full(method, path, {}, meteo, {});

% barrier 2m - 200m
path.barrier.barrier.h = 2.00
path.receiver.pos.x = 200.00
barrier2m200m = cnossos_full(method, path, {}, meteo, {});

% barrier 2m - 500m
path.barrier.barrier.h = 2.00
path.receiver.pos.x = 500.00
barrier2m500m = cnossos_full(method, path, {}, meteo, {});

% barrier 4m - 20m
path.barrier.barrier.h = 4.00
path.receiver.pos.x = 20.00
barrier2m20m = cnossos_full(method, path, {}, meteo, {});

% barrier 4m - 50m
path.barrier.barrier.h = 4.00
path.receiver.pos.x = 50.00
barrier2m50m = cnossos_full(method, path, {}, meteo, {});

% barrier 4m - 100m
path.barrier.barrier.h = 4.00
path.receiver.pos.x = 100.00
barrier2m100m = cnossos_full(method, path, {}, meteo, {});

% barrier 4m - 200m
path.barrier.barrier.h = 4.00
path.receiver.pos.x = 200.00
barrier2m200m = cnossos_full(method, path, {}, meteo, {});

% barrier 4m - 500m
path.barrier.barrier.h = 4.00
path.receiver.pos.x = 500.00
barrier2m500m = cnossos_full(method, path, {}, meteo, {});