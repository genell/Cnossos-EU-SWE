#include <iostream>
#include <sstream>
#include "CNOSSOS_AUX.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

namespace CNOSSOS
{


	// --------------------------------------------------------------------------------------------------------
	// Auxiliary functions to read a double from a string
	//
	// Parameters :
	//				s		: string containing a double
	//              def		: default value in case of an invalid string
	// --------------------------------------------------------------------------------------------------------
	double parseFloat(const string s, const double def)
	{
		double result = def;
		if (TIXML_SSCANF(s.c_str(), "%lf", &result) == 1)
			return result;
		else
			return def;
	}
	double parseFloat(const string s)
	{
		double result = 0.0;
		if (TIXML_SSCANF(s.c_str(), "%lf", &result) == 1)
			return result;
		else
			throw "Invalid float value '" + s + "'";
	}

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary functions to read an integer from a string
	//
	// Parameters :
	//				s		: string containing an integer
	//              def		: default value in case of an invalid string
	// --------------------------------------------------------------------------------------------------------
	int parseInt(const string s, const int def)
	{
		int result = def;
		if (TIXML_SSCANF(s.c_str(), "%d", &result) == 1)
			return result;
		else
			return def;
	}
	int parseInt(const string s)
	{
		int result = 0;
		if (TIXML_SSCANF(s.c_str(), "%d", &result) == 1)
			return result;
		else
			throw "Invalid integer value '" + s + "'";
	}

	// --------------------------------------------------------------------------------------------------------
	int copyFloatsFromString(const string s, double *f, const int count)
	{
		string token;
		istringstream iss(s);
		iss.imbue(locale("C", locale::numeric));
		int i = 0;
		while (iss >> token && i < count)
		{
			f[i] = parseFloat(token);
			i++;
		}
		return i;
	}

	// --------------------------------------------------------------------------------------------------------
	string stringFromFloats(const double *f, const int count, const char separator)
	{
		ostringstream oss;
		oss.imbue(locale("C", locale::numeric));
		for (int i = 0; i < count; i++)
		{
			if (i > 0)
			{
				oss << separator;
			}
			oss << f[i];
		}
		return oss.str();
	}

	// --------------------------------------------------------------------------------------------------------
	string stringFromFloat(double n)
	{
		ostringstream oss;
		oss.imbue(locale("C", LC_ALL));
		oss << n;
		return oss.str();
	}

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read the text from a named child element
	//
	// Parameters :
	//				e		: parent XML element
	//              name	: name of child element
	//              def		: default value in case of a missing element or an invalid string
	// --------------------------------------------------------------------------------------------------------
	string stringFromElement(TiXmlElement *e, const string name, const string def)
	{
		TiXmlElement *child = e->FirstChildElement(name.c_str());
		if (child != NULL)
			return child->GetText();
		else
			return def;
	}

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read a double from a named child element
	//
	// Parameters :
	//				e		: parent XML element
	//              name	: name of child element
	//              def		: default value in case of a missing element or an invalid string
	// --------------------------------------------------------------------------------------------------------
	double floatFromElement(TiXmlElement *e, const string name, const double def)
	{
		TiXmlElement *child = e->FirstChildElement(name.c_str());
		if (child != NULL)
			return parseFloat(child->GetText(), def);
		else
			return def;
	}

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read an integer from a named child element
	//
	// Parameters :
	//				e		: parent XML element
	//              name	: name of child element
	//              def		: default value in case of a missing element or an invalid string
	// --------------------------------------------------------------------------------------------------------
	int intFromElement(TiXmlElement *e, const string name, const int def)
	{
		TiXmlElement *child = e->FirstChildElement(name.c_str());
		if (child != NULL)
			return parseInt(child->GetText());
		else
			return def;
	}

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to read a bool value from a named child element
	//
	// Parameters :
	//				e		: parent XML element
	//              name	: name of child element
	//              def		: default value in case of a missing element or an invalid string
	// --------------------------------------------------------------------------------------------------------
	bool boolFromElement(TiXmlElement *e, const string name, const bool def)
	{
		string value = stringFromElement(e, name, "");
		if (value.compare("false") == 0 || value.compare("0") == 0) {
			return false;
		} else if (value.compare("true") == 0 || value.compare("1") == 0) {
			return true;
		} else {
			return def;
		}
	}
	// --------------------------------------------------------------------------------------------------------
	bool boolFromElement(TiXmlElement *e, const string name)
	{
		string value = stringFromElement(e, name, "");
		if (value.compare("0") == 0 || value.compare("false") == 0 || value.compare("no") == 0) {
			return false;
		} else if (value.compare("1") == 0 || value.compare("true") == 0 || value.compare("yes") == 0) {
			return true;
		} else {
			throw "Invalid boolean value '" + value + "'";
		}
	}

