#include <octave/oct.h>

// Cnossos deps
#include "CalculationMethod.h"
#include "PropagationPath.h"
// #include "PathResult.h"
// #include "Material.h"

struct CnossosFlatArgs {
  std::string method;
  double sourceHeight;
  double D1;
  double D;
  double receiverHeight;
  std::string receiverMaterialId;
  NDArray Lw;
};

void SetupConfig(CnossosEU::PropagationPathOptions &options, CnossosEU::PropagationPath &path, CnossosEU::CalculationMethod* method, CnossosFlatArgs &args);