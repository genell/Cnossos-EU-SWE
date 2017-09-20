/* 
 * ------------------------------------------------------------------------------------------------
 * file:		PointToPoint.hpp
 * version:		2.022
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.CSTB.txt
 * description: calculation of excess attenuation according to the Harmonoise model
 * changes:
 *
 *	14/01/2014	this header added, all other copyright and licensing notices removed
 * ------------------------------------------------------------------------------------------------- 
 */
#ifndef _PointToPoint_Included
#define _PointToPoint_Included
// ----------------------------------------------------------------------------------------------------- 
//
// project : PointToPoint.dll
// author  : Dirk Van Maercke
// company : CSTB
//
// Calculation of excess attenuation over complex ground profiles in presence of meteorological effects
//
// This work has been carried out as part of the Harmonoise project, funded by the European Commission 
// under the "Information Society Technology" program, reference IST-2000-28149.Further development has 
// been carried out as part of the Imagine project, SSPI-CT-2003-503549		
// 
// Version : 2.022
// Release : 30/05/2012
//
// ----------------------------------------------------------------------------------------------------- 
//
// special topics for interfacing with other programming languages :
//
// - all integer values are signed 32 bit values
// - all real values are floating point 64 (double precision) bit values
// - alignment of integer values on 4-byte frontiers
// - alignment of floating point values on 8-byte frontiers
//
// ----------------------------------------------------------------------------------------------------- 

#define MAJOR_ID    2
#define MINOR_ID   22

// ----------------------------------------------------------------------------------------------------- 
// the DLL has been built using the WATCOM C++ Complex class 
//
// for compatibility, use :
//
//      Complex
//      {
//          double real_part ;
//          double imag_part ;
//      } ;
//
// and remove the "complex.h" include below
//
// ----------------------------------------------------------------------------------------------------- 
//
// ADDED in version 2.018 
// 
// most modern compilers, line MSVC, support the following STL based definition of Complex
//
// #include "complex"
// typedef std::complex<double> Complex
//
// ----------------------------------------------------------------------------------------------------- 

#ifdef __WATCOMC__
   #include "math.h"
   #include "complex.h"
#else
  #include <complex>
  typedef std::complex<double> Complex ;
#endif

// limitation of datastructure sizes 
// temporaray fix, final version should use dynamic memory allocation

#define  MAX_SEG                1001   // maximum number of ground segments  
#define  MAX_FREQ                 30   // maximum number of frequency bands
#define  MAX_IMPEDANCE           100   // maximum number of default + user defined impedances

// store options in an unisnged 32 bit integer mask

#define OPTIONS unsigned int 

// options values

#define  invRayTracing          0x00000001   // enable inverse ray tracing method
#define  enableAveraging        0x00000002   // enable coherency effect
#define  enableAirAbsorption    0x00000004   // enable air absorption model
#define  enableScattering       0x00000008   // enbale scattered energy
#define  difHaddenPierce        0x00000010   // use the Hadden-Pierce diffraction model
#define  randomTerrain          0x00000020   // randomize terrain profile (for fine-tuning only)

// index values for impedances models (classification as indicated in report HAR32TR-030715-DGMR.DOC)

#define  groundDefault            0          // unless otherwise specified sigma = 500
#define  groundSigma_12           1          // class 1 : very soft, snow or moss
#define  groundSigma_32           2          // class 2 : soft forest floor
#define  groundSigma_80           3          // class 3 : soft, uncompacted loose ground 
#define  groundSigma_200          4          // class 4 : normal uncompacted ground
#define  groundSigma_500          5          // class 5 : normal compacted ground 
#define  groundSigma_2000         6          // class 6 : hard compacted ground, parking lots,..
#define  groundSigma_20000        7          // class 7 : hard surface, asphalt, concrete, water,...
#define  groundSigma_200000       8          // class 8 : very hard and very flat surfce

