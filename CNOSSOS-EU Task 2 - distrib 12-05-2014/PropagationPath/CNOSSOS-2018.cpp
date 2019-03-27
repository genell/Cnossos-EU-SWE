/* 
 * ------------------------------------------------------------------------------------------------
 * file:		CNOSSOS-2018.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implement proposed changes for amendment of CD-EU 2015/996
 * changes:
 *
 *	10/07/2018	initial version created
 *
 *  11/07/2018	experimental model for AttGround implemented and tested (see below)
 *
 *  12/07/2018	simplified model for AttGround implemented and tested
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include <algorithm>
#include "CNOSSOS-2018.h"
#include "PropagationPath.h"
#include "Material.h"
#include "VerticalExt.h"
/*
 * import name-spaces
 */
using namespace CnossosEU ;
using namespace Geometry ;
/*
 * constants
 */
static const double PI = 3.1415926 ;
static const double C0 = 340 ;
/*
 * for comparison with the experimental method set this option to "true"
 */
static bool use_sigma_version = false ;
/*
 * utility function
 */
template <class T> static T POW2 (T const t) { return t * t ; }
/*
 * standard interfaces for calculation methods
 */
CNOSSOS_2018::CNOSSOS_2018 (void)
{
}
CNOSSOS_2018::~CNOSSOS_2018 (void)
{
}
const char* CNOSSOS_2018::name (void)
{
	return "CNOSSOS-2018" ;
}
const char* CNOSSOS_2018::version (void)
{
	return "1.001" ;
}
/*
 * initialization of calculation options and parameters
 */
