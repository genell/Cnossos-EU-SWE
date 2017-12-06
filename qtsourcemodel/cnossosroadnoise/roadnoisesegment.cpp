#include "roadnoisesegment.h"
#include "roadnoiseconst.h"

#include <iomanip>

//using namespace CNOSSOS_ROADNOISE;

RoadNoiseSegment::RoadNoiseSegment(RoadNoiseCatalog *catalog)
{
  this->catalog = catalog;

  doDebug = false;

  for (int m=0;m < MAX_SRC_CAT; m++)
  {
    RoadNoiseVehicleCategory *cat = catalog->getCategory(m);
    if (cat != NULL)
    {
      this->calcPropNoise[m] = cat->calcNoise[ngPROPULSION];
      this->calcRollingNoise[m] = cat->calcNoise[ngROLLING];
    }
    else
    {
      this->calcPropNoise[m] = false;
      this->calcRollingNoise[m] = false;
    }

    this->v[m] = 0;
    this->Q[m] = 0;
    this->Fstud[m] = 0;

    for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
    {
      this->Spec[m][i] = 0;
      this->LwimTotal[m][i] = 0;
      this->LWeqlineimFactor[m][i] = 0;
      this->TotalRollingNoise[m][i] = 0;
      this->RollingNoiseFactor[m][i] =0;
      this->DeltaRollingNoiseFactor[m][i] = 0;
      this->DeltaRollingRoadCorrection[m][i] = 0;
      this->DeltaRollingTyreCorrection[m][i] =0;
      this->DeltaRollingAccCorrection[m][i] = 0;


      this->TotalPropulsionNoise[m][i] =0;
      this->PropulsionNoiseFactor[m][i] =0;
      this->DeltaPropulsionNoiseFactor[m][i] = 0;
      this->DeltaPropulsionRoadCorrection[m][i] = 0;
      this->DeltaPropulsionAccCorrection[m][i] =0;
      this->DeltaPropulsionGradientCorrection[m][i] = 0;
    }
    this->DeltaRollingTempCorrection[m] =0;
  }

  this->AccProp = NULL;
  this->TempProp = NULL;
  this->GradProp = NULL;
  this->SurfaceProp = NULL;
}

// Releases all created objects
RoadNoiseSegment::~RoadNoiseSegment()
{
  if (AccProp != NULL)
    delete AccProp;
  if (TempProp != NULL)
    delete TempProp;
  if (GradProp != NULL)
    delete GradProp;
  this->SurfaceProp = NULL;
}


// Retrieves the relevant surface class
RoadSurface* RoadNoiseSegment::setSurfaceID(const string id)
{
  for (list<RoadSurface>::iterator rsi = this->catalog->surfaces.begin();
       rsi != this->catalog->surfaces.end(); ++rsi)
  {
    if (id.compare(rsi->id) == 0)
    {
      SurfaceProp = &*rsi;
      return SurfaceProp;
    }
  }
  return NULL;
}

#pragma endregion Meta_Debug_functions

#pragma region RollingNoise_Calculation_functions

// Calculate the road surface correction for vehicle category "cat" and frequency band "freq"
// Formula III-19
double RoadNoiseSegment::CalcRollingNoiseRoadSurfaceCorrection(const int cat, const int freq)
{
  // Formula III-19
  double Alpha = this->SurfaceProp->coefficientA[cat][freq];
  double Beta = this->SurfaceProp->coefficientB[cat];
  DeltaRollingRoadCorrection[cat][freq] = Alpha + Beta * log10(this->v[cat] / catalog->refSpeed);
  return DeltaRollingRoadCorrection[cat][freq];
}

// Calculate the studded tyre correction category for vehicle category "cat" and frequency band "freq"
// Formula III-7, III-8 and III-9
double RoadNoiseSegment::CalcTyreCorrection(const int m, const int i)
{
  RoadNoiseVehicleCategory *cat = catalog->getCategory(m);
  if (this->studdedMonths > 0 && cat->calcStudded)
  {
    double Alpha = cat->studdedProps->studded[cfAlpha][i];
    double Beta = cat->studdedProps->studded[cfBeta][i];
    double delta_stud_im = 0.0;
    double v1 = this->v[m];

    // Formula III-7
    if (v1 < 50)
      delta_stud_im = Alpha + Beta * log10(50 / catalog->refSpeed);
    else if (v1 >= 50 && v1 <= 90)
      delta_stud_im = Alpha + Beta * log10(v1 / catalog->refSpeed);
    else
      delta_stud_im = Alpha + Beta * log10(90 / catalog->refSpeed);

    // Formula III-8
    double ps = this->Fstud[m] * ((double)this->studdedMonths / 12);

    // Formula III-9
    double studded_tyres_im = 10 * log10((1 - ps) + ps * pow(10,(delta_stud_im / 10)) );
    DeltaRollingTyreCorrection[m][i] = studded_tyres_im;
  }
  else
    DeltaRollingTyreCorrection[m][i] = 0.0;
  return DeltaRollingTyreCorrection[m][i];
}

