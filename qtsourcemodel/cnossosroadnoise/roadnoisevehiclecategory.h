#ifndef ROADNOISEVEHICLECATEGORY_H
#define ROADNOISEVEHICLECATEGORY_H

#include <roadnoiseconst.h>
#include <roadnoiseaux.h>
#include <QtCore>

//using namespace CNOSSOS_ROADNOISE;



class RoadNoiseVehicleCategory
{
public:
  string	id;
  string	description;
  bool	calcNoise[NUM_NOISE_GENERATORS];
  bool	calcStudded;
  double	Ksurface[MAX_FREQ_BAND_CENTRE];
  double coefficientA[NUM_NOISE_GENERATORS][MAX_FREQ_BAND_CENTRE];
  double coefficientB[NUM_NOISE_GENERATORS][MAX_FREQ_BAND_CENTRE];
  double speedVariationCoefficient[MAX_SPEED_VARIATION_TYPES][NUM_NOISE_GENERATORS];

  StuddedCategory		*studdedProps;
  GradientCategory	*gradientCorrection;

  RoadNoiseVehicleCategory();
  ~RoadNoiseVehicleCategory();
};

#endif // ROADNOISEVEHICLECATEGORY_H
