/* 
 * ------------------------------------------------------------------------------------------------
 * file:		JRC-2012.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implement specific calculation methods prescribed in JRC-2012 final report
 * changes:
 *
 *	29/10/2013	initial version
 *
 *  30/10/2013	moved calculation of mean planes into separate files
 *
 *  04/11/2013	ground effect and diffraction implemented
 *
 *  05/11/2013	finite height correction and lateral diffraction implemented
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "JRC-2012.h"
#include "PropagationPath.h"
#include "Material.h"
#include "VerticalExt.h"
#include <algorithm>

using namespace CnossosEU ;
using namespace Geometry ;

JRC2012::JRC2012 (void)
{
}

JRC2012::~JRC2012 (void)
{
}

const char* JRC2012::name (void)
{
	return "JRC_final_2012" ;
}

const char* JRC2012::version (void)
{
	return "1.001" ;
}

bool JRC2012::initCalculation (void)
{
	attFavorable = false ;
	/*
	 * fix mandatory options for the JRC-2010 method
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

Spectrum JRC2012::getExcessAttenuation (PropagationPath& path, bool favorable_condition)
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
 * calculation of Gpath according to figure VI.7 
 * 
 * note that no explicit equation is given in the text; we assume that the weighting
 * of G values is based on the length of the segments as projected on the horizontal 
 * plane (as it is in the ISO 9613-2 standard).
 */
static double getGpath (PropagationPath& path, unsigned int n1, unsigned int n2)
{
	double sumD = 0 ;
	double sumG = 0 ;
	for (unsigned int i = n1+1 ; i <= n2 ; i++)
	{
		double D = path[i].d_path - path[i-1].d_path ;
		double G = path[i].mat->getGValue() ;
		sumD += D ;
		sumG += G * D ;
	}
	assert (sumD > 0) ;
	return sumG / sumD ;
}
/*
 * by convention, the material under the source is stored in the first control point
 */
static double getGsource (PropagationPath& path)
{
	assert (path.size() >= 2) ;
	assert (path[0].mode2D == Action2D::Source) ;
	assert (path[0].mat != 0) ;
	assert (path[0].mat == path[1].mat) ;
	return path[0].mat->getGValue() ;
}
/*
 * equation VI.14
 * 
 * be aware that dp, zs and zr must be calculated relative to the mean ground plane as 
 * shown in figure VI.6 of the reference document.
 */
static double getGprime (double dp, double zs, double zr, double Gpath, double Gs)
{
	double dpMin = 30 * (zs + zr) ;
	if (dp > dpMin) return Gpath ;
	double x = dp/dpMin ;
	return x * Gpath + (1 - x) * Gs ;
}
/*
 * equation VI.17, p.88
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
 * equation VI.16, p.88
 */
static double getCf (double Gw, double freq, double dp)
{
	double wdp = getW (Gw, freq) * dp ;
	double wdp_plus_1 = 1 + wdp ;
	assert (wdp_plus_1 != 0) ;
	return dp * (1 + 3 * wdp * exp(-1 * sqrt(wdp))) / wdp_plus_1 ;
}
/*
 * equation VI.18 p.88
 */
static double getAttGroundHmin (double Gm)
{
	return -3 * (1 - Gm) ;
}
/*
 * equation VI.20 p.88
 */

static double getAttGroundFmin (double dp, double zs, double zr, double Gm)
{
	double x = 30 * (zs + zr) / dp ;
	if (x >= 1)
	{
		return -3 * (1 - Gm) ;
	}
	else
	{
		return -3 * (1 - Gm) * (1 + 2 * (1 - x)) ;
	}
}
/*
 * equation VI.15, p.88
 *
 * very strange... sections VI.4.3.c and VI.4.3.d consider a special case for Gpath = 0 whereas
 * calculations are based on Gw and Gm, and do not use Gpath explicitly... moreover, some problems 
 * were found in NMPB-2008 when using the special case under favorable propagation conditions.
 *
 * opinion of the author: the case Gpath = 0 has been introduced for optimization purposes only
 * but creates more trouble than it solves. For a "reference" implementation, we might just as well
 * remove the special case all together.
 */
