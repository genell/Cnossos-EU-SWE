#pragma once

// ----------------------------------------------------------------------------------------------------- 
//
// project : CNOSSOS_ROADNOISE.dll
// author  : Martijn Kirsten
// company : DGMR
//
// Calculation road traffic noise source emission
//
// Disclaimer + header text
//
// Version : 
// Release : 
//
// ----------------------------------------------------------------------------------------------------- 

#include "stdafx.h"
#include <iostream>
#include <list>
#include "CNOSSOS_ROADNOISE_DLL_CONST.h"

using namespace std;

namespace CNOSSOS_ROADNOISE
{	
	class RoadSurface
	{
		public:
			string	id;
			string	description;
			double	Vmin, Vmax;
			double	coefficientA[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];
			double  coefficientB[MAX_SRC_CAT];
	};
	
	// -------------------------------------------------------------------------------------------------------------
	// class to contain the factors for the studded tyre correction calculations
	// -------------------------------------------------------------------------------------------------------------
	class StuddedCategory
	{
		public:
			double	studded[NUM_CONST_FACTORS][MAX_FREQ_BAND_CENTRE];
	};
	
	// -------------------------------------------------------------------------------------------------------------
	// class to contain the factors for the gradient correction calculations
	// -------------------------------------------------------------------------------------------------------------
	class GradientCalculationParameter
	{
		public:
			bool calc;
			
			double a1,a2,a3;
			bool calcSpeedFactor;

			double Calc(const double s, const double v);
			GradientCalculationParameter();
			
	};

	// -------------------------------------------------------------------------------------------------------------
	// class that contains multiple gradient correction parameter (low, mid, high) for each vehicle category
	// -------------------------------------------------------------------------------------------------------------
	class GradientCategory
	{
		public:
			double b1, b2;
			bool calc;
			GradientCalculationParameter Classes[NUM_GRADIENT_LEVELS];
			
			double CalcValue(const double s,const double v);
		
		GradientCategory();
	};

	// -------------------------------------------------------------------------------------------------------------
	// class that contains multiple gradient correction parameter (low, mid, high) for each vehicle category
	// -------------------------------------------------------------------------------------------------------------
	class VehicleCategory
	{
		public:
			string	id;
			string	description;
			bool	calcNoise[NUM_NOISE_GENERATORS];
			bool	calcStudded;
			double	Ksurface[MAX_FREQ_BAND_CENTRE];
			double coefficientA[NUM_NOISE_GENERATORS][MAX_FREQ_BAND_CENTRE];
			double coefficientB[NUM_NOISE_GENERATORS][MAX_FREQ_BAND_CENTRE];
			double speedVariationCoefficient[MAX_SPEED_VARIATION_TYPES][NUM_NOISE_GENERATORS];
			StuddedCategory		*studdedProps;
			GradientCategory	*gradientCorrection;

		VehicleCategory();
		~VehicleCategory();
	};

	// -------------------------------------------------------------------------------------------------------------
	// class containing the properties for the acceleration correction
	// -------------------------------------------------------------------------------------------------------------
	class AccelerationProperties
	{
		public:
			double distance;
			int K;

		AccelerationProperties(double d, int K);
	};

	// -------------------------------------------------------------------------------------------------------------
	// class containing the properties for the temperature correction
	// -------------------------------------------------------------------------------------------------------------
	class TempProperties
	{
		public:
			double t;
		TempProperties(double temp);
	};

	// -------------------------------------------------------------------------------------------------------------
	// class containing the properties for the gradient correction
	// -------------------------------------------------------------------------------------------------------------
	class GradientProperties
	{
		public:
			double s;
		GradientProperties(double gradient);
	};

}

