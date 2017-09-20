#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		JRC-2012.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implements the calculation of propagation effects as described in document
 *			    JRC Reference Report "Common Noise Assessment Methods in Europe", final
 *				version 2012 (ISBN 979-92-79-25281-5).
 *				
 * changes:
 *
 *	29/10/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include "CalculationMethod.h"
#include "PropagationPath.h"
#include "VerticalExt.h"
#include "MeanPlane.h"

namespace CnossosEU 
{
	class JRC2012 : public CalculationMethod
	{
	public:

		const char* name (void) ;
		const char* version (void)  ;

		JRC2012 (void) ;
		~JRC2012 (void) ;
	
	protected:

		virtual bool initCalculation (void) ;

		virtual MeasurementType expectedMeasurementType (void) { return MeasurementType::HemiSpherical ; }
		virtual MeteoCondition::MeteoModel getDefaultMeteoModel (void) { return MeteoCondition::JRC2012 ; }

		virtual Spectrum getExcessAttenuation (PropagationPath& path, bool favorable_condition) ;
		virtual Spectrum getFiniteSizeCorrection (PropagationPath& path) ;
		virtual Spectrum getLateralDiffraction (PropagationPath& path) ;
	
	private:

		Spectrum getGroundEffect (PropagationPath& path) ;
		Spectrum getDiffraction (PropagationPath& path) ;
		Spectrum getGroundOrDiffraction (PropagationPath& path) ;

		void getGroundParameters (PropagationPath& path, unsigned int n1, unsigned int n2, 
								  double dp, double zs, double zr, double& Gpath, double& Gw, double& Gm ) ;
		Spectrum getGroundEffect (double dp, double zs, double zr, double Gpath, double Gw, double Gm) ;
		Spectrum getGroundEffect (PropagationPath& path, unsigned int n1, unsigned int n2) ;
		Spectrum getDeltaDif (double z, double e, double Ch = 1.0) ;
		
		bool attFavorable ;
		MeanPlane mean_plane_SO ;
		MeanPlane mean_plane_OR ;
		double path_difference_SR ;
		double path_difference_SiRi ;
	};
}