	// --------------------------------------------------------------------------------------------------------
	void angles_to_vector(const double horz, const double vert, double* x, double* y, double* z)
	{
		double t = horz;
		double p = M_PI_2 - vert;
		*x = sin(p) * cos(t);
		*y = sin(p) * sin(t);
		*z = cos(p);
	}
	// --------------------------------------------------------------------------------------------------------
	void angles_to_vector(const double horz, const double vert, const double radius, double* x, double* y, double* z)
	{
		angles_to_vector(horz, vert, x, y, z);
		*x *= radius;
		*y *= radius;
		*z *= radius;
	}
	// --------------------------------------------------------------------------------------------------------
	void vector_to_angles(const double x, const double y, const double z, double* horz, double* vert, double* radius)
	{
		*radius = sqrt(x*x + y*y + z*z);
		*horz = atan(y / x);
		*vert = M_PI_2 - acos(z / *radius);
	}

	// --------------------------------------------------------------------------------------------------------
	double deg_to_rad(const double degrees)
	{
		return degrees * M_PI / 180;
	}
	// --------------------------------------------------------------------------------------------------------
	double rad_to_deg(const double radians)
	{
		return radians * 180 / M_PI;
	}

	// --------------------------------------------------------------------------------------------------------
	// Auxiliary function to write an error message to the error stream
	//
	// Parameters :
	//				message	: error message
	//              fn		: name of the file being processed when the error occurred. Defaults to empty.
	//              xmlNode	: the XML node being being processed when the error occurred. Defaults to NULL.
	// --------------------------------------------------------------------------------------------------------
	void report_error(const string message, string fn, const TiXmlNode *xmlNode)
	{
		cerr << message;
		if (fn.empty() && xmlNode != NULL)
		{
			fn = xmlNode->GetDocument()->Value();
		}
		if (!fn.empty())
		{
			cerr << " (" << fn;
			if (xmlNode != NULL)
			{
				cerr << "; line " << xmlNode->Row() << ", column " << xmlNode->Column();
			}
			cerr << ")";
		}
		else if (xmlNode != NULL)
		{
			cerr << " (line " << xmlNode->Row() << ", column" << xmlNode->Column() << ")";
		}
		cerr << endl;
	}

	// --------------------------------------------------------------------------------------------------------
	bool mapStringToEnum( const string s, const string *lookup, const int count, int *e )
	{
		for (int i = 0; i < count; i++)
		{
			if (s.compare(lookup[i]) == 0)
			{
				*e = i;
				return true;
			}
		}
		return false;
	}

	// --------------------------------------------------------------------------------------------------------
	double erg(const double x)
	{
		return pow(10, x / 10);
	}
	// --------------------------------------------------------------------------------------------------------
	double dB(const double x)
	{
		return 10 * log10(x);
	}
	// --------------------------------------------------------------------------------------------------------
	double dBsum(const double x, const double y)
	{
		return dB(erg(x) + erg(y));
	}
	// --------------------------------------------------------------------------------------------------------
	double dBsum(const double x, const double y, const double z)
	{
		return dB(erg(x) + erg(y) + erg(z));
	}



}
