#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		JRC_draft_2010.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implements the calculation of propagation effects as described in document
 *			    JRC Reference Report "Common Noise Assessment Methods in Europe", draft
 *				version 2010. The implementation of this method relies on CSTB's implementation 
 *              of the Harmonoise propagation model.
 * changes:
 *
 *	18/01/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include "./CalculationMethod.h"
#include "../HarmonoiseP2P/PointToPoint.hpp"
#include "VerticalExt.h"

namespace CnossosEU 
{
	class JRCdraft2010 : public CalculationMethod
	{
		void*	p2p_struct ;
		bool	path_defined ;
		int		user_defined ;

	public:

		const char* name (void) ;
		const char* version (void)  ;

		JRCdraft2010 (void) ;
		~JRCdraft2010 (void) ;
	
	protected:

		virtual bool initCalculation (void) ;
		virtual bool exitCalculation (void) ;

		virtual MeasurementType expectedMeasurementType (void) { return MeasurementType::FreeField ; }
		virtual MeteoCondition::MeteoModel getDefaultMeteoModel (void) { return MeteoCondition::JRC2012 ; }

		virtual Spectrum getExcessAttenuation (PropagationPath& path, bool favorable_condition) ;
		virtual Spectrum getFiniteSizeCorrection (PropagationPath& path) ;

	private:

		void createPath (PropagationPath& path) ;
		int	 getMaterial (Material *mat) ;
	};
}