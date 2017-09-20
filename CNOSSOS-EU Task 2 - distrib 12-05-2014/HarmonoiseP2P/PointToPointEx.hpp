/* 
 * ------------------------------------------------------------------------------------------------
 * file:		PointToPointExt.hpp
 * version:		2.022
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.CSTB.txt
 * description: calculation of excess attenuation according to the Harmonoise model
 * changes:
 *
 *	14/01/2014	this header added, all other copyright and licensing notices removed
 * ------------------------------------------------------------------------------------------------- 
 */
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
// Version : 2.016 
// Release : 05/01/2007
//
// ----------------------------------------------------------------------------------------------------- 

#ifndef  _PointToPointEx_Included
#define  _PointToPointEx_Included

// ----------------------------------------------------------------------------------------------------- 
// include standard C interface
// ----------------------------------------------------------------------------------------------------- 

#include "./PointToPoint.hpp"

// ----------------------------------------------------------------------------------------------------- 
// some usefull shortcuts
// ----------------------------------------------------------------------------------------------------- 

#define PI ((double) 3.14159265358979323)

inline double MIN (double x, double y) { if (x < y) return x ; return y ; } ;
inline double MAX (double x, double y) { if (x > y) return x ; return y ; } ;

inline int MIN (int x, int y) { if (x < y) return x ; return y ; } ;
inline int MAX (int x, int y) { if (x > y) return x ; return y ; } ;

inline double DEG (double x) { return x * 180 / PI ; } ;
inline double RAD (double x) { return x * PI / 180 ; } ;

inline double DIST (double x, double y)
{
    return sqrt (x * x + y * y) ;
} 

inline double DIST (double x1, double y1, double x2, double y2) 
{ 
    return DIST (x1-x2, y1-y2) ; 
} ;

inline double LOG10 (double x)
{
    return 10 * log10 (x) ; 
}

inline double LOG10 (Complex x)
{
    return 10 * log10 (norm(x)) ; 
}

inline double POW10 (double x)
{
    return pow (10, x/10) ; 
}

inline int SIGN (double x)
{
    if (x < 0) return -1 ;
    if (x > 0) return  1 ;
    return 0 ;
}

// ----------------------------------------------------------------------------------------------------
// definition of a propagation path in two dimensions
// 
// external programs wishing to commuicate directly with the C++ layer without using the C wrappers
// should use a datastructure to communicate between the main program and the specialized software
// module. This datastructure is fixed size and contains no special C++ features. The calling program 
// should fill in ALL the fields marked as input".Fiels marked as "output" are available on exit of 
// the calculation procedure.
//
// special topics :
//
// - all integer values are signed 32 bit values
// - all real values are floating point 64 (double precision) bit values
// - alignment of integer values on 4-byte frontiers
// - alignment of floating point values on 8-byte frontiers
//
// ----------------------------------------------------------------------------------------------------

// ground segment under the propagation path

struct Impedance 
{
    int     model ;                  // model or user defined   
    int     dummy ;                  // dummy (respect 8 bit alignment)
    double  sigma ;                  // flow resistivity in kRayls
    double  thickness ;              // layer thickness
    Complex Z [MAX_FREQ] ;           // complex impedance of the ground or obstacle  
} ;

struct UserSegment 
{
    double x, y ;                    // position in ray path coordinates
    int    ground ;                  // ground class
} ;

struct AttDetail
{
    int     pos_src ;                // left principal point
    int     pos_rec ;                // right principal point
    int     pos_dif ;                // diffraction point (if model = ATT_DIFFRACTION)
    int     model ;                  // calculation scheme (see ATT_* constants in PointToPoint.hpp
    double  att[MAX_FREQ] ;          // attenuation spectrum
} ;

struct Segment                       
{
    double  x1, y1 ;                 // position first point
    double  x2, y2 ;                 // position second point

    int     ground ;                 // ground class 
    double  roughness ;              // ground roughness (not used)

    int     index_s  ;               // index of diffraction point on the source side
    int     index_r  ;               // index of diffraction point on the receiver side 
    
    int     convex_s ;               // convex segment with hs < 0
    int     convex_r ;               // convex segment with hr < 0
    
    double  ds, hs ;                 // position of equivalent source in local coordinates 
    double  dr, hr ;                 // position of equivalent receiver in local coordinates
    double  d_segment ;              // lenght of the segment 

    double  d_direct ;               // length of direct ray 
    double  d_image ;                // lenght of reflected ray
    double  cos_teta ;               // cosine of angle of incidence    
    double  d_specular ;             // position of specular reflexion point in local coordinates

    double  d2t_dx2 ;
    
    int     convex_hull ;            // end point of segment is on the convex hull ray path
    int     second_hull ;            // end point of segment is on the inner (secondary) hull

