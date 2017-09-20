/* 
 * ------------------------------------------------------------------------------------------------
 * file:		CnossosPropagation.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: CnossosPropagation implements a standard C-style API to the CNOSSOS-EU propagation
 *	            module. Most popular programming languages can be bound to pure C functions 
 *				defined in DLL shared libraries, either at linker level, either at run time.
 * changes:
 *
 *	03/12/2013	initial version
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "CnossosPropagation.h"
#include "PropagationPath.h"
#include "CalculationMethod.h"
#include "PathParseXML.h"
#include "PathResult.h"
#include <direct.h>
#include <string>

using namespace CnossosEU ;
using namespace System ;
using namespace Geometry ;

/**
 * \brief Hidden class for path calculation
 */
struct CNOSSOS_P2P_ENGINE
{
	ref_ptr<CalculationMethod> method ;		//!< pointer to the calculation method
	PropagationPath path ;					//!< propagation path data
	ElementarySource source ;				//<! elementary source
	PropagationPathOptions options ;		//<! calculation options
	PathResult result ;						//<! calculation results
	bool calculationDone ;					//<! set if calculation has been done
	bool hasErrors ;						//<! set if calculation has finished with error
	std::string errorMessage ;				//<! last error message
	/**
	 * \brief Constructor
	 */
	CNOSSOS_P2P_ENGINE (const char* name)
	: method(), path(), source(), options(), result(), calculationDone(false), hasErrors(false), errorMessage()
	{
		if (name) selectMethod (name) ;
		source.soundPower = Spectrum (0.0) ;
		source.frequencyWeighting = FrequencyWeighting::dBLIN ;
		source.measurementType = MeasurementType::Undefined ;
	}
	/**
	 * \brief Select the calculation method
	 */
	bool selectMethod (const char* name)
	{
		method = getCalculationMethod (name) ;
		calculationDone = false ;
		return method != 0 ;
	}
	/**
	 * \brief Process the path through the selected propagation method
	 */
	bool doCalculation (void)
	{
		if (calculationDone) return true ;
		if (method == 0) return false ;
		try
		{
			SourceExt* sourceExt = 0 ;
			unsigned int n1 = 0 ;
			unsigned int n2 = path.size()-1 ;
			if (path[n1].ext && path[n1].ext->isSource())
			{
				sourceExt = path[n1].ext.cast_to_ptr<SourceExt>() ;
			}
			else if (path[n2].ext && path[n2].ext->isSource())
			{
				sourceExt = path[n2].ext.cast_to_ptr<SourceExt>() ;
			}			
			if (sourceExt) sourceExt->source = source ;
			options.CheckSoundPowerUnits = false ;
			method->setOptions (options) ;
			method->doCalculation (path, result) ;
			hasErrors = false ;
		}
		catch (ErrorMessage &err)
		{
			//err.print() ;
			errorMessage = err.what() ;
			hasErrors = true ;
		}
		return calculationDone = true ;
	}
};
/**
 * \brief Transparent pointer to material definition
 */
struct CNOSSOS_P2P_MATERIAL : public Material { } ;
/**
 * \brief Transparent pointer to vertical extension
 */
struct CNOSSOS_P2P_EXTENSION : public VerticalExt { } ;

const char* CNOSSOS_P2P_GetVersionDLL (void)
{
	return CNOSSOS_P2P_VERSION_API ;
}

unsigned int CNOSSOS_P2P_GetFreq (double *freq)
{
	Spectrum spec ;
	if (freq) for (unsigned int i = 0 ; i < spec.size() ; ++i) freq[i] = spec.freq(i) ;
	return spec.size() ;
}

CNOSSOS_P2P_MATERIAL* CNOSSOS_P2P_GetMaterial (const char* id, bool create_if_needed)
{
	return (CNOSSOS_P2P_MATERIAL*) getMaterial (id, create_if_needed) ;
}

bool CNOSSOS_P2P_GetGValue (CNOSSOS_P2P_MATERIAL* mat, double& G)
{
	if (mat == 0) return false ;
	G = mat->getGValue() ;
	return true ;
}

bool CNOSSOS_P2P_SetGValue (CNOSSOS_P2P_MATERIAL* mat, double const& G)
{
	if (mat == 0) return false ;
	mat->setG(G) ;
	return true ;
}

