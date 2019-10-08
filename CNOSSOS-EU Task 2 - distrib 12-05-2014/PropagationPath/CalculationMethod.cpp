/* 
 * ------------------------------------------------------------------------------------------------
 * file:		CalculationMethod.cpp
 * version:		0.001
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implement shared functions used by all propagation models
 * changes:
 *
 *	18/10/2013	initial version 0.001
 *
 *  23/10/2013	implemented shared functions for air absorption, geometrical spread, absorption by 
 *				vertical walls, calculation of partial noise levels,...
 *
 *	24/10/2013	implemented meteorological weighting models 
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "CalculationMethod.h"
#include "PropagationPath.h"
#include "Material.h"
#include "MeteoCondition.h"
#include "PathResult.h"
#include "ErrorMessage.h"
#include "SystemClock.h"
#include <algorithm>

static const double PI = 3.1415926 ;

using namespace CnossosEU ;
using namespace Geometry ;
/*
 * calculate the partial noise level associated with a propagation path
 *
 * this is implemented as a three step process:
 *
 *	1) geometrical analysis of the path and construction of the Fermat ray path
 *
 *	2) calculate the different components of the noise model
 *
 *  3) calculate noise levels
 *
 * Note that the calculation of each of the different components is implemented in 
 * a distinct virtual function. Some functions are shared amongst all derived methods,
 * some have specific implementations in each of the derived methods.
 */
bool CalculationMethod::doCalculation (PropagationPath& path, PathResult& result)
{
	SystemClock clock ;
	/*
	 * setup local variables
	 */
	initCalculation() ;
	/*
	 * geometrical analysis of the path
	 */
	if (!path.analyze_path(options)) return false ;
	/*
	 * evaluate the sound power of the source
	 */
	result.Lw = options.ExcludeSoundPower ? Spectrum(0.0) : getSoundPower (path) ;
	/*
	 * convert sound power to dB(A) values if needed
	 */
	result.dBA = getFrequencyWeighting (path) ;
	/*
	 * evaluate the sound power adaptation of the source
	 */
	result.delta_Lw = getSoundPowerAdaptation (path) ;
	/*
	 * evaluate the geometrical spread
	 */
	result.AttGeo = options.ExcludeGeometricalSpread ? 0.0 : getGeometricalSpread (path) ;
	/*
	 * evaluate air absorption
	 */
	result.AttAir = options.ExcludeGeometricalSpread ? Spectrum (0.0) : getAirAbsorption (path) ;
	/*
	 * evaluate attenuation due to absorption by reflecting obstacles
	 */
	result.AttAbsMat = getAbsorption (path) ;
	/*
	 * evaluate attenuation due to lateral diffraction
	 */
	result.AttLatDif = getLateralDiffraction (path) ;
	/*
	 * evaluate attenuation due to finite size of obstacles
	 */
	result.AttSize = getFiniteSizeCorrection (path) ;
	/*
	 * evaluate excess attenuation
	 */
	result.AttF = getExcessAttenuation (path, true) ;
	result.AttH = getExcessAttenuation (path, false) ;
	/*
	 * calculate levels 
	 */
	result.LpF = getNoiseLevel (result, true) ;
	result.LpH = getNoiseLevel (result, false) ;
	/*
	 * estimate long-time averaged noise level
	 */
	result.Leq = getNoiseLevel (path, result) ;
	/*
	 * convert values to dB(A) values
	 */
	result.LpF_dBA = getNoiseLevel (result.LpF) ;
	result.LpH_dBA = getNoiseLevel (result.LpH) ;
	result.Leq_dBA = getNoiseLevel (result.Leq) ;
	/*
	 * cleanup local variables
	 */
	exitCalculation() ;
	/*
	 * update performance counters
	 */
	nbCalls++ ;
	totalCPUTime += clock.get() ;

	return true ;
}
/* 
 * get the source description. Note that the source may be associated with either the first 
 * or at the last position of the propagation path.
 */
SourceExt* CalculationMethod::getSource (PropagationPath& path, unsigned int* pos_in_path)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	unsigned int pos = -1 ;
	assert (path[n1].ext != 0) ;
	assert (path[n2].ext != 0) ;
	if (path[n1].mode2D == Action2D::Source) pos = n1 ;
	if (path[n2].mode2D == Action2D::Source) pos = n2 ;
	assert (pos != -1) ;
	SourceExt* source = path[pos].ext.cast_to_ptr<SourceExt>()  ;
	assert (source != 0) ;
	if (pos_in_path) *pos_in_path = pos ;
	return source ;
}
/* 
 * get the receiver description. Note that the receiver may be associated with either the first 
 * or at the last position of the propagation path.
 */
