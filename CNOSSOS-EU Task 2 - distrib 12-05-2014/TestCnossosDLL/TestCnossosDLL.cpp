/* 
 * ------------------------------------------------------------------------------------------------
 * file:		TestCnossosDLL.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: test software demonstrating the use of the CnossosPropagation.dll library.
 *				CnossosPropagation defines a standard C-style API to the CNOSSOS-EU propagation
 *	            module. Most popular programming languages can be bound to pure C functions 
 *				defined in DLL shared libraries, either at linker level, either at run time.
 * changes:
 *
 *	03/12/2013	initial version
 *
 *	09/12/2013	this header added
 *
 *	09/12/2013	error handling added and tested
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "stdafx.h"
#include "CnossosPropagation.h"
#include <stdio.h>
#include <conio.h>
#include <limits>
/*
 *  utility function for printing out spectral values
 */
void print_spectrum (const char* id, const char* fmt, double* val)
{
	printf ("%8.8s", id) ;
	for (unsigned int i = 0 ; i < 8 ; ++i) 
	{
		printf ("  ") ;
		printf (fmt, val[i]) ;
	}
	printf ("\n") ;
}
/*
 * main entry point
 */
#ifdef __GNUC__
int main (int argc, char* argv[])
#else
int _tmain (int argc, _TCHAR* argv[])
#endif
{
	/*
	 * check version of API against DLL
	 */
	printf ("Testing CnossosPropagation.DLL library \n") ;
	printf (".version API: %s\n", CNOSSOS_P2P_VERSION_API) ;
	printf (".version DLL: %s\n", CNOSSOS_P2P_GetVersionDLL()) ;
	/*
	 * illustrate access to material properties
	 */
	CNOSSOS_P2P_MATERIAL* mat = CNOSSOS_P2P_GetMaterial ("A4", false) ;
	double G ;
	double sigma ;
	double alpha[8] ;
	double freq[8] ;

	CNOSSOS_P2P_GetFreq (freq) ;
	CNOSSOS_P2P_GetGValue (mat,G) ;
	CNOSSOS_P2P_GetSigma (mat, sigma) ;
	CNOSSOS_P2P_GetAlpha (mat, alpha) ;

	printf ("Material ID=""A4"", G=%.2f, sigma=%.0f\n", G, sigma) ;
	print_spectrum ("freq:", "%4.0f", freq) ;
	print_spectrum ("alpha:", "%4.2f", alpha) ;
	/*
	 * create a calculation method
	 */
	CNOSSOS_P2P_ENGINE* p2p = CNOSSOS_P2P_CreateEngine ("JRC-draft-2010") ;
	printf ("Calculation method: %s, version %s\n", CNOSSOS_P2P_GetMethod (p2p), CNOSSOS_P2P_GetVersion (p2p)) ;

#ifdef _PROCESS_XML_FILE_
	//
	// use CnossosPropagation.dll to process XML input file
	//
	const char* xmlFile = "../data/flat ground - 200m.xml" ;
	printf ("Process file %s \n", xmlFile) ;
	CNOSSOS_P2P_ProcessPathFile (p2p, xmlFile) ;
#else
	//
	// create a propagation path from the application
	//
	CNOSSOS_P2P_ClearPath (p2p) ;
	unsigned int nbPoints ;
	/*
		* create source, i.e. a segment of a line source with known edn-points 
		*/
	CNOSSOS_P2P_POSITION pos ;
	pos[0] = 0.0 ;
	pos[1] = 0.0 ;
	pos[2] = 0.0 ;
	CNOSSOS_P2P_POSITION seg[2] ;
	seg[0][0] = pos[0] ;
	seg[0][1] = pos[1] - 10 ;
	seg[0][2] = pos[2] ;
	seg[1][0] = pos[0] ;
	seg[1][1] = pos[1] + 10 ;
	seg[1][2] = pos[2] ;
	nbPoints = CNOSSOS_P2P_AddToPath (p2p, pos, CNOSSOS_P2P_GetMaterial("F"), CNOSSOS_P2P_CreateLineSource (0.5, seg)) ;
	/*
		* hard surface beneath the source
		*/
	pos[0] = 10.0 ;
	pos[1] =  2.0 ;
	nbPoints = CNOSSOS_P2P_AddToPath (p2p, pos, CNOSSOS_P2P_GetMaterial("F")) ;
	/*
		* soft ground till the receiver
		*/
	pos[0] = 50.0 ;
	pos[1] = 10.0 ;
	nbPoints = CNOSSOS_P2P_AddToPath (p2p, pos, CNOSSOS_P2P_GetMaterial("D"), CNOSSOS_P2P_CreateReceiver (2.5)) ;
	/*
		* define sound power associated with the source
 		*/
	double Lw[] = { 90, 95, 100, 105, 105, 100, 95, 90 } ;
	CNOSSOS_P2P_SetSoundPower (p2p, Lw) ;
	/*
		* setup options (optionally)
		*/
	CNOSSOS_P2P_OPTIONS options ;
	CNOSSOS_P2P_GetOptions (p2p, options) ;
	options.ExcludeSoundPower = false ;
	CNOSSOS_P2P_SetOptions (p2p, options) ;
	/*
		* setup meteorological weighting
		*/
	CNOSSOS_P2P_METEO meteo ;
	CNOSSOS_P2P_GetMeteo (p2p, meteo) ;
	meteo.model = CNOSSOS_P2P_METEO_JRC2012 ;
	meteo.C0 = 3.0 ;
	meteo.pFav = 0.50 ;
	CNOSSOS_P2P_SetMeteo (p2p, meteo) ;
#endif	
	/*
		* read out the results (the first read will trigger the actual calculation) 
		*/
	double fav[8] ;
	double hom[8] ;
	double leq[8] ;
	unsigned int ok_fav = CNOSSOS_P2P_GetResult (p2p, CNOSSOS_P2P_RESULT_LP_FAV, fav) ;
	unsigned int ok_hom = CNOSSOS_P2P_GetResult (p2p, CNOSSOS_P2P_RESULT_LP_HOM, hom) ;
	unsigned int ok_leq = CNOSSOS_P2P_GetResult (p2p, CNOSSOS_P2P_RESULT_LP_AVG, leq) ;
	/*
		* print out results
		*/
	if (ok_fav > 0 && ok_hom > 0 && ok_leq > 0)
	{
		CNOSSOS_P2P_PrintPathResults (p2p) ;
		printf ("Results:\n") ;
		print_spectrum ("freq:", "%5.0f", freq) ;

		print_spectrum ("LpF:", "%5.1f", fav) ;
		print_spectrum ("LpH:", "%5.1f", hom) ;
		print_spectrum ("Leq:", "%5.1f", leq) ;
	}
	else
	{
		printf ("ERROR: %s \n", CNOSSOS_P2P_GetErrorMessage(p2p)) ;
	}
	// performance counters
	unsigned int nbCalls ;
	double cpuTime ;
	CNOSSOS_P2P_GetPerformanceCounters (p2p, nbCalls, cpuTime) ;
	printf ("Finished: %d call(s) to propagation calculations in %.2f ms \n", nbCalls, 1000*cpuTime) ;
	/*
	 * cleanup
	 */
	CNOSSOS_P2P_DeleteEngine (p2p) ;
	//
	// game over
	//
	printf ("Type any key to quit...") ;
	_getch() ;
	return 0;
}