// Calculate the rolling noise acceleration correction for vehicle category "m" and frequency band "i"
// Formula III-17
double RoadNoiseSegment::CalcAccCorrectionRolling(const int m, const int i)
{
  // if correction
  RoadNoiseVehicleCategory *cat = catalog->getCategory(m);
  if (this->AccProp != NULL)
  {
    // Formula III-17
    double Cfactor = cat->speedVariationCoefficient[this->AccProp->K][ngROLLING];
    //DeltaRollingAccCorrection[m][i] = Cfactor * max(1 - (abs(this->AccProp->distance) / 100),0);
    //TODO:
    DeltaRollingAccCorrection[m][i] = 0;
  }
  else
    DeltaRollingAccCorrection[m][i] = 0;
  return DeltaRollingAccCorrection[m][i];
}

// Calculate the rolling noise accelerationtemperature correction for each vehicle category "m"
// Formula III-10
double RoadNoiseSegment::CalcTempCorrection(const int m, const int i)
{
  if (this->TempProp != NULL)
  {
    RoadNoiseVehicleCategory *cat = catalog->getCategory(m);
    double K = cat->Ksurface[i];
    // Formula III-10
    DeltaRollingTempCorrection[m] = K * (catalog->refTemp - this->TempProp->t);

  }
  return DeltaRollingTempCorrection[m];
}

// Calculate the total rolling noise for each vehicle category "m" and frequency "i"
// Formula III-5, III-6
double RoadNoiseSegment::CalcRollingNoise(const int m, const int i)
{
  RoadNoiseVehicleCategory *cat = catalog->getCategory(m);
  // Formula III-5
  double alphaFactor = cat->coefficientA[ngROLLING][i];
  double betaFactor =  cat->coefficientB[ngROLLING][i];
  double logFactor = log10(this->v[m] / catalog->refSpeed);
  double LWR = alphaFactor + betaFactor * logFactor;

  RollingNoiseFactor[m][i] = LWR;

  // Formula III-6
  double deltaRoadSurface = CalcRollingNoiseRoadSurfaceCorrection(m, i);
  double deltaStudded = CalcTyreCorrection(m, i);
  double deltaAcc = CalcAccCorrectionRolling(m,i);
  double deltaTemp = CalcTempCorrection(m, i);
  //double deltaTemp =
  double deltaRollingNoise = deltaRoadSurface + deltaStudded + deltaAcc + deltaTemp;
  DeltaRollingNoiseFactor[m][i] = deltaRollingNoise;
  TotalRollingNoise[m][i] = LWR + deltaRollingNoise;
  return TotalRollingNoise[m][i];
}
#pragma endregion RollingNoise_Calculation_functions

#pragma region Propagation_Calculation_functions
// Calculate the road surface correction for vehicle category "m" and frequency band "i"
// Formula III-20
double RoadNoiseSegment::CalcPropulsionNoiseRoadSurfaceCorrection(const int m, const int i)
{
  // Formula III-20
  double Alpha = this->SurfaceProp->coefficientA[m][i];
  //DeltaPropulsionRoadCorrection[m][i] = min(Alpha,0);
  //TODO:
  DeltaPropulsionRoadCorrection[m][i] = 0;
  return DeltaPropulsionRoadCorrection[m][i];
}


// Calculate the gradient correction for each vehicle category "m" and frequency "i"
// Formula III-13, III-14, III-15, III-16
double RoadNoiseSegment::CalcGradientCorrection(const int m,const int i)
{
  double value = 0.0;
  if (this->GradProp != NULL)
  {
    double s = this->GradProp->s;
    double v = this->v[m];
    RoadNoiseVehicleCategory *cat = catalog->getCategory(m);
    value = cat->gradientCorrection->CalcValue(s, v);
  }
  DeltaPropulsionGradientCorrection[m][i] = value;
  return DeltaPropulsionGradientCorrection[m][i];
}