ReceiverExt* CalculationMethod::getReceiver (PropagationPath& path, unsigned int* pos_in_path)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	unsigned int pos = -1 ;
	assert (path[n1].ext != 0) ;
	assert (path[n2].ext != 0) ;
	if (path[n1].mode2D == Action2D::Receiver) pos = n1 ;
	if (path[n2].mode2D == Action2D::Receiver) pos = n2 ;
	assert (pos != -1) ;
	ReceiverExt* rec = path[pos].ext.cast_to_ptr<ReceiverExt>()  ;
	assert (rec != 0) ;
	if (pos_in_path) *pos_in_path = pos ;
	return rec ;
}
/*
 * evaluate the total propagation distance. Note that the calculations are easiest in the 
 * unfolded vertical plane, i.e. after removal of all reflections from or diffractions around 
 * vertical obstacles.
 */
double CalculationMethod::getPropagationDistance (PropagationPath& path)
{
	ControlPoint& src = path[0] ;
	ControlPoint& rec = path[path.size()-1] ;

	Point2D p1 (src.d_path, src.z_path) ;
	Point2D p2 (rec.d_path, rec.z_path) ;

	return dist (p1, p2) ;
}
/*
 * evaluate the attenuation due to geometrical spread
 */
double CalculationMethod::getGeometricalSpread (PropagationPath& path)
{
	SourceExt* source = getSource (path) ;
	assert (source != 0) ;
	SourceGeometry* geo = source->geo ;
	if (geo)
	{
		return geo->GetGeometricalSpread (path, this) ;
	}
	else
	{
		return getGeometricalSpreadUnitSource (path, 1.0) ;
	}
}
/*
 * get attenuation due to atmospheric absorption
 */
Spectrum CalculationMethod::getAirAbsorption (PropagationPath& path)
{
	double dist   = getPropagationDistance (path) ;
	return options.meteo.getAirAbsorption() * dist ;
}
/*
 * get the sound power of the source
 */
Spectrum CalculationMethod::getSoundPower (PropagationPath& path)
{
	SourceExt* source = getSource (path) ;
	assert (source != 0) ;
	/*
	 * get the equivalent source 
	 */
	ElementarySource &eqSource = source->source ;
	/* 
	 * get the direction of propagation
	 */
	Vector3D direction = path.get_propagation_direction() ;
	/*
	 * convert direction of propagation to the local coordinate system of the source
	 */
	if (source->geo)
	{
		Vector3D x_axis, y_axis, z_axis ;
		/*
		 * get local coordinate system for this source geometry
		 */
		source->geo->GetLocalCoordinateSystem (x_axis, y_axis, z_axis) ;
		/*
		 * project direction of propagation on the local coordinate system
		 */
		double dx = x_axis * direction ;
		double dy = y_axis * direction ;
		double dz = z_axis * direction ;
		/* 
		 * express direction of propagation as a vector in the local coordinate system
		 */
		direction = Vector3D (dx, dy, dz) ;
	}
	/* 
	 * get the apparent sound power of the source in the direction of propagation
	 */
	return eqSource.getSoundPower (direction) ;
};
/*
 * apply dB(A) weighting if needed
 */
Spectrum CalculationMethod::getFrequencyWeighting (PropagationPath& path)
{
	Spectrum att ;
	SourceExt* source = getSource (path) ;
	assert (source != 0) ;
	ElementarySource &eqSource = source->source ;
	if (eqSource.frequencyWeighting != FrequencyWeighting::dBA)
	{
		print_debug (".convert sound power from LIN to A-weighted values\n") ;
		for (unsigned int i = 0 ; i < att.size() ; ++i) att[i] = Spectrum::dBA(i) ;
	}
	return att ;
};
/*
 * utility function, calculate sin(x)/x for all values of x
 */
static double sinx (double x)
{
	return (x == 0) ? 1.0 : sin(x)/x ;
}
/*
 * get sound power conversion 
 */
