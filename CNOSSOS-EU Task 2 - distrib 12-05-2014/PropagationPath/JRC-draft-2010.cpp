/* 
 * ------------------------------------------------------------------------------------------------
 * file:		JRC-draft-2010.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implement specific calculation methods prescribed in JRC-draft-2010 report
 * dependency:	actual calculation of excess attenuation relies on the HarmonoiseP2P module
 * changes:
 *
 *	18/10/2013	initial version
 *
 *  24/10/2013	added support for selecting predefined or user-defined material properties
 *
 *  25/10/2013	implemented correction for finite height of reflecting obstacles based on
 *				Fresnel weighting
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "JRC-draft-2010.h"
#include "PropagationPath.h"
#include "Material.h"
#include "VerticalExt.h"
#include <stdio.h>
#include <algorithm>

using namespace CnossosEU ;
using namespace Geometry ;

#ifdef _DEBUG
	static void show_p2p_details (void* p2p_struct) ;
	#define show_details(x) show_p2p_details(x)
#else
	#define show_details(x)
#endif

JRCdraft2010::JRCdraft2010 (void)
{
	p2p_struct = P2P_Create() ;

	double freq[] = { 63, 125, 250, 500, 1000, 2000, 4000, 8000 } ;
	P2P_SetFreqArray (p2p_struct, 8, freq) ;
	P2P_SetBandwidth (p2p_struct, 1.) ;
}

JRCdraft2010::~JRCdraft2010 (void)
{
	P2P_Delete (p2p_struct) ;
}

const char* JRCdraft2010::name (void)
{
	return "JRC_draft_2010" ;
}

const char* JRCdraft2010::version (void)
{
	double v = P2P_GetVersionDLL (p2p_struct) ;
	static char buffer[64] ;
	sprintf (buffer, "%.3f", v) ;
	return buffer ;
}

bool JRCdraft2010::initCalculation (void)
{
	path_defined = false ;
	user_defined = groundUserDefined ;
	/*
	 * fix mandatory options for the JRC-draft-2010 method
	 */
	if (!options.DisableLateralDiffractions)
	{
		print_debug (".fixed mandatory option: DisableLateralDiffractions \n") ;
		options.DisableLateralDiffractions = true ;
	}
	/*
	 * select default meteorological model
	 */
	if (options.meteo.model == MeteoCondition::DEFAULT)
	{
		print_debug (".using default meteorological model as defined in JRC-2010 \n") ;
		options.meteo.model = MeteoCondition::JRC2012 ;
	}
	return true ;
}

bool JRCdraft2010::exitCalculation (void)
{
	return true ;
}

static bool almost_equal (double a, double b)
{
	return (fabs(a - b) / (a+b)) < 0.9 ;
}
int JRCdraft2010::getMaterial (Material* mat)
{
	if (!mat) return groundClass_H ;
	/*
	 * allow for user-defined impedance values
	 */
	Impedance const* imp = mat->getImpedance() ;
	if (imp)
	{
		Impedance impVal = *imp ;
		int index = user_defined++ ;
		P2P_SetImpedance (p2p_struct, index, &impVal[0]) ;
		return index ;
	}
	/*
	 * use "nearest" predefined material category
	 */
	double sigma = mat->getSigmaValue() ;
	if (sigma < 16.0)   return groundClass_A ;	// sigma = 12.5
	if (sigma < 40.0)   return groundClass_B ;	// sigma = 31.50 
	if (sigma < 100.)   return groundClass_C ;	// sigma = 80.0
	if (sigma < 250.)   return groundClass_D ;	// sigma = 200. 
	if (sigma < 625.)   return groundClass_E ;	// sigma = 500.
	if (sigma < 2500.)  return groundClass_F ;	// sigma = 2000.
	if (sigma < 25000.) return groundClass_G ;	// sigma = 20000.
	return groundClass_H ;						// sigma = 200000.
}

void JRCdraft2010::createPath (PropagationPath& path)
{
	/*
	 * reset internal path buffer
	 */
	P2P_Clear (p2p_struct) ;
	/*
	 * use first control point as local origin
	 */
	double d0 = path[0].d_path ;
	double z0 = path[0].pos.z ;
	/*
	 * add control point, note that the first control point is implicit and has 
	 * local coordinates (d = 0, z = 0)
	 */
	for (unsigned int i = 1 ; i < path.size() ; ++i)
	{
		double d = path[i].d_path - d0 ;
		double z = path[i].pos.z - z0 ;
		int    g = getMaterial (path[i].mat) ;
		P2P_AddSegment (p2p_struct, d, z, g) ;
	}
	/*
	 * set source and receiver height. Note that the JRC-draft-2010 method is reciprocal so 
	 * that we don not need to bother with paths that are given in reverse order (i.e. starting 
	 * from the source, ending at the receiver)
	 *
	 * for validation purposes: create a receiver-to-source path and set the ForceSourceToReceiver 
	 * option alternatively to true or false. Results should be strictly identical.
	 */
	int n1 = 0 ;
	int n2 = path.size() - 1 ;
	assert (path[n1].ext != 0) ;
	assert (path[n2].ext != 0) ;
	double h1 = path[n1].ext->h ;
	double h2 = path[n2].ext->h ;
	double s1 = std::max (0.1, h1/10) ;
	double s2 = std::max (0.1, h2/10) ;
	P2P_SetSourceHeight (p2p_struct, h1, s1) ;
	P2P_SetReceiverHeight (p2p_struct, h2, s2) ; 
}

