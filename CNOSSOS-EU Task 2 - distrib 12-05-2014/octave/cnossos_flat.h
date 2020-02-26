#pragma once
// include config first just to be compatible with older octaves
#include <octave/octave-config.h>
#include <octave/oct.h>

// Cnossos deps
#include "CalculationMethod.h"
#include "PropagationPath.h"
#include "PathResult.h"

struct CnossosFlatArgs {
  std::string method;
  double sourceHeight;
  double D1;
  double D;
  double receiverHeight;
  std::string receiverMaterialId;
  NDArray Lw;
};

class CnossosFlat {
private:
  CnossosFlatArgs &args;
  System::ref_ptr<CnossosEU::CalculationMethod> method;
  CnossosEU::PropagationPathOptions options;
  CnossosEU::PropagationPath path;
  CnossosEU::PathResult result;
public:
  CnossosFlat(CnossosFlatArgs &args) : args(args) {
    method = CnossosEU::getCalculationMethod(args.method.c_str());
    if (method == 0)
    {
      error("invalid method. Legal values are \"CNOSSOS-2018\", \"ISO-9613-2\", \"JRC-2012\" or \"JRC-DRAFT-2010\"\n");
    }
  };
  void setupConfig();
  void eval();
  void resultToMatrix(Matrix &matrix);
};