Spectrum CalculationMethod::getSoundPowerAdaptation (PropagationPath& path)
{
	Spectrum att(0.0) ;
	unsigned int pos ;
	SourceExt* source = getSource (path, &pos) ;
	assert (source != 0) ;
	ElementarySource const& eqSource = source->source ;
	double h = source->h ;
	double C = 0 ;
	if (eqSource.measurementType == MeasurementType::FreeField && 
		expectedMeasurementType() == MeasurementType::HemiSpherical)
	{
		/*
		 * Assume the sound power has been measured for a source placed in free field
		 * conditions, i.e. using the relationship Lp = Lw - 10.log10 (4.pi.d²).
		 * The ISO and NMPB-2008 methods calculate attenuation relative to a source
		 * whose sound power has been measured under hemispherical radiation conditions.
		 */
		C = 1 - path[pos].mat->getGValue() ; 
		print_debug (".convert sound power from free field to hemispherical conditions (C = %.2f) \n", C) ;
	}
	else if (eqSource.measurementType == MeasurementType::HemiSpherical && 
		     expectedMeasurementType() == MeasurementType::FreeField)
	{
		/*
		 * Assume the sound power has been measured for a source placed near a hard reflecting 
		 * surface (G = 0), assuming hemispherical radiation conditions; i.e. using the
		 * relationship Lp = Lw - 10.log10(4.pi.d²) + 3 dB
		 */
		C = -1 ;
		print_debug (".convert sound power from hemispherical to free field conditions (C = %.2f) \n", C) ;
	}
	/*
	 * default: no correction
	 */
	else
	{
		return att ;
	}
	/*
	 * correction is based on theoretical work by...
	 */
	for (unsigned int i = 0 ; i < att.size() ; ++i)
	{
		double k = 2 * PI * att.freq(i) / options.meteo.getSoundSpeed() ;
		double x = 2 * h * k ;
		att[i] = C * LOG10 (1 + sinx(x)) ;
	}
	return att ;
};
/*
 * get absorption due to reflections from vertical walls
 */
Spectrum CalculationMethod::getAbsorption (PropagationPath& path)
{
	Spectrum att(0.0) ;
	for (unsigned int i = 1 ; i < path.size()-1 ; ++i)
	{
		if (path[i].mode2D == Action2D::Reflection)
		{
			VerticalWallExt* wall = path[i].ext.cast_to_ptr<VerticalWallExt>()  ;
			assert (wall != 0) ;
			Material* mat = wall->mat ;
			assert (mat != 0) ;
			Spectrum alpha = mat->getAlphaValue() ;
			assert (alpha.size() == att.size()) ;
			for (unsigned int j = 0 ; j < alpha.size() ; j++)
			{
				double a = alpha[j] ;
				static const double amin = 0.0000 ;
				static const double amax = 0.9999 ;
				if (a < amin) a = amin ;
				if (a > amax) a = amax ;
				att[j] += LOG10 (1 - a) ;
			}
		}
	}
	return att ;
}
/*
 * get correction for finite size of reflecting and/or diffracting vertical obstacles
 *
 * default behavior : no correction
 */
Spectrum CalculationMethod::getFiniteSizeCorrection (PropagationPath& path)
{
	return Spectrum (0.0) ;
};
/*
 * get attenuation for lateral diffraction
 *
 * default behavior : not implemented
 */
Spectrum CalculationMethod::getLateralDiffraction (PropagationPath& path)
{
	for (unsigned int i = 1 ; i < path.size()-1 ; ++i)
	{
		if (path[i].mode2D == Action2D::Diffraction)
		{
			signal_error (ErrorMessage("Lateral diffraction not supported \n")) ;
		}
	}
	return Spectrum (0.0) ;
};
/*
 * get excess attenuation due to ground, diffraction over obstacles and meteorological conditions
 *
 * default behavior : no attenuation, i.e. calculate free field conditions
 */
Spectrum CalculationMethod::getExcessAttenuation (PropagationPath& path, bool favorable_conditions)
{
	return Spectrum (0.0) ;
};
/*
 * calculate partial noise level spectrum for the current path
 */
