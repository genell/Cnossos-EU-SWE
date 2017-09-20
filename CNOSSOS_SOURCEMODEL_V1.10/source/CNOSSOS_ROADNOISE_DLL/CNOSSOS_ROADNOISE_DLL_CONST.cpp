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
#include "CNOSSOS_ROADNOISE_DLL_CONST.h"
#include "../tinyxml/tinyxml.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

using namespace std;

namespace CNOSSOS_ROADNOISE
{	
/*	// Aux function for obtaining the rolling noise acceleration coefficient
	double getAccelerationCoeffientRolling(const int cat, const int t)
	{
		return AccelerationCoefficient[ngROLLING][cat];
	}

	// Aux function for obtaining the propulsion noise acceleration coefficient
	double getAccelerationCoeffientPropulsion(const int cat, const int t)
	{
		return AccelerationCoefficient[ngPROPULSION][cat];
	}

	// Aux function for obtaining the vehicle category coefficients
	double getConstValue(const int cat, const int band, const ConstFactor t, const NoiseGenerator ng)
	{
		return EmissionCoefficient[t][ng][cat][band];
	}
	
	// Aux function for obtaining the studded tyre correction coefficient
	double getTyreCorrectionConstFactor(const int cat, const int band, const TyreConstFactor t)
	{
		return TyreCorrection[t][cat][band];
	}


	// Aux function for obtaining the road surface coefficient
	double getSurfaceCorrectionConstFactor(const int cat, const int band, const SurfaceConstFactor t)
	{
		if (t == scALPHA)
			return RoadSurfaceCorrection[cat][band];
		else
			return RoadSurfaceCorrection[cat][MAX_FREQ_BAND_CENTRE];
	}*/

	// Aux function for printing a table of date to file
	void PrintDoubleDataTable(ofstream& fs, const double Table[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE], string name)
	{
		fs << endl << name.c_str() << endl;
		fs << "\t\t";
		for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
			fs <<  setw(8) << FreqBands[i] << "\t";	
		fs << endl << endl;


		for (int m=0; m < MAX_SRC_CAT; m++)
		{
			fs << "Cat" << m << "\t";
			for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
			{
				fs <<  setw(8) << Table[m][i] << "\t";
			}
			fs << endl;
		}
	}

	// Aux function for printing a table of date to file
	void PrintDataTable(ofstream& fs, const double Table[MAX_SRC_CAT], string name)
	{
		fs << endl << name.c_str() << endl;
		for (int i=0; i < MAX_SRC_CAT; i++)
		{
			fs << "Cat" << i << "\t" << Table[i] << endl;	
		}
	}

}