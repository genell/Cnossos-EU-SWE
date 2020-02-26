#include "cnossos_full.h"
#include "PathParseOctave.h"
#include "MeteoCondition.h"
#include "ReferenceObject.h"
#include "ElementarySource.h"
#include "Material.h"
#include "Geometry3D.h"
using namespace CnossosEU;

#ifdef DEBUG
#include <cstdio>
#include <iostream>
#define print_debug std::cout.flush();printf
#else
#define print_debug //
#endif

void
my_err_handler (const char *fmt, ...)
{
  // Do nothing!!
}

void
my_err_with_id_handler (const char *id, const char *fmt, ...)
{
  // Do nothing!!
}

DEFUN_DLD (cnossos_full, args, ,
  "Usage: cnossos_full(M, path, [options], [meteo], [materials])\n\
    M: Calculation method. Legal values are \"CNOSSOS-2018\", \"ISO-9613-2\", \"JRC-2012\" or \"JRC-DRAFT-2010\"\n\
    path: Control points \n\
    options: (optional) Calculation options \n\
    meteo: (optional) Meteorological conditions \n\
    materials: (optional) Material definitions \n\
  "
  )
{
  CnossosFullArgs fullArgs;

  // Validate arguments
  if (args.length() < 2)
    error("Function requires at least 2 parameters: M, path");

  // Method
  if (!args(0).is_string())
    error("Parameter 'M' must be a string");
  else
    fullArgs.method = args(0).string_value();

  // Path structure
  if (!args(1).is_map())
    error("Parameter 'path' must be a struct");
  else
    fullArgs.path =  args(1).scalar_map_value();

  // Options structure
  if (args.length() > 2) {
    if (args(2).is_defined() && !args(2).is_map())
      error("If present, parameter 'options' must be a struct");
    else
      fullArgs.options =  args(2).scalar_map_value();
  }

  // Meteo structure
  if (args.length() > 3) {
    if (args(3).is_defined() && !args(3).is_map())
      error("If present, parameter 'meteo' must be a struct");
    else
      fullArgs.meteo =  args(3).scalar_map_value();
  }
  
  // Materials structure
  if (args.length() > 4) {
    if (args(4).is_defined() && !args(4).is_map())
      error("If present, parameter 'materials' must be a struct");
    else
      fullArgs.materials  =  args(4).scalar_map_value();
  }
  
  // Declare unwind_protect frame which lasts as long as
  // the variable frame has scope.
  octave::unwind_protect frame;
  frame.add_fcn (set_liboctave_warning_handler,
                 current_liboctave_warning_handler);

  frame.add_fcn (set_liboctave_warning_with_id_handler,
                 current_liboctave_warning_with_id_handler);

  set_liboctave_warning_handler (my_err_handler);
  set_liboctave_warning_with_id_handler (my_err_with_id_handler);

  CnossosFull cnosFull(fullArgs);
  print_debug("CnossosFull::ctor OK\n");

#if REDIRECTDEBUG
 	freopen("output.txt","w",stdout);
  std::cout << "redirecting cout..." << std::endl;
	freopen("error.txt","w",stderr);
  std::cerr << "redirecting cerr..." << std::endl;
#endif

  cnosFull.setupConfig();
  cnosFull.eval();

  Matrix m(13,8);
  cnosFull.resultToMatrix(m);

  return octave_value(m);
}


void CnossosFull::setupConfig() {
  print_debug("CnossosFull::setupConfig() begin\n");

  if (!ParseParameters(args.options, args.meteo, options))
    error("error parsing parameters");

  options.method = method; // Attach method to options object (probably not used)

  if (!ParseMaterials(args.materials))
    error("error parsing materials");

  if (!ParsePropagationPath(args.path, path))
    error("error parsing propagation path");
  
  print_debug("CnossosFull::setupConfig() end\n");
}

void CnossosFull::eval() {
  print_debug("CnossosFull::eval() begin\n");
	method->setOptions(options) ;
  print_debug("method->setoptions(..) ok\n");
  dumpArgs();
  try {
	method->doCalculation(path, result);
  } catch (ErrorMessage &err)
	{
    printf("CNOSSOS error: %s", err.what());
  }
  print_debug("CnossosFull::eval() end\n");
}

void CnossosFull::resultToMatrix(Matrix &matrix) {
  double* pM = matrix.fortran_vec();
  for (int i=0; i<8; i++) {
    int mBase = 13*i;
    pM[0+mBase] = result.Lw.data(i); // sound power of the source
		pM[1+mBase] =	result.dBA.data(i); // dB(A) weighting 
		pM[2+mBase]	= result.delta_Lw.data(i); // sound power adapter
		pM[3+mBase] =	result.AttGeo; // geometrical spread
		pM[4+mBase] =	result.AttAir.data(i); // air absorption
		pM[5+mBase] =	result.AttAbsMat.data(i); // attenuation due to absorption by reflecting obstacles
		pM[6+mBase] =	result.AttLatDif.data(i); // attenuation due to lateral diffraction
		pM[7+mBase] =	result.AttSize.data(i); // correction for finite size of obstacles
		pM[8+mBase] =	result.AttF.data(i); // excess attenuation under favorable conditions
		pM[9+mBase] =	result.AttH.data(i); // excess attenuation under homogeneous conditions
		pM[10+mBase] = result.LpF.data(i); // sound pressure level under favorable conditions
		pM[11+mBase] = result.LpH.data(i); // sound pressure level under homogeneous conditions 
		pM[12+mBase] = result.Leq.data(i); // long-time averaged sound pressure level
  }
}

void CnossosFull::dumpArgs() {
  print_debug("options.CheckHeightLowerBound=%d\n",options.CheckHeightLowerBound);
  print_debug("options.CheckHeightUpperBound=%d\n", options.CheckHeightUpperBound);
  print_debug("options.CheckHorizontalAlignment=%d\n", options.CheckHorizontalAlignment);
  print_debug("options.CheckLateralDiffraction=%d\n", options.CheckLateralDiffraction);
  print_debug("options.CheckSoundPowerUnits=%d\n", options.CheckSoundPowerUnits);
  print_debug("options.CheckSourceSegment=%d\n", options.CheckSourceSegment);
  print_debug("options.DisableLateralDiffractions=%d\n", options.DisableLateralDiffractions);
  print_debug("options.DisableReflections=%d\n", options.DisableReflections);
  print_debug("options.ExcludeAirAbsorption=%d\n", options.ExcludeAirAbsorption);
  print_debug("options.ExcludeGeometricalSpread=%d\n", options.ExcludeGeometricalSpread);
  print_debug("options.ExcludeSoundPower=%d\n", options.ExcludeSoundPower);
  print_debug("options.ForceSourceToReceiver=%d\n", options.ForceSourceToReceiver);
  print_debug("options.IgnoreComplexPaths=%d\n", options.IgnoreComplexPaths);
  print_debug("options.SimplifyPathGeometry=%d\n", options.SimplifyPathGeometry);
  print_debug("method=%s\n", options.method->name());
  print_debug("meteo=%x\n", &options.meteo);
  print_debug("path.cp[%d]\n", path.cp.size());
  print_debug("path.info.nbDiffractions=%d\n", path.info.nbDiffractions);
  print_debug("path.info.nbLateralDiffractions=%d\n", path.info.nbLateralDiffractions);
  print_debug("path.info.nbReflections=%d\n", path.info.nbReflections);
  print_debug("path.info.pathType=%d\n", path.info.pathType);
}