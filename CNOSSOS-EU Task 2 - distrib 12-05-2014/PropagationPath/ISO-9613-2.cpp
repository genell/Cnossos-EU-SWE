/* 
 * ------------------------------------------------------------------------------------------------
 * file:		ISO-9613-2.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implements the calculation of propagation effects as described in the
 *				ISO 9613-2:1996 standard
 * changes:
 *
 *	25/10/2013	initial version
 *
 *  28/10/2013	first complete version, including lateral diffraction and height corrections
 *
 *  29/10/2013	implementation of alternative solutions for height corrections, using either
 *				3D coordinates or 2D coordinates in the unfolded propagation plane.
 *
 *  30/10/2013	added limitations to 20, respectively 25 dB for diffraction over obstacles
 *				note that this limitation does not apply to laterally diffracted paths
 *
 *  06/12/2013	bug fixed in the calculation and use of Gr, Gm and Gs
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "ISO-9613-2.h"
#include "VerticalExt.h"
#include "Material.h"
#include <algorithm>

using namespace CnossosEU ;
using namespace Geometry ;

const char* ISO_9613_2::name (void)
{
	return "ISO-9613-2:1996" ;
}

const char* ISO_9613_2::version (void)
{
	return "1.001" ;
}

bool ISO_9613_2::initCalculation (void)
{
	/*
	 * fix mandatory options for the JRC-draft-2010 method
	 */
	if (!options.CheckHeightLowerBound)
	{
		print_debug (".fixed mandatory option: CheckHeightLowerBound \n") ;
		options.CheckHeightLowerBound = true ;
	}
	if (!options.CheckHeightUpperBound)
	{
		print_debug (".fixed mandatory option: CheckHeightUpperBound \n") ;
		options.CheckHeightUpperBound = true ;
	}
	if (!options.CheckLateralDiffraction)
	{
		print_debug (".fixed mandatory option: CheckLateralDiffraction \n") ;
		options.CheckLateralDiffraction = true ;
	}
	if (!options.IgnoreComplexPaths)
	{
		print_debug (".fixed mandatory option: IgnoreComplexPaths \n") ;
		options.IgnoreComplexPaths = true ;
	}
	/*
	 * select default meteorological model
	 */
	if (options.meteo.model == MeteoCondition::DEFAULT)
	{
		print_debug (".using default meteorological model as defined in ISO-9613-2 \n") ;
		options.meteo.model = MeteoCondition::ISO9613 ;
	}
	
	return true ;
}

