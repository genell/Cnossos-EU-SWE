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

#ifdef CNOSSOS_DLL_EXPORTS
#define CNOSSOS_DLL_API __declspec(dllexport) 
#else
#define CNOSSOS_DLL_API __declspec(dllimport) 
#endif

#include <string>
using namespace std;

// -------------------------------------------------------------------------------------------------------------
// external interface using a C++ class containing static functions
// -------------------------------------------------------------------------------------------------------------

namespace CNOSSOS_ROADNOISE
{  
	// Init the DLL
	CNOSSOS_DLL_API int InitDLL();

	// Release the DLL
	CNOSSOS_DLL_API int ReleaseDLL(void);

	// Calculate the source power given by the infile and the place the results in the outfile
	CNOSSOS_DLL_API int CalcFromFile(const string infile, const string outfile);

	// Get the current version of the Cnossos Road Source module shared library.
	CNOSSOS_DLL_API char* GetVersionDLL (void);
}