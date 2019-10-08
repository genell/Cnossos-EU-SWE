# Cnossos plugin for GNU Octave - user guide

## cnossos_flat

_`cnossos_flat(M, HS, D1, D, HR, I, [L1..L8])`_

* `M` Calculation method. Valid values are `"CNOSSOS-2018"`, `"ISO-9613-2"`, `"JRC-2012"` or `"JRC-DRAFT-2010"`
* `HS` Source height
* `D1` Road distance
* `D` Receiver distance
* `HR` Receiver height
* `I` Receiver material
* `L1..L8` Sound power spectrum

### Example

```matlab
cnossos_flat("JRC-2012", 0.5, 5, 100, 2, "C", [0 0 0 0 0 0 0 0])
```

## cnossos_full

_`cnossos_full(M, path, [options], [meteo], [materials])`_

* `M` Calculation method. Valid values are `"CNOSSOS-2018"`, `"ISO-9613-2"`, `"JRC-2012"` or `"JRC-DRAFT-2010"`
* `path` Control points. Struct type
* `options` _(optional)_ Calculation options. Struct type
* `meteo` _(optional)_ Meteorological conditions. Struct type
* `materials` _(optional)_ Material definitions. Struct type

### path

Path is a struct where the members are named control point definitions. The member name is the control point name. There must be at least 2 control point definitions in the path.

> NOTE: CNOSSOS requires the points to be ordered in increasing distance, but Octave sorts the struct members in alphabetical order. Therefore, the control point names must be named or prefixed to satisfy the CNOSSOS ordering requirement - for example: `p0_source`, `p1_border`, `p2_receiver`.

A control point consists of a position, a material reference and an extension definition.
The extension definition can be any of `source`, `receiver`, `barrier`, `wall` or `edge`

```matlab
% control point position
cp0.pos.x = 10 %x-coordinate (double)
cp0.pos.y = 20 %y-coordinate (double)
cp0.pos.z = 30 %z-coordinate (double)

%material reference
cp0.mat = "H"

%extension structure - one of source/receiver/barrier/wall/edge
cp0.<ext>..
```

#### source

* `h` (double) (mandatory) height
* `Lw` (struct) (optional) sound power spectrum
  * `spectrum` (array[8] of double)
  * `measurementType` (string) one of `"FreeField"`, `"HemiSpherical"`
  * `sourceType` (string) one of `"PointSource"`, `"LineSource"`, `"AreaSource"`
  * `frequencyWeighting` (string) one of `"LIN"`, `"dBA"`
* `<geo>` (struct) one of:
  * `pointSource`
    * `orientation` (struct) (mandatory)
      * `x` (double)
      * `y` (double)
      * `z` (double)
  * `lineSource`
    * `length` (double) (mandatory)
    * `orientation` (struct) (mandatory)
      * `x` (double)
      * `y` (double)
      * `z` (double)
  * `areaSource`
    * `area` (double) (mandatory)
    * `orientation` (struct) (mandatory)
      * `x` (double)
      * `y` (double)
      * `z` (double)
  * `lineSegment`
    * `posStart` (struct) (mandatory)
      * `x` (double)
      * `y` (double)
      * `z` (double)
    * `posEnd` (struct) (mandatory)
      * `x` (double)
      * `y` (double)
      * `z` (double)
    * `fixedAngle` (double) (mandatory)

```matlab
%a source control point (p0_src)

%position
p0_src.pos.x = 10 %x-coordinate (double)
p0_src.pos.y = 20 %y-coordinate (double)
p0_src.pos.z = 30 %z-coordinate (double)

%material reference
p0_src.mat = "H"

%extension, type "source"
p0_src.source.h = 10
%extension geometry (line)
p0_src.source.lineSource.length = 100
p0_src.source.lineSource.orientation.x = 10
p0_src.source.lineSource.orientation.y = 20
p0_src.source.lineSource.orientation.z = 30
```

#### receiver

* `h` (double) (mandatory) height

```matlab
% a receiver control point (p0_rcv)

% position
p0_rcv.pos.x = 10 %x-coordinate (double)
p0_rcv.pos.y: 20 %y-coordinate (double)
p0_rcv.pos.z: 30 %z-coordinate (double)

%material reference
p0_rcv.pos.mat = "H"

%extension, type "receiver"
p0_rcv.receiver.h = 10
```

