#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		Spectrum.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.CSTB.txt
 * description: support for simplified representation of spectral values.
 *				In general, spectral values are represented as a list of (frequency, value) pairs. 
 *				For the purpose of the CNOSSOS-EU implementation all spectral data are given in 8 
 *				octave bands with fixed center frequencies.
 * changes:
 *
 *	18/10/2013	initial version
 *
 *  21/10/2013	added LIN/LOG conversion funcions
 *
 *  23/10/2013	added operations and binary functions
 *
 *  23/10/2013	added complex spectra type
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include <assert.h>
#include <limits>
#include <complex>
#include <float.h>


namespace CnossosEU
{
	extern double negative_inf ;
	extern double positive_inf ;
	/*
	 * enable or disable IEEE representation of +/-infinity
	 * if disabled, the library will use an arbitrary large value to represent infinity
	 */
	void SetInfinityMode (bool on_off = true) ;
	/*
	 * lin-log conversion for single values
	*/
	static inline double LOG10 (double x)
	{
		return (x > 0) ? 10. * log10(x) : negative_inf ;
	}
	static inline double POW10 (double x)
	{
		return _finite(x) ? pow(10., x/10.) : 0.0 ;
	}
	static inline double LOG20 (double x)
	{
		return (x > 0) ? 20. * log10(x) : negative_inf ;
	}
	static inline double POW20 (double x)
	{
		return _finite(x) ? pow(10., x/20.) : 0.0 ;
	}

	struct Spectrum
	{
		/*
		 * number of frequency bands
		 */
		static const int nbFreq = 8 ;
		/*
		 * internal representation of spectra by means of fixd-size array
		 */
		double val[nbFreq] ;
		/*
		 * accessors for size, center frequency and values associated with frequency bands
		 */
		size_t size (void) const { return nbFreq ; }
		static double freq (unsigned int index) ;
		static double dBA (unsigned int index) ;
		double data (unsigned int index) const { assert (index < nbFreq) ; return val[index] ; }
		/*
		 * direct access to values by means of indexed notation (no checking)
		 */
		double& operator[] (unsigned int index) { return val[index] ; }
		double const& operator[] (unsigned int index) const { return val[index] ; }
		/*
		 * default constructor
		 */
		explicit Spectrum (double init_value = 0) 
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] = init_value ; 
		}
		/*
		 * constructor from array of values
		 */
		Spectrum (double const values[8])
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] = values[i] ; 
		}
		/*
		 * copy constructor
		 */
		Spectrum (Spectrum const& other)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] = other.val[i] ; 
		}
		/*
		 * assignment operator
		 */
		Spectrum& operator= (Spectrum const& other)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] = other.val[i] ; 
			return *this ;
		}
		/*
		 * unary operators, combine spectrum + spectrum
		 */
		Spectrum& operator- (void)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] = -val[i] ; 
			return *this ;
		}
		/*
		 * internal operators, combine this spectrum with another spectrum
		 */
		Spectrum& operator+= (Spectrum const& other)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] += other.val[i] ; 
			return *this ;
		}
		Spectrum& operator-= (Spectrum const& other)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] -= other.val[i] ; 
			return *this ;
		}
		Spectrum& operator*= (Spectrum const& other)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] *= other.val[i] ; 
			return *this ;
		}
		Spectrum& operator/= (Spectrum const& other)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] /= other.val[i] ; 
			return *this ;
		}
		/*
		 * internal operators, combine this spectrum with a numerical constant
		 */
		Spectrum& operator+= (double value)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] += value ; 
			return *this ;
		}
		Spectrum& operator-= (double value)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] -= value ; 
			return *this ;
		}
		Spectrum& operator*= (double value)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] *= value ; 
			return *this ;
		}
		Spectrum& operator/= (double value)
		{ 
			for (unsigned int i = 0 ; i < nbFreq ; ++i) val[i] /= value ; 
			return *this ;
		}
	};
	/*
	 * binary operators on spectra
	 */
	inline Spectrum operator+ (Spectrum const& s1, Spectrum const& s2)
	{
		Spectrum res(s1) ;
		return res += s2 ;
	};
	inline Spectrum operator- (Spectrum const& s1, Spectrum const& s2)
	{
		Spectrum res(s1) ;
		return res -= s2 ;
	};
	inline Spectrum operator* (Spectrum const& s1, Spectrum const& s2)
	{
		Spectrum res(s1) ;
		return res *= s2 ;
	};
	inline Spectrum operator/ (Spectrum const& s1, Spectrum const& s2)
	{
		Spectrum res(s1) ;
		return res /= s2 ;
	};
	/*
	 * binary operators on spectrum + numerical value
	 */
	inline Spectrum operator+ (Spectrum const& s1, double value)
	{
		Spectrum res(s1) ;
		return res += value ;
	};
	inline Spectrum operator- (Spectrum const& s1, double value)
	{
		Spectrum res(s1) ;
		return res -= value ;
	};
	inline Spectrum operator* (Spectrum const& s1, double value)
	{
		Spectrum res(s1) ;
		return res *= value ;
	};
	inline Spectrum operator* (double value, Spectrum const& s1)
	{
		Spectrum res(s1) ;
		return res *= value ;
	};
	inline Spectrum operator/ (Spectrum const& s1, double value)
	{
		Spectrum res(s1) ;
		return res /= value ;
	};
	/*
	 * lin-log conversion for spectra
	 */
	inline Spectrum LOG10 (Spectrum const& other)
	{
		Spectrum res (other);
		for (unsigned int i = 0 ; i < res.size() ; ++i) { res[i] = LOG10 (res[i]) ; }
		return res ;
	};
	inline Spectrum POW10 (Spectrum const& other)
	{
		Spectrum res (other) ;
		for (unsigned int i = 0 ; i < res.size() ; ++i) { res[i] = POW10 (res[i]) ; }
		return res ;
	};

	typedef std::complex<double> Complex ;
	
	struct ComplexSpectrum
	{
		/*
		 * internal representation of spectra by means of fixd-size array
		 */
		Complex val[Spectrum::nbFreq] ;
		/*
		 * accessors for size, center frequency and values associated with frequency bands
		 */
		size_t  size (void) { return Spectrum::nbFreq ; }
		double  freq (unsigned int index) { return Spectrum::freq(index) ; }
		Complex data (unsigned int index) { assert (index < size()) ; return val[index] ; }
		/*
		 * direct access to values by means of indexed notation (no checking)
		 */
		Complex      & operator[] (unsigned int index) { return val[index] ; }
		Complex const& operator[] (unsigned int index) const { return val[index] ; }
	} ;
}