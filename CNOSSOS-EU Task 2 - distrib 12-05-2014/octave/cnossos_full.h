#pragma once
// include config first just to be compatible with older octaves
#include <octave/octave-config.h>
#include <octave/oct.h>

// Cnossos deps
#include "CalculationMethod.h"
#include "PropagationPath.h"
#include "PathResult.h"

struct CnossosFullArgs {
  std::string method;
  octave_scalar_map path;
  octave_scalar_map options;
  octave_scalar_map meteo;
  octave_scalar_map materials;
};

class CnossosFull {
private:
  CnossosFullArgs &args;
  System::ref_ptr<CnossosEU::CalculationMethod> method;
  CnossosEU::PropagationPathOptions options;
  CnossosEU::PropagationPath path;
  CnossosEU::PathResult result;
public:
  CnossosFull(CnossosFullArgs &args) : args(args) {
    method = CnossosEU::getCalculationMethod(args.method.c_str());
    if (method == 0)
    {
      error("invalid method. Legal values are \"CNOSSOS-2018\", \"ISO-9613-2\", \"JRC-2012\" or \"JRC-DRAFT-2010\"\n");
    }
  };
  ~CnossosFull() { }
  void setupConfig();
  void eval();
  void resultToMatrix(Matrix &matrix);
  void dumpArgs();
};