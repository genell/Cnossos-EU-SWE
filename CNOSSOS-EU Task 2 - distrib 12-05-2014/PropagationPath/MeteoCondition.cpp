#include "MeteoCondition.h"

using namespace CnossosEU ;

double MeteoCondition::getSoundSpeed (void)
{
	return 331.3 * sqrt (1 + temperature / 273.15) ;
}

Spectrum MeteoCondition::getAirAbsorption (void)
{
	double tref = 293.15 ;
	double tair = 273.15 + temperature ;
	double tcor = tair/tref ;
	double xmol = humidity * pow (10., 4.6151 - 6.8346 * pow (273.16 / tair, 1.261));

	double frqO = 24. + 40400. * xmol * ((.02 + xmol) / (0.391 + xmol)) ;
	double frqN = pow (tcor,-0.5) * (9. + 280. * xmol * exp (-4.17 * (pow (tcor,-1./3.) - 1.))) ;

	Spectrum att ;
	for (unsigned int i = 0 ; i < att.size() ; ++i)
	{
		double freq = att.freq(i) ;
		double a1 = 0.01275 * exp (-2239.1 / tair) / (frqO + (freq * freq / frqO)) ;
		double a2 = 0.10680 * exp (-3352.0 / tair) / (frqN + (freq * freq / frqN)) ;
		double a0 = 8.686 * freq * freq
			* (1.84e-11 * pow(tcor,0.5) + pow(tcor,-2.5) * (a1 + a2)) ;

		att[i]= -a0 ;
	}
	return att ;
}