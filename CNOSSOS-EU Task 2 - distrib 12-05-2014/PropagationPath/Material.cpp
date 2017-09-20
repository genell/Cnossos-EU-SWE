/* 
 * ------------------------------------------------------------------------------------------------
 * file:		Material.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implement material properties and shared material list
 * changes:
 *
 *	18/01/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include "Material.h"

#include <string>
#include <map>
using namespace std ;
using namespace CnossosEU ;
using namespace System ;
/*
 * global table of materials
 */
typedef map < string, ref_ptr<Material> > MaterialList ;
static MaterialList matList ;
/*
 * utility function: lookup material table
 */
static Material* findMaterial (const char* id, bool create_if_needed) 
{ 
	Material* mat = matList[id] ;
	if (mat == 0 && create_if_needed)
	{
		mat = matList[id] = new Material() ;
	}
	return mat ;
}
/*
 * construct initial table of materials
 *
 * see table VI.1 from JRC_2012 report 
 */
static void initMaterialList (void)
{
	Material* mat ;
	/*
	 * predefined ground types, see table VI.1 of the JRC-2012 report
	 */
	mat = findMaterial ("A", true) ; mat->setG (1.0) ; mat->setSigma (12.5) ;
	mat = findMaterial ("B", true) ; mat->setG (1.0) ; mat->setSigma (31.5) ;
	mat = findMaterial ("C", true) ; mat->setG (1.0) ; mat->setSigma (80.) ;
	mat = findMaterial ("D", true) ; mat->setG (1.0) ; mat->setSigma (200.) ;
	mat = findMaterial ("E", true) ; mat->setG (0.7) ; mat->setSigma (500.) ;
	mat = findMaterial ("F", true) ; mat->setG (0.3) ; mat->setSigma (2000) ;
	mat = findMaterial ("G", true) ; mat->setG (0.0) ; mat->setSigma (20000) ;
	mat = findMaterial ("H", true) ; mat->setG (0.0) ; mat->setSigma (200000.) ;
	/*
	 * predefined material types, see report IMA10-TR250506-CSTB05.doc, deliverable D4 of the 
	 * Imagine project (FP6 contract SSP1-CT-2003-503549-IMAGINE)
	 */
	mat = findMaterial ("A0", true) ; mat->setG (0.0) ; mat->setSigma (20000.) ;
	mat = findMaterial ("A1", true) ; mat->setG (1.0) ; mat->setSigma (2000.) ;
	mat = findMaterial ("A2", true) ; mat->setG (1.0) ; mat->setSigma (250.) ;
	mat = findMaterial ("A3", true) ; mat->setG (1.0) ; mat->setSigma (80.) ;
	mat = findMaterial ("A4", true) ; mat->setG (1.0) ; mat->setSigma (40.) ;
}
/*
 * public function: access material table 
 */
Material* CnossosEU::getMaterial (const char* id, bool create_if_needed) 
{ 
	if (matList.empty()) initMaterialList() ;
	return findMaterial (id, create_if_needed) ;
}
/*
 * utility function for converting G values into equivalent sigma values
 *
 * see table VI.1 from JRC_2012 report 
 */
static double convert_G_to_sigma (double G)
{
	if (G < 0) G = 0 ;
	if (G > 1) G = 1 ;
	return 20000 * pow (10., -2 * pow (G, 3./5.)) ;
}
/*
 * utility function for sigma values into equivalent G values
 *
 * see table VI.1 from JRC_2012 report 
 */
static double convert_sigma_to_G (double sigma)
{
	if (sigma > 20000) sigma = 20000 ;
	if (sigma < 200)   sigma = 200 ;
	return pow (0.5 * log10(sigma/20000), 5./3.) ;
}
/*
 * calculate Sabine absorption coefficient as a function of complex impedance
 *
 * see Morse & Ingard, "Theoretical Acoustics", McGraw-Hill (1968), eq. 9.5.8 page 580
 */
Spectrum convert_impedance_to_alpha (ComplexSpectrum impedance)
{
	Spectrum alpha ;
	for (unsigned int i = 0 ; i < impedance.size() ; ++i)
	{
		Complex z = 1.0 / impedance[i] ;
		double x = z.real() ;
		double y = z.imag() ;
		double a1 = (x * x - y * y) / y ;
		double a2 = y / (x * x + y * y + x) ;
		double a3 = ((x + 1) *(x + 1) + y * y) / (x * x + y * y) ;
		alpha[i] = 8 * x * (1 + a1 * atan(a2) - x * log(a3)) ;
	}
	return alpha ;
}
/*
 * calculate complex impedance using Delany-Bazley two-parameter impedance model
 *
 * param sigma	flow resistivity of the absorbing layer in kPas/m²
 *
 * param layer	thickness of the layer (in meters) or zero for an infinite layer
 */
Impedance convert_sigma_to_impedance (double sigma, double layer)
{
	ComplexSpectrum Z ;
	const double PI = 3.1415926 ;
	for (unsigned int i = 0 ; i < Z.size() ; ++i)
	{
		double freq = Z.freq(i) ;
		double s = log (freq / sigma) ;
		double x = 1. + 9.08 * exp (-.75 * s) ; 
		double y = 11.9 * exp (-0.73 * s) ;     
		Z[i] = Complex(x,y) ;
		/*
		 * finite thickness layers (for numerical reasons restricted to sigma < 1.E+6 Rayls)
		 */
		if (layer > 0 && sigma < 1000)
		{
			double s = 1000 * sigma / freq ;
			double c = 340 ;
			Complex k = 2 * PI * freq / c * Complex (1 + 0.0858 * pow(s, 0.70), 0.175 * pow(s, 0.59)) ;
			Complex j(0,1) ;
			Z[i] = Z[i] / tanh (-j * k * layer) ;
		}
	}
	return Z ;
}
/*
 * get or estimate the material's the flow resistivity 
 */
double Material::getSigmaValue (void)
{
	return sigma ? *sigma : convert_G_to_sigma (G_value) ;
}
/*
 * get or estimate the material's absorption coefficients
 */
Spectrum Material::getAlphaValue (void)
{
	if (alpha) return *alpha ;
	Impedance Z = getImpedanceValue() ;
	return convert_impedance_to_alpha (Z) ;
}
/*
 * get or estimate the material's acoustical impedance
 */
Impedance Material::getImpedanceValue (void)
{
	if (impedance) return *impedance ;

	double sigma = getSigmaValue() ;
	return convert_sigma_to_impedance (sigma, 0.0) ;
}

void Material::setSigma (double _sigma)
{ 
	if (sigma) 
		*sigma = _sigma ;
	else
		sigma = new double (_sigma) ;
}

void Material::setAlpha (Spectrum const& _alpha) 
{ 
	if (alpha)
		*alpha = _alpha ;
	else
		alpha = new Spectrum (_alpha) ;
}
void Material::setImpedance (Impedance const& _impedance) 
{ 
	if (impedance)
		*impedance = _impedance ;
	else
		impedance = new Impedance (_impedance) ;
}