Spectrum ISO_9613_2::getExcessAttenuation (PropagationPath& path, bool favorable_condition)
{
	Spectrum att(0.0) ;
	/*
	 * ISO 9613-2 supports a single meteorological condition called "downwind"
	 */
	if (!favorable_condition) return Spectrum (negative_inf) ;

	/*
	 * calculation of laterally diffracted paths (see equations 4 and 13)
	 */
	if (path.info.pathType == PathInfo::LateralDiffractedPath)
	{
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
	 * note that we do not implement eq. 12 because it turns out impossible "to calculate
	 * the ground attenuation in absence of the barrier (i.e. with the screening obstacle
	 * removed)" for anything else than a thin barrier.
	 *
	 * assumption: if the path is blocked, diffraction is dominant over ground reflections
	 */
	else if (path.info.pathType == PathInfo::DiffractedPath)
	{
		att = getDiffraction (path) ;
	}
	/*
	 * line of sight from the source to the receiver is not blocked by any obstacle but
	 * might be influenced by a potential barrier below the line of sight.
	 *
	 * note that we do not implement eq. 12 because, in case of low sources, diffraction by
	 * the order of the infrastructure will be dominant in almost all configurations, thus
	 * ignoring all effects of soft ground...
	 *
	 * in order to determine whether an obstacle below the line of sight has a significant 
	 * effect or not, we use a transition based on Raleigh's criterion.
	 */
	else
	{
		assert (path.info.pathType == PathInfo::PartialDiffractedPath) ;
		att = getGroundOrDiffraction (path) ;
	}
	return -att ;
}
/*
 * evaluate partial ground effects
 */
Spectrum getAground (double G, double h, double dp)
{
	Spectrum att ;
	assert (att.size() == 8) ;

	assert (att.freq(0) == 63) ;
	att[0] = -1.5 ; 

	assert (att.freq(1) == 125) ;
	att[1] = -1.5 + G * (1.5 + 3.0 * exp (-.12 * (h-5.) * (h-5.)) * (1. - exp (-dp/50.))
		   + 5.7 * exp (-.09 * h * h) * (1. - exp (-2.8E-6 * dp * dp))) ;
	
	assert (att.freq(2) == 250) ;
	att[2] = -1.5 + G * (1.5 + 8.6 * exp (-.09 * h * h) * (1. - exp (-dp/50.))) ;

	assert (att.freq(3) == 500) ;
	att[3] = -1.5 + G * (1.5 + 14. * exp (-.46 * h * h) * (1. - exp (-dp/50.))) ;

	assert (att.freq(4) == 1000) ;
	att[4] = -1.5 + G * (1.5 + 5.0 * exp (-.90 * h * h) * (1. - exp (-dp/50.))) ;
	
	assert (att.freq(5) == 2000) ;
	att[5] = -1.5 + 1.5 * G ;

	assert (att.freq(6) == 4000) ;
	att[6] = -1.5 + 1.5 * G ;

	assert (att.freq(7) == 8000) ;
	att[7] = -1.5 + 1.5 * G ;

	return att ;
}
/*
 * evaluate averaged G values in source, receiver and mid region
 */
void GetAveragedGround (PropagationPath& path, double& Gs, double& Gm, double& Gr, double& q)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	/*
	 * source region (from source to receiver, maximum length = 30 * hS)
	 */
	double hS = path[n1].z_path - path[n1].pos.z ;
	double dSmin = path[n1].d_path ;
	double dSmax = dSmin + 30 * hS ;
	/*
	 * receiver region (from receiver to source, maximum lenght = 30 * hR)
	 */
	double hR = path[n2].z_path - path[n2].pos.z ;
	double dRmax = path[n2].d_path ;
	double dRmin = dRmax - 30 * hR ;
	/*
	 * source region cannot extend beyond the receiver
	 */
	if (dSmax > dRmax) dSmax = dRmax ;
	/*
	 * receiver region cannot extend beyond the source
	 */
	if (dRmin < dSmin) dRmin = dSmin ;
	/*
	 * limits of the mid-region (if it exists)
	 */
	double dMmin = dSmax ;
	double dMmax = dRmin ;
	bool   mid_region = (dMmin < dMmax) ? true : false ;

	Gs = Gr = Gm = 0 ;
	double Ds = 0 ;
	double Dr = 0 ;
	double Dm = 0 ;

	for (unsigned int i = n1 ; i < n2 ; i++)
	{	
		/*
		 * length of the segment projected on the horizontal plane
		 */
		double d1 = path[i].d_path ;
		double d2 = path[i+1].d_path ;
		/*
		 * get G value for material associated with the segment
		 * note that materials are encoded at the end points of the segment
		 */
		Material* mat = path[i+1].mat ;
		assert (mat != 0) ;
		double G  = mat->getGValue() ;
		/*
		 * intersect segment with source region
		 */
		double d1S = std::max (d1, dSmin) ;
		double d2S = std::min (d2, dSmax) ;
		if (d1S < d2S)
		{
			Gs += G * (d2S - d1S) ;
			Ds += (d2S - d1S) ;
		}
		/*
		 * intersect segment with mid-region (if it exists)
		 */
		if (mid_region)
		{
			double d1M = std::max (d1, dMmin) ;
			double d2M = std::min (d2, dMmax) ;
			if (d1M < d2M)
			{
				Gm += G * (d2M - d1M) ;
				Dm += (d2M - d1M) ;
			}
		}
		/*
		 * intersect segment with receiver region
		 */
		double d1R = std::max (d1, dRmin) ;
		double d2R = std::min (d2, dRmax) ;
		if (d1R < d2R)
		{
			Gr += G * (d2R - d1R) ;
			Dr += (d2R - d1R) ;
		}
	}
	/*
	 * take averaged values (weighted by horizontal distance)
	 */

	assert (fabs (Ds - (dSmax - dSmin)) < 0.01)  ;
	assert (Ds > 0) ;
	Gs = Gs / Ds ;

	assert (fabs (Dr - (dRmax - dRmin)) < 0.01)  ;
	assert (Dr > 0) ;
	Gr = Gr / Dr ;

	if (mid_region)
	{
		assert (fabs (Dm - (dMmax - dMmin)) < 0.01)  ;
		assert (Dm > 0) ;
		Gm = Gm / Dm ;
		q = Dm / (Ds + Dm + Dr) ;
	}
	else
	{
		Gm = 0 ; 
		q = 0 ;
	}
	print_debug ("Gs = %.2f (d = %.1fm) \n", Gs, Ds) ;
	print_debug ("Gm = %.2f (d = %.1fm) \n", Gm, Dm) ;
	print_debug ("Gr = %.2f (d = %.1fm) \n", Gr, Dr) ;
}
/* 
 * evaluate the ground effect over the path
 */
