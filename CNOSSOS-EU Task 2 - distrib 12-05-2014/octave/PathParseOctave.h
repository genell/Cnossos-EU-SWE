#pragma once
#include "../SimpleXML/SimpleXML.h"
#include "../PropagationPath/PropagationPath.h"
#include "../PropagationPath/CalculationMethod.h"
#include "../PropagationPath/ErrorMessage.h"
#include <octave/oct.h>

bool ParsePropagationPath (const octave_scalar_map &optPath, CnossosEU::PropagationPath& path);
bool ParseParameters (const octave_scalar_map &aOptions, const octave_scalar_map &aMeteo, CnossosEU::PropagationPathOptions& options);
bool ParseMaterials (const octave_scalar_map &materials);