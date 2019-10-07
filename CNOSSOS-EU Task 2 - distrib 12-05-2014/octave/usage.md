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

  print_debug("path.cp[%d]\n", path.cp.size());
  print_debug("path.info.nbDiffractions=%d\n", path.info.nbDiffractions);
  print_debug("path.info.nbLateralDiffractions=%d\n", path.info.nbLateralDiffractions);
  print_debug("path.info.nbReflections=%d\n", path.info.nbReflections);
  print_debug("path.info.pathType=%d\n", path.info.pathType);

Materials is a struct where the members are named control point definitions. The member name is the control point name. There must be at least 2 control point definitions in the path.

A control point consists of a position, a material reference and an extension definition.
The extension definition can be any of `source`, `receiver`, `barrier`, `wall` or `edge`

```matlab
%control point structure
cp =
{
  %position structure
  pos =
    {
      x: 10 %x-coordinate (double)
      y: 20 %y-coordinate (double)
      z: 30 %z-coordinate (double)
    }
  %material reference
  mat = "H"
  %extension structure - one of source/receiver/barrier/wall/edge
  <ext> = { .. }
}
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
%a source control point
src =
{
  %position structure
  pos =
    {
      x: 10 %x-coordinate (double)
      y: 20 %y-coordinate (double)
      z: 30 %z-coordinate (double)
    }
  %material reference
  mat = "H"
  %extension
  source =
    {
      h = 10
      %line geometry
      lineSource =
        {
          length = 100
          orientation =
            {
              x = 10
              y = 20
              z = 30
            }
        }
    }
}
```

#### receiver

* `h` (double) (mandatory) height

```matlab
%a receiver control point
rcv =
{
  %position structure
  pos =
    {
      x: 10 %x-coordinate (double)
      y: 20 %y-coordinate (double)
      z: 30 %z-coordinate (double)
    }
  %material reference
  mat = "H"
  %extension
  receiver =
    {
      h = 10
    }
}
```

#### barrier

* `h` (double) (mandatory) height
* `mat` (string) material reference (optional)

```matlab
%a barrier control point
barr =
{
  %position structure
  pos =
    {
      x: 10 %x-coordinate (double)
      y: 20 %y-coordinate (double)
      z: 30 %z-coordinate (double)
    }
  %material reference
  mat = "H"
  %extension
  barrier =
    {
      h = 10
      %material reference
      mat = "A0"
    }
  }
```

#### wall

* `h` (double) (mandatory) height
* `mat` (string) material reference (optional)

```matlab
%a wall control point
cp_wall =
{
  %position structure
  pos =
    {
      x: 10 %x-coordinate (double)
      y: 20 %y-coordinate (double)
      z: 30 %z-coordinate (double)
    }
  %material reference
  mat = "H"
  %extension
  wall =
    {
      h = 10
      %material reference
      mat = "A0"
    }
  }
```

#### edge

* `h` (double) (mandatory) height

```matlab
%an edge control point
cp_edge =
{
  %position structure
  pos =
    {
      x: 10 %x-coordinate (double)
      y: 20 %y-coordinate (double)
      z: 30 %z-coordinate (double)
    }
  %material reference
  mat = "H"
  %extension
  edge =
    {
      h = 10
    }
  }
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
options =
  {
    CheckHeightLowerBound = false
    CheckHeightUpperBound = false
    ExcludeAirAbsorption = false
    ExcludeGeometricalSpread = true
    ExcludeSoundPower = true
    SimplifyPathGeometry = true
  }
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
meteo =
  {
    model = "NMPB-2008"
    temperature = 23.0
    humidity = 0.0
    pFav = 0.0
    C0 = 0.0
  }
```

### materials

Materials is a struct where the members are named material definitions. The member name is the material name

```matlab
materials =
  {
    materialA = { .. }
    materialB = { .. }
    another_material = { .. }
  }
```

The material definitions contain the following members:

* `G` (double)
* `sigma` (double)
* `alpha` (spectrum - Array[8] of double)

Example:

```matlab
materials =
  {
    H =
      {
        G = 1.0
        sigma = 250.0
        alpha = [0,0,0,0,0,0,0,0]
      }
    M =
      {
        G = 0
        sigma = 2500.0
        alpha = [0,0,0,0,0,0,0,0]
      }
  }
```

## Complete examples

```matlab
% same as "barrier 1m - 20m.xml"
method = "ISO-9613-2"
meteo = {
  model = "DEFAULT"
  temperature = 15.0
  humidity = 70.0
  pFav = 0.50
  C0 = 3.00
}
path = {
  % sourcer
  source = {
    pos = {
      x = 0
      y = 0
      z = 0
    }
    mat = "H"
    source = {
      h = 0.05
      Lw = {
        spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
        sourceType = "PointSource"
        measurementType = "HemiSpherical"
        frequencyWeighting = "LIN"
      }
    }
  }
  % barrier at the border of hard platform at x = 10m, groundType = H
  barrier = {
    pos = {
      x = 10.0
      z = 0.0
    }
    mat = "H"
    barrier = {
      h = 1.00
      mat = "A0"
    }
  }
  % receiver at x = 20m, groundType = D
  receiver = {
    pos = {
      x = 20.00
      z = 0.00
    }
    mat = "D"
    receiver = {
      h = 2.5
    }
  }
}

exec cnossos_full(method, path, {}, meteo, {})
```