Spectrum ISO_9613_2::getGroundEffect (PropagationPath& path)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	/*
	 * calculate averaged G values in three regions
	 */
	double Gs, Gr, Gm ;
	double q ;
	GetAveragedGround (path, Gs, Gm, Gr, q) ;
	/*
	 * evaluate parameters hS, hR and dp
	 */
	double hS = path[n1].z_path - path[n1].pos.z ;
	double dS = path[n1].d_path ;
	double hR = path[n2].z_path - path[n2].pos.z ;
	double dR = path[n2].d_path ;
	double dp = dR - dS ;
	assert (hS >= 0) ;
	assert (hR >= 0) ;
	assert (dp > 0) ;
	/*
	 * evaluate As, Am and Ar
	 */
	Spectrum As = getAground (Gs, hS, dp) ;
	Spectrum Ar = getAground (Gr, hR, dp) ;
	Spectrum Am (0.0) ;
	if (q > 0)
	{
		double A0 = Am[0] = -3*q ;
		double A1 = -3*q* (1 - Gm) ;
		for (unsigned int i = 0 ; i < Am.size() ; ++i) Am[i] = (i == 0) ? A0 : A1 ;
	}
	/*
	 * return sum of three components
	 */
	return As + Am + Ar ;
}
/*
 * evaluate the path difference for a diffracted path 
 */
static double getPathDifference (std::vector<Point2D> pos, double a, double& dS, double& e, double& dR, double& dSR)
{
	assert (pos.size() > 2) ;

	unsigned int n1 = 0 ;
	unsigned int n2 = pos.size()-1 ;
	/*
	 * direct path
	 */
	dSR = dist (pos[n1], pos[n2]) ;
	dSR = sqrt (dSR * dSR + a * a ) ;
	/* 
	 * a diffracted path is decomposed in three components
	 */
	e  = 0 ;
	dS = 0 ;
	dR = 0 ;
	n1++ ;
	for (unsigned int i = n1 ; i <= n2 ; ++i)
	{
		double  dd = dist (pos[i-1], pos[i]) ;
		if (i == n1) dS += dd ;
		else if (i == n2) dR += dd ;
		else e += dd ;
	}
	/*
	 * total length of diffracted path
	 */
	double dif = dS + e + dR ;
	dif = sqrt (dif * dif + a * a) ;
	/*
	 * return path difference
	 */
	return dif - dSR ;
}
/*
 * calculate Dz according to eq.14 of the standard
 */
static double getDz (double z, double e, double lambda, double Kmet, bool lateral = false)
{
	/*
	 * C2 = 20 if image sources on the ground are to be accounted for
	 * C2 = 40 if no image source are to be accounted for
	 *
	 * in case of lateral diffraction, do not account for images because these
	 * are already taken into account in the calculation of Aground
	 */
	double C2 = lateral ? 40 : 20 ;
	double C3 = 1 ;
	if (e > 0)
	{
		double x = 5 * lambda / e ;
		C3 = (1 + x * x) / (1./3 + x * x) ;
	}
	double x = 3 + (C2/lambda)* C3 * z * Kmet ;
	/*
	 * in case z < 0, we might end up by taking the logarithm of a negative value...
	 * we clamp x to values x >= 1 so that Dz > 0 dB in all cases
	 */
	if (x < 1) x = 1.0 ;
	return LOG10 (x) ;
}
/*
 * general formula for both horizontal and vertical diffraction
 *
 */