// Calculate the propulsion noise acceleration correction for vehicle category "m" and frequency band "i"
// Formula III-18
double RoadNoiseSegment::CalcAccCorrectionPropulsion(const int m, const int i)
{

  RoadNoiseVehicleCategory *cat = catalog->getCategory(m);
  if (this->AccProp != NULL)
  {
    // Formula III-18
    double Cfactor = cat->speedVariationCoefficient[this->AccProp->K][ngPROPULSION];
    //DeltaPropulsionAccCorrection[m][i] = Cfactor * max(1 - (abs(this->AccProp->distance) / 100),0);
    //TODO:
    DeltaPropulsionAccCorrection[m][i] = 0;
  }
  else
    DeltaPropulsionAccCorrection[m][i] = 0;
  return DeltaPropulsionAccCorrection[m][i];

}


// Calculate the total propulsion noise for each vehicle category "m" and frequency band "i"
// Formula III-11, III-12
double RoadNoiseSegment::CalcPropulsionNoise(const int m, const int i)
{
  RoadNoiseVehicleCategory *cat = catalog->getCategory(m);
  // Formula III-11
  double alphaFactor = cat->coefficientA[ngPROPULSION][i];
  double betaFactor =  cat->coefficientB[ngPROPULSION][i];
  double Factor = ((this->v[m] - catalog->refSpeed) / catalog->refSpeed);
  double LWR = alphaFactor + betaFactor * Factor;

  PropulsionNoiseFactor[m][i] = LWR;
  // Correction coefficients
  double deltaRoadSurface = CalcPropulsionNoiseRoadSurfaceCorrection(m, i);
  double deltaAcc = CalcAccCorrectionPropulsion(m,i);
  double delaGradient = CalcGradientCorrection(m,i);

  double deltaPropagationNoise = deltaRoadSurface + deltaAcc + delaGradient;
  DeltaPropulsionNoiseFactor[m][i] = deltaPropagationNoise;

  TotalPropulsionNoise[m][i] = LWR + deltaPropagationNoise;
  return TotalPropulsionNoise[m][i];
}

#pragma endregion Propagation_Calculation_functions

#pragma region HighLevel_Calculation_functions
// Calculates the source power for each vehicle category and frequency
// Formula III-1, III-3, III-4
int RoadNoiseSegment::CalcSegment()
{
  for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
    TotalSpec[i] = 0.0;

  for (int m=0; m < catalog->numCategories; m++)
  {
    for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
    {
      double Lwim = 0.0;

      double PropNoise=0.0;
      double RollingNoise=0.0;
      if (this->calcPropNoise[m])
        PropNoise = CalcPropulsionNoise(m,i);
      if (this->calcRollingNoise[m])
        RollingNoise = CalcRollingNoise(m,i);

      Lwim = 0.0;
      if (this->calcPropNoise[m] && !this->calcRollingNoise[m])
        // Formula III-2
        Lwim = PropNoise;
      else if (!this->calcPropNoise[m] && this->calcRollingNoise[m])
        // Formula III-2
        Lwim = RollingNoise;
      else if (this->calcPropNoise[m] && this->calcRollingNoise[m])
        // Formula III-3
        Lwim = 10 * log10(pow(10,(RollingNoise / 10)) + pow(10,(PropNoise / 10)));
      LwimTotal[m][i] = Lwim;

      double Lw_eq_line_im = 0.0;
      if (this->Q[m] > 0.0 && this->v[m] > 0.0)
      {
        // Formula III-1
        Lw_eq_line_im = 10 * log10(this->Q[m] / (1000 * this->v[m]));
        LWeqlineimFactor[m][i] = Lw_eq_line_im;

        Spec[m][i] = Lwim  + Lw_eq_line_im;

        // Energetic summation
        TotalSpec[i] += pow(10, (Spec[m][i] / 10));
      }
      else
      {
        LWeqlineimFactor[m][i] = 0.0;
        Spec[m][i] = 0.0;
      }
    }
  }

  // Finalize the energetic summation of each band
  for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
    TotalSpec[i] = 10 * log10(TotalSpec[i]);

  return 0;
}