bool CNOSSOS_P2P_GetSigma (CNOSSOS_P2P_MATERIAL* mat, double& sigma)
{
	if (mat == 0) return false ;
	sigma = mat->getSigmaValue() ;
	return true ;
}

bool CNOSSOS_P2P_SetSigma (CNOSSOS_P2P_MATERIAL* mat, double const& sigma)
{
	if (mat == 0) return false ;
	mat->setSigma(sigma) ;
	return true ;
}

bool CNOSSOS_P2P_GetAlpha (CNOSSOS_P2P_MATERIAL* mat, double* alpha)
{
	if (mat == 0) return false ;
	Spectrum spec = mat->getAlphaValue() ;
	for (unsigned int i = 0 ; i < spec.size() ; ++i) alpha[i] = spec[i] ;
	return true ;
}

bool CNOSSOS_P2P_SetAlpha (CNOSSOS_P2P_MATERIAL* mat, double* alpha)
{
	if (mat == 0) return false ;
	mat->setAlpha (Spectrum(alpha)) ;
	return true ;
}

CNOSSOS_P2P_ENGINE* CNOSSOS_P2P_CreateEngine (const char* name)
{
	return new CNOSSOS_P2P_ENGINE (name) ;
}

bool CNOSSOS_P2P_DeleteEngine (CNOSSOS_P2P_ENGINE* p2p)
{
	if (!p2p) return false ;
	delete p2p ;
	return true ;
}

const char* CNOSSOS_P2P_GetMethod (CNOSSOS_P2P_ENGINE* p2p)
{
	if (p2p == 0 || p2p->method == 0) return "Unknown" ;
	return p2p->method->name() ;
}

const char* CNOSSOS_P2P_GetVersion (CNOSSOS_P2P_ENGINE* p2p)
{
	if (p2p == 0 || p2p->method == 0) return "Unknown" ;
	return p2p->method->version() ;
}

bool CNOSSOS_P2P_SelectMethod (CNOSSOS_P2P_ENGINE* p2p, const char* name)
{
	return p2p->selectMethod (name) ;
}

CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreatePointSource (double const& h)
{
	return (CNOSSOS_P2P_EXTENSION*) new SourceExt (h) ;
}

CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateReceiver (double const& h)
{
	return (CNOSSOS_P2P_EXTENSION*) new ReceiverExt (h) ;
}

CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateBarrier (double const& h, CNOSSOS_P2P_MATERIAL* mat)
{
	return (CNOSSOS_P2P_EXTENSION*) new BarrierExt (h, mat) ;
}

CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateVerticalWall (double const& h, CNOSSOS_P2P_MATERIAL* mat)
{
	return (CNOSSOS_P2P_EXTENSION*) new VerticalWallExt (h, mat) ;
}

CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateVerticalEdge (double const& h)
{
	return (CNOSSOS_P2P_EXTENSION*) new VerticalEdgeExt (h) ;
}

CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateLineSource (double const& h, 
													 CNOSSOS_P2P_POSITION const segment[2], 
													 double const& fixedAngle)
{
	SourceExt* sourceExt = new SourceExt(h) ;
	Point3D p1 (segment[0][0], segment[0][1], segment[0][2]) ;
	Point3D p2 (segment[1][0], segment[1][1], segment[1][2]) ;
	sourceExt->geo= new LineSegment (p1, p2, fixedAngle) ;
	return (CNOSSOS_P2P_EXTENSION*) sourceExt ;
}

bool CNOSSOS_P2P_ClearPath (CNOSSOS_P2P_ENGINE* p2p)
{
	if (p2p == 0) return false ;
	p2p->path.resize(0) ;
	p2p->calculationDone = false ;
	return true ;
}

unsigned int CNOSSOS_P2P_AddToPath (CNOSSOS_P2P_ENGINE* p2p, 
								    CNOSSOS_P2P_POSITION const& pos,
									CNOSSOS_P2P_MATERIAL* mat,
									CNOSSOS_P2P_EXTENSION* ext)
{
	if (p2p == 0) return 0 ;
	ControlPoint cp ;
	cp.pos = Point3D (pos[0], pos[1], pos[2]) ;
	cp.mat = mat ;
	cp.ext = ext ;
	p2p->path.cp.push_back (cp) ;
	p2p->calculationDone = false ;
	return p2p->path.cp.size() ;
}