Spectrum ISO_9613_2::getDiffraction (std::vector<Point2D> const& pos, double a, PathInfo::PathType type)
{
	double dSR = 0 ;
	double dS  = 0 ;
	double dR  = 0 ;
	double e = 0 ;
	double z = getPathDifference (pos, a, dS, e, dR, dSR) ;
	double Kmet = 1 ;
	bool   lateral = false ;
	switch (type)
	{
	case PathInfo::DiffractedPath :
		 Kmet = exp (-sqrt(dS*dR*dSR/(2*z))/2000.) ; 
		 break ;
	case PathInfo::PartialDiffractedPath :
		 assert (e == 0) ;
		 z = -z ; 
		 break ;
	case PathInfo::LateralDiffractedPath :
		lateral = true ;
		break ;
	default:
		assert ("How did you ever get here ?") ;
	}

	Spectrum att ;
	for (unsigned int i = 0 ; i < att.size() ; ++i)
	{
		double lambda = options.meteo.getSoundSpeed() / att.freq(i) ;
		att[i] = getDz (z, e, lambda, Kmet, lateral) ;
	}
	
	return att ;
}
/*
 * calculate attenuation due to diffraction in the propagation plane
 */
Spectrum ISO_9613_2::getDiffraction (PropagationPath& path)
{
	std::vector<Point2D> pos ;

	unsigned int n1 = 0 ;
	unsigned int n2 = path.size() - 1 ;

	for (unsigned int i = n1 ; i <= n2 ; ++i)
	{
		if (i == n1 || i == n2 || 
			path[i].mode3D == Action3D::Diffraction || 
			path[i].mode3D == Action3D::DiffractionBLOS)
		{
			pos.push_back (Point2D(path[i].d_path, path[i].z_path)) ;
		}
	}
	/*
	 * using a = 0 because the path difference is calculated in the propagation plane
	 * see issue log...
	 */
	Spectrum att = getDiffraction (pos, 0.0, path.info.pathType) ;
	/*
	 * limit the effect of diffraction due to atmospheric turbulence
	 */
	double limit = path.info.nbDiffractions > 1 ? 25.0 : 20.0 ;
	for (unsigned int i = 0 ; i < att.size() ; ++i) att[i] = std::min (att[i], limit) ;
	/*
	 * return attenuation
	 */
	return att ;
}
/*
 * calculate the attenuation due to diffraction around a vertical edge
 *
 * note that the standard doesn't give details for the calculation of the parameters of a laterally 
 * diffracted path.. we assume that all parameters (e.g. z and e) can be determined (up to sufficient 
 * accuracy) from the projection of the path on the horizontal plane.
 */
Spectrum ISO_9613_2::getLateralDiffraction (PropagationPath& path)
{
	if (path.info.pathType != PathInfo::LateralDiffractedPath) return Spectrum (0.0);

	unsigned int n1 = 0 ;
	unsigned int n2 = path.size() - 1 ;
	std::vector<Point2D> pos ;

	for (unsigned int i = n1 ; i <= n2 ; ++i)
	{
		if (path[i].mode2D == Action2D::Diffraction || i == n1 || i == n2)
		{
			pos.push_back (path[i].pos) ;
		}
	}
	assert (pos.size() > 2) ;
	/*
	 * a = component of distance parallel to the barrier edge
	 */
	double a = path[n2].z_path - path[n1].z_path ;
	/*
	 * use general formula to evaluate diffraction effects
	 */
	return -getDiffraction (pos, a, path.info.pathType) ;
}
/*
 * mixed model for almost diffracted paths
 *
 * note that, in the current implementation, we use this model only in case the diffracting edge is below 
 * the line of sight from the source to the receiver ; however it might be adapted to the case of the line 
 * of sight being blocked...
 */