// --------------------------------------------------------------------------------------------------------
// private function to dump all available (partial) results in a output file (for debug purposes)
//
// Parameter :
//				debugfile		: the file to contained all output
//
//
// --------------------------------------------------------------------------------------------------------
bool RoadNoiseSegment::writeDebugData(const string debugfile)
{
  // DEBUG DATA UITSPUGEN

  ofstream myfile;
  myfile.open(debugfile);
  if (!myfile)
  {
    cerr << "Unable to write to file " << debugfile << endl;
    return false;
  }
  myfile.imbue(locale(""));

  // Input data
  myfile << "Sep=\t" << endl;
  myfile << "Cat\tID\tQuantity\tSpeed\tFstud" << endl;
  for (int i=0; i < catalog->numCategories; i++)
    myfile << "Cat" << i << "\t" << catalog->getCategory(i)->id << "\t" << this->Q[i] << "\t" << this->v[i] << "\t" << this->Fstud[i] << endl;

  if (this->TempProp == NULL)
    myfile << "No temperature data found" << endl;
  else
    myfile << "Use temperature: " << this->TempProp->t << "°C" << endl;

  if (this->AccProp == NULL)
    myfile << "No acceleration properties found" << endl;
  else
  {
    if (this->AccProp->K == 1)
      myfile << "Acceleration: CROSSING at distance of: " << this->AccProp->distance << " m" << endl;
    else
      myfile << "Acceleration: ROUNDABOUT at distance of: " << this->AccProp->distance << " m" << endl ;
  }

  if (this->GradProp == NULL)
    myfile << "No gradient properties" << endl;
  else
    myfile << "Roadsegment has gradient of: " << this->GradProp->s << "%" << endl << endl;

  if (this->SurfaceProp == NULL)
    myfile << "Road surface reference: <none>" << endl;
  else
    myfile << "Road surface reference: " << this->SurfaceProp->id << endl;

  myfile << "Roadsegment has the following studded tyre properties:" << endl;
  myfile << "Months per year: " << this->studdedMonths << endl;
  PrintDataTable		(myfile, this->Fstud, "Fraction of vehicles with studded tyres:");
  myfile << endl;

  // Rolling Noise
  PrintDoubleDataTable(myfile, this->DeltaRollingAccCorrection, "Rolling noise acceleration correction"); // ΔLwr,acc
  PrintDoubleDataTable(myfile, this->DeltaRollingTyreCorrection, "Rolling noise tyre correction"); // ΔLstudded
  PrintDataTable		(myfile, this->DeltaRollingTempCorrection, "Temperature correction"); // ΔLw,temp
  PrintDoubleDataTable(myfile, this->DeltaRollingRoadCorrection, "Rolling noise road correction"); // ΔLwr,road
  PrintDoubleDataTable(myfile, this->DeltaRollingNoiseFactor, "Total rolling noise correction"); // ΔLwr
  PrintDoubleDataTable(myfile, this->RollingNoiseFactor, "Total rolling noise factor");
  PrintDoubleDataTable(myfile, this->TotalRollingNoise, "Total rolling noise"); // Lwr

  // Propulsion noise
  PrintDoubleDataTable(myfile, this->DeltaPropulsionAccCorrection, "Propulsion noise acceleration correction"); // ΔLwp,acc
  PrintDoubleDataTable(myfile, this->DeltaPropulsionGradientCorrection, "Propulsion noise gradient correction"); // ΔLwp,grad
  PrintDoubleDataTable(myfile, this->DeltaPropulsionRoadCorrection, "Propulsion noise road correction"); // ΔLwp,road
  PrintDoubleDataTable(myfile, this->DeltaPropulsionNoiseFactor, "Total propulsion noise correction"); // ΔLwp
  PrintDoubleDataTable(myfile, this->PropulsionNoiseFactor, "Total propulsion noise factor");
  PrintDoubleDataTable(myfile, this->TotalPropulsionNoise, "Total propulsion noise"); // Lwp

  // totals
  PrintDoubleDataTable(myfile, this->LwimTotal, "Lw;i,m - Total noise");
  PrintDoubleDataTable(myfile, this->LWeqlineimFactor, "Total Lweqline_im factor");

  // Output
  PrintDoubleDataTable(myfile, this->Spec, "Lw;eq,i,m - Total result");

  myfile << endl << "Cumulated total result" << endl;
  myfile << "\t\t";
  for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
    myfile << setw(8) << FreqBands[i] << "\t";
  myfile << endl << endl;
  myfile << "\t\t";
  for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
    myfile << setw(8) << this->TotalSpec[i] << "\t";
  myfile << endl;

  myfile.close();
  return true;

}
