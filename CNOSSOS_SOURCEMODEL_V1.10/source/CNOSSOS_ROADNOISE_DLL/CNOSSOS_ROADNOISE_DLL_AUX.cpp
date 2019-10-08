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
#include "CNOSSOS_ROADNOISE_DLL_AUX.h"
#include <iostream>
#include <cmath>

using namespace std;

namespace CNOSSOS_ROADNOISE
{	

	// -------------------------------------------------------------------------------------------------------------
	// constructor for the vehicle category class
	// -------------------------------------------------------------------------------------------------------------
	VehicleCategory::VehicleCategory()
	{
		this->gradientCorrection = new GradientCategory();
	}

	// -------------------------------------------------------------------------------------------------------------
	// destructor for the vehicle category class
	// -------------------------------------------------------------------------------------------------------------
	VehicleCategory::~VehicleCategory()
	{
		delete this->gradientCorrection;
		if (this->studdedProps != NULL)
			delete this->studdedProps;
	}

	// -------------------------------------------------------------------------------------------------------------
	// constructor for acceleration properties class
	// -------------------------------------------------------------------------------------------------------------
	AccelerationProperties::AccelerationProperties(double d, int K)
	{
		this->distance = d;
		this->K = K;
	}

	// -------------------------------------------------------------------------------------------------------------
	// constructor for temperature properties class
	// -------------------------------------------------------------------------------------------------------------
	TempProperties::TempProperties(double temp)
	{
		this->t = temp;
	}
	
	// -------------------------------------------------------------------------------------------------------------
	// constructor for gradient properties class
	// -------------------------------------------------------------------------------------------------------------
	GradientProperties::GradientProperties(double gradient)
	{
		this->s = gradient;
	}

	// -------------------------------------------------------------------------------------------------------------
	// constructor for gradient calculation parameter class
	// -------------------------------------------------------------------------------------------------------------
	GradientCalculationParameter::GradientCalculationParameter()
	{
		this->calc = true;
		this->a1 = 0;
		this->a2 = 0;
		this->a3 = 0;
		this->calcSpeedFactor = false;
	}

	// -------------------------------------------------------------------------------------------------------------
	// calculation function a specific class of gradient calculations
	// -------------------------------------------------------------------------------------------------------------
	double GradientCalculationParameter::Calc(const double s, const double v)
	{
		if (this->calc)
		{
			double firstFac = (min(12.0,s) + this->a1) / this->a2;

			double speedFactor = 1.0;
			if (this->calcSpeedFactor)
			{
				speedFactor = (v - this->a3) / 100;
			}
			return firstFac * speedFactor;
		}
		else
			return 0.0;
	}
	
	// -------------------------------------------------------------------------------------------------------------
	// constructor for gradient category class
	// -------------------------------------------------------------------------------------------------------------
	GradientCategory::GradientCategory()
	{
		this->calc = false;
		this->b1 = 0;
		this->b2 = 0;
	}

	// -------------------------------------------------------------------------------------------------------------
	// functions that calculates the gradient correction based on the available classes en boundaries
	// -------------------------------------------------------------------------------------------------------------
	double GradientCategory::CalcValue(const double s, const double v)
	{
		if (this->calc)
		{
			if (s < this->b1)
			{
				return this->Classes[0].Calc(-s,v);
			}
			else if (s > this->b2)
			{
				return this->Classes[1].Calc(s,v);
			}
			else
			{
				return 0.0; // this->Classes[1].Calc(s,v);
			}
		}
		else
		{
			return 0.0;
		}
	}
}