Spectrum ISO_9613_2::getGroundOrDiffraction (PropagationPath& path)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size() - 1 ;
	assert (path[n1].ext != 0) ;
	assert (path[n2].ext != 0) ;
	/*
	 * find the potentially diffracting edge below the line of sight ; by definition
	 * this is the control point that maximizes the (negative) path difference for 
	 * the actual source and receiver position.
	 */
	unsigned int pos_dif = 0 ;
	for (pos_dif = n1 ; pos_dif < n2 ; ++pos_dif) 
	{
		if (path[pos_dif].mode3D == Action3D::DiffractionBLOS) break ;
	}
	assert (pos_dif > n1) ;
	assert (pos_dif < n2) ;
	Point2D dif_edge (path[pos_dif].d_path, path[pos_dif].pos.z) ;
	/*
	 * consider images of source and receiver above local ground, assuming the ground 
	 * is "almost flat", i.e. in line with the basic assumptions of the standard.
	 *
	 * note that in the JRC-2012 method we might use the flat ground approximation instead
	 * and construct the image source and receiver more accurately.
	 */
	Point2D real_src  (path[n1].d_path, path[n1].pos.z + path[n1].ext->h) ;
	Point2D image_src (path[n1].d_path, path[n1].pos.z - path[n1].ext->h) ;
	Point2D real_rec ( path[n2].d_path, path[n1].pos.z + path[n2].ext->h) ;
	Point2D image_rec (path[n2].d_path, path[n1].pos.z - path[n2].ext->h) ;
	/*
	 * calculate path differences for real and image path
	 *
	 * note that path differences are negative in case the diffracting edge is below the direct 
	 * line from real/image source to image/real receiver
	 */
	double delta_real  = PropagationPath::get_path_difference (real_src,  dif_edge, real_rec) ;
	double delta_image = PropagationPath::get_path_difference (image_src, dif_edge, image_rec) ;
	double delta_dif   = delta_real + delta_image ;
	/*
	 * calculate attenuations both with and without diffraction
	 */
	Spectrum Aground = getGroundEffect (path) ;
	Spectrum Abar    = getDiffraction (path);
	/*
	 * handle transition between both models
	 */
	Spectrum att ;
	for (unsigned int i = 0 ; i < att.size() ; ++i)
	{
		/*
		 * check for flatness of ground ; in case of almost flat ground, ignore diffraction
		 * note that we use Raleigh's criterion to evaluate the flatness of the ground, which makes
		 * the determination of flatness a frequency-dependent matter (as it should be).
		 */
		double lambda = options.meteo.getSoundSpeed() / att.freq (i) ;
		if (delta_dif < lambda/4)
		{
			att[i] = Aground[i] ;
		}
		else
		{
			/*
			 * diffraction destroys ground effects over soft ground, but hard ground may reduce the
			 * effect of diffraction... This is taken into account as discussed in CNOSSOS-EU WG5 
			 */
			if (Aground[i] < 0) Abar[i] += Aground[i] ;
			/*
			 * now, use the standard method for combining ground and diffraction
			 */
			att[i] = std::max (Aground[i], Abar[i]) ;
		}
	}
	return att ;
}
/*
 * Get correction for the finite size of the reflecting obstacles.
 *
 * Note that, for the purpose of strategic noise mapping, we assume that the height of any obstacle 
 * in the model is smaller than its horizontal size and that we therefore only use the height as a 
 * criterion to check the requirements of section 7.5 of the standard. This is coherent with the fact 
 * that the propagation path does not contain information on the horizontal extent of the reflecting 
 * plane. As such, the method might not be suited for detailed studies of industrial sites (which is
 * anyhow that's outside the scope of the CNOSSOS-EU project).
 *
 * Figure 8 of the standard suggest that the correction being calculated in 3D coordinates which is a 
 * rather complicated matter (see blow). Moreover it assumes that the Fresnel zone is the same size in 
 * the horizontal and vertical direction, i.e. that the intersection of the Fresnel ellipsoid with the 
 * reflecting plane is a circle (instead of an ellipse), which size is determined by the angle of 
 * incidence (beta) as measured in 3D.
 *
 * Alternatively, one can consider that the height of the reflecting obstacle should be compared to 
 * the height of the Fresnel ellipse at the reflection point (and not to its horizontal extent). This 
 * is done by first considering the intersection of the ellipsoid with the propagation plane (giving 
 * way to a 2D Fresnel ellipse) and then to consider the intersection of this ellipse with the reflecting 
 * plane. This process has the advantage that it relies solely on calculations in 2D in the unfolded 
 * propagation plane. It is this second solution that was retained in the Harmonoise/Imagine projects.
 *
 * Below, we provide two implementations:
 *
 * - a 3D implementation that follows the standards as strictly as possible,
 *
 * - a 2D implementation that uses formula 19 in order to evaluate the (approximate) height of the 
 *   Fresnel ellipse in the unfolded propagation plane.
 *
 * For the purpose of strategic noise mapping, we recommend the use of the 2D solution.
 */
