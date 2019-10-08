%
% area source test
%

clear;

method = "JRC-2012";

options.CheckSoundPowerUnits = false;

meteo.model = "ISO-9613-2";
meteo.temperature = 20.0;
meteo.humidity = 60.0;
meteo.pFav = 0.50;
meteo.C0 = 3.00;

% user defined material propertes
materials.UserDefined.G = 1.0;
materials.UserDefined.sigma = 50;
% for testing only (change G value for predefined ground type "H"
materials.H.G = 0.05;
% for testing only (change absorption value for predefined type "A3"
materials.A3.G = 1.0;
materials.A3.alpha = [0.35 0.50 0.75 0.85 0.95 0.99 0.99 0.95];

% sourcer
path.p0_source.pos.x = 0;
path.p0_source.pos.y = 0;
path.p0_source.pos.z = 0;
path.p0_source.mat = "H";
path.p0_source.source.h = 0.05;
path.p0_source.source.Lw.spectrum = [100.5 96.6 97.9 96.6 92.1 89.1 85.3 81.9];
path.p0_source.source.Lw.sourceType="LineSource";
path.p0_source.source.Lw.measurementType="HemiSpherical";
path.p0_source.source.Lw.frequencyWeighting="LIN";
% source geometry in the horizontal plane = segment from Y=-10 to Y=+10m
path.p0_source.source.areaSource.area = 10.0;
path.p0_source.source.areaSource.orientation.x = 1.0;
path.p0_source.source.areaSource.orientation.y = 0.0;
path.p0_source.source.areaSource.orientation.z = 0.0;

% border of hard platform at x = 7.5m, groundType = H
path.p1_border.pos.x = 7.50;
path.p1_border.pos.z = 0.00;
path.p1_border.mat = "H";

% berm, 7.5m width, 2.5m high
path.p2_berm.pos.x = 10.00;
path.p2_berm.pos.z = 2.50;
path.p2_berm.mat = "H";

path.p3_ground1.pos.x = 12.50;
path.p3_ground1.pos.z = 2.50;
path.p3_ground1.mat = "UserDefined";

path.p4_ground2.pos.x = 15.00;
path.p4_ground2.pos.z = 0.00;
path.p4_ground2.mat = "UserDefined";

% receiver at x = 100m, h = 4m, groundType = UserDefined
path.p5_receiver.pos.x = 100.00;
path.p5_receiver.pos.z = 0.00;
path.p5_receiver.mat = "D";
path.p5_receiver.receiver.h = 4.0;

areasource = cnossos_full(method, path, options, meteo, materials);
%help cnossos_full