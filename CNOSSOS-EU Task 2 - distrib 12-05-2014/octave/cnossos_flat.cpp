#include "cnossos_flat.h"
#include "MeteoCondition.h"
#include "ReferenceObject.h"
#include "ElementarySource.h"
#include "Material.h"
#include "Geometry3D.h"
#include "PathResult.h"

using namespace CnossosEU ;

//CnossosEU::PropagationPath path;
//CnossosEU::PathResult result;

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
DEFUN_DLD (cnossos_flat, args, ,
  "Usage: cnossos_flat(M, HS, D1, D, HR, I, [L1..L8])\n\
    M: Calculation method. Legal values are \"ISO-9613-2\", \"JRC-2012\" or \"JRC-DRAFT-2010\"\n\
    HS: Source height\n\
    D1: ??? \n\
    D: ??? \n\
    HR: ??? \n\
    I: ??? \n\
    L1..L8: ??? \n\
  "
  )
{
  // octave_stdout << "Hello World has "
  //               << args.length () << " input arguments and "
  //               << 1 << " output arguments.\n";

  // Validate arguments
  if (args.length() != 7)
    error("Expected exactly 7 parameters: M, HS, Dl, D, HR, I, [L1..L8]");
  if (!args(0).is_string())
    error("Parameter 'M' must be a string");
  if (!args(1).is_double_type())
    error("Parameter 'HS' must be a number");
  if (!args(2).is_double_type())
    error("Parameter 'D1' must be a number");
  if (!args(3).is_double_type())
    error("Parameter 'D' must be a number");
  if (!args(4).is_double_type())
    error("Parameter 'HR' must be a number");
  if (!args(5).is_string())
    error("Parameter 'I' must be a string");
  if (args(6).ndims() != 2 || args(6).dims().elem(1) != 8)
    error("Parameter 'L' must be an array of length 8");

  System::ref_ptr<CalculationMethod> method = getCalculationMethod(args(0).string_value().c_str());
  if (method == 0)
  {
      error("invalid method specified on command line, using input file instead\n") ;
  }


  CnossosFlatArgs flatArgs;
  PropagationPathOptions options;
  PropagationPath path;

  flatArgs.method = args(0).string_value();
  flatArgs.sourceHeight = args(1).double_value();
  flatArgs.D1 = args(2).double_value();
  flatArgs.D = args(3).double_value();
  flatArgs.receiverHeight = args(4).double_value();
  flatArgs.receiverMaterialId = args(5).string_value();
  flatArgs.Lw = args(6).array_value();

  SetupConfig(options, path, method, flatArgs);

  for (int i=0; i<flatArgs.Lw.numel(); i++) {
    octave_stdout << "L" << i << "=" << flatArgs.Lw(i) << std::endl;
  }

	PathResult result;
	method->setOptions (options) ;
	method->doCalculation (path, result) ;

  return octave_value (Matrix(13,8,123));
}

void SetupConfig(PropagationPathOptions &options, PropagationPath &path, CalculationMethod* method, CnossosFlatArgs &args) {
  /*
  <method>
    <select id="_M_" />
    <meteo model="DEFAULT">
      <temperature> 15.0 </temperature>
      <humidity>    70.0 </humidity>
      <pFav> 0.50 </pFav>
      <C0>   3.00 </C0>
    </meteo>
  </method>
  */
  options.method = method;
  options.meteo.model = MeteoCondition::MeteoModel::DEFAULT;
  options.meteo.temperature = 15;
  options.meteo.humidity = 70;
  options.meteo.pFav = 0.5;
  options.meteo.C0 = 3;

  path.clear();
  /*
  <path>
  <cp id="source">
    <pos>
      <x> 0.00 </x> <y> 0.00 </y> <z> 0.00 </z>
    </pos>
    <ext>
      <source>
        <h> _HS_ </h>
        <Lw sourceType="PointSource" measurementType="HemiSpherical" frequencyWeighting="LIN">_L1_ _L2_ _L3_ _L4_ _L5_ _L6_ _L7_ _L8_</Lw>
      </source>
    </ext>
  </cp>
  */
  ControlPoint cpSource;
  // cpSource.pos.x = 0 ;
	// cpSource.pos.y = 0 ;
	// cpSource.pos.z = 0 ;
	cpSource.mat = getMaterial ("H") ;
	// cpSource.ext = 0 ;
  // source.pos defaults to [0 0 0] 
  // source.mat defaults to none
  SourceExt srcExt;
  // srcExt.geo = 0;
  srcExt.h = args.sourceHeight;
  srcExt.source.spectrumType = SpectrumType::PointSource;
  srcExt.source.measurementType = MeasurementType::HemiSpherical;
  srcExt.source.frequencyWeighting = FrequencyWeighting::dBLIN;
  srcExt.source.soundPower = Spectrum(args.Lw.data());  
  cpSource.ext = new SourceExt(srcExt);
  path.add(cpSource);

  /*
  <cp id= "roadsurf">
    <pos>
      <x> _D1_ </x> <z> 0.00 </z>
    </pos>
    <mat id = "H" />
  </cp>
  */
  ControlPoint cpRoadsurf;
  cpRoadsurf.pos = Geometry::Point3D(args.D1, 0, 0);
  cpRoadsurf.mat = getMaterial ("H");
  cpRoadsurf.ext = 0;
  path.add(cpRoadsurf);

  /*
  <cp id= "receiver">
    <pos>
      <x> _D_ </x> <z> 0.00 </z>
    </pos>
    <mat id = "_I_" />
    <ext>
      <receiver>
        <h> _HR_ </h>
      </receiver>
    </ext>
  </cp>
  */
  ControlPoint receiver;
  receiver.pos = Geometry::Point3D(args.D, 0, 0);
  receiver.mat = getMaterial(args.receiverMaterialId.c_str());
  receiver.ext = new ReceiverExt();
  receiver.ext->h = args.receiverHeight;
  path.add(receiver);
}

//"JRC-2012",0.5,5,100,2,"C",[0 0 0 0 0 0 0 0]
int main( int argc, const char* argv[] ) { 

  System::ref_ptr<CalculationMethod> method = getCalculationMethod("JRC-2012");
  if (method == 0)
  {
      error("invalid method specified on command line, using input file instead\n") ;
  }

  CnossosFlatArgs flatArgs;
  PropagationPathOptions options;
  PropagationPath path;

  flatArgs.method = "JRC-2012";
  flatArgs.sourceHeight = 0.5;
  flatArgs.D1 = 5;
  flatArgs.D = 100;
  flatArgs.receiverHeight = 2;
  flatArgs.receiverMaterialId = "C";
  flatArgs.Lw = Array<double>();

  SetupConfig(options, path, method, flatArgs);

	PathResult result;
	method->setOptions (options) ;
	method->doCalculation (path, result) ;

}