Spectrum ISO_9613_2::getFiniteSizeCorrection (PropagationPath& path)
{
	return getHeightCorrection2D (path) ;
}
/*
 * strict implementation of section 7.5 of the standard, using 3D coordinates
 */
Spectrum ISO_9613_2::getHeightCorrection3D (PropagationPath& path)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size() - 1 ;
	Spectrum att (0.0) ;
	for (unsigned int i = n1+1 ; i < n2 ; ++i)
	{
		/*
		 * According to fig.8 of the standard, the size of the Fresnel weight is calculated using
		 * the positions of the (real) source and receiver. This doesn't work for multiple reflected 
		 * paths. Also, this approach is doubtful in case the path has been diffracted over an horizontal 
		 * edge. In order to generalize the approach, we calculate the "equivalent" source and receiver 
		 * at the "right" distance in the direction of the 3D  ray path before and after the specular 
		 * reflection point.
		 *
		 * Note that the ray path can be reconstructed from the horizontal projection of the control
		 * points (X,Y coordinates), combined with the height of the ray path as determined in the 
		 * unfolded propagation plane (using z_path as the Z coordinate).
		 */
		if (path[i].mode2D != Action2D::Reflection) continue ;
		/*
		 * p0 = origin = specular reflections point
		 */
		Point3D p0 = path[i].pos ; p0.z = path[i].z_path ;
		/*
		 * v1 = vector from the origin towards the equivalent source position
		 */
		unsigned int i1 = i-1 ;
		while (i1 > n1 && path[i1].mode2D != Action2D::Reflection 
					   && path[i1].mode2D != Action2D::Diffraction
					   && path[i1].mode3D != Action3D::Diffraction) i1-- ;
		Point3D  p1 = path[i1].pos ; p1.z = path[i1].z_path ;
		Vector3D v1 = p1 - p0 ;
		if (i1 != n1) v1 = v1 * ((path[i].d_path - path[n1].d_path) / (path[i].d_path - path[i1].d_path)) ;
		/*
		 * v2 = vector from the origin towards the equivalent receiver position
		 */
		unsigned int i2 = i+1 ;
		while (i2 < n2 && path[i2].mode2D != Action2D::Reflection 
					   && path[i2].mode2D != Action2D::Diffraction 
					   && path[i2].mode3D != Action3D::Diffraction) i2++ ;
		Point3D  p2 = path[i2].pos ; p2.z = path[i2].z_path ;
		Vector3D v2 = p2 - p0 ;
		if (i2 != n2) v2 = v2 * ((path[n2].d_path - path[i].d_path) / (path[i2].d_path - path[i].d_path)) ;
		/*
		 * normalize vectors and calculate length
		 */
		double d1 = norm (v1) ; v1 = v1/d1 ;
		double d2 = norm (v2) ; v2 = v2/d2 ;
		/*
		 * reconstruct normal vector
		 */
		Vector3D n = v1 + v2 ; n = n / norm (n) ;
		/*
		 * take co-sinus of the angle of incidence (in 3D)
		 *
		 * Note that taking a single (3D) value for the angle of incidence is equivalent to assuming
		 * that the intersection of the Fresnel zone with the reflector can be approximated by a 
		 * circle (instead of an ellipse). Alternatively, one can consider that the height of the reflecting 
		 * obstacle should be compared to the  height of the Fresnel ellipse at the reflection point (and not 
		 * to its horizontal extent). A simple way to correct this would imply changing the standard, i.e. by
		 * setting "Lmin = min (L.cos_beta, H)" which is valid for "almost" horizontal propagation paths and 
		 * would avoid calculation of the angle beta altogether. 
		 */
		double cos_beta = n * v1 ;
		/*
		 * size of the obstacle (assume height is smaller than length)
		 */
		assert (path[i].ext != NULL) ;
		double lmin = path[i].ext->h ;
		/*
		 * get the absorption coefficients for this reflexion
		 */
		Spectrum alpha(0.0) ;
		VerticalWallExt* wall = path[i].ext.cast_to_ptr<VerticalWallExt>()  ;
		assert (wall != 0) ;
		if (wall->mat) alpha = wall->mat->getAlphaValue() ;
		/*
		 * Check requirements for reflections as specified in section 7.5 of the standard.  Note that the 
		 * standard says to "ignore" paths that do not meet the requirements... which means we just throw 
		 * away whatever we have already calculated at this point (by setting the attenuation to infinity)		 
		 */
		for (unsigned int k = 0 ; k < att.size() ; ++k)
		{
			/*
			 * rule 1: a specular reflection can be constructed - this is guaranteed from the
			 * geometrical analysis of the path by means of the mandatory options 
			 */
			assert (options.CheckHeightLowerBound == true) ;
			assert (options.CheckHeightUpperBound == true) ;
			/*
			 * rule 2: the magnitude of the sound reflection coefficient must be greater than 0.2
			 */
			if (alpha[k] < 0.2)	att[k] = positive_inf ; 
			/*
			 * rule 3: compare size of the obstacle and size of the Fresnel zone
			 */
			double lambda  = options.meteo.getSoundSpeed () / att.freq(k) ;
			double fresnel = sqrt (2*lambda*d1*d2/(d1+d2)) ;
			if (lmin*cos_beta < fresnel) att[k] = positive_inf ;
		}
	}
	return att ;
}
/*
 * Alternative implementation of section 7.5 of the standard, using 2D coordinates in the unfolded
 * propagation plane only.
 *
 * It my be noted that the formula from the standard are not much more than Fresnel weighting, using
 * an approximation for the size of the Fresnel zone associated with the reflection and that it might 
 * be just as easy to use the general (accurate) formulas instead... 
 *
 * Note that we have ignored any limitations on the magnitude of the reflection coefficients...
 */