Spectrum JRC2012::getGroundEffect (double dp, double zs, double zr, double Gpath, double Gw, double Gm)
{
	const double PI = 3.1415926 ;
	const double a0 = 2.E-4 ;
	double attMin ;

	if (attFavorable)
	{
		/* 
		 * equation VI.19
		 *
		 * note: the reference text doesn't say what to do in the case zs = zr = 0
		 * proposed solution : ignore height corrections...
		 */
		double zsr = zs + zr ;
		if (zsr > 0)
		{
			double dzt = 0.006 * dp / zsr ; 
			zs = zs + a0 * pow (zs * dp / zsr, 2) / 2 + dzt ;
			zr = zr + a0 * pow (zr * dp / zsr, 2) / 2 + dzt ;
		}
		attMin = getAttGroundFmin (dp, zs, zr, Gm) ;
	}
	else
	{
		attMin = getAttGroundHmin (Gm) ;
	}
	/*
	 * special case in both homogeneous and favorable conditions
	 *
	 * as a consequence of test cases, it might be decided NOT to use the simplification
	 * under favorable conditions, or even to remove oit all together...
	 */
	if (Gpath == 0 /* && !attFavorable */) return Spectrum (attMin) ;
	/*
	 * general case
	 */
	Spectrum att ;
	/*
	 * Last minute modification by Dirk Van Maercke (12/05/2014)
	 *
	 * For high sources or receivers, projection on the mean plane may lead to negative
	 * propagation distances. Taking the square root of a negative value leads to undefined
	 * results and/or application crashing.
	 *
	 * Proposed solution: ignore ground reflections in case of negative propagation distances 
	 */
	if (dp > 0)
	{
		for (unsigned int i = 0 ; i < att.size() ; ++i)
		{
			double Cf = getCf (Gw, att.freq(i), dp) ;
			assert (Cf >= 0) ;
			double k = 2 * PI * att.freq(i) / options.meteo.getSoundSpeed() ;
			assert (dp > 0) ;
			double Ak = pow (2 * k / dp, 2) ; 
			double As = zs * zs - sqrt (2 * Cf/k) * zs + Cf/k ;
			assert (As > 0) ;
			double Ar = zr * zr - sqrt (2 * Cf/k) * zr + Cf/k ;
			assert (Ar > 0) ;
			double Ag = -LOG10 (Ak * As * Ar) ;
			att[i] = std::max (Ag, attMin) ;
		}
	}
	return att ;
}
/*
 * table VI.2, p.88
 */
void JRC2012::getGroundParameters (PropagationPath& path, unsigned int n1, unsigned int n2,  
								   double dp, double zs, double zr, double& Gpath, double& Gw, double& Gm)
{
	Gpath   = getGpath (path, n1, n2) ;
	/*
	 * n1 == 0 in case of Aground or Delta_ground(S,O)
	 */
	if (n1 == 0)
	{
		double Gsource = getGsource (path) ;
		double Gprime  = getGprime (dp, zs, zr, Gpath, Gsource);
		Gm = Gprime ;
		Gw = attFavorable ? Gpath : Gprime ;
	}
	else
	{
		Gw = Gm = Gpath ;
	}
}
/* 
 * evaluate the ground effect over the path
 */
Spectrum JRC2012::getGroundEffect (PropagationPath& path, unsigned int n1, unsigned int n2) 
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
	 * zs = height of the source relative to the mean plane
	 * zr = height of the receiver relative to the mean plane
	 */
	double dp = R.x - S.x ;
	double zs = S.y ;
	double zr = R.y ;
	print_debug (".dp = %.2f zs = %.2f, zr = %.2f \n", dp, zs, zr) ;
	/*
	 * determine Gpath, Gw and Gm
	 */
	double Gpath ;
	double Gw ;
	double Gm ;
	getGroundParameters (path, n1, n2, dp, zs, zr, Gpath, Gw, Gm) ;
	print_debug (".Gpath = %.2f, Gw = %.2f Gm = %.2f \n", Gpath, Gw, Gm) ;
	/*
	 * calculate the ground effect as a function of dp, zs, zr and the G values
	 */
	Spectrum att = getGroundEffect (dp, zs, zr, Gpath, Gw, Gm) ;
	for (unsigned int i = 0 ; i < att.size() ; ++i)
	{
		print_debug (".freq=%5.0fHz Agr=%5.2f \n", att.freq(i), att[i]) ;
	}
	return att ;
}
/*
 * calculate attenuation due to ground effect in the propagation plane
 */
Spectrum JRC2012::getGroundEffect (PropagationPath& path)
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
Spectrum JRC2012::getDeltaDif (double z, double e, double Ch)
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
Spectrum JRC2012::getDiffraction (PropagationPath& path)
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
Spectrum JRC2012::getLateralDiffraction (PropagationPath& path)
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
	double d = getPathDifference (S, O, R, gamma, &e) ;
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
Spectrum JRC2012::getGroundOrDiffraction (PropagationPath& path)
{
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
Spectrum JRC2012::getFiniteSizeCorrection (PropagationPath& path)
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