unsigned int CNOSSOS_P2P_GetResult (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_RESULT index, double * result)
{
	if (p2p == 0) return 0 ;
	if (!p2p->doCalculation()) return 0 ;
	if (p2p->hasErrors) return 0 ;

	switch (index)
	{
	case CNOSSOS_P2P_RESULT_LP_AVG_dBA:
		if (result) *result = p2p->result.Leq_dBA ; 
		return 1 ;
	case CNOSSOS_P2P_RESULT_LP_FAV_dBA:
		if (result) *result = p2p->result.LpF_dBA ; 
		return 1 ;
	case CNOSSOS_P2P_RESULT_LP_HOM_dBA:
		if (result) *result = p2p->result.LpH_dBA ; 
		return 1 ;
	case CNOSSOS_P2P_RESULT_ATT_GEO :
		if (result)*result = p2p->result.AttGeo ;
		return 1 ;
	}

	Spectrum spec(0.0) ;
	switch (index)
	{
	case CNOSSOS_P2P_RESULT_LP_AVG:
		spec = p2p->result.Leq ;
		break ;
	case CNOSSOS_P2P_RESULT_LP_FAV:
		spec = p2p->result.LpF ;
		break ;
	case CNOSSOS_P2P_RESULT_LP_HOM:
		spec = p2p->result.LpH ;
		break ;
	case CNOSSOS_P2P_RESULT_ATT_FAV:
		spec = p2p->result.AttF ;
		break ;
	case CNOSSOS_P2P_RESULT_ATT_HOM:
		spec = p2p->result.AttH ;
		break ;
	case CNOSSOS_P2P_RESULT_LW_SOURCE :
		spec = p2p->result.Lw ;
		break ;
	case CNOSSOS_P2P_RESULT_DELTA_LW :
		spec = p2p->result.delta_Lw ;
		break ;
	case CNOSSOS_P2P_RESULT_ATT_ATM :
		spec = p2p->result.AttAir ;
		break ;
	case CNOSSOS_P2P_RESULT_ATT_REF :
		spec = p2p->result.AttAbsMat ;
		break ;
	case CNOSSOS_P2P_RESULT_ATT_DIF :
		spec = p2p->result.AttLatDif ;
		break ;
	case CNOSSOS_P2P_RESULT_ATT_SIZE :
		spec = p2p->result.AttSize ;
		break ;
	default:
		return 0 ;
	}

	if (result)
	{
		for (unsigned int i = 0 ; i < spec.size() ; ++i) 
		{
			result[i] = _finite (spec[i]) ? spec[i] : -99.9 ;
		}
	}
	return spec.size() ;
}

bool CNOSSOS_P2P_SetSoundPower (CNOSSOS_P2P_ENGINE* p2p, 
								double const* Lw,
							    CNOSSOS_P2P_SPECTRUM_WEIGHTING weighting,
								CNOSSOS_P2P_LW_MEASUREMENT_TYPE type)
{
	if (p2p == 0) return false ;

	p2p->source.soundPower = Spectrum (Lw) ;
	
	p2p->source.frequencyWeighting = FrequencyWeighting::dBLIN ;
	if (weighting == CNOSSOS_P2P_SPECTRUM_dBA) p2p->source.frequencyWeighting = FrequencyWeighting::dBA ;
	
	p2p->source.measurementType == MeasurementType::Undefined ;
	if (type == CNOSSOS_P2P_LW_HEMISPHERICAL) p2p->source.measurementType = MeasurementType::HemiSpherical ;
	if (type == CNOSSOS_P2P_LW_FREEFIELD) p2p->source.measurementType = MeasurementType::FreeField ;
	
	p2p->calculationDone = false ;
	return true ;
}

const char* CNOSSOS_P2P_GetErrorMessage (CNOSSOS_P2P_ENGINE* p2p)
{
	if (p2p == 0) return "Invalid argument" ;
	return p2p->errorMessage.c_str () ;
}

bool CNOSSOS_P2P_PrintPathData (CNOSSOS_P2P_ENGINE* p2p)
{
	if (p2p == 0) return false ;
	p2p->path.print_input_data() ;
	return true ;
}

bool CNOSSOS_P2P_PrintPathResults (CNOSSOS_P2P_ENGINE* p2p) 
{
	if (p2p == 0) return false ;
	print_results_to_stdout (p2p->result) ;
	return true ;
}