#define  groundUserDefined       10          // user defined impedance classes start here...

// equivalent ground classes

#define groundClass_A		groundSigma_12
#define groundClass_B		groundSigma_32
#define groundClass_C		groundSigma_80
#define groundClass_D		groundSigma_200
#define groundClass_E		groundSigma_500
#define groundClass_F		groundSigma_2000
#define groundClass_G		groundSigma_20000
#define groundClass_H		groundSigma_200000

// equivalent absorption classes

#define absorptionClass_A0	groundSigma_20000
#define absorptionClass_A1	groundSigma_2000
#define absorptionClass_A2	groundSigma_200
#define absorptionClass_A3	groundSigma_80
#define absorptionClass_A4	groundSigma_32

// access to partial results

#define ATT_EXCESS_GLOBAL         0
#define ATT_DIFFRACTION           1
#define ATT_GROUND                2
#define ATT_GROUND_LIN            3
#define ATT_GROUND_LOG            4
#define ATT_TRANS_LIN_LOG         5
#define ATT_DIFFRACTION_BLOS      6
#define ATT_GROUND_BLOS           7
#define ATT_DIFF_GROUND_BLOS      8
#define ATT_GROUND_FLAT           9
#define ATT_TRANS_BLOS_FLAT      10

// modifiers required in order to generate standard windows calling conventions

#define  __EXPORTTYPE   extern "C" __declspec(dllexport)
#define  __CALLTYPE    

// -------------------------------------------------------------------------------------------------------------
// external interface using standard C functions
// -------------------------------------------------------------------------------------------------------------

// create and destroy the internal data

__EXPORTTYPE void*    __CALLTYPE  P2P_Create (void) ;
__EXPORTTYPE void     __CALLTYPE  P2P_Delete (void* p2p_struct) ;

// get current version of the DLL file

__EXPORTTYPE double   __CALLTYPE  P2P_GetVersionDLL (void* p2p_struct) ;

// setup frequency range

__EXPORTTYPE int      __CALLTYPE  P2P_GetNbFreq (void* p2p_struct) ;

__EXPORTTYPE int      __CALLTYPE  P2P_SetFreqArray (void* p2p_struct, int nfreq, double *freq) ;
__EXPORTTYPE int      __CALLTYPE  P2P_GetFreqArray (void* p2p_struct, double *freq) ;

__EXPORTTYPE double   __CALLTYPE  P2P_GetFreq (void* p2p_struct, int index) ;

// set/get bandwidth (parameter is expressed in octaves, e.g. user should enter 1, 1/3, 1/12,...) 

__EXPORTTYPE int      __CALLTYPE  P2P_SetBandwidth (void* p2p_struct, double bw) ;
__EXPORTTYPE double   __CALLTYPE  P2P_GetBandwidth (void* p2p_struct) ;

// setup impedances

__EXPORTTYPE int      __CALLTYPE  P2P_SetImpedanceDB (void* p2p_struct, int index, double sigma, double thickness) ;
__EXPORTTYPE int      __CALLTYPE  P2P_SetImpedance (void* p2p_struct, int index, Complex *imp) ;
__EXPORTTYPE int      __CALLTYPE  P2P_SetImpedanceRI (void* p2p_struct, int index, double *imp) ;

__EXPORTTYPE int      __CALLTYPE  P2P_GetImpedanceDB (void* p2p_struct, int index, double *sigma, double *thickness) ;
__EXPORTTYPE int      __CALLTYPE  P2P_GetImpedance (void* p2p_struct, int index, Complex *imp) ;
__EXPORTTYPE int      __CALLTYPE  P2P_GetImpedanceRI (void* p2p_struct, int index, double *imp) ;

// setup air absorption