Spectrum CalculationMethod::getNoiseLevel (PathResult& result, bool favorable_conditions)
{
	Spectrum Lp (result.Lw) ;
	Lp += result.dBA ;
	Lp += result.delta_Lw ;
	Lp += result.AttGeo ;
	Lp += result.AttAir ;
	Lp += result.AttAbsMat ;
	Lp += result.AttLatDif ;
	Lp += result.AttSize ;
	if (favorable_conditions)
		Lp += result.AttF ;
	else
		Lp += result.AttH ;
	return Lp ;
}
/*
 * calculate global noise level 
 */
double CalculationMethod::getNoiseLevel (Spectrum& Lp)
{
	double sum = 0 ;
	for (unsigned int i = 0 ; i < Lp.size() ; ++i) sum += POW10 (Lp[i]) ;
	return LOG10 (sum) ;
}
/*
 * calculate long-time averaged noise level
 *
 * note that any of the point-to-point models can be used with any of the meteorological models
 */
Spectrum CalculationMethod::getNoiseLevel (PropagationPath& path, PathResult& result)
{
	assert (options.meteo.model != MeteoCondition::DEFAULT) ;

	if (options.meteo.model == MeteoCondition::ISO9613)
	{
		double C0 = options.meteo.C0 ;
		unsigned int n1 = 0 ;
		unsigned int n2 = path.size()-1 ;
		double hSR = 10 * (path[n1].ext->h + path[n2].ext->h) ;
		double dSR = path[n2].d_path - path[n1].d_path ;
		double Cmet = C0 * ((dSR < hSR) ? 0.0 : (1.0 - hSR / dSR)) ;
		print_debug (".Estimate long-time averaged noise level using Cmet = %.1f dB(A) \n", Cmet) ;
		return result.LpF - Cmet ;
	}
	else 
	{
		double pFav = options.meteo.pFav ;
		pFav = std::max (0.0, std::min (pFav, 1.0)) ;
		print_debug (".Estimate long-time averaged noise level using pFav = %.1f%% \n", 100 * pFav) ;
		return LOG10 (pFav * POW10 (result.LpF) + ( 1 - pFav) * POW10 (result.LpH)) ;
	}
}
/*
 * evaluate the attenuation with distance for a point source
 */
double CalculationMethod::getGeometricalSpreadUnitSource (PropagationPath& path, double units)
{
	const double PI = 3.1415926 ;
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	double d2 ;
	/*
	 * For a laterally diffracted path, the geometrical spread is calculated for the real 
	 * source/real receiver. This is stated explicitly in JRC-2012 (see VI.4.4.e) but not 
	 * in ISO 9613-2. Still we assume it is true for both methods because in both methods,
	 * diffraction is characterized by means of an "insertion loss" factor, i.e. the level
	 * difference between a configuration with/without the barrier.
	 *
	 * Note that this would not be true for the Harmonoise propagation model which is based
	 * on diffraction coefficients and total path length (see e.g. A.D. Pierce, "Acoustics", 
	 * chapter 9.9). Physical methods can be used deep in the shadow region (i.e. for total
	 * deflection angles >> 180°) whereas the approximate models cannot.
	 */
	if (path.info.pathType == PathInfo::LateralDiffractedPath)
	{
		assert (path.info.nbReflections == 0) ;
		Point3D p1 = path[n1].pos ; p1.z = path[n1].z_path ;
		Point3D p2 = path[n2].pos ; p1.z = path[n2].z_path ;
		d2 = dist2 (p1,p2) ;
	}
	else
	/*
	 * For all other paths, calculate the propagation distance in the unfolded propagation 
	 * path. For a path containing reflections from vertical obstacles this is equivalent 
	 * to constructing the image source (or receiver) through all reflectors.
	 */
	{
		Point2D p1 (path[n1].d_path, path[n1].z_path) ;
		Point2D p2 (path[n2].d_path, path[n2].z_path) ;
		d2 = dist2 (p1,p2) ;
	}
	return LOG10 (units / (4*PI*d2)) ;
}
/*
 * evaluate the attenuation with distance for a line source segment of finite length
 */
