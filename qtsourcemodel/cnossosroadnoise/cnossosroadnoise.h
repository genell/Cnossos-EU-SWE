#ifndef CNOSSOSROADNOISE_H
#define CNOSSOSROADNOISE_H

#include "cnossosroadnoise_global.h"
#include "roadnoisesegment.h"

class CNOSSOSROADNOISESHARED_EXPORT CnossosRoadNoise
{

public:
  CnossosRoadNoise();

  int InitDLL();
  int ReleaseDLL( void );
  int CalcFromFile(string infile, string outfile);

private:
  char const* greet();

  int CalcSegment(void);
  void SetTemperatureProperties(const double temperature);
  void SetGradientProperties(const double gradient);
  void SetAccelerationProperties(const double distance, const int junctionType);
  void SetStuddedMonths(const int months);
  void SetSpeed(const int cat, const double speed);
  void SetTraffic(const int cat, const double amount);
  string getDllPath();
  void WriteToXMLFile(const string fn);
};

// static pointer containing the actual RoadNoiseSegment object
static RoadNoiseSegment* currentSegment;


#endif // CNOSSOSROADNOISE_H
