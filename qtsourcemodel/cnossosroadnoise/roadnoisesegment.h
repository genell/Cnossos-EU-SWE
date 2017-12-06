#ifndef ROADNOISESEGMENT_H
#define ROADNOISESEGMENT_H

#include "roadnoisecatalog.h"
#include "roadnoisesegment.h"


class RoadNoiseSegment
{
public:
private:
  RoadNoiseCatalog *catalog;

  // Calculation function for each term
  double CalcRollingNoise(const int cat, const int freq);
  double CalcPropulsionNoise(const int cat, const int freq);

  double CalcRollingNoiseRoadSurfaceCorrection(const int cat, const int freq);
  double CalcPropulsionNoiseRoadSurfaceCorrection(const int cat, const int freq);
  double CalcTyreCorrection(const int cat, const int freq);
  double CalcAccCorrectionPropulsion(const int cat, const int freq);
  double CalcAccCorrectionRolling(const int cat, const int freq);
  double CalcTempCorrection(const int cat, const int freq);
  double CalcGradientCorrection(const int cat,const int freq);


public:
  bool doDebug;

  // Speed and traffic intensities
  double v[MAX_SRC_CAT];
  double Q[MAX_SRC_CAT];

  // Studded tyre fractions and months of use of studded tyres
  double	Fstud[MAX_SRC_CAT];
  int		studdedMonths;

  // calculation properties
  bool calcRollingNoise[MAX_SRC_CAT];
  bool calcPropNoise[MAX_SRC_CAT];

  // tables containing intermediate results
  double LwimTotal[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double LWeqlineimFactor[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double TotalRollingNoise[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double RollingNoiseFactor[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double DeltaRollingNoiseFactor[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double DeltaRollingRoadCorrection[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double DeltaRollingTyreCorrection[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double DeltaRollingAccCorrection[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double DeltaRollingTempCorrection[MAX_SRC_CAT];

  double TotalPropulsionNoise[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double PropulsionNoiseFactor[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double DeltaPropulsionNoiseFactor[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double DeltaPropulsionRoadCorrection[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double DeltaPropulsionAccCorrection[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double DeltaPropulsionGradientCorrection[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];

  // final result
  double Spec[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
  double TotalSpec[MAX_FREQ_BAND_CENTRE];

  // optional property classes
  AccelerationProperties* AccProp;
  TempProperties* TempProp;
  GradientProperties* GradProp;
  RoadSurface *SurfaceProp;

  // default constructor and destructor
  RoadNoiseSegment(RoadNoiseCatalog *catalog);
  ~RoadNoiseSegment();

  // function to populate the road segment from file
  bool loadFromXMLFile(const string fn);
  bool saveResultsToXMLFile(const string fn);
  bool writeDebugData(const string debugfile);

  // property setter for road surface
  RoadSurface* setSurfaceID(const string id);

  // aux function for printing the RoadNoiseSegment object to screen
  void PrintSegment();

  // calculate the source power of the current road segment
  int CalcSegment();
};

#endif // ROADNOISESEGMENT_H
