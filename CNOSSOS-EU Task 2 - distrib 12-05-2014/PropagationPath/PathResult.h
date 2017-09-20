#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		PathResult.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: store intermediate results as produced by the calculation methods 
 * changes:
 *
 *	23/10/2013	initial version
 *
 * ------------------------------------------------------------------------------------------------- 
 */

#include "Spectrum.h"
#include <string.h> 

namespace CnossosEU
{
	struct PathResult
	{
		Spectrum	Lw ;			// sound power of the source
		Spectrum	dBA ;			// dB(A) weighting 
		Spectrum	delta_Lw ;		// sound power adapter
		double		AttGeo ;		// geometrical spread
		Spectrum	AttAir ;		// air absorption
		Spectrum	AttAbsMat ;		// attenuation due to absorption by reflecting obstacles
		Spectrum	AttLatDif ;		// attenuation due to lateral diffraction
		Spectrum	AttSize ;		// correction for finite size of obstacles
		Spectrum	AttF ;		    // excess attenuation under favorable conditions
		Spectrum	AttH ;			// excess attenuation under homogeneous conditions
		Spectrum	LpF ;			// sound pressure level under favorable conditions
		Spectrum	LpH ;			// sound pressure level under homogeneous conditions 
		Spectrum	Leq ;			// long-time averaged sound pressure level
		double		LpF_dBA ;		// dB(A) level under favorable conditions
		double		LpH_dBA ;		// dB(A) level under homogeneous conditions
		double		Leq_dBA ;		// long-time averaged dB(A) level

		PathResult (void) { memset (this, 0, sizeof(PathResult)) ; } ;
	};

	void print_results_to_stdout (PathResult& path) ;

	bool output_results_to_XML (const char* filename, PathResult& path) ;
}
