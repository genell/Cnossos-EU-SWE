<?xml version="1.0" encoding="UTF-8" ?>

% same as "area source.xml"
method = "JRC-2012"
options = {
	CheckSoundPowerUnits = false
}
meteo = {
  model = "ISO-9613-2"
  temperature = 20.0
  humidity = 60.0
  pFav = 0.50
  C0 = 3.00
}
materials = {
	% user defined material propertes
	UserDefined = {
		G = 1.0
		sigma = 50
	}
	% for testing only (change G value for predefined ground type "H"
	H = {
		G = 0.05
	}
	% for testing only (change absorption value for predefined type "A3"
	A3 = {
		G = 1.0
		alpha = [0.35 0.50 0.75 0.85 0.95 0.99 0.99 0.95]
	}
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
			% <import file="CNOSSOS_Road_Output.xml" /> No import support yet
			% source geometry in the horizontal plane = segment from Y=-10 to Y=+10m
			areaSource = {
				area = 10.0
				orientation = {
					x = 1.0
					y = 0.0
					z = 0.0
				}
			}


      h = 0.05

      Lw = {
        spectrum = [80.0 90.0 95.0 100.0 100.0 100.0 95.0 90.0]
        sourceType = "PointSource"
        measurementType = "HemiSpherical"
        frequencyWeighting = "LIN"
      }
    }
  }
  % border of hard platform at x = 7.5m, groundType = H
  border = {
    pos = {
      x = 7.50
      z = 0.00
    }
    mat = "H"
  }
  % berm, 7.5m width, 2.5m high
  berm = {
    pos = {
      x = 10.00
      z = 2.50
    }
    mat = "H"
  }
	ground1 = {
    pos = {
      x = 12.50
      z = 2.50
    }
    mat = "UserDefined"
  }
	ground1 = {
    pos = {
      x = 15.00
      z = 0.00
    }
    mat = "UserDefined"
  }
	% receiver at x = 100m, h = 4m, groundType = UserDefined
	receiver = {
    pos = {
      x = 100.00
      z = 0.00
    }
    mat = "D"
		receiver = {
			h = 4.0
		}
  } 
}

result = cnossos_full(method, path, {}, meteo, {})
