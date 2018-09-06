#pragma once
#include <string>
using namespace std;

#ifndef WIN32
#define CNOSSOS_DLL_API extern "C"
#elif CNOSSOS_DLL_EXPORTS
#define CNOSSOS_DLL_API __declspec(dllexport) 
#else
#define CNOSSOS_DLL_API __declspec(dllimport) 
#endif



namespace CNOSSOS_RAILNOISE
{
	// Init the DLL
	CNOSSOS_DLL_API int InitDLL();

	// Release the DLL
	CNOSSOS_DLL_API int ReleaseDLL(void);

	// Calculate the source power given by the infile and the place the results in the outfile
	CNOSSOS_DLL_API int CalcFromFile(const string infile, const string outfile);

	// Get the current version of the Cnossos Railway Source module shared library.
	CNOSSOS_DLL_API char* GetVersionDLL (void);
}