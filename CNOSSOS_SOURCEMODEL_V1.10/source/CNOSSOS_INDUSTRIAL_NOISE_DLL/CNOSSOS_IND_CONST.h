#pragma once

#include "stdafx.h"
#include <string>

using namespace std;

namespace CNOSSOS_INDUSTRIAL_NOISE
{
	const string XML_DATA_VERSION = "V1.0";

	const string ERRMSG_WRONG_ROOT_ELEMENT = "Unexpected root element";
	const string ERRMSG_MISSING_ELEMENT = "Element not found";
	const string ERRMSG_MISSING_OR_INVALID_ATTRIBUTES = "Invalid or missing attributes";

	// Enumeration describing type of source
	enum SourceType { stUnknown, stPoint, stLine, stArea, NUM_SOURCE_TYPES };
	const string SourceTypeNames[NUM_SOURCE_TYPES] = { "", "PointSource", "LineSource", "AreaSource" };

	// Enumeration describing directionality of source
	enum SourceDirectionality { sdUnknown, sdHemispherical, sdOmnidirectional, NUM_SOURCE_DIRECTIONALITIES };
	const string SourceDirectionalityNames[NUM_SOURCE_DIRECTIONALITIES] = { "Unknown", "HemiSpherical", "FreeField" };

	// Number of frequency bands
	const int MAX_FREQ_BAND_CENTRE = 8;

	// Frequency bands
	const int FreqBands[MAX_FREQ_BAND_CENTRE] = {63, 125, 250, 500, 1000, 2000, 4000, 8000};

	// Frequency values;
	typedef double Frequencies[MAX_FREQ_BAND_CENTRE];

}

