/* 
 * ------------------------------------------------------------------------------------------------
 * file:		Spectrum.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.CSTB.txt
 * description: support for simplified representation of spectral values.
 * changes:
 *
 *	18/10/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include "./Spectrum.h"

using namespace CnossosEU ;

static double _fixedFreq[]    = {   63,   125,  250,  500, 1000, 2000, 4000, 8000 } ;
/*
 * note that dB(A) weighting is not defined for 1/1 octave bands. An estimate can be made by 
 * using 1/3 octave band values, assuming a constant spectral density in each 1/3 octave band
 */
static double _dBAweighting[] = {-25.2, -15.6, -8.4, -3.1,  0.0,  1.2,  0.9, -2.4 } ; 

namespace CnossosEU
{
	double negative_inf = -std::numeric_limits<double>::infinity() ;
	double positive_inf =  std::numeric_limits<double>::infinity() ;

	void SetInfinityMode (bool on_off)
	{
		if (on_off)
		{
			negative_inf = -std::numeric_limits<double>::infinity() ;
			positive_inf =  std::numeric_limits<double>::infinity() ;
		}
		else
		{
			negative_inf = -std::numeric_limits<double>::max() ;
			positive_inf =  std::numeric_limits<double>::max() ;
		}
	}

	double Spectrum::freq (unsigned int index)
	{
		assert (index < nbFreq) ;
		return _fixedFreq[index] ;
	}

	double Spectrum::dBA (unsigned int index)
	{
		assert (index < nbFreq) ;
		return _dBAweighting[index] ;
	}
}