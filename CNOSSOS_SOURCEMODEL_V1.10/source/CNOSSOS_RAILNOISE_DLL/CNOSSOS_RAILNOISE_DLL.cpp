#pragma once

#include "CNOSSOS_RAILNOISE_DLL_DATA.h"
#include "CNOSSOS_RAILNOISE_DLL.h"
#include "CNOSSOS_RAILNOISE_DLL_AUX.h"
#include "../tinyxml/tinyxml.h"
#include <stdexcept>
#include <exception>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>

using namespace std;

namespace CNOSSOS_RAILNOISE
{
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
		return "./";
		#endif
	}

	RailCatalogue *catalogue;
	RailSection *currentSection;

	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Get the current version of the Cnossos Railway Source module shared library.
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
			catalogue = new RailCatalogue();
			if (!catalogue->load_from_xml_files(dllPath + "CNOSSOS_Rail_Track.xml", dllPath + "CNOSSOS_Rail_Vehicles.xml"))
				return -1;

			// initialize current source set
			currentSection = new RailSection(catalogue);
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
	int ReleaseDLL( void ) {
		try {
			if (currentSection != NULL)
				delete currentSection;
			delete catalogue;

			return 0;
		}
		catch (...) {
			return -1;
		}
	}

	// --------------------------------------------------------------------------------------------------------
	/// <summary>
	/// Calculates from file.
	/// </summary>
	/// <param name="infile">The input XML file.</param>
	/// <param name="outfile">The output XML file.</param>
	/// <returns>0 = OK, 1 = error while loading, 2 = error while calculating, 3 = error while saving, 4 = internal error</returns>
	int CalcFromFile(const string infile, const string outfile)
	{
		int result = 0;
		if (currentSection == NULL)
		{
			return 4;
		}
		else if (currentSection->load_from_xml_file(infile))
		{
			if (!currentSection->calculate()) {
				cerr << "Error while calculating " << outfile << "." << endl;
				result = 2;
			}
				
			if (!currentSection->save_results_to_xml_file(outfile))
			{
				cerr << "Error while trying to save " << outfile << "." << endl;
				if (result == 0)
					result = 2;
			}

			if (currentSection->doDebug)
			{
				string debugfile = outfile;
				debugfile = debugfile.substr(0, debugfile.find_last_of(".")) + ".csv";
				if (currentSection->writeDebugData(debugfile))
				{
					cout << "Test file saved to " << debugfile << "." << endl;
				}
			}

			return result;
		}
		return 1;
	}

}