double CalculationMethod::getGeometricalSpreadLineSource (PropagationPath& path, Point3D& q1, 
														  Point3D& q2, double fixed_angle)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = path.size()-1 ;
	assert (path[n1].ext != 0) ;
	assert (path[n2].ext != 0) ;
	bool source_to_receiver = path[n1].ext->isSource() ;
	/*
	 * real or image receiver
	 */
	Point3D rec ;
	/*
	 * If there are no reflections, use the position of the real receiver
	 */
	if (path.info.nbReflections == 0)
	{
		/*
		 * Note that, in case of a laterally diffracted path, the geometrical spread is
		 * calculated as for a direct propagation from source to receiver, ignoring the
		 * positions of the laterally diffracting edges; see remark above. For this reason,
		 * it is not possible to combine lateral diffraction with reflections...
		*/
		int irec = source_to_receiver ? n2 : n1 ;
		rec = Point3D (path[irec].pos.x, path[irec].pos.y, path[irec].z_path) ;
	}
	else
	{
		/*
		 * Combinations of lateral diffractions and reflections are not allowed
		 */
		assert (path.info.nbLateralDiffractions == 0) ;
		/*
		 * Construct the image receiver in the horizontal plane
		 */
		unsigned int isrc  = source_to_receiver ? n1 : n2 ;
		unsigned int irec  = source_to_receiver ? n2 : n1 ;
		unsigned int istep = source_to_receiver ?  1 : -1 ;
		unsigned int iref ;
		/*
		 * move from the source to the first reflection point
		 */
		for (iref = isrc ; iref != irec ; iref += istep)
		{
			if (path[iref].mode2D == Action2D::Reflection) break ;
		}
		assert (iref != isrc) ;
		assert (iref != irec) ;
		/*
		 * construct the image receiver in the direction of the reflection position (as seen,
		 * from the source) at the right distance behind the reflecting wall. 
		 */
		Point2D  origin (path[isrc].pos) ;
		Point2D  posrec (path[iref].pos) ;
		double d1 = path[iref].d_path - path[isrc].d_path ;
		double d2 = path[irec].d_path - path[isrc].d_path ;
		posrec = origin + (posrec - origin) * d2/d1 ;
		/*
		 * reflections through vertical walls do not change the height of the image receiver,
		 * diffractions over obstacles are ignored when calculating the spherical divergence
		 * because all insertion losses are all calculated relative to free field. 
		 */
		rec = Point3D (posrec.x, posrec.y, path[irec].z_path) ;
	}
	/*
	 * add source height above ground to the ground segment given on input
	 */
	unsigned int isrc  = source_to_receiver ? n1 : n2 ;
	double h = path[isrc].z_path - path[isrc].pos.z ;
	Point3D p1 (q1) ; p1.z += h ;
	Point3D p2 (q2) ; p2.z += h ;
	/*
	 * note that, from here on all calculations are in 3D
	 */
	print_debug (".LineSourceIntegration \n") ;
	print_debug (" REC x=%.1f y=%.1f z=%.1f \n", rec.x, rec.y, rec.z) ;
	print_debug (" S1  x=%.1f y=%.1f z=%.1f \n", p1.x, p1.y, p1.z) ;
	print_debug (" S2  x=%.1f y=%.1f z=%.1f \n", p2.x, p2.y, p2.z) ;
	/*
	 * construct normal vector aligned with the segment
	 */
	double d = dist (p1, p2) ; 
	Vector3D n = (p2 - p1) / d ;
	/*
	 * construct the projection of the receiver on the source segment
	 */
	double   t = (rec - p1) * n ;
	Point3D  x = p1 + t * n ;
	/*
	 * shortest distance from the receiver to the segment
	 */
	double   R = dist (rec, x) ;
	if (R < 1.0)
	{
		R = 1.0 ;
		print_debug (".too close to the source -> increase distance to %.1fm \n", R) ;
	}
	/*
	 * angle of view of the segment as seen from the (image) receiver
	 */
	double aa ;
	if (fixed_angle != 0)
	{
		printf (".using fixed angular resolution of %.3f degrees \n", fixed_angle) ;
		aa = fixed_angle * PI / 180 ;
	}
	else
	{
		/*
		 * calculate (signed) distance from the footprint of the receiver on the segment  
		 * to the segment's end points.
		 */
		double s1 = -t ;
		double s2 = d - t ;
		double a1 = atan (s1/R) ;
		double a2 = atan (s2/R) ;
		aa = fabs (a1 - a2) ;
	}
	/*
	 * calculate geometrical spread
	 */
	print_debug (".Rmin  = %.1f m \n", R) ;
	print_debug (".Angle = %.3f degrees \n", 180. * aa / PI) ;
	return LOG10 (aa / (4. * PI * R)) ;
}