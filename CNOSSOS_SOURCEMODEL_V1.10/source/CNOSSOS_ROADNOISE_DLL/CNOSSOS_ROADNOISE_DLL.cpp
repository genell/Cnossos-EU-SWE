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

#include "CNOSSOS_ROADNOISE_DLL_DATA.h"
#include "CNOSSOS_ROADNOISE_DLL.h"
#include "../tinyxml/tinyxml.h"
#include <stdexcept>
#include <stdlib.h>
#include <exception>
#include <iostream>
#include <sstream>
#include <fstream>


using namespace std;

namespace CNOSSOS_ROADNOISE
{


	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Get the current version of the Cnossos Road Source module shared library.
	/// </summary>
	/// <returns>String encoded version of the shared library</returns>
	CNOSSOS_DLL_API char* GetVersionDLL (void)
	{
		return "1.00";
	};

	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Release the dll
	///
	/// Frees the created segment.
	/// </summary>
	/// <returns>0 if all went well, -1 if something went wrong</returns>
	int ReleaseDLL( void )
	{
		try
		{
			if (currentSegment != NULL)
				delete currentSegment;

			return 0;
		}
		catch (...)
		{
			return -1;
		}

	}

	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Calculates the current roadsegment
	/// </summary>
	/// <returns>0 if all is OK, an error code if something went wrong.</returns>
	// --------------------------------------------------------------------------------------------------------

	int CalcSegment(void)
	{
		if (currentSegment != NULL)
			return currentSegment->CalcSegment();
		else
			return -1;
	}


	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Set the properties for the temperature correection
	/// </summary>
	/// <param name='temperature'>The average yearly air temperature, in �C</param>
	// --------------------------------------------------------------------------------------------------------
	void SetTemperatureProperties(const double temperature)
	{
		if (currentSegment != NULL)
		{
			currentSegment->TempProp = new TempProperties(temperature);
		}
	}


	// --------------------------------------------------------------------------------------------------------
	// Set the properties for the gradient correection
	//
	// Parameter :
	//				gradient	: The gradient of the current roadsegment in %		
	// 
	// --------------------------------------------------------------------------------------------------------
	void SetGradientProperties(const double gradient)
	{
		if (currentSegment != NULL)
		{
			currentSegment->GradProp = new GradientProperties(gradient);
		}
	}

	// --------------------------------------------------------------------------------------------------------
	// Set the properties for the acceleration correction
	//
	// Parameter :
	//				distance	: distance in m to the junction		
	//				junctionType : 1 = crossing with traffic ligths, 2 = roundabout
	// 
	// --------------------------------------------------------------------------------------------------------
	void SetAccelerationProperties(const double distance, const int junctionType)
	{
		if (currentSegment != NULL)
		{
			currentSegment->AccProp = new AccelerationProperties(distance, junctionType);
			
		}
	}

	// --------------------------------------------------------------------------------------------------------
	// Set the properties for the studded tyre correction
	//
	// Parameter :
	//				months		: amount of months a year studded tyres are used
	// 
	// --------------------------------------------------------------------------------------------------------
	void SetStuddedMonths(const int months)
	{
		if (currentSegment != NULL)
		{
			currentSegment->studdedMonths = months;
		}
	}

	// --------------------------------------------------------------------------------------------------------
	// Set the the speed for a specific category of vehicles
	//
	// Parameter :
	//				cat			: the category of vehicles ( 0 .. 5)
	//				speed		: the speed of the category of vehicles
	// 
	// --------------------------------------------------------------------------------------------------------
	void SetSpeed(const int cat, const double speed)
	{
		if (currentSegment != NULL)
		{
			currentSegment->v[cat] = speed;
		}
	}


	// --------------------------------------------------------------------------------------------------------
	// Set the the amount for a specific category of vehicles
	//
	// Parameter :
	//				cat			: the category of vehicles ( 0 .. 5)
	//				amount		: the amount of vehicles of category "cat" on the roadsegment 
	// 
	// --------------------------------------------------------------------------------------------------------
	void SetTraffic(const int cat, const double amount)
	{
		if (currentSegment != NULL)
		{
			currentSegment->Q[cat] = amount;
		}
	}




	// --------------------------------------------------------------------------------------------------------
	string getDllPath()
	{
#ifdef WIN32
		char path[MAX_PATH];
		HMODULE hm = NULL;
		if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
				GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCSTR) &getDllPath, &hm))
		{
			int ret = GetLastError();
			fprintf(stderr, "GetModuleHandle returned %d\n", ret);
		}
		GetModuleFileNameA(hm, path, sizeof(path));	
		return path;
#else
		// Non-windows just use current directory for now
		return "./";
#endif
	}
	
	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Creates a new road segment and read the required settings and data files
	/// </summary>
	/// <returns>0 if finished correctly, -1 otherwise</returns>
	// --------------------------------------------------------------------------------------------------------
    int InitDLL()
    {
		try
		{
			catalog = new RoadNoiseCatalog();
			
			string dllPath = getDllPath();
			dllPath = dllPath.substr(0, dllPath.find_last_of("\\/") + 1);
			catalog->LoadRoadParamsFromFile(dllPath + "cnossos_road_params.xml");
			catalog->LoadRoadSurfacesFromFile(dllPath + "cnossos_road_surfaces.xml");

			currentSegment = new RoadSegment(catalog);
		}
		catch (...)
		{
			return -1;
		}
		return 0;        
    }

	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Reads a xml input file, calculates the roadsegment source power and outputs the results the an xml output file
	/// </summary>
	/// <param name='infile'>filename of the xml input file</param>
	/// <param name='outfile'>filename of the xml output file</param>
	/// <returns>0 = OK, 1 = error while loading, 2 = error while calculating, 3 = error while saving, 4 = internal error</returns>
	// --------------------------------------------------------------------------------------------------------
	int CalcFromFile(const string infile, const string outfile)
	{
		int result = 0; // 0 = OK, 1 = error while loading, 2 = error while calculating, 3 = error while saving, 4 = internal error
		if (currentSegment == NULL)
		{
			return 4;
		}
		else if (currentSegment->loadFromXMLFile(infile))
		{
			result = currentSegment->CalcSegment();
			if (!currentSegment->saveResultsToXMLFile(outfile))
			{
				cerr << "Error while trying to save " << outfile << endl;
				if (result == 0)
					result = 2;
			}

			if (currentSegment->doDebug)
			{
				string debugfile = outfile;
				debugfile = debugfile.substr(0, debugfile.find_last_of(".")) + ".csv";
				if (currentSegment->writeDebugData(debugfile))
				{
					cout << "Test file saved to " << debugfile << "." << endl;
				}
				
			}
			
			return result;
		}
		return 1;
	}

	// --------------------------------------------------------------------------------------------------------
	void WriteToXMLFile(const string fn)
	{
		if (currentSegment != NULL)
			currentSegment->saveResultsToXMLFile(fn);
	}

}