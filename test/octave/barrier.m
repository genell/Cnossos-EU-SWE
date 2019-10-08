%
% all barrier tests
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
path.p0_source.source.h = 0.05
path.p0_source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.p0_source.source.Lw.sourceType = "PointSource"
path.p0_source.source.Lw.measurementType = "HemiSpherical"
path.p0_source.source.Lw.frequencyWeighting = "LIN"

% barrier at the border of hard platform, groundType = H
path.p1_barrier.pos.x = 10.0
path.p1_barrier.pos.z = 0.0
path.p1_barrier.mat = "H"
path.p1_barrier.barrier.h = 1.00
path.p1_barrier.barrier.mat = "A0"

% receiver, groundType = D
path.p2_receiver.pos.x = 20.00
path.p2_receiver.pos.z = 0.00
path.p2_receiver.mat = "D"
path.p2_receiver.receiver.h = 2.5

%
% Tests
%

% barrier 1m - 20m
path.p1_barrier.barrier.h = 1.00
path.p2_receiver.pos.x = 20.00
barrier1m20m = cnossos_full(method, path, options, meteo, materials);

% barrier 1m - 50m
path.p1_barrier.barrier.h = 1.00
path.p2_receiver.pos.x = 50.00
barrier1m50m = cnossos_full(method, path, options, meteo, materials);

% barrier 1m - 100m
path.p1_barrier.barrier.h = 1.00
path.p2_receiver.pos.x = 100.00
barrier1m100m = cnossos_full(method, path, options, meteo, materials);

% barrier 1m - 200m
path.p1_barrier.barrier.h = 1.00
path.p2_receiver.pos.x = 200.00
barrier1m200m = cnossos_full(method, path, options, meteo, materials);

% barrier 1m - 500m
path.p1_barrier.barrier.h = 1.00
path.p2_receiver.pos.x = 500.00
barrier1m500m = cnossos_full(method, path, options, meteo, materials);

% barrier 2m - 20m
path.p1_barrier.barrier.h = 2.00
path.p2_receiver.pos.x = 20.00
barrier2m20m = cnossos_full(method, path, options, meteo, materials);

% barrier 2m - 50m
path.p1_barrier.barrier.h = 2.00
path.p2_receiver.pos.x = 50.00
barrier2m50m = cnossos_full(method, path, options, meteo, materials);

% barrier 2m - 100m
path.p1_barrier.barrier.h = 2.00
path.p2_receiver.pos.x = 100.00
barrier2m100m = cnossos_full(method, path, options, meteo, materials);

% barrier 2m - 200m
path.p1_barrier.barrier.h = 2.00
path.p2_receiver.pos.x = 200.00
barrier2m200m = cnossos_full(method, path, options, meteo, materials);

% barrier 2m - 500m
path.p1_barrier.barrier.h = 2.00
path.p2_receiver.pos.x = 500.00
barrier2m500m = cnossos_full(method, path, options, meteo, materials);

% barrier 4m - 20m
path.p1_barrier.barrier.h = 4.00
path.p2_receiver.pos.x = 20.00
barrier4m20m = cnossos_full(method, path, options, meteo, materials);

% barrier 4m - 50m
path.p1_barrier.barrier.h = 4.00
path.p2_receiver.pos.x = 50.00
barrier4m50m = cnossos_full(method, path, options, meteo, materials);

% barrier 4m - 100m
path.p1_barrier.barrier.h = 4.00
path.p2_receiver.pos.x = 100.00
barrier4m100m = cnossos_full(method, path, options, meteo, materials);

% barrier 4m - 200m
path.p1_barrier.barrier.h = 4.00
path.p2_receiver.pos.x = 200.00
barrier4m200m = cnossos_full(method, path, options, meteo, materials);

% barrier 4m - 500m
path.p1_barrier.barrier.h = 4.00
path.p2_receiver.pos.x = 500.00
barrier4m500m = cnossos_full(method, path, options, meteo, materials);