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
#include <fstream>

using namespace std;

namespace CNOSSOS_ROADNOISE
{	
	const string XML_DATA_VERSION = "V1.0";
	
	// enumeration describing the noise generator
	enum NoiseGenerator { ngROLLING, ngPROPULSION, NUM_NOISE_GENERATORS };

	// enumeration for obtaining the correct acceleration coefficients
	enum ConstFactor { cfAlpha, cfBeta, NUM_CONST_FACTORS };

	// enumeration for obtaining the correct tyre coefficients
	enum TyreConstFactor { tcALPHA, tcBETA, tcNUM_CONST };

	// enumeration for obtaining the correct surface coefficients
	enum SurfaceConstFactor { scALPHA, scBETA, scNUM_CONST };

	// Nr of categories
	const int MAX_SRC_CAT = 10;

	// Nr of frequency bands
	const int MAX_FREQ_BAND_CENTRE = 8;

	// Frequency bands
	const int FreqBands[MAX_FREQ_BAND_CENTRE] = {63, 125, 250, 500, 1000, 2000, 4000, 8000};

	// Nr of gradient levels
	const int NUM_GRADIENT_LEVELS = 2;

	// Nr of speed variation types (crossings, roundabouts etc)
	const int MAX_SPEED_VARIATION_TYPES = 10;

	// Reference temperature
	const int DEFAULT_REF_TEMP = 20;

	// Reference speed
	const double DEFAULT_REF_SPEED = 70;

	// Minimal allowed speed
	const double DEFAULT_MIN_SPEED = 20;

	// Source height
	const double DEFAULT_SRC_HEIGHT = 0.05;

	// array for tyre correction coefficients
//	static double TyreCorrection[tcNUM_CONST][MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];

	// array for road surface coefficients
//	static double RoadSurfaceCorrection[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE + 1];	 // TODO: move to RoadSurface class

	// array for vehicle emission coefficients
//	static double EmissionCoefficient[NUM_CONST_FACTORS][NUM_NOISE_GENERATORS][MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE];

	// array for acceleration coefficients
//	static double AccelerationCoefficient[NUM_NOISE_GENERATORS][MAX_SRC_CAT];

	
	// Aux function for obtaining the correct coefficients from the various tables
//	double getAccelerationCoeffientRolling(const int cat, const int t);
//	double getAccelerationCoeffientPropulsion(const int cat, const int t);
//	double getConstValue(const int cat, const int band,const ConstFactor t, const NoiseGenerator ng);
//	double getTyreCorrectionConstFactor(const int cat, const int band, const TyreConstFactor t);
//	double getSurfaceCorrectionConstFactor(const int cat, const int band, const SurfaceConstFactor t);

	
	// Aux functions for printing the results to file
	void PrintDoubleDataTable(ofstream& fs, const double Table[MAX_SRC_CAT][MAX_FREQ_BAND_CENTRE], string name);
	void PrintDataTable(ofstream& fs, const double Table[MAX_SRC_CAT], string name);

	// Aux function for loading constant files
//	void LoadRoadParamsXML(const string fn);
//	void LoadRoadSurfaceXML(const string fn);
//	void LoadCoefficientFile(const string fn);
//	void LoadAccCoefficientFile(const string fn);
//	void LoadTyreCorrectionFile(const string fn);
//	void LoadRoadSurfaceCorrectionFile(const string fn);
}