Spectrum  ISO_9613_2::getHeightCorrection2D (PropagationPath& path)
{
	Spectrum att(0.0) ;
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	for (unsigned int i = n1+1 ; i < n2 ; ++i)
	{
		if (path[i].mode2D != Action2D::Reflection) continue ;
		assert (path[i].ext != 0) ;
		/*
		 * size of the obstacle (assuming the length is smaller than the height)
		 */
		double lmin = path[i].ext->h ;
		/*
		 * specular reflection point (in the unfolded propagation plane, using 2D coordinates)
		 */
		Point2D p0 (path[i].d_path, path[i].z_path) ;
		/*
		 * get caustic position to the left, as we operate in the vertical plane we
		 * consider vertical edges to act as equivalent source and/or receiver
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
		/*
		 * distance toward the source / the receiver
		 */
		double d1 = dist (p1, p0) ;
		double d2 = dist (p2, p0) ;
		/*
		 * co-sinus of angle of incidence 
		 */
		double tg_beta = (path[i2].z_path - path[i1].z_path) / (path[i2].d_path - path[i1].d_path) ;
		double cos_beta = 1. / (1. + tg_beta * tg_beta) ;
		/*
		 * Check requirements for reflections as specified in section 7.5 of the standard.  Note that the 
		 * standard says to "ignore" paths that do not meet the requirements... which means we just throw 
		 * away whatever we have already calculated at this point (by setting the attenuation to infinity)		 
		 */
		for (unsigned int k = 0 ; k < att.size() ; ++k)
		{
			double lambda  = options.meteo.getSoundSpeed () / att.freq(k) ;
			double fresnel = sqrt (2*lambda*d1*d2/(d1+d2)) ;
			if (lmin*cos_beta < fresnel) att[k] = positive_inf ;
		}
	}
	return att ;
}


