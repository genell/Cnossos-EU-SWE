#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CNOSSOS_DLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CNOSSOS_DLL_EXPORTS functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifndef WIN32
#define CNOSSOS_DLL_API extern "C"
#elif CNOSSOS_DLL_EXPORTS
#define CNOSSOS_DLL_API __declspec(dllexport)
#else
#define CNOSSOS_DLL_API __declspec(dllimport)
#endif

#include <string>
using namespace std;

namespace CNOSSOS_INDUSTRIAL_NOISE
{
	// Init the DLL
	CNOSSOS_DLL_API int InitDLL();

	// Release the DLL
	CNOSSOS_DLL_API int ReleaseDLL(void);

	// Calculate the source power given by the infile and the place the results in the outfile
	CNOSSOS_DLL_API int CalcFromFile(const string infile, const string outfile);

	// Get the current version of the Cnossos Industrial Source module shared library.
	CNOSSOS_DLL_API char* GetVersionDLL (void);
}