bool CNOSSOS_2018::initCalculation (void)
{
	attFavorable = false ;
	/*
	 * fix mandatory options for the CNOSSOS-2018 methods (inherited from JRC-2012
	 */
	if (!options.IgnoreComplexPaths)
	{
		print_debug (".fixed mandatory option: IgnoreComplexPaths \n") ;
		options.IgnoreComplexPaths = true ;
	}
	if (!options.ForceSourceToReceiver)
	{
		print_debug (".fixed mandatory option: ForceSourceToReceiver \n") ;
		options.ForceSourceToReceiver = true ;
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

Spectrum CNOSSOS_2018::getExcessAttenuation (PropagationPath& path, bool favorable_condition)
{
	Spectrum att (0.0) ;
	attFavorable = favorable_condition ;
	print_debug ("Start calculation for %s conditions\n", attFavorable ? "favorable" : "homogeneous") ;
	/*
	 * calculation of laterally diffracted paths (see eq.14 VI.33 and VI.34)
	 */
	if (path.info.pathType == PathInfo::LateralDiffractedPath)
	{
		assert (path.info.nbReflections == 0) ;
		assert (path.info.nbDiffractions == 0) ;
		assert (path.info.nbLateralDiffractions > 0) ;
		att = getGroundEffect (path) ;
	}
	/*
	 * special case of a path over perfectly flat ground
	 */
	else if (path.info.pathType == PathInfo::DirectPath)
	{
		att = getGroundEffect (path) ;
	}
	/*
	 * calculation of path blocked by at least one obstacle in the propagation plane
	 *
	 * note that the type of the path is determined assuming homogeneous propagation
	 * conditions and that, according to the text on p.90 (section VI.4.4) the type of 
	 * the path might change under favorable conditions, i.e. if the curved ray path 
	 * passes high enough above the diffraction edge (as shown in figure VI.10, 3rd case). 
	 *
	 * this implies that, under favorable propagation conditions, we need to consider
	 * both cases, with and without diffraction, and return the correct value based on
	 * the lambda/20 criterion.
	 */
	else if (path.info.pathType == PathInfo::DiffractedPath)
	{
		if (attFavorable)
			att = getGroundOrDiffraction (path) ;
		else
			att = getDiffraction (path) ;
	}
	/*
	 * the line of sight from the source to the receiver is not blocked by any obstacle 
	 * but propagation might be influenced by the potential effect of a barrier below 
	 * the line of sight.
	 */
	else
	{
		assert (path.info.pathType == PathInfo::PartialDiffractedPath) ;
		att = getGroundOrDiffraction (path) ;
	}
	return -att ;
}
/*
 * construct mean plan for a sequence of path control points
 */
static MeanPlane getMeanPlane (PropagationPath& path, unsigned int n1, unsigned int n2) 
{
	std::vector<Point2D> profile ;
	for (unsigned int i = n1 ; i <= n2 ; ++i)
	{
		profile.push_back (Point2D (path[i].d_path,path[i].pos.z)) ;
	}
	return MeanPlane (profile) ;
}
/*
 * generalized procedure for calculating averaged G value over a part of the propagation path
 */
static double getGpath (PropagationPath& path, double dmin, double dmax)
{
	assert (dmax >= dmin) ;
	if (dmax == dmin)
	{
		dmin -= 0.05 ;
		dmax += 0.05 ;
	}

	double sumD = 0 ;
	double sumG = 0 ;
	for (unsigned int i = 1 ; i < path.size() ; ++i)
	{
		double d1 = std::max (dmin, path[i-1].d_path) ;
		double d2 = std::min (dmax, path[i].d_path) ;
		if (d2 > d1)
		{
			double D = d2 - d1 ;
			double G = path[i].mat->getGValue() ;
			sumD += D ;
			sumG += G * D ;
		}
	}
	assert (sumD > 0) ;
	return sumD > 0 ? sumG / sumD : 0.0 ;
}
// --------------------------------------------------------------------------------------------
//
// Experimental version using sigma values
//
// This version has been used to get as close as possible to the Harmonoise/Imagine model
// based on Chien-Soroka's formula for calculating the spherical reflexion coefficient
//
// --------------------------------------------------------------------------------------------
/*
 * interpolation based on table (2.5.a)
 */
static double convert_G_to_sigma (double G)
{
	if (G < 0) G = 0 ;
	if (G > 1) G = 1 ;
	/*
	 * note: we might allow G values greater than 1 in order to compare with other methods
	 * e.g. to get results comparable with ISO 9613-2, we might use G = 1.25 in order to
	 * get the same position of the frequency dip in both methods.
	 */
	return 20000 * pow (10., -2 * pow (G, 3./5.)) ;
}
/*
 * equation (2.5.17) rewritten in its original form
 */
static double getW_sigma (double sigma, double freq, double dp)
{
	assert (sigma > 0) ;
	/*
	 * Delauny-Bazley impedance model
	 */
	double k = 2 * PI * freq / C0 ;
	double s = log (freq / sigma) ;
	double x = 1. + 9.08 * exp (-0.75 * s) ; 
	double y = 11.9 * exp (-0.73 * s) ;     
	/*
	 * return parameter W = k.d/|Z|²
	 */
	return k *  dp / (x*x + y*y) ;
}
/*
 * approximate reflexion coefficient as a real value, ignore phase shifts
 */
#include <complex>
static double getR_sigma (double sigma, double freq, double cos_teta)
{
	assert (sigma > 0) ;
	/*
	 * Delauny-Bazley impedance model
	 */
	double k = 2 * PI * freq / C0 ;
	double s = log (freq / sigma) ;
	double x = 1. + 9.08 * exp (-0.75 * s) ; 
	double y = 11.9 * exp (-0.73 * s) ;     
	/*
	 * calculate complex reflexion coefficient
	 */
	std::complex<double> Z (x,y) ;
	std::complex<double> R = (Z * cos_teta - 1.0 ) / (Z * cos_teta + 1.0) ;
	/*
	 * return apparent (real) reflexion coefficient
	 */
	return norm (1.0 + R) - 1.0 ;
}
/*
 * equation (2.5.16) in its original form, using sigma as input value instead of G
 */
static double getCf_sigma (double sigma, double freq, double dp)
{
	double wdp = getW_sigma (sigma, freq, dp) ;
	return (1 + 3 * wdp * exp(-sqrt(wdp))) / (1 + wdp) ;
}
/*
 * modified version of equation (2.5.15)
 */
static Spectrum getAttGround_sigma (double dp, double zs, double zr, double Gm)
{
	assert (dp > 0) ;
	assert (zr >= 0) ;
	assert (zs >= 0) ;
	assert (Gm >= 0) ;
	assert (Gm <= 1) ;
	/*
	 * convert G into equivalent sigma value
	 */
	double sigma = convert_G_to_sigma (Gm) ;
	/*
	 * approximate value of the specular reflexion coefficient 
	 */
	double R = 2 * exp (-3.0 * pow (Gm, 0.33)) - 1 ;
	printf ("R = %.3f \n", R) ;
	/*
	 * path length difference
	 */
	double dr = sqrt (POW2(dp) + POW2(zs+zr)) - sqrt (POW2(dp) + POW2(zs-zr)) ;
	/*
	 * adaptation of equation (2.5.15) 
	 */
	Spectrum att ;
	for (unsigned int i = 0 ; i < att.size() ; ++i)
	{
		double f = att.freq(i) ;
		double k = 2 * PI * f / C0 ;
		/*
		 * evaluate the equivalent reflection coefficient, i.e.based on the fact that 
		 * JdF's formulas are in fact an approximation of the Chien-Soroka analytical 
		 * formula for the ground effect for almost grazing sound propagation.
		 *
		 * In fact eq. (2.5.15) returns an approximation of 
		 *
		 *		Aground = -20 log (2.Fw) = -20 log (1 + R + (1-R).Fw)
		 *
		 * in the special case of R = -1, where R is the plane wave reflexion coefficient
		 *
		 *		R = (Z * cos_teta - 1) / (Z * cos_teta + 1)
		 *
		 * and cos_teta = 0 in case of grazing incidence.
		 */
		double Ak = 2 * k / dp ; 

		double Cf = getCf_sigma (sigma, f, dp) * dp / k ;
		//double Cf = getCf (Gm, f, dp) * dp / k ;
		assert (Cf >= 0) ;
		
		double As = zs * zs - sqrt (2 * Cf) * zs + Cf ;
		assert (As > 0) ;
		
		double Ar = zr * zr - sqrt (2 * Cf) * zr + Cf ;
		assert (Ar > 0) ;
		
		double F = std::min (0.5 * Ak * sqrt(As * Ar), 1.0)  ; 
		/*
		 * approximate value of the spherical reflexion coefficient
		 */
		double Q = R + (1 - R) * F ;
		/*
		 * limit ground effect in the high frequency range due to loss of coherency
		 *
		 * note: in case of destructive interference, we want "full" effect, but in case of
		 * constructive interference we must limit the full effect to +3dB in order to be
		 * compatible with the ISO methods for measuring the sound power.
		 */
		double C = (0.7 - 0.3 * Q) * (1 - exp (-1. / (0.5*k*dr))) ; 
		/*
		 * ground effect expressed as attenuation in decibels 
		 */
		att[i] = -LOG10 (POW2 (1 + C * Q)) ;
	}
	return att ;
}
// --------------------------------------------------------------------------------------------
//
// Proposed version for CNOSSOS-2018
//
// This version is based on a minimal adaptation of the method described in CD-EU 2015/996
// reproducing as much as possible the results of the experimental method above
//
// --------------------------------------------------------------------------------------------
/*
 * equation (2.5.17)
 */
static double getW (double Gw, double freq)
{
	assert (Gw >= 0) ;
	assert (freq > 0) ;
	double div = pow(freq,1.5) * pow(Gw,2.6) + 1.3 * 1000 * pow(freq,0.75) * pow(Gw,1.3) + 1.16 * 1000000;
	assert (div != 0) ;
	return 0.0185 * (pow(freq,2.5) * pow(Gw,2.6)) / div;
}
/*
 * equation (2.5.16)
 */
static double getCf (double Gw, double freq, double dp)
{
	double wdp = getW (Gw, freq) * dp ;
	double wdp_plus_1 = 1 + wdp ;
	assert (wdp_plus_1 != 0) ;
	return (1 + 3 * wdp * exp(-1 * sqrt(wdp))) / wdp_plus_1 ;
}
/*
 * modified version of equation (2.5.15)
 */
static Spectrum getAttGround (double dp, double zs, double zr, double Gm)
{
	if (use_sigma_version) return getAttGround_sigma (dp, zs, zr, Gm) ;

	assert (dp > 0) ;
	assert (zr >= 0) ;
	assert (zs >= 0) ;
	assert (Gm >= 0) ;
	assert (Gm <= 1) ;
	/*
	 * convert G into equivalent sigma value
	 */
	double sigma = convert_G_to_sigma (Gm) ;
	/*
	 * path length difference
	 */
	double dr = sqrt (POW2(dp) + POW2(zs+zr)) - sqrt (POW2(dp) + POW2(zs-zr)) ;
	/*
	 * adaptation of equation (2.5.15) 
	 */
	Spectrum att ;
	for (unsigned int i = 0 ; i < att.size() ; ++i)
	{
		double f = att.freq(i) ;
		double k = 2 * PI * f / C0 ;
		/*
		 * use equation (2.5.15) as the calculation of an equivalent reflexion
		 * coefficient R such that Agound = 20 log (1 + R)
		 */
		double Ak = 2 * k / dp ; 

		double Cf = getCf (Gm, f, dp) * dp / k ;
		assert (Cf >= 0) ;
		
		double As = zs * zs - sqrt (2 * Cf) * zs + Cf ;
		assert (As > 0) ;
		
		double Ar = zr * zr - sqrt (2 * Cf) * zr + Cf ;
		assert (Ar > 0) ;
		
		double R = Ak * sqrt(As * Ar) - 1.0  ; 
		/*
		 * clip the reflexion coefficient in order to keep the ground effect in the
		 * range [-20 dB, +3 dB]
		 */
		if (R < -0.90) R = -0.90 ;
		if (R >  0.40) R =  0.40 ;
		/*
		 * limit ground effect in the high frequency range due to loss of coherency
		 */
		double C = 1 - exp (-1. / (k*dr)) ; 
		/*
		 * ground effect expressed as attenuation in decibels 
		 */
		att[i] = -LOG10 (POW2 (1 + C * R)) ;
	}
	return att ;
}
/* 
 * evaluate the ground effect over the path
 */
Spectrum CNOSSOS_2018::getGroundEffect (PropagationPath& path, unsigned int n1, unsigned int n2) 
{
	unsigned int m1 = 0 ;
	unsigned int m2 = path.size() -1 ;
	assert (n1 == m1 || n2 == m2) ;
	print_debug ("Calculate ground effect between positions %u and %u \n", n1, n2) ;
	/*
	 * calculate the mean plane
	 */
	MeanPlane mean_plane = getMeanPlane (path, n1, n2) ;
	print_debug (".mean plane O (%.2f %.2f), Ox = (%.2f, %.2f), Oy = (%.2f, %.2f) \n", 
		          mean_plane.origin.x, mean_plane.origin.y,
				  mean_plane.x_axis.x, mean_plane.x_axis.y,
				  mean_plane.y_axis.x, mean_plane.y_axis.y) ;
	/*
	 * in case of diffraction, save the mean plane because it is needed to calculate
	 * delta_dif(S,O) and delta_dif (O,R)
	 */
	if (n1 != m1) mean_plane_OR = mean_plane ;
	if (n2 != m2) mean_plane_SO = mean_plane ;
	/*
	 * in case of diffraction, the fictive source or receiver is the profile point corresponding 
	 * to the diffracting edge
	 */
	Point2D  S_path (path[n1].d_path, (n1 == m1) ? path[n1].z_path : path[n1].pos.z) ;
	Point2D  R_path (path[n2].d_path, (n2 == m2) ? path[n2].z_path : path[n2].pos.z) ;
	/*
	 * project source and receiver on the mean plane, make sure z >= 0
	 */
	Vector2D S = mean_plane.project (S_path, true) ;
	Vector2D R = mean_plane.project (R_path, true) ;
	/*
	 * see figures VI.6 and VI.8
	 *
	 * dp = propagation distance projected on the mean plane
	 * hs = height of the source relative to the mean plane
	 * hr = height of the receiver relative to the mean plane
	 */
	double dp = std::max (0.05, R.x - S.x) ;
	double hs = std::max (0.05, S.y) ;
	double hr = std::max (0.05, R.y) ;

	print_debug (".dp = %.2f zs = %.2f, zr = %.2f \n", dp, hs, hr) ;
	/*
	 * equivalent height of source and receiver under homogeneous / favorable propagation conditions
	 */	
	double Ra = std::max (8 * dp, 5000.) ;
	double dR = attFavorable ? 0.125 * dp * dp / Ra : 0.0 ;
	double zr = hr + POW2(hr) / (POW2(hs) + POW2(hr)) * dR ;
	double zs = hs + POW2(hs) / (POW2(hs) + POW2(hr)) * dR ;
	double zm = (zs + zr)/2 ;
	/*
	 * pseudo Fresnel weighting left/right of the specular reflexion point
	 */
	double d1 = path[n1].d_path ;
	double d3 = path[n2].d_path ;
	double d2 = d1 + (d3 - d1) * zs / (zs + zr) ;
	double Gs = getGpath (path, d1, d2) ;
	double Gr = getGpath (path, d2, d3) ;
	Spectrum As = getAttGround (dp, zs, zr, Gs) ;
	Spectrum Ar = getAttGround (dp, zs, zr, Gr) ;
	/*
	 * correction for additional paths in case of favorable propagation conditions
	 */
	double q = 0.0 ;
	if (attFavorable) q = exp (-30. * (hs + hr) / dp) ;
	/*
	 * evaluate averaged ground reflexion coefficient to be used for additional reflexions
	 * in case of favorable propagation conditions.
	 */
	Spectrum Am ;
	if (q > 0)
	{
		double Gm = getGpath (path, d1, d3) ;
		Am = getAttGround (dp, zm, zm, Gm) ;
	}
	/*
	 * limit attenuation due to turbulent scattering
	*/		
	Spectrum Ad ;
	for (int i = 0 ; i < Ad.size() ; ++i)
	{
		Ad[i] = 25 + 0.3 * LOG10 (As.freq(i)/1000) - LOG10 (dp/100) ;
	}
	/*
	 * return combined effects as an equivalent ground effect
	 */
	Spectrum Ag ;
	for (int i = 0 ; i < Ag.size() ; ++i)
	{	/*
		 * Rs = specular reflexion or principal reflection near the source or the receiver,
		 *	    is calculated based on pseudo Fresnel weighting centered at the specular
		 *		reflexion point.
		 */
		double Rs = POW10 (-(Ar[i] + As[i])/2) - 1.0 ;
		/*
		 * Rm = extra reflections in case of favorable propagation conditions
		 */
		double Rm = std::max (0.0, POW10 (-Am[i]) - 1) ;
		/*
		 * Rd = scattering due to atmospheric turbulence
		 */
		double Rd = POW10 (-Ad[i]) ;
		/* 
		 * sum of all contributions, note we can adjust the "q" factor to take into account
		 * higher order reflections
		 */
		Ag[i] = -LOG10 (1 + Rs + 3 * q * Rm + Rd) ;
	}
	return Ag ;
}

/*
 * calculate attenuation due to ground effect in the propagation plane
 */
Spectrum CNOSSOS_2018::getGroundEffect (PropagationPath& path)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	return getGroundEffect (path, n1, n2) ;
}
/*
 * Equation VI-21
 */
static double getDeltaDif (double z, double lambda, double e, double Ch = 1.0)
{
	double Cpp = 1 ;
	if (e > 0.3)
	{
		double x = 5 * lambda / e ;
		Cpp = (1 + x * x) / (1./3 + x * x) ;
	}
	double x = 3 + (40/lambda)* Cpp * z  ;
	/*
	 * take x >= 1 so that Dz >= 0 dB in all cases
	 */
	if (x < 1) x = 1.0 ;
	return Ch * LOG10 (x) ;
}
/*
 * Equation VI-21
 */
Spectrum CNOSSOS_2018::getDeltaDif (double z, double e, double Ch)
{
	Spectrum att ;
	for (unsigned int i = 0 ; i < att.size() ; ++i)
	{
		double lambda = options.meteo.getSoundSpeed() / att.freq(i) ;
		att[i] = ::getDeltaDif (z, lambda, e, Ch) ;
	}
	return att ;
}
/*
 * formula VI.25
 */
static double arc_length (Point2D const& p1, Point2D const&p2, double gamma)
{
	return 2 * gamma * asin (dist(p1,p2) / (2 * gamma)) ;
}
/*
 * calculate total length of a polygonal line
 */
static double total_length (std::vector<Point2D> const& O)
{
	double length = 0 ;
	for (unsigned int i = 1 ; i < O.size() ; ++i) length += dist (O[i-1], O[i]) ;
	return length ;
}
/*
 * section VI.4.4.c
 */
static double getPathDifference (Point2D const& S, std::vector<Point2D> const& O, Point2D const& R, 
								 double gamma = 0, double *e = NULL)
{
	/*
	 * check whether the diffracting edge blocks the straight ray from S to R
	 *
	 * note that we cannot rely on information already stored in the propagation path because this 
	 * function will be called for both the real source and receiver but also their images 
	 */
	bool blos = false ;
	if (O.size() == 1)
	{
		blos = PropagationPath::get_path_difference (S, O[0], R) < 0 ;
	}

	double dist_diff = 0 ;
	if (gamma == 0)
	{
		/*
		 * figure VI.9 (used both in the propagation plane and for lateral diffraction)
		*/
		dist_diff += total_length (O) ;
		if (e != NULL) *e = dist_diff ;
		dist_diff += dist (S, O.front()) ;
		dist_diff += dist (O.back(), R) ;
		dist_diff -= dist (S,R) ;
		if (blos) dist_diff = -dist_diff ;
	}
	else
	{
		/*
		 * figure VI.10 (favorable conditions in the propagation plane)
		*/
		if (blos)
		{
			assert (O.size() == 1) ;
			double d1 = O[0].x - S.x ;
			double d2 = R.x - S.x ;
			Point2D A = S + (R - S) * (d1/d2) ;
			dist_diff = 2*arc_length (S, A, gamma) + 2*arc_length (A, R, gamma) 
				      - arc_length (S, O[0], gamma) - arc_length (O[0], R, gamma) - arc_length (S, R, gamma) ;
		}
		else
		{
			for (unsigned int i = 1 ; i < O.size() ; ++i) dist_diff += arc_length (O[i-1], O[i], gamma) ;
			if (e != NULL) *e = dist_diff ;
			dist_diff += arc_length (S, O.front(), gamma) ;
			dist_diff += arc_length (O.back(), R, gamma) ;
			dist_diff -= arc_length (S, R, gamma) ;
		}
	}
	return dist_diff ;
}
/*
 * calculate attenuation due to diffraction in the propagation plane
 *
 * note that we ALWAYS set Ch=1 in formula VI-21 because:
 *
 * 1) it is unclear how to calculate h0 in case of multiple diffraction
 * 2) it is unclear how to calculate h0 under favorable propagation conditions
 * 3) numerical experiments with the NMPB-2008 method have shown strange behavior of the
 *    method in case of diffraction caused by a low obstacle (h0 almost zero) that doesn't 
 *	  block the direct line of sight
 * 4) correction for low height obstacles is redundant with Raleigh's "flatness" criterion 
 *	  used to determine whether diffraction should be included or not. Raleigh's criterion 
 *    is global, i.e. it evaluates the flatness of the terrain over the whole propagation 
 *    distance between the source and the receiver.The Ch correction is local and independent
 *	  of source and receiver positions relative to  the diffracting edge. A typical case where
 *    the Ch correction certainly doesn't work is the two-plane wedge case which would always 
 *    have h0=0 and thus Ch=0. One would (wrongly) conclude from this that a wedge never has 
 *    any diffraction effect, no matter the opening angle of the wedge.
 */
Spectrum CNOSSOS_2018::getDiffraction (PropagationPath& path)
{
	print_debug ("Calculate diffraction \n") ;
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	/*
	 * create source S, receiver R and list of diffraction points O[i], i = 1...N
	 */
	Point2D S (path[n1].d_path, path[n1].z_path) ;
	Point2D R (path[n2].d_path, path[n2].z_path) ;
	std::vector<Point2D> O ;
	unsigned int pos_O1 = -1 ;
	unsigned int pos_On = -1 ;
	for (unsigned int i = n1+1 ; i < n2 ; ++i)
	{
		if (path[i].mode3D == Action3D::Diffraction || path[i].mode3D == Action3D::DiffractionBLOS) 
		{
			O.push_back (Point2D (path[i].d_path, path[i].pos.z)) ;

			if (pos_O1 == -1) pos_O1 = i ;
			pos_On = i ;
		}
	}
	assert (pos_O1 != -1) ;
	assert (pos_On != -1) ;
	/*
	 * calculate path difference
	 */
	double e = 0 ;
	double d = 0 ;
	double gamma = 0 ;
	if (attFavorable) 
	{
		/*
		 * evaluate 1/r, the inverse of the ray curvature by means of formula VI.24
		 */
		gamma = std::max (1000., 8.*(path[n2].d_path - path[n1].d_path));
		/*
		 * get path difference under favorable propagation conditions (see figure VI.10)
		 */
		d = getPathDifference (S, O, R, gamma, NULL) ;
		/*
		 * determine e under homogeneous conditions (see figureVI.9) ??
		 */
		e = total_length(O) ;
	}
	else
	{
		/* 
		 * get path difference under homogeneous conditions (see figure VI.9)
		 */
		d = getPathDifference (S, O, R, gamma, &e) ;
	}
	/*
	 * for debugging only
	 */
	print_debug (".path_difference = %.5f (e = %.2f)\n", d, e) ;
	if (path.info.pathType == PathInfo::DiffractedPath)
	{
		if (d < 0) print_debug ("WARNING: path difference < 0 for a diffracted path \n") ;
	}
	else if (path.info.pathType == PathInfo::PartialDiffractedPath)
	{
		if (d > 0) print_debug ("WARNING: path difference > 0 for a direct path \n") ;
	}
	/*
	 * evaluate pure diffraction effect
	 */
	Spectrum delta_dif_SR = getDeltaDif (d, e) ;
	/*
	 * calculate ground effect on source and receiver side
	 */
	Spectrum Aground_SO = getGroundEffect (path, n1, pos_O1) ;
	Spectrum Aground_OR = getGroundEffect (path, pos_On, n2) ;
	/*
	 * get weighting function on the source side
	 */
	Vector2D Si = mean_plane_SO.image (S) ;
	double   dS = getPathDifference (Si, O, R, gamma) ;
	Spectrum delta_dif_SO = getDeltaDif (dS, e) ;
	/*
	 * get weighting function on the receiver side
	 */
	Vector2D Ri = mean_plane_OR.image (R) ;
	double   dR = getPathDifference (S, O, Ri, gamma) ;
	Spectrum delta_dif_OR = getDeltaDif (dR, e) ;
	/*
	* add weighted ground effects
	*/
	print_debug ("Diffraction + ground \n") ;
	Spectrum delta_dif ;
	for (unsigned int i = 0 ; i < delta_dif.size() ; ++i)
	{
		/*
		 * get pure diffraction effect, apply lower limit =25 dB as specified in section VI.4.4.b
		 */
		double delta_pure_diff = std::min (25.0, delta_dif_SR[i]) ;
		/*
		 * weighted ground effect on the source side according to eq. VI-31
		 */
		double delta_ground_SO = -LOG20 (1 + (POW20(-Aground_SO[i]) - 1) * POW20 (delta_dif_SR[i] - delta_dif_SO[i])) ;
		/*
		 * weighted ground effect on the receiver side according to eq. VI-32
		 */
		double delta_ground_OR = -LOG20 (1 + (POW20(-Aground_OR[i]) - 1) * POW20 (delta_dif_SR[i] - delta_dif_OR[i])) ;
		/*
		 * attenuation in case of diffraction according to eq. VI-30
		 */
		print_debug (".freq=%5.0fHz Dif(SR)=%6.2f Dgr(SO)=%6.2f Dgr(OR)=%6.2f", delta_dif.freq(i), delta_pure_diff, delta_ground_SO, delta_ground_OR) ;
		delta_dif[i] = delta_pure_diff + delta_ground_SO + delta_ground_OR ;
		print_debug (" Adif=%6.2f\n", delta_dif[i]) ;
	}
	/*
	 * save path differences used to evaluate Raleigh's flatness criterion
	 */
	path_difference_SR = d ;
	path_difference_SiRi = getPathDifference (Si, O, Ri, gamma) ;
	/*
	 * return attenuation due to diffraction + ground
	 */
	return delta_dif ;
}
/*
 * calculate the attenuation due to diffraction around a vertical edge
 *
 * note that the standard doesn't give details for the calculation of the parameters of a laterally 
 * diffracted path.. we assume that all parameters (e.g. z and e) can be determined (up to sufficient 
 * accuracy) from the projection of the path on the horizontal plane.
 */
Spectrum CNOSSOS_2018::getLateralDiffraction (PropagationPath& path)
{
	if (path.info.pathType != PathInfo::LateralDiffractedPath) return Spectrum (0.0);

	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	/*
	 * create source S, receiver R and list of diffraction points O[i], i = 1...N
	 */
	Point2D S (path[n1].pos.x, path[n1].pos.y) ;
	Point2D R (path[n2].pos.x, path[n2].pos.y) ;
	std::vector<Point2D> O ;
	for (unsigned int i = n1+1 ; i < n2 ; ++i)
	{
		if (path[i].mode2D == Action2D::Diffraction) 
		{
			O.push_back (Point2D (path[i].pos.x, path[i].pos.y)) ;
		}
	}
	assert (O.size() > 0) ;
	/*
	 * calculate path difference
	 */
	double e = 0 ;
	double gamma = 0 ;
	/* 
	 * never consider diffraction below the line of sight !
	 */
	double d = fabs (getPathDifference (S, O, R, gamma, &e)) ;
	return -getDeltaDif (d, e) ;
}
/*
 * mixed model for almost diffracted paths
 * 
 * note that, under favorable conditions, this model is also used instead of the pure diffraction
 * model because curved rays may change the type of the path, i.e. the path difference used in the
 * criteria given in the introduction of section VI.4.4 must be determined separately under favorable 
 * and homogeneous conditions (it even may change sign); it is therefore possible that the diffraction 
 * effect is dominant under homogeneous conditions but not under favorable conditions...
 */
Spectrum CNOSSOS_2018::getGroundOrDiffraction (PropagationPath& path)
{
	return getGroundEffect (path) ;

	Spectrum Adif = getDiffraction (path) ;
	if (path_difference_SR > 0) return Adif ;

	Spectrum Agr  = getGroundEffect (path) ;
	Spectrum Att ;

	print_debug ("Select ground or diffraction \n") ;
	print_debug (".Delta = %.5f, delta_image = %.5f \n", path_difference_SR, path_difference_SiRi) ;
	for (unsigned int i = 0 ; i < Att.size() ; ++i)
	{
		double lambda = options.meteo.getSoundSpeed() / Att.freq(i) ;
		/*
		 * test Raleigh's flatness criterion
		 */
		if ((path_difference_SR + path_difference_SiRi) < lambda/4)
		{
			print_debug (".freq=%5.0fHz: ground (flatness) \n", Att.freq(i)) ;
			Att[i] = Agr[i] ;
		}
		/*
		 * test for significant diffraction effect
		 */
		else if (path_difference_SR < -lambda/20)
		{
			print_debug (".freq=%5.0fHz: ground (delta < -lambda/20) \n", Att.freq(i)) ;
			Att[i] = Agr[i] ;
		}
		else 
		{
			print_debug (".freq=%5.0fHz: diffraction \n", Att.freq(i)) ;
			Att[i] = Adif[i] ;
		}
	}
	return Att ;
}
/*
 * Get correction for the finite size of the reflecting obstacles.
 */
Spectrum CNOSSOS_2018::getFiniteSizeCorrection (PropagationPath& path)
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
			 * equivalent source and receiver positions
			 */
			Point2D p1 (path[i1].d_path, path[i1].z_path) ;
			Point2D pd (path[i].d_path, path[i].pos.z + path[i].ext->h) ;
			Point2D p2 (path[i2].d_path, path[i2].z_path) ;
			/*
			 * path difference (with opposite sign)
			 */
			double z = -PropagationPath::get_path_difference (p1, pd, p2) ;
			/*
			 * e is set to zero and equation VI.37 is equivalent to eq. VI.22
			 */
			double e = 0 ;
			/*
			 * no idea how Ch might be defined for reflecting obstacles
			 */
			double Ch = 1 ;
			/*
			 * evaluate retro-diffraction effect according to eq.14 VI.37
			 */
			att += getDeltaDif (z, e, Ch) ;
		}
	}
	return -att ;
}
