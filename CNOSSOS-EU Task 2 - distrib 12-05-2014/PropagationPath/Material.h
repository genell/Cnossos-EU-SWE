#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		Material.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: manage acoustical properties associated with material 
 * changes:
 *
 *	18/10/2013	initial version
 *
 *  23/10/2013	added impedance as material property
 *
 *  23/10/2013	added support for evaluating default impedances and absorption coefficients
 * ------------------------------------------------------------------------------------------------- 
 */
#include "Spectrum.h"
#include "ReferenceObject.h"

namespace CnossosEU
{
	/*
	 * impedances are represented by complex spectral values
	 */
	typedef ComplexSpectrum Impedance ;
	/*
	 * The Material class encodes material properties used in different prediction methods.
	 *
	 * The following material properties are currently supported :
	 *
	 *		G		   a single value used in the calculation of ground effects in ISO-9613-2 and
	 *				   similar methods.
	 *
	 *		sigma	   a single value used for generating impedance values in the JRC-2010-draft
	 *				   method (formerly known as Harmonoise/Imagine)
	 * 
	 *		alpha	   absorption coefficients versus frequency. Absorption is used in all methods
	 *				   to estimate the attenuation due to reflections from vertical obstacles.
	 *
	 *		impedance  user-defined impedance values
	 *
	 * Note : G values are mandatory, whereas the other properties are optional. The library has 
	 *        internal support for generating appropriate values for missing properties.
	 *
	 * Note : advanced impedance modeling, a feature supported by the Harmonoise/Imagine method
	 *        is not, for the time being, supported by the CNOSSOS-EU project.
	 */
	class Material : public System::ReferenceObject
	{
	public:

		void      setG (double G) { G_value = G ;}
		void	  setSigma (double sigma) ;
		void	  setAlpha (Spectrum const& alpha) ;
		void	  setImpedance (Impedance const& impedance) ;

		double    getGValue (void) { return G_value ; }

		double const*    getSigma (void) { return sigma ; } 
		Spectrum const*  getAlpha (void) { return alpha ; }
		Impedance const* getImpedance (void) { return impedance ; }

		double	   getSigmaValue (void) ;
		Spectrum   getAlphaValue (void) ;
		Impedance  getImpedanceValue (void) ;
		
		Material (double G = 0) : G_value (G), sigma (0), alpha(0), impedance(0) { }
		~Material (void)
		{
			if (sigma) delete sigma ;
			if (alpha) delete alpha ;
			if (impedance) delete impedance ;
		}

	private:

		double	   G_value ;
		double*	   sigma ;
		Spectrum*  alpha ;
		Impedance* impedance ;
	} ;
	/*
	 * Materials are stored in a common database and accessed through textual identifiers
	 *
	 * On initialization, the database contains default values for 8 ground types and 4
	 * absorbing materials, as defined in the CNOSSOS-EU documentation. 
	 */
	Material* getMaterial (const char* id, bool create_if_needed = false) ;
}