#### barrier

* `h` (double) (mandatory) height
* `mat` (string) material reference (optional)

```matlab
%a barrier control point (p0_barr)

% position
p0_barr.pos.x = 10 %x-coordinate (double)
p0_barr.pos.y = 20 %y-coordinate (double)
p0_barr.pos.z = 30 %z-coordinate (double)

%material reference
p0_barr.mat = "H"

%extension, type "barrier"
p0_barr.barrier.h = 10
p0_barr.barrier.mat = "A0" %material reference
```

#### wall

* `h` (double) (mandatory) height
* `mat` (string) material reference (optional)

```matlab
%a wall control point (cp0_wall)

%position
cp0_wall.pos.x = 10 %x-coordinate (double)
cp0_wall.pos.y = 20 %y-coordinate (double)
cp0_wall.pos.z = 30 %z-coordinate (double)

%material reference
cp0_wall.mat = "H"

%extension, type "wall"
cp0_wall.wall.h = 10
cp0_wall.wall.mat = "A0" %material reference
```

#### edge

* `h` (double) (mandatory) height

```matlab
%an edge control point (cp0_edge)

%position
cp0_edge.pos.x = 10 %x-coordinate (double)
cp0_edge.pos.y = 20 %y-coordinate (double)
cp0_edge.pos.z = 30 %z-coordinate (double)

%material reference
cp0_edge.mat = "H"

%extension, type "edge"
cp0_edge.edge.h = 10
```

### options

Options parameter is a struct with the following valid members. All option parameters are optional and of type bool.

* `CheckHeightLowerBound` (boolean)
* `CheckHeightUpperBound` (boolean)
* `CheckHorizontalAlignment` (boolean)
* `CheckLateralDiffraction` (boolean)
* `CheckSoundPowerUnits` (boolean)
* `CheckSourceSegment` (boolean)
* `DisableLateralDiffractions` (boolean)
* `DisableReflections` (boolean)
* `ExcludeAirAbsorption` (boolean)
* `ExcludeGeometricalSpread` (boolean)
* `ExcludeSoundPower` (boolean)
* `ForceSourceToReceiver` (boolean)
* `IgnoreComplexPaths` (boolean)
* `SimplifyPathGeometry` (boolean)

Example:

```matlab
options.CheckHeightLowerBound = false
options.CheckHeightUpperBound = false
options.ExcludeAirAbsorption = false
options.ExcludeGeometricalSpread = true
options.ExcludeSoundPower = true
options.SimplifyPathGeometry = true
```

### meteo

Meteo is a struct with the following members:

* `model` (string) Valid values are `"ISO-9613-2"`, `"JRC-2012"` and `"NMPB-2008"`
* `temperature` (double)
* `humidity` (double)
* `pFav` (double)
* `C0` (double)

Example:

```matlab
meteo.model = "NMPB-2008"
meteo.temperature = 23.0
meteo.humidity = 0.0
meteo.pFav = 0.0
meteo.C0 = 0.0
```

### materials

Materials is a struct where the members are named material definitions. The member name is the material name

```matlab
materials.materialA..
materials.materialB..
materials.another_material..
```

The material definitions contain the following members:

* `G` (double)
* `sigma` (double)
* `alpha` (spectrum - Array[8] of double)

Example:

```matlab
%material "H"
materials.H.G = 1.0
materials.H.sigma = 250.0
materials.H.alpha = [0,0,0,0,0,0,0,0]

%material "M"
materials.M.G = 0
materials.M.sigma = 2500.0
materials.M.alpha = [0,0,0,0,0,0,0,0]
```

## Complete examples

```matlab
% same as "barrier 1m - 20m.xml"

method = "ISO-9613-2"

options.dummy = true; % we need a dummy placeholder for the options struct

meteo.model = "DEFAULT"
meteo.temperature = 15.0
meteo.humidity = 70.0
meteo.pFav = 0.50
meteo.C0 = 3.00

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

barrier1m20m = cnossos_full(method, path, options, meteo); % the last parameter; "materials" can be omitted
```