bool CNOSSOS_P2P_SetOptions (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_OPTIONS const& _options)
{
	if (p2p == 0) return false ;
	PropagationPathOptions &options = p2p->options ;
	options.CheckHorizontalAlignment   = _options.CheckHorizontalAlignment ;
	options.CheckLateralDiffraction    = _options.CheckLateralDiffraction ;
	options.CheckHeightLowerBound	   = _options.CheckHeightLowerBound	;	
	options.CheckHeightUpperBound	   = _options.CheckHeightUpperBound	;
	options.CheckSourceSegment	       = _options.CheckSourceSegment;
	options.CheckSoundPowerUnits       = _options.CheckSoundPowerUnits ;
	options.ForceSourceToReceiver	   = _options.ForceSourceToReceiver	;
	options.SimplifyPathGeometry	   = _options.SimplifyPathGeometry ;	
	options.DisableReflections	       = _options.DisableReflections ;		
	options.DisableLateralDiffractions = _options.DisableLateralDiffractions ;
	options.IgnoreComplexPaths    = _options.IgnoreComplexPaths ;
	options.ExcludeSoundPower		   = _options.ExcludeSoundPower ;	
	options.ExcludeGeometricalSpread   = _options.ExcludeGeometricalSpread ;
	options.ExcludeAirAbsorption	   = _options.ExcludeAirAbsorption ;	
	return true ;
}

bool CNOSSOS_P2P_GetOptions (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_OPTIONS & _options)
{
	if (p2p == 0) return false ;
	PropagationPathOptions &options = p2p->options ;
	_options.CheckHorizontalAlignment   = options.CheckHorizontalAlignment ;
	_options.CheckLateralDiffraction    = options.CheckLateralDiffraction ;
	_options.CheckHeightLowerBound	     = options.CheckHeightLowerBound	;	
	_options.CheckHeightUpperBound	     = options.CheckHeightUpperBound	;
	_options.CheckSourceSegment	     = options.CheckSourceSegment;
	_options.CheckSoundPowerUnits       = options.CheckSoundPowerUnits ;
	_options.ForceSourceToReceiver	     = options.ForceSourceToReceiver	;
	_options.SimplifyPathGeometry	     = options.SimplifyPathGeometry ;	
	_options.DisableReflections	     = options.DisableReflections ;		
	_options.DisableLateralDiffractions = options.DisableLateralDiffractions ;
	_options.IgnoreComplexPaths    = options.IgnoreComplexPaths ;
	_options.ExcludeSoundPower			 = options.ExcludeSoundPower ;	
	_options.ExcludeGeometricalSpread	 = options.ExcludeGeometricalSpread ;
	_options.ExcludeAirAbsorption		 = options.ExcludeAirAbsorption ;	
	return true ;
}

bool CNOSSOS_P2P_SetMeteo (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_METEO const& _meteo)
{
	if (p2p == 0) return false ;
	MeteoCondition& meteo = p2p->options.meteo ;
	meteo.model = MeteoCondition::DEFAULT ;
	if (_meteo.model == CNOSSOS_P2P_METEO_ISO9613) meteo.model = MeteoCondition::ISO9613 ;
	if (_meteo.model == CNOSSOS_P2P_METEO_JRC2012) meteo.model = MeteoCondition::JRC2012 ;
	meteo.C0 = _meteo.C0 ;
	meteo.pFav = _meteo.pFav ;
	meteo.temperature = _meteo.temperature ;
	meteo.humidity = _meteo.humidity ;
	return true ;
}

bool CNOSSOS_P2P_GetMeteo (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_METEO & _meteo)
{
	if (p2p == 0) return false ;
	MeteoCondition& meteo = p2p->options.meteo ;
	_meteo.model = CNOSSOS_P2P_METEO_DEFAULT ;
	if (meteo.model == MeteoCondition::ISO9613) _meteo.model = CNOSSOS_P2P_METEO_ISO9613  ;
	if (meteo.model == MeteoCondition::JRC2012) _meteo.model = CNOSSOS_P2P_METEO_JRC2012 ;
	_meteo.C0 = meteo.C0 ;
	_meteo.pFav = meteo.pFav ;
	_meteo.temperature = meteo.temperature ;
	_meteo.humidity = meteo.humidity ;
	return true ;
}
/* 
 * local utility function (undocumented)
 */
