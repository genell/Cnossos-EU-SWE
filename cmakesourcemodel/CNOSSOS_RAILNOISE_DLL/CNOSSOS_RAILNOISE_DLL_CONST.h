#pragma once

#include "stdafx.h"
#include <iostream>
#include <fstream>

using namespace std;

namespace CNOSSOS_RAILNOISE
{	
	const string XML_DATA_VERSION = "V1.0";

	const double PI = 3.14159265358979323846;

	const string ERRMSG_WRONG_ROOT_ELEMENT = "Unexpected root element";
	const string ERRMSG_MISSING_ELEMENT = "Element not found";
	const string ERRMSG_MISSING_OR_INVALID_ELEMENT = "Invalid or missing element";
	const string ERRMSG_MISSING_OR_INVALID_ATTRIBUTES = "Invalid or missing attributes";
	const string ERRMSG_WRONG_VERSION = "Unexpected version";

	const int SRC_HEIGHT = 2;
	const int MAX_FREQ_BAND_CENTRE = 8;
	const int MAX_FREQ_BAND = 3 * MAX_FREQ_BAND_CENTRE;
	
	const double FreqBands[MAX_FREQ_BAND] = {50,63,80,100,125,160,200,250,315,400,500,630,800,1000,1250,1600,2000,2500,3150,4000,5000,6300,8000,10000};

	const int MAX_WAVELENGTH = 32;
	const double Wavelengths[MAX_WAVELENGTH] = {100,80,63,50,40,31.5,25,20,16,12.5,10,8,6.3,5,4,3.15,2.5,2,1.6,1.25,1,0.8,0.63,0.5,0.4,0.315,0.25,0.2,0.16,0.125,0.1,0.08};

	enum RunningConditionEnum { constant, accelerating, decelerating, idling, NUM_RUNNING_CONDITIONS };
	const string RunningConditionNames[NUM_RUNNING_CONDITIONS] = { "constant", "accelerating", "decelerating", "idling" };
	enum PhysicalSourceEnum { A, B, NUM_PHYSICAL_SOURCES };
	const string PhysicalSourceNames[NUM_PHYSICAL_SOURCES] = { "A", "B" };
	enum SourceTypeEnum { rolling, traction, aerodynamic, NUM_SOURCE_TYPES };
	const string SourceTypeNames[NUM_SOURCE_TYPES] = { "rolling", "traction", "aerodynamic" };

	// Enumeration describing type of source
	enum SourceGeometryTypeEnum { stUnknown, stPoint, stLine, stArea, NUM_SOURCE_GEOMETRY_TYPES };
	const string SourceGeometryTypeNames[NUM_SOURCE_GEOMETRY_TYPES] = { "", "PointSource", "LineSource", "AreaSource" };

	void PrintDoubleDataTable(ofstream& fs, const double Table[SRC_HEIGHT][MAX_FREQ_BAND], string name);
}
