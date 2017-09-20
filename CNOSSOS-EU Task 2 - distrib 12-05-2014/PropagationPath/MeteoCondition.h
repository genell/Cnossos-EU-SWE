#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		MeteoCondition.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: encode influence of meteorological conditions on propagation  
 * changes:
 *
 *	24/10/2013	initial version
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "Spectrum.h"

namespace CnossosEU
{
	struct MeteoCondition
	{
		enum MeteoModel
		{
			DEFAULT   = 0,	// use default model for the selected point-to-point propagation model
			JRC2012	  = 1,  // force using JRC2010 methodology for calculating long-time averaged noise levels
			ISO9613   = 2   // force using ISO9613-2 methodology for calculating long-time averaged noise levels
		};

		MeteoModel model ;

		double	C0 ;
		double	pFav ;
		double	temperature ;
		double  humidity ;

		MeteoCondition (void)
		{
			model = DEFAULT ;
			C0 = 0 ;
			pFav = 100 ;
			temperature = 15 ;
			humidity = 70 ;
		}

		Spectrum getAirAbsorption (void) ;
		double	 getSoundSpeed (void) ;
	};
}