    double  a_fresnel [MAX_FREQ] ;   // size of Fresnel ellipse (in the propagation plane)
    double  d_fresnel [MAX_FREQ] ;   // position of the center of the Fresnel ellipse
    double  w_fresnel [MAX_FREQ] ;   // Fresnel weight

    Complex Q [MAX_FREQ] ;           // spherical reflection coefficient
    Complex D [MAX_FREQ] ;           // distance / diffraction weighting
    
    double  C [MAX_FREQ] ;           // coherence coefficient
} ;

// debug details

struct DebugParam 
{
   const char* name ;
   double      value ;
} ;

// full propagation path

struct PropagationPath
{
    int         majorID ;                 // output : major version ID
    int         minorID ;                 // output : minor version ID
    
    int         nbFreq ;                  // input : number of frequencies used
    int         maxFreq ;                 // memory managment, fixed value
    double      freq[MAX_FREQ] ;          // input : array of frequency values
    double      band_width ;              // input : bandwidth of integration (in octaves)
    
    OPTIONS     options ;                 // input : options used in the calculation scheme
    
    double      hSource ;                 // input : source position
    double      hReceiver ;               // input : receiver position

    double      delta_hSource ;           // input : incertainty on source height
    double      delta_hReceiver ;         // input : incertainty on receiver height
    
    double      c_sound ;                 // input : sound speed (m/s)
    
    double      a_meteo ;                 // input : equivalent lineair sound speed gradient (1/s)
    double      b_meteo ;                 // input : equivalent logarithmic sound speed gradient (m/s)
    double      c_meteo ;                 // input : turbulence strenght (Kolmogorov structure parameter)
    double      d_meteo ;                 // input : displacement height 

    double      R_meteo ;                 // output : equivalent ray curvature

    double      centerFreq ;              // internal : transition frequency
    
    double      abs_air[MAX_FREQ] ;       // input : air absorption coefficients in dB/m
    
    int         nbUserSegment ;           // input : number of user defined segments
    int         maxUserSegment ;          // internal, memory management
    UserSegment userSegment[MAX_SEG] ;    // input : user defined segments
    double      expand_distance ;         // input : distance expansion factor 
    int         userSplitSegment ;        // input : refinement of segment subdivision

    int         nbImpedance ;             // input : number of impedance classes
    int         maxImpedance ;            // internal, memory management
    Impedance   impedance[MAX_IMPEDANCE]; // input : ground impedance models
    
    int         nbSeg ;                   // output : number of segments
    int         maxSeg ;                  // internal, memory management
    Segment     seg[MAX_SEG] ;            // output 

 // save calculation details
    
    int         nbDetails ;               // output : number of segments
    int         maxDetails ;              // internal memory management
    AttDetail   detail[8*MAX_SEG] ;       // output : partial and total excess attenuation

 // data exchange for debug
 
    DebugParam  p1 ;
    DebugParam  p2 ;
    DebugParam  p3 ;
    DebugParam  p4 ;
    DebugParam  p5 ;
    DebugParam  p6 ;
 
 // CPU timer inside DLL functions
 
    time_t  cpu_timer ;          

 // constructor & destructor
 
    PropagationPath() ;
    ~PropagationPath() ;
    
 // public member functions

    int  DoPointToPoint (void) ;
 
 // internal utilities
 
    void InitImpedances (void) ;
    void InitAirAbsorption (void) ;
    void SetupAirAbsorption (double temp, double hum) ;

    void CreateSegments (void) ;
    void DoCurvature (void) ;

    void GetVertex (int i, double &x, double &y) ;
    void FindConvexHull (int i1, int i2) ;
    void FindSecondHull (int i1, int i2) ;
    
    void CumulDiffraction (int is, int id, int ir) ;
    
    void LocalCoordinates (int is, int ir) ;    
    void FresnelParameters (int is, int ir, double centerFreq) ;
    void FresnelWeights (int is, int ir, double centerFreq) ;
    void InitialFresnelWeights (int is, int ir) ;
    void ModifiedFresnelWeights (int is, int ir) ;
    
    void GroundEffect (int is, int ir) ;
    void PartialGroundEffect (int is, int ir) ;
    
    void GetCoherence (int is, int ir) ;
    void GetReflexion (int is, int ir) ;
    void GetDiffraction (int is, int ir) ;
    
    void ExcessAttenuation (void) ;
    
    void MakeImage (double xo, double yo, int i, double &xi, double &yi) ;
    void SingleDiffraction (double xs, double ys, double xd, double yd, double xr, double yr, Complex *pfield) ;
    void DirectField (double xs, double ys, double xr, double yr, Complex *pfield) ;

    Complex Pdif_Deygout (double teta, double Rs, double Rr, double k) ;

} ;

#endif

