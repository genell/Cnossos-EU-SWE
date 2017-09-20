// CNOSSOS_INDUSTRIAL_NOISE_DLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CNOSSOS_INDUSTRIAL_NOISE_DLL.h"
#include "CNOSSOS_IND_DATA.h"
#include <iostream>

using namespace std;

namespace CNOSSOS_INDUSTRIAL_NOISE
{

	// --------------------------------------------------------------------------------------------------------
	string getDllPath()
	{
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
	}

	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Get the current version of the Cnossos Industrial Source module shared library.
	/// </summary>
	/// <returns>String encoded version of the shared library</returns>
	CNOSSOS_DLL_API char* GetVersionDLL (void)
	{
		return "1.00";
	};

	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Initializes the DLL.
	/// </summary>
	/// <returns>0 if all went well, -1 if something went wrong</returns>
	int InitDLL()
	{
		try {

			// Figure out this DLL's path
			string dllPath = getDllPath();
			dllPath = dllPath.substr(0, dllPath.find_last_of("\\/") + 1);

			// create and load catalogue
			catalogue = new IndustryCatalogue();
			if (!catalogue->loadFromXmlFile(dllPath + "cnossos_industry_catalogue.xml"))
				return -1;

			// initialize current source set
			currentSourceSet = new IndustrySourceSet(catalogue);
			return 0;
		}
		catch (...)
		{
			return -1;
		}
	};

	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Releases the DLL.
	/// </summary>
	/// <returns>0 if all went well, -1 if something went wrong</returns>
	int ReleaseDLL( void )
	{
		try
		{
			// destroy current source set
			delete currentSourceSet;
			// destroy catalogue
			delete catalogue;

			return 0;
		}
		catch (...)
		{
			return -1;
		}
	};

	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Calculates from file.
	/// </summary>
	/// <param name="infile">The input XML file.</param>
	/// <param name="outfile">The output XML file.</param>
	/// <returns>0 = OK, 1 = error while loading, 2 = error while calculating, 3 = error while saving, 4 = internal error</returns>
	int CalcFromFile(const string infile, const string outfile)
	{
		TCHAR buffer[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buffer);
		
		int result = 0;
		if (currentSourceSet == NULL)
		{
			return 4;
		}
		else if (currentSourceSet->loadFromXmlFile(infile))
		{
			result = currentSourceSet->Calculate();
			if (!currentSourceSet->saveResultsToXmlFile(outfile))
			{
				cerr << "Error while trying to save " << outfile << "." << endl;
				if (result == 0)
					result = 2;
			}

			if (currentSourceSet->doDebug)
			{
				string debugfile = outfile;
				debugfile = debugfile.substr(0, debugfile.find_last_of(".")) + ".csv";
				if (currentSourceSet->writeDebugData(debugfile))
				{
					cout << "Test file saved to " << debugfile << "." << endl;
				}
			}

			return result;
		}
		return 1;
	};

}
