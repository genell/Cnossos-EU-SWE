#include "stdafx.h"
#include <string>
#include "../tinyxml/tinyxml.h"
#include "../tinyxml/tinystr.h"

using namespace std;

namespace CNOSSOS
{
	
	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read a double from a string
	//
	// Parameters :
	//				s		: string containing a double
	//              def		: default value in case of an invalid string
	// --------------------------------------------------------------------------------------------------------
	double parseFloat(const string s);
	double parseFloat(const string s, const double def);

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read an integer from a string
	//
	// Parameters :
	//				s		: string containing an integer
	//              def		: default value in case of an invalid string
	// --------------------------------------------------------------------------------------------------------
	int parseInt(const string s);
	int parseInt(const string s, const int def);
	
	int copyFloatsFromString(const string s, double *f, const int count);

	string stringFromFloats(const double *f, const int count, const char separator = ' ');

	string stringFromFloat(double n);

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read the text from a named child element
	//
	// Parameters :
	//				e		: parent XML element
	//              name	: name of child element
	//              def		: default value in case of a missing element or an invalid string
	// --------------------------------------------------------------------------------------------------------
	string stringFromElement(TiXmlElement *e, const string name, const string def = "");

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read a double from a named child element
	//
	// Parameters :
	//				e		: parent XML element
	//              name	: name of child element
	//              def		: default value in case of a missing element or an invalid string
	// --------------------------------------------------------------------------------------------------------
	double floatFromElement(TiXmlElement *e, const string name, const double def = 0.0);

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read an integer from a named child element
	//
	// Parameters :
	//				e		: parent XML element
	//              name	: name of child element
	//              def		: default value in case of a missing element or an invalid string
	// --------------------------------------------------------------------------------------------------------
	int intFromElement(TiXmlElement *e, const string name, const int def = 0);

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read an integer from a named child element
	//
	// Parameters :
	//				e		: parent XML element
	//              name	: name of child element
	//              def		: default value in case of a missing element or an invalid string
	// --------------------------------------------------------------------------------------------------------
	bool boolFromElement(TiXmlElement *e, const string name, const bool def);
	bool boolFromElement(TiXmlElement *e, const string name);

	
	bool mapStringToEnum(const string s, const string *lookup, const int count, int *e);

	double erg(const double x);
	double dB(const double x);
	double dBsum(const double x, const double y);
	double dBsum(const double x, const double y, const double z);

	void angles_to_vector(const double horz, const double vert, double* x, double* y, double* z);
	void angles_to_vector(const double horz, const double vert, const double radius, double* x, double* y, double* z);
	void vector_to_angles(const double x, const double y, const double z, double* horz, double* vert, double* radius);

	double deg_to_rad(const double degrees);
	double rad_to_deg(const double radians);

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to write an error message to the error stream
	//
	// Parameters :
	//				message	: error message
	//              fn		: name of the file being processed when the error occurred. Defaults to empty.
	//              xmlNode	: the XML node being being processed when the error occurred. Defaults to NULL.
	// --------------------------------------------------------------------------------------------------------
	void report_error(const string message, string fn = "", const TiXmlNode *xmlNode = NULL);


}