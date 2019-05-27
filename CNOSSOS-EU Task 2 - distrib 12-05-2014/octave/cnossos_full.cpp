#include "cnossos_full.h"
#include "PathParseOctave.h"
#include "MeteoCondition.h"
#include "ReferenceObject.h"
#include "ElementarySource.h"
#include "Material.h"
#include "Geometry3D.h"

using namespace CnossosEU ;

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

/**
 * cnossos_flat("JRC-2012", 0.5, 5, 100, 2, "C", [0 0 0 0 0 0 0 0])
 * _M_ = "JRC-2012"
 * _HS_ = 0.5
 * _D1_ = 5
 * _D_ = 100
 * _HR_ = 2
 * _I_ = "C"
 * _L1..L8_ = [0 0 0 0 0 0 0 0] (NDArray)
 */
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

  fullArgs.method = args(0).string_value();

  // Path structure
  if (!args(1).isstruct())
    error("Parameter 'path' must be a struct");

  fullArgs.path =  args(1).scalar_map_value();

  // Options structure
  if (args.length() > 2 && !args(2).isnull() && !args(2).isstruct())
    error("If present, parameter 'options' must be a struct");

  fullArgs.options =  args(2).scalar_map_value();

  // Meteo structure
  if (args.length() > 3 && !args(3).isnull() && !args(3).isstruct())
    error("If present, parameter 'meteo' must be a struct");

  fullArgs.meteo =  args(3).scalar_map_value();

  // Materials structure
  if (args.length() > 4 && !args(4).isnull() && !args(4).isstruct())
    error("If present, parameter 'materials' must be a struct");

  fullArgs.materials  =  args(4).scalar_map_value();

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
  cnosFull.setupConfig();
  // cnFlat.eval();

  Matrix m(13,8);
  // cnFlat.resultToMatrix(m);

  return octave_value(m);
}

// bool ParseOctaveParameters (CnossosFullArgs, PropagationPath& path, PropagationPathOptions& options) 
// {
//   path.clear() ;

//   static const char* root = "CNOSSOS-EU" ;
//   if (!checkTagName (node, root)) 
//   {
//     signal_error (XMLMissingTag (root, node)) ;
//     return false ;
//   }

//   node = node->GetFirstChild() ;

//   if (ParseParameters (node, options)) node = node->GetNextEntity() ;

//   if (ParseMaterials (node)) node = node->GetNextEntity() ;

//   if (ParsePropagationPath (node, path)) node = node->GetNextEntity() ; 

//   if (node != 0)
//   {
//     signal_error (XMLUnexpectedTag (node)) ;
//     return false ;
//   }
//   return true ;
// }


void CnossosFull::setupConfig() {

  if (!ParseParameters(args.options, args.meteo, options))
    error("error parsing parameters");

  // if (!ParseMaterials(args.materials))
  //   error("error parsing materials");

  // if (!ParsePropagationPath(args.path, path))
  //   error("error parsing propagation path");
}

void CnossosFull::eval() {
	method->setOptions(options) ;
	method->doCalculation(path, result) ;
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

// Test stub
int main( int argc, char** argv ) { 
  CnossosFullArgs fullArgs;
  fullArgs.method = "JRC-2012";

  CnossosFull cnFlat(fullArgs);
  cnFlat.setupConfig();
  cnFlat.eval();
}