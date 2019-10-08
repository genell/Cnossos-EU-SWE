% same as "barrier 1m - 20m.xml"
method = "ISO-9613-2";
meteo = {
  model = "DEFAULT"
  temperature = 15.0
  humidity = 70.0
  pFav = 0.50
  C0 = 3.00
};
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
};

result = cnossos_full(method, path, {}, meteo, {});