Spectrum JRCdraft2010::getExcessAttenuation (PropagationPath& path, bool favorable_condition)
{
	/*
	 * create the path inside the P2P module
	 */
	if (!path_defined) createPath (path) ;
	/*
	 * set options and sound speed profile
	 */
	P2P_SetOptions (p2p_struct, enableAveraging | enableScattering) ;
	double C_sound = options.meteo.getSoundSpeed() ;
	P2P_SetSoundSpeed (p2p_struct, C_sound) ;
	double A_meteo = favorable_condition ? 0.07 : 0.00 ;
	double B_meteo = 0 ;
	double C_meteo = 1.E-5 ;
	double D_meteo = 0.00 ;
	P2P_SetSoundSpeedProfile (p2p_struct, A_meteo, B_meteo, C_meteo, D_meteo) ;
	/*
	 * calculate and return results
	 */
	Spectrum att ;
	assert (P2P_GetNbFreq(p2p_struct) == att.size()) ;
	P2P_GetResults (p2p_struct, &att[0]) ;

	show_details (p2p_struct) ;
	return att ;
}

Spectrum  JRCdraft2010::getFiniteSizeCorrection (PropagationPath& path)
{
	Spectrum att(0.0) ;
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	for (unsigned int i = n1+1 ; i < n2 ; ++i)
	{
		if (path[i].mode2D == Action2D::Reflection || path[i].mode2D == Action2D::Diffraction)
		{
			assert (path[i].ext != 0) ;
			/*
			 * get caustic position to the left, as we operate in the vertical plane we
			 * consider horizontal edges to act as equivalent sources and/or receivers
			 */
			unsigned int i1 = i-1 ;
			while (i1 > n1 && path[i1].mode3D != Action3D::Diffraction) i1-- ;
			/*
			* get caustic position to the right
			*/
			unsigned int i2 = i+1 ;
			while (i2 < n2 && path[i2].mode3D != Action3D::Diffraction) i2++ ;
			/*
			 * geometry of Fresnel problem : equivalent source and receiver positions
			 */
			Point2D p1 (path[i1].d_path, path[i1].z_path) ;
			Point2D p2 (path[i2].d_path, path[i2].z_path) ;
			Point2D seg[2] ;
			/*
			 * geometry of Fresnel problem : equivalent segment includes the image of the barrier 
			 * on the ground, i.e. a reflection near the bottom of the barrier is not affected.
			 *
			 * note that this principle can easily be extended to tilted barriers...
			 */
			seg[0] = Point2D (path[i].d_path, path[i].pos.z - path[i].ext->h) ;
			seg[1] = Point2D (path[i].d_path, path[i].pos.z + path[i].ext->h) ;
			/*
			 * calculate weight and attenuation in frequency bands
			 */
			for (unsigned int k = 0 ; k < att.size() ; ++k)
			{
				double lambda = options.meteo.getSoundSpeed() / att.freq(k) ;
				double weight = PropagationPath::get_Fresnel_weighting (p1, p2, seg, lambda/4) ;
				att[k] += LOG10 (weight) ;
			}
		}
	}
	return att ;
}

static void show_p2p_details (void* p2p)
{
	const char* model_component[]=
	{
		"GLOBAL         ", 
		"DIFFRACTION    ",
		"GROUND         ",
		"   GROUND_LIN  ",
		"   GROUND_LOG  ",
		"   MIX_LOG_LIN ",
		"  DIFFR_BLOS   ",
		"  GROUND_BLOS  ",
		" DIFF+GROUND   ",
		" FLAT_GROUND   ",
		" MIX_DIF_FLAT  "
	} ;

	assert (P2P_GetNbFreq (p2p) == 8) ;

	printf ("---------------------------------------------------------------------------\n") ;
	printf ("Using HarmonoiseP2P.DLL V%.3f \n", P2P_GetVersionDLL (p2p)) ;
	double R = P2P_GetRayCurvature (p2p) ;
	if (R == 0)
		printf ("Propagation conditions: HOMOGENEOUS\n") ;
	else
		printf ("Propagation conditions: FAVOURABLE, R=%.0fm\n", 1/R) ;
	printf ("---------------------------------------------------------------------------\n") ;
	printf ("Component       POS SRC REC") ;
	Spectrum att ;
	for (unsigned int k = 0 ; k < 8 ; k++) 
		if (att.freq(k) < 1000) 
			printf ("%4.0fHz", att.freq(k)) ;
		else
			printf ("%3.0fkHz", att.freq(k)/1000) ;
	printf ("\n") ;
	printf ("---------------------------------------------------------------------------\n") ;

	for (int i = 0 ; i < P2P_GetNbDetails(p2p) ; i++)
	{
		int pos_dif ;
		int pos_rec ;
		int pos_src ;
		int model ;
		double att[8] ;

		P2P_GetDetails (p2p,i, &model, &pos_rec,&pos_dif, &pos_src, att) ;
	
		printf ("%s", model_component[model]) ;
		if (pos_dif > 0) 
			printf (" %3d", pos_dif) ;
		else
			printf ("    ") ;
		printf (" %3d %3d", pos_src, pos_rec) ;
		for (unsigned int k = 0 ; k < 8 ; k++) printf (" %5.1f", att[k]) ;
		
		printf ("\n") ;
	}

	printf ("---------------------------------------------------------------------------\n") ;
}

