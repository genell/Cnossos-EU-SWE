#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		ISOWithMeteo.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description:	implements a user-defined ISOWithMeteo calculation method
 *
 * rationale:	the ISO method allows calculation for a single "moderately favorable" propagation
 *			    condition. This value is then used to derive a long-time averaged noise level
 *              by means of a C0 correction term. C0 is defined by the end-user or by local
 *				authorities. In some countries/regions, a published method exist to link C0
 *			    to local weather statistics. 
 *
 *				However, the C0 method has two major pitfalls: first,it is a constant for all 
 *				propagation directions and secondly (and more important), it does not take into 
 *				account the nature of the ground. It is a well known fact that favorable propagation 
 *				conditions mainly destroy ground effect (i.e. the destructive interference of a 
 *				direct and a ground-reflected ray path that occurs under homogeneous propagation 
 *				conditions). Therefore, the effects of meteorological conditions are much larger 
 *				over soft ground than over hard ground and appropriate C0 values should be used 
 *				accordingly.
 *
 *				This is an attempt to overcome the above mentioned difficulties by letting 
 *				the ISO method making an estimate of the attenuation under favorable propagation 
 *				conditions. The frequency of occurrence of favorable and unfavorable propagation 
 *				conditions can then be taken into account similarly to the French NMPB methods. 
 *				Note that the frequency of occurrence of favorable propagation conditions can be 
 *				evaluated in a well defined way from historical recordings of meteorological data 
 *				(e.g. from the ERA40 database) and this process can easily be automated for all 
 *				European countries.
 *
 * changes:
 *
 *	09/12/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include "ISO-9613-2.h"

namespace MyExtension
{
	using namespace CnossosEU ;

	class IsoWithMeteo : public ISO_9613_2
	{
	public:

		inline const char* name (void) { return "IsoWithMeteo" ; }
		inline const char* version (void)  { return "0.001" ; }

		IsoWithMeteo(void) : ISO_9613_2() { }

		inline 	virtual Spectrum getExcessAttenuation (PropagationPath& path, bool favorable_condition)
		{
			/*
			 * use the ISO-9613-2 method to evaluate the attenuation under favorable 
			 * propagation conditions
			 */
			Spectrum att = ISO_9613_2::getExcessAttenuation (path, true) ;
			/*
			 * make an estimate for the attenuation under homogeneous propagation conditions
			 *
			 * the correction term is a frequency-independent constant, depending on:
			 *	- the propagation distance
			 *  - the height of the source and the receiver
			 *  - the (averaged) nature of the ground
			 */
			if (!favorable_condition)
			{
				unsigned int n1 = 0 ;
				unsigned int n2 = path.size()-1 ;
				/* 
				 * propagation distance, source and receiver heights
				 */
				double dp = path[n2].d_path - path[n1].d_path ; 
				double hS = path[n1].z_path - path[n1].pos.z ;
				double hR = path[n2].z_path - path[n2].pos.z ;
				/*
				 * get averaged G value for propagation path
				 */
				double Gm = getAveragedGround (path) ;
				/*
				 * maximum value of the correction term as a function of averaged 
				 * ground and propagation distance
				 */
				double dm = std::max (1.0, dp/25) ;
				double C0 = (1 + 5 * Gm) * pow (log10(dm), 1.5) ;
				/*
				 * reduce effect for short distance / higher sources and receivers
				 */
				double x = (hS+hR)/dp ;
				double hd = std::max (0.0, 1 - 10 * x) ;
				double Cg = C0 * hd ;
				/*
				 * apply correction
				 */
				att -= Cg ;
			}
			return att ;
		}
	private:

		inline double getAveragedGround (PropagationPath& path)
		{
			double sumD = 0 ;
			double sumG = 0 ;
			for (unsigned int i = 1 ; i < path.size() ; ++i)
			{
				double G = path[i].mat->getGValue() ;
				double d = path[i].d_path - path[i-1].d_path ;
				sumG += G * d ;
				sumD += d ;
			}
			return sumG/sumD ;
		}
	};
}