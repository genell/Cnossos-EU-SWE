#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		COSSOS-2018.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implements the calculation of propagation effects as described in the EU
 *			    Directive COM (EU) 2015/996 + proposed modifications by the WG Amendments
 *				
 * changes:
 *
 *	10/07/2018	initial version created
 * ------------------------------------------------------------------------------------------------- 
 */
#include "CalculationMethod.h"
#include "PropagationPath.h"
#include "VerticalExt.h"
#include "MeanPlane.h"

namespace CnossosEU 
{
	class CNOSSOS_2018 : public CalculationMethod
	{
	public:

		const char* name (void) ;
		const char* version (void)  ;

		CNOSSOS_2018 (void) ;
		~CNOSSOS_2018 (void) ;
	
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