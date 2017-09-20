#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		ElementarySource.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: data structures for storing sound power associated with elementary sources
 * changes:
 *
 *	13/112013	initial version: interfacing with elementary source description has been moved
 *				to a separate file. This module interfaces with the output of the CNOSSOS-EU
 *				source modules developed by DGMR.
 *
 *  02/12/2013  added support for evaluating source directivity
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "Spectrum.h"
#include "Geometry3D.h"

namespace CnossosEU
{
	/*
	 * Sound power units
	 */
	struct SpectrumType
	{
		enum Type
		{
			Undefined = 0,		// sound power units undefined
			PointSource = 1,	// sound power per unit source
			LineSource = 2,		// sound power per unit length (dB/m)
			AreaSource = 3,		// sound power per unit area (dB/m²)
		} type ;

		SpectrumType (SpectrumType::Type _type = Undefined) : type(_type) { }

		//Type operator= (SpectrumType::Type _type) { return type = _type ; }
		Type operator= (SpectrumType const& _other) { return type = _other.type ; }

		//bool operator== (SpectrumType::Type _type) { return type == _type ; }
		bool operator== (SpectrumType const& other) const { return type == other.type ; }

		//bool operator!= (SpectrumType::Type _type) { return type != _type ; }
		bool operator!= (SpectrumType const& other) const { return type != other.type ; }
	} ;
	/*
	 * Sound power measurement conditions
	 */
	struct MeasurementType
	{
		enum Type
		{
			Undefined = 0,		// sound power measurement conditions unknown
			FreeField = 1,		// sound power measured in free field conditions
			HemiSpherical = 2	// sound power measured in hemispherical conditions
		} type ;

		MeasurementType (MeasurementType::Type _type = Undefined) : type(_type) { }

		//Type operator= (MeasurementType::Type _type) { return type = _type ; }
		Type operator= (MeasurementType const& _other) { return type = _other.type ; }

		//bool operator== (MeasurementType::Type _type) { return type == _type ; }
		bool operator== (MeasurementType const& _other) const { return type == _other.type ; }

		//bool operator!= (MeasurementType::Type _type) { return type != _type ; }
		bool operator!= (MeasurementType const& _other) const { return type != _other.type ; }
	} ;
	/*
	 * Sound power measurement conditions
	 */
	struct FrequencyWeighting
	{
		enum Type
		{
			Undefined = 0,	// sound power units undefined
			dBLIN = 1,		// sound power expressed in dB units
			dBA = 2			// sound power expressed in dBA units
		} type ;

		FrequencyWeighting (FrequencyWeighting::Type _type = Undefined) : type(_type) { }

		//Type operator= (FrequencyWeighting::Type _type) { return type = _type ; }
		Type operator= (FrequencyWeighting const& _other) { return type = _other.type ; }

		//bool operator== (FrequencyWeighting::Type _type) { return type == _type ; }
		bool operator== (FrequencyWeighting const& _other) const { return type == _other.type ; }

		//bool operator!= (FrequencyWeighting::Type _type) { return type != _type ; }
		bool operator!= (FrequencyWeighting const& _other) const { return type != _other.type ; }
	} ;
	/*
	 * Elementary source
	 */
	struct ElementarySource
	{
		double				sourceHeight ;
		SpectrumType		spectrumType ;
		MeasurementType		measurementType ;
		FrequencyWeighting	frequencyWeighting ;
		Spectrum			soundPower ;

		ElementarySource (void)
		: sourceHeight()
		, spectrumType()
		, measurementType()
		, frequencyWeighting()
		, soundPower() { } ;
		/*
		 * by definition, elementary sources are characterized by apparent sound power,
		 * i.e. apparent sound power as radiated in a given direction.
		 *
		 * real applications would evaluate the directivity of the source here; however
		 * for the purpose of the CNOSSOS-EU project, it is assumed that the source modules
		 * return directional sound power and that there is no further correction needed.
		 */
		Spectrum getSoundPower (Geometry::Vector3D const& direction)
		{
			return soundPower ;
		}
	} ;
} ;