static char* getcwd (void)
{
	return _getcwd (NULL,0) ;
}
/* 
 * local utility function (undocumented)
 */
static char* getcwd (const char* fileName)
{
	char* new_cwd = _strdup (fileName) ;
	for (int i = strlen(new_cwd)-1 ; i > 0 ; --i)
	{
		if (new_cwd[i] == '/' || new_cwd[i] == '\\') 
		{
			new_cwd[i] = 0 ; 
			break ;
		}
	}
	return new_cwd ;
}

bool CNOSSOS_P2P_ProcessPathFile (CNOSSOS_P2P_ENGINE* p2p, const char* inputFile,const char* outputFile)
{
	char* old_dir = 0 ;
	char* new_dir = 0 ;
	XMLFileLoader xmlFile ;

	try
	{
		p2p->calculationDone = false ;
		p2p->hasErrors = true ;
		/*
		 * read the input file into memory
		 */
		if (!xmlFile.ParseFile (inputFile))
		{
			signal_error (XMLSyntaxError (xmlFile)) ;
			return 0 ;
		}
		/*
		 * change the current working directory so that all other filenames
		 * will be considered relative to the folder containing the input file
		 */
		old_dir = getcwd() ;
		new_dir = getcwd(inputFile) ;
		_chdir (new_dir) ;
		print_debug ("Changing current working directory to %s \n", _getcwd(0,0))	;
		/*
		 * parse the XML-DOM structure into the application object/class schema
		 */
		ParsePathFromFile (xmlFile.GetRoot(), p2p->path, p2p->options) ;
		/*
		 * print out information about the actual propagation method
		 */
		if (p2p->method == 0) p2p->method = p2p->options.method ;
		print_debug ("Calculate path \n") ;
		print_debug (".Method:  %s \n", p2p->method->name()) ;
		print_debug (".Version: %s \n", p2p->method->version());

		p2p->method->setOptions (p2p->options) ;
		p2p->method->doCalculation (p2p->path, p2p->result) ;
		/*
		 * print out information about the actual meteorological data used in the evaluation
		 * of the long-time averaged noise level. Note that this information is available on
		 * output of the calculation (because each calculation method may select the most
		 * appropriate meteorological model).
		 */
		PropagationPathOptions const& options = p2p->method->getOptions() ;
		if (options.meteo.model == MeteoCondition::ISO9613)
		{
			print_debug (".Meteo:   ISO-9613-2, C0=%.1f dB\n", options.meteo.C0) ;
		}
		else 
		{
			print_debug (".Meteo:   JRC-2012, pFav=%.1f%% \n", 100 * options.meteo.pFav) ;
		}
		/*
		 * write results to output file
		 */
		if (outputFile != 0 && strlen(outputFile) > 0)
		{
			print_debug ("Creating output file %s \n", outputFile) ;
			output_results_to_XML (outputFile, p2p->result) ;
		}
		p2p->calculationDone = true ;
		p2p->hasErrors = false ;
	}
	/*
	 * handle any error that might have occurred while reading or processing the file
	 */
	catch (ErrorMessage& err)
	{
		p2p->calculationDone = true ;
		p2p->hasErrors = true ;
		p2p->errorMessage = err.what () ;
		/*
		 * delete output file (if specified and existing)
		 */
		if (outputFile != 0 && _access (outputFile,0) == 0)
		{
			remove (outputFile) ;
		}
		print_debug ("No results available...\n") ;
		print_debug ("No output file generated\n") ;
	}
	/*
	 * reset current working directory to initial value
	 */
	if (old_dir) 
	{
		_chdir(old_dir) ;
		print_debug ("Reset current working directory to %s \n", old_dir)	;
		free (old_dir) ;
		free (new_dir) ;
	}

	return !p2p->hasErrors ;
}

bool CNOSSOS_P2P_GetPerformanceCounters (CNOSSOS_P2P_ENGINE* p2p, unsigned int & nbCalls, double & cpuTime)
{
	if (p2p == 0 || p2p->method == 0) 
	{
		nbCalls = 0 ;
		cpuTime = 0.0 ;
		return false ;
	}
	p2p->method->getPerformanceCounter (nbCalls, cpuTime) ;
	return true ;
}

void CNOSSOS_P2P_SetInfinityMode (bool on_off)
{
	SetInfinityMode (on_off) ;
}