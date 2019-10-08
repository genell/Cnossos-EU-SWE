%
% line source test
%

method = "JRC-2012"

options.CheckSoundPowerUnits = false

meteo.model = "ISO-9613-2"
meteo.temperature = 20.0
meteo.humidity = 60.0
meteo.pFav = 0.50
meteo.C0 = 3.00

% user defined material propertes
materials.UserDefined.G = 1.0
materials.UserDefined.sigma = 50
% for testing only (change G value for predefined ground type "H"
materials.H.G = 0.05
% for testing only (change absorption value for predefined type "A3"
materials.A3.G = 1.0
materials.A3.alpha = [0.35 0.50 0.75 0.85 0.95 0.99 0.99 0.95]

% sourcer
path.source.pos.x = 0
path.source.pos.y = 0
path.source.pos.z = 0
path.source.mat = "H"
path.source.source.h = 0.05
path.source.source.Lw.spectrum = [100.5 96.6 97.9 96.6 92.1 89.1 85.3 81.9]
path.source.source.Lw.sourceType = "LineSource"
path.source.source.Lw.measurementType = "HemiSpherical"
path.source.source.Lw.frequencyWeighting = "LIN"
path.source.source.lineSource.length = 10.0
path.source.source.lineSource.orientation.x = 1.0
path.source.source.lineSource.orientation.y = 0.0
path.source.source.lineSource.orientation.z = 0.0

% border of hard platform at x = 7.5m, groundType = H
path.border.pos.x = 7.50
path.border.pos.z = 0.00
path.border.mat = "H"

% berm, 7.5m width, 2.5m high
path.berm.pos.x = 10.00
path.berm.pos.z = 2.50
path.berm.mat = "H"

path.ground1.pos.x = 12.50
path.ground1.pos.z = 2.50
path.ground1.mat = "UserDefined"

path.ground2.pos.x = 15.00
path.ground2.pos.z = 0.00
path.ground2.mat = "UserDefined"

% receiver at x = 100m, h = 4m, groundType = UserDefined
path.receiver.pos.x = 100.00
path.receiver.pos.z = 0.00
path.receiver.mat = "D"
path.receiver.receiver.h = 4.0

linesource = cnossos_full(method, path, options, meteo, materials)