__EXPORTTYPE int      __CALLTYPE  P2P_SetAirAbsorption (void* p2p_struct, double* abs_air) ;
__EXPORTTYPE int      __CALLTYPE  P2P_GetAirAbsorption (void* p2p_struct, double* abs_air) ;
__EXPORTTYPE int      __CALLTYPE  P2P_SetupAirAbsorption (void* p2p_struct, double temp, double hum) ;

// setup segments

__EXPORTTYPE int      __CALLTYPE  P2P_Clear (void* p2p_struct) ;
__EXPORTTYPE int      __CALLTYPE  P2P_AddSegment (void* p2p_struct, double x, double y, int impedance_id) ;

__EXPORTTYPE int      __CALLTYPE  P2P_SetDistanceFactor (void* p2p_struct, double factor) ;
__EXPORTTYPE double   __CALLTYPE  P2P_GetDistanceFactor (void* p2p_struct) ;

__EXPORTTYPE int      __CALLTYPE  P2P_SetSegmentSplitFactor (void* p2p_struct, int factor) ;
__EXPORTTYPE int      __CALLTYPE  P2P_GetSegmentSplitFactor (void* p2p_struct) ;

// setup source and receiver height

__EXPORTTYPE int      __CALLTYPE  P2P_SetSourceHeight (void* p2p_struct, double hs, double delta_hs) ;
__EXPORTTYPE double   __CALLTYPE  P2P_GetSourceHeight (void* p2p_struct) ;

__EXPORTTYPE int      __CALLTYPE  P2P_SetReceiverHeight (void* p2p_struct, double hr, double delta_hr) ;
__EXPORTTYPE double   __CALLTYPE  P2P_GetReceiverHeight (void* p2p_struct) ;

// setup calculation parameters

__EXPORTTYPE OPTIONS  __CALLTYPE  P2P_SetOptions (void* p2p_struct, OPTIONS select_options) ;
__EXPORTTYPE OPTIONS  __CALLTYPE  P2P_GetOptions (void* p2p_struct) ;

// setup sound speed profile

__EXPORTTYPE int      __CALLTYPE  P2P_SetSoundSpeed (void* p2p_struct, double c0) ;
__EXPORTTYPE double   __CALLTYPE  P2P_GetSoundSpeed (void* p2p_struct) ;

__EXPORTTYPE int      __CALLTYPE  P2P_SetSoundSpeedProfile (void* p2p_struct, double  A, double  B, double  C, double  D) ;
__EXPORTTYPE int      __CALLTYPE  P2P_GetSoundSpeedProfile (void* p2p_struct, double *A, double *B, double *C, double *D) ;

__EXPORTTYPE double   __CALLTYPE  P2P_SetRayCurvature (void* p2p_struct, double invR) ;
__EXPORTTYPE double   __CALLTYPE  P2P_GetRayCurvature (void* p2p_struct) ;

// setup sound speed profile from meteorological data

__EXPORTTYPE int      __CALLTYPE  P2P_SetupMeteoParameters (void* p2p_struct, double windSpeed, double cosWind, int stabilityClass) ;

// get the results

__EXPORTTYPE int      __CALLTYPE  P2P_GetResults (void* p2p_struct, double *att) ;

// get averaged results over N meteorological conditions
// note : after calling this function,GetDetails may be used to access the results for each condition

__EXPORTTYPE int      __CALLTYPE  P2P_GetAveragedResults (void* p2p_struct, int nb_cond, double *rd, double *wd, double *att) ;

// access to calculation details

__EXPORTTYPE int      __CALLTYPE P2P_GetNbDetails (void *p2P_struct) ;
__EXPORTTYPE int      __CALLTYPE P2P_GetDetails (void *p2P_struct, int index, int *model, int *pos_src, int *pos_dif, int *pos_rec, double *att) ;

// auxiliary functions

__EXPORTTYPE void     __CALLTYPE  AUX_RandomGauss (int n, double *val) ;

__EXPORTTYPE double   __CALLTYPE  P2P_GetTimerCPU (void* p2p_struct, bool reset_clock) ;

#endif


