#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		ISO-9613-2.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implements the calculation of propagation effects as described in the
 *				ISO 9613-2:1996 standard
 * changes:
 *
 *	25/10/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include "CalculationMethod.h"
#include "PropagationPath.h"
#include "VerticalExt.h"

namespace CnossosEU 
{
	class ISO_9613_2 : public CalculationMethod
	{
	public:

		const char* name (void) ;
		const char* version (void)  ;

		ISO_9613_2 (void) { } ;
		~ISO_9613_2 (void) { } ;
	
	protected:

		virtual bool initCalculation (void) ;

		virtual MeasurementType expectedMeasurementType (void) { return MeasurementType::HemiSpherical ; }
		virtual MeteoCondition::MeteoModel getDefaultMeteoModel (void) { return MeteoCondition::ISO9613 ; }

		virtual Spectrum getExcessAttenuation (PropagationPath& path, bool favorable_condition) ;
		virtual Spectrum getFiniteSizeCorrection (PropagationPath& path) ;
		virtual Spectrum getLateralDiffraction (PropagationPath& path) ;
	
	private:

		Spectrum getGroundEffect (PropagationPath& path) ;
		Spectrum getDiffraction (PropagationPath& path) ;
		Spectrum getDiffraction (std::vector<Geometry::Point2D> const& pos, double a, PathInfo::PathType type) ;
		Spectrum getDiffractionBLOS (PropagationPath& path) ;
		Spectrum getGroundOrDiffraction (PropagationPath& path) ;
		Spectrum getHeightCorrection2D (PropagationPath& path) ;
		Spectrum getHeightCorrection3D (PropagationPath& path) ;
	};
}