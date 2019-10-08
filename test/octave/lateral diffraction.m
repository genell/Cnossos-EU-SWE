%
% all lateral diffraction tests
%

method = "ISO-9613-2"

options.CheckHeightUpperBound = false
options.CheckHeightLowerBound = false
options.ExcludeSoundPower = false
options.ForceSourceToReceiver = false

meteo.model = "ISO-9613-2"
meteo.temperature = 20.0
meteo.humidity = 60.0
meteo.pFav = 0.50
meteo.C0 = 3.00

% user defined material propertes
materials.UserDefined.G = 0.9
materials.UserDefined.sigma = 250
materials.UserDefined.alpha = [0.10 0.20 0.30 0.40 0.50 0.60 0.70 0.80]
% for testing only (change G value for predefined ground type "H"
materials.H.G = 0.01
% for testing only (change absorption value for predefined type "A3"
materials.A3.G = 1.0
materials.A3.alpha = [0.35 0.50 0.75 0.85 0.95 0.99 0.99 0.95]

% sourcer
path.source.pos.x = 0
path.source.pos.y = 0
path.source.pos.z = 0
path.source.mat = "H"
path.source.source.h = 0.50
path.source.source.Lw.spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
path.source.source.Lw.sourceType = "PointSource"
path.source.source.Lw.measurementType = "HemiSpherical"
path.source.source.Lw.frequencyWeighting = "LIN"

% diffracting edge #1
path.edge1.pos.x = 10.00
path.edge1.pos.y = 2.00
path.edge1.pos.z = 0.00
path.edge1.mat = "H"
path.edge1.edge.h = 3.00

% diffracting edge #2
path.edge2.pos.x = 20.00
path.edge2.pos.y = 2.00
path.edge2.pos.z = 0.00
path.edge2.mat = "H"
path.edge2.edge.h = 3.00

% receiver, groundType = D
path.receiver.pos.x = 50.00
path.receiver.pos.y = 0.00
path.receiver.pos.z = 0.00
path.receiver.mat = "H"
path.receiver.receiver.h = 8.5

%
% Tests
%

% lateral diffraction
path.receiver.pos.y = 0.00
path.receiver.receiver.h = 2.5
latdiff = cnossos_full(method, path, options, meteo, materials);

% lateral diffraction too-high
path.receiver.pos.y = 0.00
path.receiver.receiver.h = 8.5
latdifftoohigh = cnossos_full(method, path, options, meteo, materials);

% lateral diffraction non-convex
path.receiver.pos.y = 5.00
path.receiver.receiver.h = 2.5
latdiffnonconvex = cnossos_full(method, path, options, meteo, materials);
