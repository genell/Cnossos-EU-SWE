/* 
 * ------------------------------------------------------------------------------------------------
 * file:		PointToPoint.cpp
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

#ifdef CPP_NEW_DELETE
#include "SystemMemory.h"
#else
#include <new>
#endif

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "PointToPointEx.hpp"


// minimum value for source & receiver heights

// modif DVM 28.11.2008 : no espilon on heights...
// as a consequence : we must test for division by zero whenever (hS + hR) = 0...

const double HSR_MIN = 0 ;

const double DIST_MIN = 0.01 ;

// ---------------------------------------------------------------------------------------------------------
// 
// Calculate parameters of Fresnel ellipse in 2D
//
// input :
//
//      dd = distance in the plane between source's and receiver's foot-points
//      z1 = height of the source above the plane
//      z2 = height of the receiver above the plane
//      fn = extend of the Fresnel ellipse ;
//           this value is ADDED to the distance between receiver and image source !
// output :
//
//      a  = great axis of the ellipse (parallel to the line from source's to receiver's foor-point)
//      b  = small axis of the ellipse (perpendicular to the line from source's to receiver's foor-point)
//      c  = relative position of the center of the ellipse on the source-receiver line
//
// to calculate the position of the ellipse use :
//
//      d1 = c * dd    = distance between center of the ellipse and the source's foot-point
//      d2 = (1-c) * d = distance between center of the ellipse and the receiver's foot-point
//
// works fine even for d = 0, z1 or z2 = 0 ; do not use with negative input values.
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// ---------------------------------------------------------------------------------------------------------

void FresnelEllipse (double dd, double z1, double z2, double fn, 
                     double &a, double &b, double &c)
{
    double dz = z1 + z2 ;
    double df = sqrt(dd * dd + dz * dz) + fn ;

    double aa = (z1 * z1 - z2 * z2) / (df * df - dd * dd) ;

    double d1 = dd * (1 + aa) / 2 ;
    double d2 = dd * (1 - aa) / 2 ;

    double c1 = df * df ;
    double c2 = z1 * z1 + d1 * d1 ;
    double c3 = z2 * z2 + d2 * d2 ;
    double c4 = c2 - c3 ;
    double c5 = c2 + c3 ;
    double c6 = c1 * c1 + c4 * c4 - 2 * c1 * c5;
    double c7 = c1 - dd * dd ;

    if (c6 > 0)
    {
       a = 0.5 * sqrt (c6 / c7) ;
       b = 0.5 * sqrt (c6 / c1) ;
    }
    else
    {
       a = b = 0 ; 
    }
    c = (1 + aa) / 2 ;
}

// ----------------------------------------------------------------------------------------------------- 
// Impedance model from Delany and Bazley (sigma is given in kRayls)
//
// returns the normalized impedance using exp(+jkr) convention for spatial wave dependence
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// ----------------------------------------------------------------------------------------------------- 

Complex impedance_D_and_B (double sigma, double d, double freq, double c = 340.)
{
    double s = log (freq / sigma) ;
    double x = 1. + 9.08 * exp (-.75 * s) ; 
    double y = 11.9 * exp (-0.73 * s) ;     

    Complex Z(x,y) ;

 // finite thickness layers (for numerical reasons restricted to sigma < 1.E+6 Rayls)
 
    if (d > 0 && sigma < 1000)
    {
        s = 1000 * sigma/freq ;
        Complex k = 2 * PI * freq / c * Complex (1 + 0.0858 * pow(s, 0.70), 0.175 * pow(s, 0.59)) ;
        Complex j (0,1) ;
        
        Z = Z / tanh (-j * k * d) ;
    }

    return Z ;
}

// -----------------------------------------------------------------------------------------------------
// Auxiliary Fresnel integrals
//
// Reference : Nord 2000 report AV 1849/00 Annex F
// -----------------------------------------------------------------------------------------------------

double FresnelAuxF (double arg)
{
    static double a[] =
    {
         0.49997531354311, 
         0.00185249867385, 
        -0.80731059547652, 
         1.15348730691625,
        -0.89550049255859,
         0.44933436012454,
        -0.15130803310630,
         0.03357197760359,
        -0.00447236493671,
         0.00023357512010,
         0.00002262763737,
        -0.00000418231569,
         0.00000019048125
    } ;

    double x = fabs(arg) ;
    double y = 0 ;
    if (x > 5)
    {
        y = (1 / PI) / x ;
    }
    else
    {
        y = a[12] ;
        for (int i = 11 ; i >= 0 ; i--)
        {
            y = y * x + a[i] ;
        }
    }
    return (arg < 0) ? -y : y ;
}

double FresnelAuxG (double arg)
{
    static double a[] =
    {
        0.50002414586702,
       -1.00151717179967,
        0.80070190014386,
       -0.06004025873978,
       -0.50298686904881,
        0.55984929401694,
       -0.33675804584105,
        0.13198388204736,
       -0.03513592318103,
        0.00631958394266,
       -0.00073624261723,
        0.00005018358067,
       -0.00000151974284
    } ;

    double x = fabs(arg) ;
    double y = 0 ;
    if (x > 5)
    {
        y = (1 / PI) / x ;
        y = y * y / x ;
    }
    else
    {
        y = a[12] ;
        for (int i = 11 ; i >= 0 ; i--)
        {
            y = y * x + a[i] ;
        }
    }
    return (arg < 0) ? -y : y ;
}

Complex FresnelAux (double x)
{
    return Complex (FresnelAuxF(x), -FresnelAuxG(x)) ;
}

// ----------------------------------------------------------------------------------------------------
// Fref : correction factor for spherical reflection coefficient 
//
// Reference : Chien & Soroko, JSV, 1980
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// ----------------------------------------------------------------------------------------------------

Complex Fref (Complex z)
{
   Complex  j(0,1) ;
   Complex  erf ;

// first we calculate w(z) = exp(-z^2) * erfc(-j*z) for (x > 0, y > 0)

   double  x  = z.real() ;
   double  y  = z.imag() ;

// approximations for large values of the arguments 

   if (fabs(x) > 3.9 || fabs(y) > 3)
   {
      Complex z1 = Complex (fabs(x), fabs(y));
      Complex z2 = z1 * z1 ; 
      
      if ( fabs(x) > 6. || fabs(y) > 6. )
      {
          erf = j * z1 * (0.5124242 / (z2 - 0.2752551) + 0.05176536 / (z2 - 2.724745)) ;
      } 
      else   
      {
          erf = j * z1 * (0.461313500 / (z2 - 0.1901635) 
                        + 0.099992160 / (z2 - 1.7844927) 
                        + 0.002883894 / (z2 - 5.5253437)) ;
      }
      
   // careful about the signs here !   

      if (x < 0) erf = conj(erf) ;
      if (y < 0) erf = 2 * exp(y*y-x*x) * Complex(cos(2*x*y), -sin(2*x*y)) - conj(erf) ;
   }
   
// series development for small values of x and y

   else
   {
      double h  = 0.8;
      double a1 = cos (2*x*y);
      double b1 = sin (2*x*y);
      double c1 = exp (-2*y*PI/h) - cos(2*x*PI/h);
      double d1 = sin (2*x*PI/h);
      double cd = c1*c1 + d1*d1;
      double p2,q2 ;

      if (cd == 0)
      {
         p2 = q2 = 1;
      }
      else
      {
         p2 = 2 * exp (-1*(x*x+2*y*PI/h-y*y))*(a1*c1-b1*d1) / cd;
         q2 = 2 * exp (-1*(x*x+2*y*PI/h-y*y))*(a1*d1+b1*c1) / cd;
      }

      double eh = 0.000001 ;
      double h1 = 0 ;
      double h2 = 0 ;
      for (int n = 1 ; n <= 5 ; n++)
      {
         double x1 = (y*y+x*x+n*n*h*h);
         double x2 = (y*y-x*x+n*n*h*h);
         double x3 = (y*y+x*x-n*n*h*h);
         h1 += exp(-1*(n*n)*h*h)*x1/(x2*x2+4*y*y*x*x);
         h2 += exp(-1*(n*n)*h*h)*x3/(x2*x2+4*y*y*x*x);
      }

      double hyx = h*y / (PI*(x*x+y*y)) + 2*y*h*h1/PI - y*eh/PI;
      double kyx = h*x / (PI*(x*x+y*y)) + 2*x*h*h2/PI + x*eh/PI;

      if (y < PI/h)
      {
         hyx = hyx + p2;
         kyx = kyx - q2;
      }

      if (y == PI/h)
      {
         hyx = hyx + 0.5 * p2;
         kyx = kyx - 0.5 * q2;
      }

      erf = Complex (hyx, kyx) ;
   }

// now return Fref(w) = 1 + j * sqrt(PI) * z * [exp(-z^2) * erfc(-j*z)]

   Complex ret_val = 1. + j * sqrt(PI) * z * erf ;

   return ret_val ;
}

// ----------------------------------------------------------------------------------------------------- 
// Calculation of spherical reflection coefficient Q (Z, teta, freq, dist)
//
// Reference : Chien & Soroko, JSV, 1980
//
// modif DVM 08.11.2006 : correction of F for almost grazing incidence
//
// corrected ground effects for surface waves based on work by J.Defrance (thesis): in case 
// of grazing incidence (as in case of wide barriers) the ground attenuation for the surface
// wase is reduced to att_sw = 6 + 0.3 * (att - 6). In this case, the Chien-Soroak predicts 
// R = -1 and att = 20 log |F(w)|. 
//
// Applying the heuristic correction we get att_sw = 6 log |F(w)|. In order to get a smooth 
// transition we replace F(w) by pow (F(w), N) where N is a transition function that equals 
// 0.3 when both the source and the receiver are on the surface and 1 if at least one is large 
// compared to the wavelenght.

// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// ----------------------------------------------------------------------------------------------------- 

Complex Qref (Complex Z, double cos_teta, double kr, double N)
{
   Complex  j(0,1) ;

// plane wave reflection coefficient   

   Complex Rp = (Z * cos_teta - 1.) / (Z * cos_teta + 1.) ;
   
// correction for spherical reflection, parameter w = (1+j)/2 * sqrt(kr) * (cos(teta)+1./z)

   Complex w = (j+1.)/2. * sqrt(kr) * (cos_teta + 1./Z) ;
   
// spherical reflection coefficient

   Complex Qc = Rp + (1. - Rp) * pow (Fref(w), N) ;

   return Qc ;
}

// --------------------------------------------------------------------------------------------------------
// Error function for real arguments
// 
// Copyright : Numerical Recipes in C, 2nd edition; Cambridge University Press, p.221
// --------------------------------------------------------------------------------------------------------

double erfc (double x)
{
	double z = fabs(x) ;
	double t = 1 / (1 + 0.5 * z) ;

    z = t * exp (-z*z -1.26551223 + t* (1.00002368 + t * (0.37409196 + t * (0.09678418 
      + t * (-0.18628806 + t * (0.27886807 + t * (-1.13520398 + t * (1.48851587 
      + t * (-0.82215223 + t * 0.17087277)))))))));

    return (x >= 0) ? (1 - z) : (z - 1) ;
}  

// --------------------------------------------------------------------------------------------------------
// Elementary Frensel weighting functions 
//
// the function behaves as:
//     F(-inf) = 0.0
//     F(0)    = 0.5
//     F(+inf) = 1.0
//
// When used for Fresnel weighting, use w(d1,d2) = F(x2) - F(x1) where the argument should be  given by : 
// x = (d - dc) / a ; with d = position of end point of segment, dc = position center of Fresnel ellipse, 
// a = large axis of the ellipse. 
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

double FresnelFraction (double x)
{
   if (x < -1) return 0 ;
   if (x >  1) return 1 ; 
   return 1 - (acos(x) - x * sqrt (1 - x * x)) / PI ;
}

// -----------------------------------------------------------------------------------------------------
// General lowpass filter
// 
// this filter behaves as a two-pole lowpass filter for large values of its arguments
// but has a sharper cutoff at X = 1 
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// -----------------------------------------------------------------------------------------------------

inline double LowpassFilter (double X)
{
    if (X <= 0) return 1 ;
    return (1 - exp(-1/(X*X))) ;
}

// -----------------------------------------------------------------------------------------------------
// Hadden & Pierce : calculation of diffracted field components
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// -----------------------------------------------------------------------------------------------------

Complex HaddenPierce (double beta, double teta, double Rs, double Rr, double k, bool include_direct)
{
    double r  = Rs + Rr ;
    double nu = PI / beta ;
    double at = (teta - beta - PI) * nu / 2 ;
    if (teta < PI) at += PI ;

    double aa = fabs(at) ;
    double ct = cos (aa) ;
    double cn = sqrt (nu * nu + (2 * Rs * Rr / (r * r) + 0.5) * ct * ct) ; 
    double b  = sqrt (4 * k * Rs * Rr / (PI * r)) * ct / cn;

    Complex ad = FresnelAux (b) ;

    Complex ep = exp (Complex (0, PI/4)) ;

 // modif DVM 14.04.2003 : special case sin(x)/x as x tends to 0

    double sin_aa = sin(aa) ;
    if (sin_aa < 1.E-5) sin_aa = 1 ; else sin_aa /= aa ;

    Complex en = (PI/sqrt(2.)) * (sin_aa) * ep * ad / (cn / nu) ;

    Complex pd = -1/(PI * r) * at * en * exp(Complex(0,k*r)) ;

 // modif DVM 18.10.2003 : add optical field if requested
 
    if (include_direct && (teta < PI))
    {
       double rd = sqrt (Rs * Rs + Rr * Rr - 2 * Rs * Rr * cos(teta)) ;
       pd += (1/rd) * exp (Complex (0, k * rd)) ;
    }
    
    return pd ;
}

// -----------------------------------------------------------------------------------------------------
// Hadden-Pierce approximation for diffraction 
//
// - wedge replaced by a straight screen (beta = 2 * PI)
// - optical field added if diffraction point below line-of-sight (teta < PI)
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// -----------------------------------------------------------------------------------------------------

Complex Pdif_HaddenPierce (double teta, double Rs, double Rr, double k)
{
   return HaddenPierce (2*PI, teta, Rs, Rr, k, true) ;    
}

// --------------------------------------------------------------------------------------------------------
// Deygout : approximation of insertion loss as a function of Fresnel number N = 2 delta / lambda
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

double FresnelApprox (double n)
{
    double dn = 0 ;
    
    if (n < -0.25)
    {
        dn = 0 ;
    }   
    else if (n < 0)
    {
        dn = -6 + 12 * sqrt(-n) ;
    }
    else if (n < 0.25)
    {
        dn = -6 - 12 * sqrt(n) ;
    }
    else if (n < 1)
    {
        dn = -8 - 8 * sqrt(n) ;
    }
    else
    {
        dn = -16 - 10 * log10 (n) ;
    }
    
    return dn ;
}

// -----------------------------------------------------------------------------------------------------
// Deygout approximation for diffraction 
//
// modifications for teta > PI by D.Van Maercke
//  - direct distance replaced by D = Rr + Rs in exp(jkD)/D
//  - path difference calculated by second order approximation in teta
// 
// Note :
//
//   In many cases we will need the excess attenuation = 20 log |Pdif/ Pdir| with Pdir = exp(jkR)/R 
//   and FresnelApprox may provide an immediate approximation of this value. However, in the general 
//   case we will need the ratio P_image / P_direct.Therefore, DO NOT TRY TO SIMPLIFY THIS FORMULAE 
//   by removing the exp(jkR)/R factor.
//
//   As it is, Pdif_Deygout provides an approximation of Pdif_Hadden_Pierce with similar arguments.
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// -----------------------------------------------------------------------------------------------------

Complex PropagationPath::Pdif_Deygout (double teta, double Rs, double Rr, double k)
{
   double  lambda = 2 * PI / k ;

   double d1, d2, d4, rd, dd ;

// special treatment for angular arguments in the range PI to 4.PI ; angles > 2.PI must be allowed 
// because we use the same convention for angles as in the Hadden-Pierce model

   if (teta > PI) 
   {
       rd = Rr + Rs ;
       d1 = sqrt(Rs * Rr) * (teta - PI) / (Rs + Rr) ;
       d2 = d1 * d1 ;
       d4 = d2 * d2 ;
       dd = rd * (d2/2 + d4/3) ;
   }
   else
   {
       rd = sqrt (Rs * Rs + Rr * Rr - 2 * Rs * Rr * cos(teta)) ;
       dd = rd - (Rs + Rr) ;
   }

   double nd = 2 * dd / lambda ;
   double xd = FresnelApprox(nd) ;

   return (1/rd) * pow (10, xd/20) * exp (Complex (0.0, k*rd)) ;
}

// --------------------------------------------------------------------------------------------------------
// Calculate the free field Green's function
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::DirectField (double xs, double ys, double xr, double yr, Complex *pfield)
{
    double d = DIST (xs, ys, xr, yr) ;
    
    for (int i = 0 ; i < nbFreq ; i++)
    {
        double k = 2 * PI * freq[i] / c_sound ;
        
        pfield[i] = 1/d * exp (Complex (0.0, k * d)) ;
    }
}                    

// --------------------------------------------------------------------------------------------------------
// Calculate Green's function including diffraction by a semi-infinite plane screen
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::SingleDiffraction (double xs, double ys, double xd, double yd, double xr, double yr, 
                                         Complex *pfield)
{
    xs -= xd ;
    ys -= yd ;
    xr -= xd ;
    yr -= yd ;
 
 // calculate distance arguments
 
    double d1 = DIST (xs, ys) ;
    double d2 = DIST (xr, yr) ;

 // calculate angle arguments in the range (0, 2*PI) 
 
    double t1 = atan2 (-xs, ys) ; if (t1 <= 0) t1 += 2 * PI ;
    double t2 = atan2 ( xr, yr) ; if (t2 <= 0) t2 += 2 * PI ;
 
 // total angular argument T = T1 + T2. Note that T can be in the range 0 to 4 * PI, 
 // images of the source and the receiver may extend into virtual "image" space as usual 
 // in Sommerfeld's diffraction theory
 
    double teta = t1 + t2 ;
 
 // calculate diffracted field using Hadden-Pierce or Deygout approximation 
 
    for (int i = 0 ; i < nbFreq ; i++)
    {
        double k = 2 * PI * freq[i] / c_sound ;
       
        if (options & difHaddenPierce)
        {
            pfield[i] = Pdif_HaddenPierce (teta, d1, d2, k) ;
        }
        else
        {
            pfield[i] = Pdif_Deygout (teta, d1, d2, k) ;
        }        
    }    
}                    

// --------------------------------------------------------------------------------------------------------
// CumulateDiffraction
//
// Total diffraction is found by recursively looking for the most important diffraction edge
//
// After the most important diffraction edge is found and the corresponding attenuation due to diffraction
// computed, the process continues to the left and right of this point replacing the original source or
// receiver position with the newly determined diffraction point.
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::CumulDiffraction (int is, int id, int ir) 
{
    Complex p_free [MAX_FREQ] ;
    Complex p_diff [MAX_FREQ] ;
  
    double xs, ys, xr, yr, xd, yd ;
    
    GetVertex (is, xs, ys) ;
    GetVertex (ir, xr, yr) ;
    GetVertex (id, xd, yd) ;

 // calculate free field and diffracted field contributions
 
    DirectField (xs, ys, xr, yr, p_free) ;
    SingleDiffraction (xs, ys, xd, yd, xr, yr, p_diff) ;

 // calculate insertion loss by diffraction 
 
    double att_dif [MAX_FREQ] ;
    for (int j = 0 ; j < nbFreq ; j++)
    {
       att_dif[j] = 10 * log10 (norm(p_diff[j] / p_free[j])) ;
    }

 // save details
 
    detail[nbDetails].pos_src = is ;
    detail[nbDetails].pos_rec = ir ;
    detail[nbDetails].pos_dif = id ;
    detail[nbDetails].model    = ATT_DIFFRACTION ;
    for (int j = 0 ; j < nbFreq ; j++) detail[nbDetails].att[j] = att_dif[j] ;
    nbDetails++ ;
}

// --------------------------------------------------------------------------------------------------------
// FindConvexHull
// 
// Recursively construct the convex hull linking the source to the receiver over all possible diffracting
// edges
//
// After finding the convex hull and calculating the associated diffraction effects, calculate ground
// effects in between successive points of the convex hull.
//
// In case of concave ground, ground effects can be estimated using the Fresnel zone approach. In case 
// the profile contains at least one convex segment (and therefore a secondary diffraction point), a 
// transition is to be made between the ground model with and without diffraction.
//
// Notes :

//
//    The convex hull contains the source, the receiver and any diffraction point above the LOS
//
//    More than one secondary diffraction point (below LOS) may exist inbetween convex hull points but only
//    the most important secondary diffraction point is included in the geometrical model. The effects of the 
//    other secondary diffraction points is taken into account by means of the convex segment model.
//
//    The geometrical analysis is frequency independent  
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::FindConvexHull (int is, int ir)
{
    int i ;
    
 // a single segment contains no diffraction points...
 
    if ((ir - is) == 1)
    {
        GroundEffect (is, ir) ;
        return ;
    }
 
 // equivalent source & receiver positions
 
    double xs, ys, xr, yr ;
    
    GetVertex (is, xs, ys) ;
    GetVertex (ir, xr, yr) ;

 // search most important primary diffraction point above line-of-sight 
 
    int    ipos_dif = 0 ;
    double diff_max = 0 ;
    double dist_direct = DIST (xs,ys, xr,yr) ;
    
    for (i = is+1 ; i < ir ; i++)
    {
       double xp, yp ;
       
       GetVertex (i, xp, yp) ;
       
       double diff = DIST (xs,ys, xp,yp) + DIST (xp,yp, xr,yr) - dist_direct ;
       double sign = (xp - xs) * (ys - yr) + (yp - ys) * (xr - xs) ;
       if (sign < 0) continue ;
       if (ipos_dif == 0 || diff > diff_max)
       {
           ipos_dif = i ;
           diff_max = diff ;
       }
    }

 // no primary diffraction point exists, try diffraction below line of sight
 
    if (ipos_dif == 0)
    {
       FindSecondHull (is, ir) ; 
       return ;
    }

 // mark point as primary diffraction point 
 
    seg[ipos_dif].convex_hull = true ;

 // calculate attenuation due to diffraction by this edge
 
    CumulDiffraction (is, ipos_dif, ir) ;      

    for (i = is ; i < ipos_dif ; i++) seg[i].index_r = ipos_dif ;
    for (i = ipos_dif ; i < ir ; i++) seg[i].index_s = ipos_dif ;

 // and continue search process recursively on both sides of this edge
 
    FindConvexHull (is, ipos_dif) ;
    FindConvexHull (ipos_dif, ir) ;
}


// --------------------------------------------------------------------------------------------------------
// FindSecondHull
//
// search for A SINGLE secondary diffraction point below the LOS 
// make a transition between flat ground and diffraction below the line of sight model  
//
// note: some optimisation is possible here as FindConvexHull has already examined the most diffraction edge
// also, we could calculate the transition factor first and not calling the two models if T = 0 or T = 1
// at some frequencies...
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::FindSecondHull (int is, int ir)
{
    int i,j ;

 // calculate ground effect using flat ground model

    GroundEffect (is, ir) ;
        
 // a single segment contains certainly no diffraction points...
 
    if ((ir - is) == 1) return ;
    
 // look for secondary diffraction only at the end points of convex segments
 // that is to say : ignore points common to two succesive convex segments
 
    double xs, ys, xr, yr ;

    GetVertex (is, xs, ys) ;
    GetVertex (ir, xr, yr) ;
    
    int    ipos_dif = 0 ;
    double diff_max = 0 ;
    double dist_direct = DIST (xs,ys, xr,yr) ;
      
    for (i = is+1 ; i < ir ; i++)
    {
       if (seg[i-1].convex_r || seg[i].convex_s)
       {
          double xp, yp ;
          
          GetVertex (i, xp, yp) ;
          
          double diff = DIST (xs,ys, xp,yp) + DIST (xp,yp, xr,yr) - dist_direct ;
          if (ipos_dif == 0 || diff < diff_max)
          {
              ipos_dif = i ;
              diff_max = diff ;
          }
       }
    }

 // no second diffraction point, just keep the ground effect previously calculated

    if (ipos_dif == 0) return ;

 // get result from detail buffer, mark result as intermediate GROUND_FLAT model 
 
    double att_flat[MAX_FREQ] ;
    int    npos = nbDetails-1 ;

    detail[npos].model = ATT_GROUND_FLAT ;
    
    for (j = 0 ; j < nbFreq ; j++) att_flat[j] = detail[npos].att[j] ;
    
 // prepare for transition : find most important reflexion point (if it exists)
 // only count "real" reflexion points, ignore any convex segments here
 /*
    double diff_ref = DIST (xs-xr, ys+yr) - DIST (xs-xr, ys-yr) ;

    for (i = is ; i < ir ; i++)
    {
        if (seg[i].convex_s || seg[i].convex_r) continue ;
       
        double diff = seg[i].d_image - seg[i].d_direct ;
        
        if (diff < diff_ref) diff_ref = diff ;
    }
*/

 // note : if there are only convex segments, we set: diff_ref = 0
 //        if there is at least one convaxe segment, we must have: diff_ref < diff_max
 
    
// test for flatness (this is the same as Rayleigh's criteria for sigma_h > lambda/8)
// note that we include convex segments here because convex segments almost certainly inidcate the 
// presence of a screening object and/or a diffracting edge of an embankment.
// Fresnel weighting assures that too small convex segments will be eliminated anyway.

/*   
    double dif_avg [MAX_FREQ] ;
    
    for (j = 0 ; j < nbFreq ; j++)
    {
        double d = 0 ;
        double w = 0 ;
        
        for (i = is ; i < ir ; i++)
        {
            d += seg[i].w_fresnel[j] * (seg[i].d_image - seg[i].d_direct) ;
            w += seg[i].w_fresnel[j] ;
        }
        
        if (w != 0) d = d/w ; else d = diff_ref ;
        dif_avg[j] = d ;
    }
*/

// modif dvm 09.09.2010 : new parameter to estimate the "flatness" of the terrain
// the parameter is defined as the std.dev. of the path difference over the Fresnel zone
// for flat terrain, it equals zero, otherwise it is a parameter for roughness according
// to the Raleigh criteria ; i.e. if it is smaller than 1/8 of the wavelength, the terrain
// can be considered as flat.

    double dif_avg [MAX_FREQ] ;

    for (j = 0 ; j < nbFreq ; j++)
    {
        double d1 = 0 ;
        double d2 = 0 ;
        double w = 0 ;
        
        for (i = is ; i < ir ; i++)
        {
            double delta_spec = seg[i].d_image - seg[i].d_direct ;
            if (seg[i].convex_s || seg[i].convex_r) delta_spec = -delta_spec ;
            d1 += seg[i].w_fresnel[j] * delta_spec ;
            d2 += seg[i].w_fresnel[j] * delta_spec * delta_spec ;
            w  += seg[i].w_fresnel[j] ;
        }
        if (w > 0)
        {
            d1 = d1 / w ;
            d2 = d2 / w ;       
            d1 = sqrt (MAX(0.0, d2 - d1*d1)) ;
        }
        dif_avg[j] = d1 ;
    }

// mark this point as a secondary diffracting edge
    
    seg[ipos_dif].convex_hull = true ;
    seg[ipos_dif].second_hull = true ;
   
 // now calculate excess attenuation using the combined model : diffraction + ground effects on 
 // source and receiver side. 
 // Do not further resursions in order to find other secondary diffracting edges.
 
    CumulDiffraction (is, ipos_dif, ir) ;
    
    for (i = is ; i < ipos_dif ; i++) seg[i].index_r = ipos_dif ;
    for (i = ipos_dif ; i < ir ; i++) seg[i].index_s = ipos_dif ;

    GroundEffect (is, ipos_dif) ;
    GroundEffect (ipos_dif, ir) ;

 // remove primary diffraction
 
    seg[ipos_dif].convex_hull = false ;

 // get results from detail buffer, mark results as intermediate DIFFACTION_BLOS and GROUND_BLOS
 // cumulate diffraction + ground effect on both sides of the diffraction point 
 
    double att_blos[MAX_FREQ] ;

    for (j = 0 ; j < nbFreq ; j++) att_blos[j] = 0 ;
    
    for (i = npos ; i < nbDetails ; i++)
    {
       if (detail[i].model == ATT_DIFFRACTION)
       {
           detail[i].model = ATT_DIFFRACTION_BLOS ;
           for (j = 0 ; j < nbFreq ; j++) att_blos[j] += detail[i].att[j] ;
       }
       else if (detail[i].model == ATT_GROUND)
       {
           detail[i].model = ATT_GROUND_BLOS ;
           for (j = 0 ; j < nbFreq ; j++) att_blos[j] += detail[i].att[j] ;
       }
    }

 // save details for diffraction below line of sight model
 
    npos = nbDetails++ ;
    detail[npos].model = ATT_DIFF_GROUND_BLOS ;
    detail[npos].pos_src = is ;
    detail[npos].pos_rec = ir ;
    detail[npos].pos_dif = -1 ;
    for (j = 0 ; j < nbFreq ; j++) detail[npos].att[j] = att_blos[j] ;

 // transition between diffraction and ground model
 //
 // the transition function is based on three criteria
 //
 //   1. distance between stationnary points (shortest specular reflection version shortest diffracted path)
 //   2. effective height of the screen (Rayleigh's criteria for roughness)
 //   3. continuity of solution on the line of sight (must use diffraction model immediate below LOS)
 //
 // because the third criteria must be satisfied no matter the other ones, we have : C3 OR (C2 AND C1)
 
 // DVM 25.01.2006: bug reported by Ronald Hordijk (DGMR) "bad index dig_avg[j]" has been corrected.

 // DVM 25.01.2006: transition function X3 changed to exp(-t²/2)

    double att[MAX_FREQ] ;
    double trans [MAX_FREQ] ;

/*
    for (j = 0 ; j < nbFreq ; j++)
    {
        double lambda = c_sound / freq[j] ;
        
        double t1 = (diff_max - diff_ref) / (lambda/4) ;
        double t2 = (dif_avg[j] - diff_max) / (lambda/8) ;
        double t3 = diff_max / (lambda/256) ;
        
        double X1 = 1 ; if (t1 > 0) X1 = exp (-0.5 * t1 * t1) ;
        double X2 = 1 ; if (t2 > 0) X2 = exp (-0.5 * t2 * t2) ;
        double X3 = 1 ; if (t3 > 0) X3 = exp (-0.5 * t3 * t3) ;

        trans[j] = X3 + (1 - X3) * X1 * (1 - X2) ;
        
        att[j] = trans[j] * att_blos[j] + (1-trans[j]) * att_flat[j] ;      
    }
 */
 

 // modif dvm 09.09.2010 : changed transition from flat to diffraction below the
 // line of sight. Use the flat ground model when the terrain is flat (X1 == 1) 
 // and propagation is not too close to the line of sight (X2 == 0) ; use the
 // diffraction model in all other cases.
 
    for (j = 0 ; j < nbFreq ; j++)
    {
        double lambda = c_sound / freq[j] ;
            
        double t1 = dif_avg[j] / (lambda/8) ;
        double t2 = diff_max / (lambda/128) ;
        
        double X1 = LowpassFilter (t1) ; 
        double X2 = exp(-t2) / pow (1 + pow (t2, 1/2.), 1/3.) ;

        trans[j] = X1 * (1 - X2) ;

        att[j] = (1 - trans[j]) * att_blos[j] + trans[j] * att_flat[j] ;      
    }
 
 // save details about transition function
 
    npos = nbDetails++ ;
    detail[npos].model = ATT_TRANS_BLOS_FLAT ;
    detail[npos].pos_src = is ;
    detail[npos].pos_rec = ir ;
    detail[npos].pos_dif = -1 ;
    for (j = 0 ; j < nbFreq ; j++) detail[npos].att[j] = trans[j] ;

 // save combined ground attenuation
 
    npos = nbDetails++ ;
    detail[npos].model = ATT_GROUND ;
    detail[npos].pos_src = is ;
    detail[npos].pos_rec = ir ;
    detail[npos].pos_dif = -1 ;
    for (j = 0 ; j < nbFreq ; j++) detail[npos].att[j] = att[j] ;
}


// --------------------------------------------------------------------------------------------------------
// Calculate local coordinates for source & receiver for each segment
//
// - the local origin is choosen equal to the first point of each segment
// - the D axis is along the segment, directed from the first to the second point 
// - the H axis is perpendicular to the D axis, rotated 90° counter-clockwise
// - (ds, hs) are the coordinates of the source in the local coordinate system
// - (dr, hr) are teh coordinates of the receiver in the local coordinate system
// - in local coordinates, the segment extends from (d=0,h=0) to (d=dd,h=0)
// - the specular reflexion point is given by (d=dsr,h=0) 
//
// Note : "convex" segments are handled by projecting the source and/or receiver on the segment's base
//        line using a circular projection method that ensures that the source is always on the left of
//        P1 and the receiver to the right of P2. Frensel weights will than be calcalated using these
//        "virtual" source and receiver positions. The contribution of such segments will be reduced by
//        solving an equivalent diffraction problem, using the end points of the segment as diffracting
//        edges.
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::LocalCoordinates (int i1, int i2)
{
    for (int i = i1 ; i < i2  ; i++)
    {
        int is = seg[i].index_s ;
        int ir = seg[i].index_r ;

    // relative source and receiver position 

        double xs, ys, xr , yr ;
        
        GetVertex (is, xs, ys) ;
        GetVertex (ir, xr, yr) ;

    //  segment coordinates

        double x1 = seg[i].x1 ;
        double y1 = seg[i].y1 ;
        double x2 = seg[i+1].x1 ;
        double y2 = seg[i+1].y1 ;

     // set new origin to (x1,y1) and calculate unit vectors for d and h directions

        double xd = x2 - x1 ;
        double yd = y2 - y1 ;
        double dd = DIST (xd, yd) ;
        xd /= dd ;
        yd /= dd ;
        double xh = -yd ;
        double yh =  xd ;

     // express source and receiver in (d,h) coordinates 
     // in local coordinates, the segment is represented by P1(d=0, h=0) and P2(d=dd, h=0) 
     // the source is at Ps(d=ds,h=hs), the receiver at Pr(d=dr, h=hr) ;
     
        double ds = (xs - x1) * xd + (ys - y1) * yd ;
        double hs = (xs - x1) * xh + (ys - y1) * yh ;
        double dr = (xr - x1) * xd + (yr - y1) * yd ;
        double hr = (xr - x1) * xh + (yr - y1) * yh ;

     // modification DVM 06.04.2004 : approximate Fresnel weight & diffraction for convex segments 
     // replace by equivalent wedge problem
     // first inverse sign in order to calculate the intersection of the ray with the segment
     
        seg[i].convex_s = false ;
        seg[i].convex_r = false ;
        
        if (hs < 0) 
        { 
            seg[i].convex_s = true ;
            hs = -hs ;
        } 
        
        if (hr < 0) 
        { 
            seg[i].convex_r = true ; 
            hr = - hr ;
        }
     
        //if (hs < HSR_MIN) hs = HSR_MIN ;
        //if (hr < HSR_MIN) hr = HSR_MIN ;
        
     // lenght of the segment ; in local coordinates the segment extends from [h=0,d=0] to [h=0,d=d_segment]
     
        seg[i].d_segment = dd ;
        
     // relative position of specular reflexion point

     // modif DVM 28.11.2008, version 2.019
     // check for condition (hS = hR = 0)
     
        if ((hs + hr) == 0)
        {
            seg[i].d_specular = (ds + dr) / 2 ;
        }
        else
        {
            seg[i].d_specular = ds + hs * (dr - ds) / (hs + hr)  ;           
        }
        
     // source and receiver position in local coordinates
     
        seg[i].ds = ds ;
        seg[i].hs = hs ;
        seg[i].dr = dr ;
        seg[i].hr = hr ;

     // calculate distances for direct & reflected rays in local coordinates

        seg[i].d_direct = DIST (dr - ds, hs - hr) ;        
        seg[i].d_image  = DIST (dr - ds, hs + hr) ;        

     // calculate angle of incidence

        seg[i].cos_teta = (hs + hr) / seg[i].d_image ;
    }
}

// --------------------------------------------------------------------------------------------------------
// Calculate parameters of the Fresnel ellipse for each segment
// 
// Note : LocalCoordinates must be called first to initialize the (d,h) coordinates of the source and the
//        the receiver with respect to these segments.
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::FresnelParameters (int i1, int i2, double centerFreq)
{
    int i, j ;
    
    for (i = i1 ; i < i2 ; i++)
    {
        for (j = 0 ; j < nbFreq ; j++)
        {
           double ds = seg[i].ds ;
           double hs = seg[i].hs ;
           double dr = seg[i].dr ;
           double hr = seg[i].hr ;

         // ----------------------------------------------------------------------------------------------
         // Fine-tuning, DVM, 03.05.2005 : modified Fresnel weights for cases with/without diffraction.
         //
         // First testing of the Harmpnoise model suggested that a Fresnel fraction of 1/4 or 1/8 gave
         // best results over flat ground without meteorological refraction.
         //
         // Later it was found however that in case of diffraction the Frensel fraction should be reduced 
         // to 1/8 or 1/16 in the in order to get more accurate results.
         //
         // It turned out that this did not work very well under favourable meteorlological refraction.
         // Following a suggestion by Birger Plovsing it was found that a reduction of the Fresnel 
         // fraction to 1/16 or 1/32 worked better in the low frequency range. 
         //
         // Smaller Fresnel fractions however lead to higer levels in the high frequency range. At the 
         // limit of the high frequency range, optical acoustics prevail and the Fresnel fraction should
         // be taken larger. 
         // 
         // Therefore, we define a frequency dependent Fresnel fraction which starts from 1/32 in the
         // low frequency range to 1/2 or 1 in the high frequency range. Note that the Fresnel fraction
         // may become larger than 1 in the (very) high frequency range ; this may seen non physical but
         // it will reduce ground effects in this frequency range which is more in line with empirical
         // models like ISO 9613-2 (and with experimental results?).
         //
         // modif DVM 20.01.2006 : restriction nd < 1 in the high frequency range removed 
         // ----------------------------------------------------------------------------------------------

            double nd = 8 ;
                                      
            if (centerFreq != 0)
            {
                double fd = freq[j] / centerFreq  ;
                nd = 32 * LowpassFilter (fd) ; 
            }
          
            double ld = c_sound / freq[j] ;    
            double fn = ld / nd ;         
            
         // calculate position & size of the ellipse
        
            double a, b, dsf ;

            FresnelEllipse (fabs(dr-ds), hs, hr, fn, a, b, dsf) ;

         // position center of Fresnel ellipse & specular reflexion point in local coordinates
         
            double d_fresnel  = ds + dsf * (dr-ds) ;
            
            seg[i].a_fresnel[j] = a ;
            seg[i].d_fresnel[j] = d_fresnel ;            
        }
    }        
}

// --------------------------------------------------------------------------------------------------------
// Calculate Fresnel weights of all segments
// 
// Note : LocalCoordinates must be called first to initialize the (d,h) coordinates of the source and the
//        the receiver with respect to these segments.
//
// Note : FresnelParametes must be called first to determine the position and the size of the Frensel 
//        ellipse. 
//
// When freqCenter = 0 : use the original defintions of Hottershall & Harriet
//                 > 0 : use modified Fresnel weights with centering of the weighting function on the
//                       specular reflection point. "FreqCenter" is used as a transition parameter between
//                       the original - low frequency - model and the modified - high frequency - model.
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::FresnelWeights (int i1, int i2, double freqCenter)
{
    int i, j ;

    for (i = i1 ; i < i2 ; i++)
    {        
        for (j = 0 ; j < nbFreq ; j++)
        {
           double d_fresnel  = seg[i].d_fresnel[j] ;
           double d_specular = seg[i].d_specular ;
           double a_fresnel  = seg[i].a_fresnel[j] ;
           double d_start    = 0 ;
           double d_end      = seg[i].d_segment ;
 
        // characteristic points : d = fz-a, d = fz, d = fz + a

           double fz = d_fresnel ;
           double f1 = d_fresnel - a_fresnel ;
           double f2 = d_fresnel + a_fresnel ;

        // move center of Fresnel weighting toward specular reflexion point
        
           if (freqCenter > 0)
           {
              double X = freq[j] / freqCenter ;
              double F = 1 / (1 + X * X)  ;
              fz = d_fresnel + (1 - F) * (d_specular - d_fresnel) ;                         
           }
        
        // move the origin of the coordinate system to (d = fz) and get parameters of the coordinate 
        // transform x' = x / (alpha * x + beta) so that :
        //     f(f1-fz) = -1
        //     f(0)     =  0
        //     f(f2-fz) =  1
        // note that if fz did not move in the previous step, we get x' = x/a, which is equivalent
        // to the original model of Hottershal & Hariett ; if on the other hand fz has moved to the   
        // specular reflexion point, this model will be close to the modified Nord 2000 model because
        // we then have : S(r)/SFZ(r) = S(s)/SFZ(s) = 1/2.

           f1 -= fz ;
           f2 -= fz ;

           double alpha = (f2 + f1) / (f2 - f1) ;
           double beta  = -2 * f1 * f2 / (f2 - f1) ;

        // normalize segment coordinates to [-1,1] on [f1, f2] ; 

           double d1 = d_start - fz ; d1 = d1 / (alpha * d1 + beta) ;
           double d2 = d_end   - fz ; d2 = d2 / (alpha * d2 + beta) ;

        // calculate Fresnel weight ; if the end point of the segment coincide with the source or receiver
        // position, extend the segment to ds = -inf and/or dr = +inf and set FW(-inf) = 0, FW(+inf) = 1 

           if (i == seg[i].index_s  ) d1 = 0 ; else d1 = FresnelFraction (d1) ;
           if (i == seg[i].index_r-1) d2 = 1 ; else d2 = FresnelFraction (d2) ;
        
        // store data in segment

           seg[i].w_fresnel[j] = (d2 - d1) ;                      
        }
    }        
}

// --------------------------------------------------------------------------------------------------------
// Calculate Fresnel weights of all segments according to the original Hottershall and Harriet defintions
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::InitialFresnelWeights (int is, int ir)
{
    centerFreq = 0 ;
    
    LocalCoordinates (is, ir) ;
    FresnelParameters (is, ir, 0.0) ;
    FresnelWeights (is, ir, 0.0) ;
}


// --------------------------------------------------------------------------------------------------------
// Calculate modified Fresnel weights of all segments
//
// As in Nord2000 a frequency dependent transition is made between the original - low frequency - model and
// a modified - high frequency - model that centers the weighting function on the specular relfection point.
//
// The determination of the transition frequency is based on a proposal from Nord 2000 but has been
// modified here so that :
//
//      1) it can be applied over non flat ground 
//      2) it does not use the "sigma" values to determine the "softest" ground type
//      3) only segments with positive original Fresnel weights are included in the tests
//
// Instead the "maximum total phase difference" is used to separate low frequency behaviour from high
// frequency model. High frequency behaviour if the total phase is larger than PI.
// 
// As in Nord2000, the determination of the transition frequency can go wrong in case of a very small 
// absorbing strip on an overall hard ground...
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::ModifiedFresnelWeights (int is, int ir) 
{
    double phi_max[MAX_FREQ] ;

 // determine maximum total phase difference (includes path difference + phase due to finite impedance)
 
 // modif dvm 12.10.2004 : exclude convex segments & segments with zero fresnel weight when looking for 
 // transition frequency !
 
 // modif dvm 09.09.2010 : take into account convex segments by means of negative path differences

    for (int j = 0 ; j < nbFreq ; j++)
    {
        phi_max[j] = 0 ;

        double max_b = 0 ;
        
        for (int i = is ; i < ir ; i++)
        {
            if (seg[i].w_fresnel[j] == 0) continue ;
            
            double k = 2 * PI * freq[j] / c_sound ;
            double d = seg[i].d_image - seg[i].d_direct ;
            if (seg[i].convex_s || seg[i].convex_r) d = -d ;

            double w = seg[i].w_fresnel[j] ;
            double b = k * d + arg (seg[i].Q[j]) ;

            if (b > max_b) max_b = b ;
            
        }
        
        phi_max[j] = max_b * 180/PI ;
    }

 // determine low frequency / high frequency limits
 
    double f1 = 20 ;
    double f2 = 20000 ;
    double PHI_MIN =  90 ;
    double PHI_MAX = 180 ;
    
 // determine fmin so that phi(fmin) = PI/2

	int j ;
    for (j = 0 ; j < nbFreq ; j++)
    {
       if (phi_max[j] >  PHI_MIN) break ; 
    }

 // ---------------------------------------------------------------------------------------------
 // Modification DVM 26.05.2005 : use linear interpolation between frequency bands
 // In case the model is instructed to use octave bands instead of the default 1/3 octave bands,
 // approxiamtion of fmin/fmax to the closest center frequency might betoo rough. Interpolation
 // will give refine the estimation of the transition frequency.
 // ---------------------------------------------------------------------------------------------
 
    if (j == 0)
    {
        f1 = freq[0] / 2 ;
    }
    else if (j == nbFreq)
    {
        f1 = freq[nbFreq-1] ;
    }
    else
    {
        f1 = freq[j-1] + (freq[j] - freq[j-1]) * (PHI_MIN - phi_max[j-1]) / (phi_max[j] - phi_max[j-1]) ;
    }

 // determine fmax so that phi(fmax) = PI
 
    for (j = 0 ; j < nbFreq ; j++)
    {
       if (phi_max[j] > PHI_MAX) break ; 
    }    

// modif DVM 26.05.2005 : use linear interpolation between frequency bands 
// this should give better results when the model is used in larger frequency bands 
 
    if (j == 0)
    {
        f2 = freq[0] ;
    }
    else if (j == nbFreq)
    {
        f2 = freq[nbFreq-1] * 2 ;
    }
    else
    {
        f2 = freq[j-1] + (freq[j] - freq[j-1]) * (PHI_MAX - phi_max[j-1]) / (phi_max[j] - phi_max[j-1]) ;
    }

 // estimate transition frequency
 
    centerFreq = sqrt(f1 * f2) ; 

 // calculate modified fresnel weights
 
    p5.name = "CenterFreq" ;
    p5.value = centerFreq ;
    
    FresnelParameters (is, ir, centerFreq) ;
    FresnelWeights (is, ir, centerFreq) ;
}


// --------------------------------------------------------------------------------------------------------
// MakeImage utility function : get position of the image of a point with respect to one segment
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::MakeImage (double xo, double yo, int i, double &xi, double &yi)
{
    double x1 = seg[i].x1 ;
    double y1 = seg[i].y1 ;
    double x2 = seg[i+1].x1 ;
    double y2 = seg[i+1].y1 ;
    double dd = DIST (x1, y1, x2, y2) ;
    double xn = (y1 - y2) / dd ;
    double yn = (x2 - x1) / dd ;
    
    dd = (xo - x1) * xn + (yo - y1) * yn ;
    
    xi = xo - 2 * dd * xn ;
    yi = yo - 2 * dd * yn ;
}

// --------------------------------------------------------------------------------------------------------
// GetDiffraction : find the equivalent diffraction weight "D" for each segment
//
// Calculation of the field weighting factor D; distinguish 4 cases:
//
//    - no diffraction, source to receiver
//    - reflection between source and diffracting edge
//    - reflection between diffracting edge and receiver
//    - reflection in between two diffracting edges
//
// Special corrections apply to convex ground segments ; i.e. extra diffraction is added using the 
// intersection of the direct ray path wiht the segment as a fictitious diffracting edge.
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::GetDiffraction (int i1, int i2)
{
    int i,j ;
    
    double xs, ys ;     // source position
    double xr, yr ;     // receiver position
    double xsi, ysi ;   // image of the source through the segment
    double xri, yri ;   // image of the receiver through the segment
    double xds, yds ;   // diffraction point on the source side
    double xdr, ydr ;   // diffraction point on the receiver side

    Complex p_direct[MAX_FREQ] ;            
    Complex p_image [MAX_FREQ] ;

    for (i = i1 ; i < i2 ; i++)
    {
        if (seg[i].w_fresnel[0] == 0)
        {
            for (j = 0 ; j < nbFreq ; j++) seg[i].D[j] = 0 ;
            continue ;
        }  
       
     // equivalent source and receiver positions
     // convex hull points may act as equivalent source/receiver position
     
        int ids = seg[i].index_s ;
        int idr = seg[i].index_r ;
        
        GetVertex (ids, xds, yds) ;
        GetVertex (idr, xdr, ydr) ;
        
     // real source and receiver positions
     
        int pos_src = 0 ;
        int pos_rec = nbSeg ;
        
        GetVertex (pos_src, xs, ys) ;
        GetVertex (pos_rec, xr, yr) ;
        
     // no diffraction, 
     // weight is defined by d = D'/D with D = 1/R exp (-jkR), D' = 1/R' exp(-jkR')  
     
        if (ids == pos_src && idr == pos_rec)
        {
            for (j = 0 ; j < nbFreq ; j++)
            {
                double k = 2 * PI * freq[j] / c_sound ;
                double d1 = seg[i].d_direct ;
                double d2 = seg[i].d_image  ; 
               
                seg[i].D[j] = d1 / d2 * exp (Complex(0.0, k*(d2-d1))) ;
            }
        }
    
     // diffraction on the source side only
     // weight is defined by d = Diff (S,D,R) / Diff (S,D,R')
     // where R' is the image of R through the segment under consideration   

        else if (ids != pos_src && idr == pos_rec)
        {
            MakeImage (xr, yr, i, xri, yri) ;
            
            SingleDiffraction (xs, ys, xds, yds, xr,  yr,  p_direct) ;
            SingleDiffraction (xs, ys, xds, yds, xri, yri, p_image) ;

            for (j = 0 ; j < nbFreq ; j++)
            {
               seg[i].D[j] = (p_image[j] / p_direct[j]) ;
            }
        }
        
     // diffraction on the receiver side only
     // weight is defined by d = Diff (S',D,R) / Diff (S,D,R)
     // where S' is the image of S through the segment under consideration
    
        else if (ids == pos_src && idr != pos_rec)
        {            
            MakeImage (xs, ys, i, xsi, ysi) ;
            
            SingleDiffraction (xs,  ys,  xdr, ydr, xr, yr, p_direct) ;
            SingleDiffraction (xsi, ysi, xdr, ydr, xr, yr, p_image) ;

            for (j = 0 ; j < nbFreq ; j++)
            {
               seg[i].D[j] = (p_image[j] / p_direct[j]) ;
            }
        }
        
     // diffraction on both sides
     // weight is defined by d = ((Diff (D1,D2,R) / Diff (D1',D2,R)) * ((Diff (S,D1,D2) / Diff (S,D1,D2'))
     // where D1' is the image of D1 through the segment under consideration and D2' the image of D2
    
        else 
        {
         // source = S, diffraction point = D1, receiver = D2 (image receiver = D2')
         
            MakeImage (xdr, ydr, i, xri, yri) ;            
            
            SingleDiffraction (xs, ys, xds, yds, xdr, ydr, p_direct) ;
            SingleDiffraction (xs, ys, xds, yds, xri, yri, p_image) ;

            for (j = 0 ; j < nbFreq ; j++)
            {
               seg[i].D[j] = (p_image[j] / p_direct[j]) ;
            }

         // source = D1, diffraction point = D2, receiver = R (image source = D1')
         
            MakeImage (xds, yds, i, xsi, ysi) ;
            
            SingleDiffraction (xds, yds, xdr, ydr, xr, yr, p_direct) ;
            SingleDiffraction (xsi, ysi, xdr, ydr, xr, yr, p_image) ;

            for (j = 0 ; j < nbFreq ; j++)
            {
               seg[i].D[j] *= (p_image[j] / p_direct[j]) ;
            }
        }
        
     // -------------------------------------------------------------------------------------------------------
     // modification dvm 13.06.2005 for convex segments
     // -------------------------------------------------------------------------------------------------------
     
     // add extra diffraction for convex segment (source behind segment)
     // use specular reflection point as secondary diffracting edge 
     // calculate diffraction from equivalent source position to image receiver
     // note: the factor 2 is required to account for image source on the source side of the wedge
          
        if (seg[i].convex_s)
        {
            MakeImage (xdr, ydr, i, xri, yri) ;
            
            double xdd = seg[i].x1 + seg[i].d_specular * (seg[i].x2 - seg[i].x1) / seg[i].d_segment ;
            double ydd = seg[i].y1 + seg[i].d_specular * (seg[i].y2 - seg[i].y1) / seg[i].d_segment ;
            
            DirectField (xds, yds, xri, yri, p_direct) ;
            SingleDiffraction (xds, yds, xdd, ydd, xri, yri, p_image) ;

            for (int j = 0 ; j < nbFreq ; j++)
            {
               seg[i].D[j] *= (2. * p_image[j] / p_direct[j]) ; 
            } 
        }
    
     // add extra diffraction for convex segment (receiver behind segment)
     // use specular reflection point as secondary diffracting edge 
     // calculate diffraction from image source position to equivalent receiver position
     // note: the factor 2 is required to account for the image on the receiver side of the wedge
    
        else if (seg[i].convex_r)
        {
            MakeImage (xds, yds, i, xsi, ysi) ;
            
            double xdd = seg[i].x1 + seg[i].d_specular * (seg[i].x2 - seg[i].x1) / seg[i].d_segment ;
            double ydd = seg[i].y1 + seg[i].d_specular * (seg[i].y2 - seg[i].y1) / seg[i].d_segment ;
            
            DirectField (xsi, ysi, xdr, ydr, p_direct) ;
            SingleDiffraction (xsi, ysi, xdd, ydd, xdr, ydr, p_image) ;

            for (int j = 0 ; j < nbFreq ; j++)
            {
               seg[i].D[j] *= (2. * p_image[j] / p_direct[j]) ; 
            } 
        }
    }
    
}

// --------------------------------------------------------------------------------------------------------
// estimate partial coherence coefficient 
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::GetCoherence (int i1, int i2)
{
    int i,j ;
    
    if (options & enableAveraging)
    {
        for (i = i1 ; i < i2 ; i++)
        {      
            for (j = 0 ; j < nbFreq ; j++)
            {
                if (seg[i].w_fresnel[j] == 0)
                {
                    seg[i].C[j] = 0 ;
                    continue ;
                }  

             // include loss of coherency due to the following effects :
             // - finite frequency band width
             // - incertainties on sound speed
             // - incertainties on source and receiver height
 
                double hs = seg[i].hs ;
                double hr = seg[i].hr ;
                double dh = fabs (seg[i].dr - seg[i].ds) ;

                double xf = pow (2. , band_width/2) ;
                
                double stddev_f = freq[j] * (xf - 1/xf) / 3. ;
             
             // modif DVM 28.11.2008 version 2.019
             // no variation of sound speed with time (this might become an input parameter)
             
                double stddev_c = 0 ;
                double stddev_s = ((seg[i].index_s == 0)     ? delta_hSource   : 0.00) ;
                double stddev_r = ((seg[i].index_r == nbSeg) ? delta_hReceiver : 0.00) ;
         
                double dr = DIST (dh, hs+hr) - DIST(dh, hs-hr) ;
                double kc  = 2 * PI * freq[j] / c_sound  ;
                double phi = kc * dr ;
          
                double Xf = stddev_f / freq[j] ;
                double Xc = stddev_c / c_sound ;
                double Xs = 0 ;
                double Xr = 0 ;
          
                if (hs != 0) Xs = stddev_s / hs ;
                if (Xs >  1) Xs = 1 ;
                if (hr != 0) Xr = stddev_r / hr ;
                if (Xr >  1) Xr = 1 ;

                double X = phi * sqrt (Xf * Xf + Xc * Xc + Xs * Xs + Xr * Xr) ;
          
                double C1 = exp (-0.5 * X * X) ;

             // add loss of coherence due to turbulence (Kolmogorov model)

             // modif DVM 28.11.2008, version 2.019
             // handle special case hS = hR = 0
             
                double dt = ((hs + hr) == 0) ? 0.0 : (hs * hr / (hs + hr)) ;
                double C2 = exp (- 0.1365 * c_meteo * kc * kc * pow(fabs(dt),5./3.) * dh) ;
          
                seg[i].C[j] = C1 * C2 ;

            }
        }
    }
    else
    {
        for (i = i1 ; i < i2 ; i++)
        {
           for (j = 0 ; j < nbFreq ; j++) seg[i].C[j] = 1 ;  
        }
    }
}

// --------------------------------------------------------------------------------------------------------
// Calculate spherical reflection coefficient Q for each segment
//
// Note : local coordinates and initial / modified fresnel weights should be calculated before 
//        calling this function
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::GetReflexion (int is, int ir)
{
    int i,j ;

 // calculate spherical reflection coefficients for each segment 
 
 // modif DVM 07.07.2005 : calculate reflexion coefficient for all segments even if fresnel 
 // weight is zero. We may need this value AFTER estimation of the transition frequency and
 // applying the modified Fresnel weights.
 
    for (i = is ; i < ir ; i++)
    {
        double d_direct = seg[i].d_direct ;
        double d_image  = seg[i].d_image ;
        double cos_teta = seg[i].cos_teta ;
        int    index    = seg[i].ground ;

        for (j = 0 ; j < nbFreq ; j++)
        {
        // modif DVM 08.11.2006 : modified reflection coefficient for almost grazing incidence
        // based on thesis J.Defrance. See calculation Qref. The transition function N is based 
        // on the height of the ray above the plane (in local coordinates) compared to the wavelenght. 
        
        // double hBL = (c_sound / freq[j]) / 32 ;
        // double hSR = (seg[i].hs + seg[i].hr) / 2 ;
           
        // modif DVM 07.09.2010
        // only correct if BOTH source and receiver are inside the transition region
        
           double hBL = (c_sound / freq[j]) / 16 ;
           double hSR = MAX (seg[i].hs, seg[i].hr) ;
           double X = hSR / hBL ;
           double N = 1 - 0.7 * exp(-X) ;

           double   kd = 2 * PI * freq[j] / c_sound ;
           Complex  Z  = impedance[index].Z[j] ;
           
           seg[i].Q[j] = Qref (Z, cos_teta, kd * d_image, N) ;            
        }        
    }
}

// --------------------------------------------------------------------------------------------------------
// Calculate ground effect using flat and valley model 
//
// Note : local coordinates and initial / modified fresnel weights should be calculated before 
//        calling this function
//
// Modif 30.01.2008: as suggested by Erik Salomons, the GetCoherence function should be called after 
//                   modifying the Fresnel weights.
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::GroundEffect (int is, int ir)
{
    int i, j ;
   
    Complex attCOH [MAX_FREQ] ;
    double  attLIN [MAX_FREQ] ;
    double  attLOG [MAX_FREQ] ;
    double  sumW   [MAX_FREQ] ;
    double  attSEG [MAX_FREQ] ;
       
    for (j = 0 ; j < nbFreq ; j++) attLIN[j] = 0 ;
    for (j = 0 ; j < nbFreq ; j++) attCOH[j] = 0 ;
    for (j = 0 ; j < nbFreq ; j++) attLOG[j] = 0 ;
    for (j = 0 ; j < nbFreq ; j++) sumW[j]   = 0 ;
    
 // initialize local coordinates and calculate initial Fresnel weights
 
    InitialFresnelWeights (is, ir) ;
 
 // calculate Z and Q per segment ; Q is needed to calculate the transition frequency
 
    GetReflexion (is, ir) ;

 // modify Fresnel weights (this requires that Q has been determined...)
    
    ModifiedFresnelWeights (is, ir) ; 

 // get field weighting (direct or diffracted) êr segment
 
    GetDiffraction (is, ir) ;

 // calculate coherence factors per segment
 
    GetCoherence (is, ir) ;
    
 // calculate LIN and LOG ground effects; sum contributions using fresnel weighting

    for (i = is ; i < ir ; i++)
    {
        for (j = 0 ; j < nbFreq ; j++)
        {          
           Complex Q = seg[i].Q[j] * seg[i].D[j] ;
           double  w = seg[i].w_fresnel[j] ; 
           double  C = seg[i].C[j] ;
           if (w > 0)
           {
               attLOG[j] += w * LOG10 (norm (1. + C*Q) + (1. - C*C) * norm(Q)) ;
               attCOH[j] += w * Q * C ;
               attLIN[j] += w * (1. - C*C) * norm(Q) ;
               sumW[j]   += w ;
           }
       }
    }
 
 // LIN model: add coherent + incoherent parts and transform to log values 

    for (j = 0 ; j < nbFreq ; j++)
    {
        attLIN[j] = LOG10 (attLIN[j] + norm (1. + attCOH[j])) ;
    }
    
 // --------------------------------------------------------------------------------------------
 // transition between LIN and LOG model 
 // modified by DVM, 16.09.2005 V2.011 : transition between LIN and LOG model modified.
 // use LOG model in the high frequency range... this seems unexpected but works well enough.
 // --------------------------------------------------------------------------------------------
   
    double trans[MAX_FREQ] ;
   
    for (j = 0 ; j < nbFreq ; j++)
    {    
         double F = freq[j] / centerFreq ;
         double X = sumW[j] / sqrt (1 + F * F); 
         double T = LowpassFilter(X) ;
         
         attSEG[j] = T * attLOG[j] + (1 - T) * attLIN[j] ;            
         trans[j]  = T ;
    }

 // save details
 
    detail[nbDetails].pos_src = is ;
    detail[nbDetails].pos_rec = ir ;
    detail[nbDetails].pos_dif = -1 ;
    detail[nbDetails].model    = ATT_GROUND_LIN ;
    for (j = 0 ; j < nbFreq ; j++) detail[nbDetails].att[j] = attLIN[j] ;
    nbDetails++ ;

    detail[nbDetails].pos_src = is ;
    detail[nbDetails].pos_rec = ir ;
    detail[nbDetails].pos_dif = -1 ;
    detail[nbDetails].model    = ATT_GROUND_LOG ;
    for (j = 0 ; j < nbFreq ; j++) detail[nbDetails].att[j] = attLOG[j] ;
    nbDetails++ ;

    detail[nbDetails].pos_src = is ;
    detail[nbDetails].pos_rec = ir ;
    detail[nbDetails].pos_dif = -1 ;
    detail[nbDetails].model    = ATT_TRANS_LIN_LOG ;
    for (j = 0 ; j < nbFreq ; j++) detail[nbDetails].att[j] = trans[j] ;
    nbDetails++ ;

    detail[nbDetails].pos_src = is ;
    detail[nbDetails].pos_rec = ir ;
    detail[nbDetails].pos_dif = -1 ;
    detail[nbDetails].model    = ATT_GROUND ;
    for (j = 0 ; j < nbFreq ; j++) detail[nbDetails].att[j] = attSEG[j] ;
    nbDetails++ ;
} ;

// --------------------------------------------------------------------------------------------------------
// ExcessAttenuation
//
// Combine together diffraction, ground effects, fresnel weights and coherence effects
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::ExcessAttenuation (void)
{
// initialize segment data

    for (int i = 0 ; i <= nbSeg ; i++)
    {
        seg[i].convex_hull = false ;
        seg[i].second_hull = false ;
        seg[i].index_s     = 0 ;
        seg[i].index_r     = nbSeg ;
        for (int j = 0 ; j < nbFreq ; j++) seg[i].w_fresnel[j] = 0 ;
    }
    
 // decompose problem and recursively calculate diffraction and ground effects

    FindConvexHull (0, nbSeg) ;

 // cumulate ground & diffraction effects to obtain total excess attenuation

    int npos = nbDetails++ ;

    detail[npos].model = ATT_EXCESS_GLOBAL ;
    detail[npos].pos_src = 0 ;
    detail[npos].pos_rec = nbSeg ;
    detail[npos].pos_dif = -1 ;
   
    for (int j = 0 ; j < nbFreq ; j++) 
    {
        detail[npos].att[j] = 0 ;
   
        for (int i = 0 ; i < nbDetails-1 ; i++)
        {
            if (detail[i].model == ATT_DIFFRACTION || detail[i].model == ATT_GROUND)
            {
                detail[npos].att[j] += detail[i].att[j] ;
            }
        }
    }

// add scattering 

    if ((options & enableScattering) && c_meteo > 0)
    {
       double x = seg[nbSeg].x1 - seg[0].x1 ;
       double y = seg[nbSeg].y1 - seg[0].y1 ;
       double d = DIST (x,y) ;
       
       for (int j = 0 ; j < nbFreq ; j++)
       {
            double A1 = detail[npos].att[j] ;
            double A2 = 25 + 10 * log10 (c_meteo) + 3 * log10 (freq[j]/1000) + 10 * log10 (d/100) ;
            double A3 = 10 * log10 (pow(10.,A1/10) + pow(10.,A2/10)) ;
            detail[npos].att[j] = A3 ;
        }
    }
    
// include air absorption 

    if (options & enableAirAbsorption)
    {
       double x1,y1, x2,y2 ;
       
       GetVertex (0, x1, y1) ;
       GetVertex (nbSeg, x2, y2) ;
     
       double d = DIST (x2 - x1, y2 - y1) ;
       
       for (int j = 0 ; j < nbFreq ; j++)
       {
            detail[npos].att[j] -= abs_air[j] * d ;
       }
    }
}

// ---------------------------------------------------------------------------------------------------------
// Default frequency values 
// ---------------------------------------------------------------------------------------------------------

static double defaultFreq[] = {  20,   25,   32,   40,   50,   63,   80,   100,   125,   160,
                                200,  250,  315,  400,  500,  630,  800,  1000,  1250,  1600,
                               2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000 } ;

// --------------------------------------------------------------------------------------------------------
// Create and initialiaze a PointToPoint calculation engine
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

PropagationPath::PropagationPath (void)
{
    majorID = MAJOR_ID ;
    minorID = MINOR_ID ;

    maxFreq = MAX_FREQ ;
    nbFreq  = MAX_FREQ - 3 ;
    
    for (int i = 0 ; i < maxFreq ; i++) freq[i] = defaultFreq[i+1] ;

    band_width = 1./3. ;
    
    c_sound = 340 ;
    a_meteo = 0 ;
    b_meteo = 0 ;
    c_meteo = 0 ;
    d_meteo = 0 ;
        
    options  = enableAveraging ;
    maxUserSegment = MAX_SEG ;   
    nbUserSegment  = 0 ;
    userSplitSegment = 1 ;
    expand_distance = 1 ;
    
    maxSeg = MAX_SEG ;
    nbSeg  = 0 ;
    
    hSource   = 1.00 ;
    hReceiver = 5.00 ;
    
    delta_hSource   = 0.10 ;
    delta_hReceiver = 0.50 ;

    nbImpedance    = MAX_IMPEDANCE ;
    maxImpedance   = MAX_IMPEDANCE ;

    InitImpedances() ;
    InitAirAbsorption() ;
    
    cpu_timer = 0 ;
}

// --------------------------------------------------------------------------------------------------------
// Delete the PointToPoint calculation engine
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

PropagationPath::~PropagationPath (void)
{
} ;

// --------------------------------------------------------------------------------------------------------
// Start the PointToPoint calculation engine on the current problem
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

int PropagationPath::DoPointToPoint (void)
{
// reset results

    nbDetails = 0 ;

// initialize debug

    p1.name = "" ; p1.value = 0.0 ;
    p2.name = "" ; p2.value = 0.0 ;
    p3.name = "" ; p3.value = 0.0 ;
    p4.name = "" ; p4.value = 0.0 ;
    p5.name = "" ; p5.value = 0.0 ;
    p6.name = "" ; p6.value = 0.0 ;
 
 // error checking
 
    if (nbFreq < 1 || nbUserSegment < 1) return 0 ;

 // preparation : transform user segments into internal segments
 
    CreateSegments() ;
 
 // simumate meteo effects by curved ground analogy
 
    DoCurvature () ;
 
 // now calculate excess attenuation
 
    ExcessAttenuation() ;

 // everything's OK, return 1
 
    return 1 ;
} ;

// --------------------------------------------------------------------------------------------------------
// default impedances models
// --------------------------------------------------------------------------------------------------------

static const int nbDefaultSigma = 9 ;
static double defaultSigma[nbDefaultSigma] =
{
    500, 12.5, 32, 80, 200, 500, 2000, 20000, 200000
} ;

// --------------------------------------------------------------------------------------------------------
// Fill in default impedances 
//
// See also : constants defined in PointToPoint.hpp
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::InitImpedances (void)
{
 // fill in default sigma class values
 
    for (int i = 0 ; i < nbDefaultSigma ; i++)
    {
        impedance[i].model = true ;
        impedance[i].sigma = defaultSigma[i] ;
        impedance[i].thickness = 0 ;
        
        for (int j = 0 ; j < nbFreq ; j++) 
        {
           impedance[i].Z[j] = impedance_D_and_B (impedance[i].sigma, 
                                                  impedance[i].thickness, 
                                                  freq[j], 
                                                  c_sound) ;
        }
    }
    
    for (int i = groundUserDefined ; i < maxImpedance ; i++)
    {
        impedance[i] = impedance[0] ;
    }
}

// --------------------------------------------------------------------------------------------------------
// auxiliary function : generate a sequence of random numbers with Gaussian distribution 
// 
// reference : Box-Muller method, Numerical Recipes in C, 2nd Edition, p.289
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void AUX_RandomGauss (int n, double *val)
{
   static unsigned int idum = 123456 ;
   
   for (int i = 0 ; i < n ; )
   {
      idum = 1664525 * idum + 1013904223 ;

      double v1 = 1 - 2 * ((double) (idum >> 16) / 65536.) ;
      double v2 = 1 - 2 * ((double) (idum & 0xFFFF) / 65536.) ;
      double vr = v1 * v1 + v2 * v2 ;

      if (vr > 1 || vr == 0) continue ;

      vr = sqrt (-2 * log (vr) / vr) ;
      val[i++] = v1 * vr ;
      val[i++] = v2 * vr ;
   }
}

// --------------------------------------------------------------------------------------------------------
// create the internal segmentation of the ground profile
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::CreateSegments (void)
{
   double dsr = userSegment[nbUserSegment-1].x ;
/*   
   double maxLen = 50 ;
   int    subDiv = (dsr / maxLen + 0.5) ;
 
// modif DVM 05.11.2005 : changed segment subdivision for meteo curvature. For short distances taking 3 
// subdivisions is sufficient ; for larger distances the ground profile is subdivided in segments of 
// approximately 50m.

   if (subDiv < 3)
   {
       subDiv = 3 ;
       maxLen = dsr / subDiv ;
   }
   else if (subDiv > 20)
   {
       subDiv = 20 ;
       maxLen = dsr / subDiv ;
   }
 */

// modif DVM 30.10.2008 suggested by Erik Salomons
// for normal use (i.e. when calculating Lden): userSplitSegment = 1, 
// when continuity of results as a function of meteorological parameters is important, 
// smaller segments should be taken into consideration by setting userSplitSegments > 1
 
   double maxLen = MIN(dsr/3.,MAX(50., dsr/20.)) / userSplitSegment ;
 
start_again :

   nbSeg = 0 ;
   
   double x1 = 0 ;
   double y1 = 0 ;
   
   for (int i = 0 ; i < nbUserSegment ; i++)
   {
       double x2 = userSegment[i].x * expand_distance ;
       double y2 = userSegment[i].y ;
       
       double dx = x2 - x1 ; 
       double dy = y2 - y1 ;
       
       //if (fabs(dx) < 0.01 && fabs(dy) < 0.01) continue ;
       
    // modif DVM 28.11.2008 version 2.019
    //
    // 1) as suggested by Erik Salomons, use ceil() function to round to the next integer
    //    equal or greater dan dx/maxLen (even if the result cannot be guaranteed due to
    //    rounding and limited precision of floating point calculations).
    //
    // 2) no splitting of segments is required in case of a homogeneous atmosphere,
    //    i.e. without any meteorological refraction (that is A_meteo = 0 and B_meteo = 0).
    //
    // 3) use real lenght (dd) of segment instead of horizontal projection (dx)

       
       double dd = dx ; // sqrt (dx * dx + dy * dy) ;

       int ndiv = (a_meteo == 0 && b_meteo == 0) ? 1 : (int) ceil (dd / maxLen) ;
       
       for (int j = 0 ; j < ndiv ; j++)
       {
          seg[nbSeg].x1 = x1 + j * dx / ndiv ;
          seg[nbSeg].y1 = y1 + j * dy / ndiv ;
          seg[nbSeg].x2 = x1 + (j + 1) * dx / ndiv ;
          seg[nbSeg].y2 = y1 + (j + 1) * dy / ndiv ;
          seg[nbSeg].ground = userSegment[i].ground ; 
          nbSeg++ ;

       // maximum number of segments exceeded, start again using larger steps
       
          if (nbSeg == maxSeg)
          {
              maxLen *= 2 ;
              goto start_again ;
          }
       }
       
       x1 = x2 ;
       y1 = y2 ;
   }   

// this is for testing only... do not use randomTerrain option in normal operation

   if (options & randomTerrain)
   {
      double random[MAX_SEG] ;
      double sigma = MIN (0.25, MIN (hSource, hReceiver)/2) ; 
   
      AUX_RandomGauss (nbSeg, random) ;
   
      for (int i = 1 ; i < nbSeg ; i++)
      {
         double d = seg[i].x1 / seg[nbSeg-1].x2 ;
       
         seg[i-1].y2 = seg[i].y1 = seg[i].y1 + 4 * sigma * random[i] * d * (1 - d) ;
      }      
   }
   
 // add "false" segment to store (x1, y1) at all points 0,1... nseg
 
    seg[nbSeg].x1 = seg[nbSeg-1].x2 ;
    seg[nbSeg].y1 = seg[nbSeg-1].y2 ;
}

// --------------------------------------------------------------------------------------------------------
// GetVertex : utility to get coordinates of segment points ;
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::GetVertex (int i, double &x, double &y)
{
    double yd = HSR_MIN ;
    
    if (i == 0)     yd = MAX (hSource,   HSR_MIN) ;
    if (i == nbSeg) yd = MAX (hReceiver, HSR_MIN) ;
    
    x = seg[i].x1 ;
    y = seg[i].y1 + yd ;
}

// --------------------------------------------------------------------------------------------------------
// do ground curvature based on equivalent lineair sound speed gradient
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

void PropagationPath::DoCurvature (void)
{
    R_meteo = 0 ;
  
    double dSR = seg[nbSeg-1].x2 - seg[0].x1 ;
    double hS  = hSource ; 
    double hR  = hReceiver ;
    double hM  = (hS + hR) / 2 ;
    double RA = 0 ;
    double RB = 0 ;
 
 // the linear part of the sound speed gradient 
 
    RA = a_meteo / c_sound ;
 
 // the logarithmic part of the sound speed gradient
 
 // modif DMV 20.01.2006 : separate determination for the cases b < 0 and b > 0
 // for b > 0 : use equivalent gradient integrated between source/receiver and the ray's turning point
 // for b < 0 : use equivelent gradient evaluated at hM = (hS + hR) / 2 
 
 // modif DMV 04.01.2007 : new algorithm to correct RB for higher sources & receivers
 // the effect is most observable when hS is very different from hR...
 
    if (b_meteo != 0)
    {
        if (b_meteo > 0)
        {  
            double tg = (hS - hR) / dSR ;
            double dd = DIST (dSR, hS - hR) ;
            double k  = sqrt (b_meteo / (2 * PI * c_sound)) ;
            double G  = (1 + 4 * k * k) / ( 1 - 4 * k * k) ;
            double A = 1 + tg * tg - G * G ;
            double B = hM * (1 + tg * tg) ;
            double C = B * hM + dd * dd / 4 ;
            double D = B * B - A * C ;
            if (D > 0)
            {
                RB = (sqrt (B * B - A * C) - B) / C ;
                if (RB < 0) RB = 0 ;
            }
            else
            {
                RB = 0 ;
            }
        }
        else
        {  
           RB = (b_meteo / c_sound) / hM ;
        }
    }
    
 // combine the ray curvatures according to 1/R = 1/RA + 1/RB
 
    R_meteo = RA + RB ;  

 // heuristic correction for displacement height...
 
    if (d_meteo > 0)
    {
        double hMax = hM + dSR * dSR * MAX (0.0, R_meteo) / 8 ;
        R_meteo *= exp (-d_meteo/hMax) ;
    }
    
 // avoid division by zero, ignore very small equivalent gradients 

    if (fabs(R_meteo) < 1.E-6) 
    {
        R_meteo = 0 ;
        return ;
    }

    p1.name = "R_meteo" ; p1.value = R_meteo ;
    
 // modif DVM 18.11.2005
 // limits of the model: R should be larger than D/2 in order for the conformal mapping to work!

    double R_min = 0.5 / dSR ;

 // before 2.019 : R_meteo = SIGN (R_meteo) * R_min * (1 - exp(-fabs(R_meteo)/R_min)) ;
  
 // modif DVM 28.11.2008, version 2.019
 // hard clipping on R_min instead of smoothing (as suggested by Erik Salomons)
   
    if (fabs(R_meteo) > R_min) R_meteo = SIGN(R_meteo) * R_min ;
    
 // modif DVM 02.05.2005 : ground curvature based on conformal mapping
 
    double xc = 0.5 * (seg[0].x1 + seg[nbSeg-1].x2) ;
    double yc = 0.5 * (seg[0].y1 + seg[nbSeg-1].y2) + hM;

    for (int i = 0 ; i <= nbSeg ; i++)
    {
        double  x = seg[i].x1 - xc ;
        double  y = seg[i].y1 - yc ;
        
        Complex z(x,y) ;       
        Complex c(0, 2 * (hM + 1/R_meteo)) ;

        Complex w = c * z / (z + c) ;
        
        seg[i].x1 = w.real() ;
        seg[i].y1 = w.imag() ;
        
        if (i > 0)
        {
            seg[i-1].x2 = seg[i].x1 ;
            seg[i-1].y2 = seg[i].y1 ;
        }
    }
}

// --------------------------------------------------------------------------------------------------------
// SetupAirAbsorption : setup air absoption coefficients as a function of temperature and humidity
//
// air absorption is calculated according to ISO 9613-1
//
// coefficients are evaluated at the center of the frequency bands
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// --------------------------------------------------------------------------------------------------------

const double defaultTEMP = 15. ; 
const double defaultHUM  = 70. ;

void PropagationPath::InitAirAbsorption (void)
{
    SetupAirAbsorption (defaultTEMP, defaultHUM) ;
}

void PropagationPath::SetupAirAbsorption (double temp, double hum)
{
   if (temp < -30) temp = -30 ;
   if (temp >  70) temp =  70 ;
   if (hum  <   0) hum  =   0 ;
   if (hum  > 100) hum  = 100 ;
   
   for (int j = 0 ; j < nbFreq ; j++)
   {
        double f    = freq[j] ; 
        double tref = 293.15 ;
        double tair = 273.15 + temp ;
        double tcor = tair/tref ;
        double xmol = hum * pow (10, 4.6151 - 6.834 * pow (273.16 / tair, 1.261));

        double frqO = 24 + 40400. * xmol * ((.02 + xmol) / (0.391 + xmol)) ;
        double frqN = pow (tcor,-0.5)
                    * (9 + 280 * xmol * exp (-4.17 * (pow (tcor,-1./3.) - 1.))) ;

        double a1 = 0.01275 * exp (-2239.1 / tair) / (frqO + (f * f / frqO)) ;
        double a2 = 0.10680 * exp (-3352.0 / tair) / (frqN + (f * f / frqN)) ;
        double a0 = 8.686 * f * f * (1.84e-11 * pow(tcor,0.5) + pow(tcor,-2.5) * (a1 + a2)) ;

        abs_air[j] = a0 ;
   }   
}

// ---------------------------------------------------------------------------------------------------------
// standard C interface
// 
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// ---------------------------------------------------------------------------------------------------------

// create / delete internal datastructrure

void* P2P_Create (void) 
{
    PropagationPath *path = new PropagationPath() ;   
    return (void *) path ;
}

void P2P_Delete (void* p2p_struct) 
{
    if (p2p_struct == 0) return ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    delete path ;
}

// info : return the current DLL version

double P2P_GetVersionDLL (void* p2p_struct) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    return path->majorID + 0.001 * path->minorID ;    
}

// get frequency range

int P2P_GetNbFreq (void* p2p_struct) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    return path->nbFreq ;    
}

double P2P_GetFreq (void* p2p_struct, int index) 
{
    if (p2p_struct == 0) return 0.0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    if (index < 0 || index >= path->nbFreq) return 0.0 ;
    return path->freq[index] ;
}

int P2P_GetFreqArray (void* p2p_struct, double *freq) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    int n = path->nbFreq ;
    
    for (int i = 0 ; i < n ; i++) freq[i] = path->freq[i] ;
    
    return n;
}

// setup user defined frequency range
// be carefull : P2P will automatically reset all frequency dependant values to their default values
// in particular : user defined impedances values and air absorption coefficients will be lost after 
// callling this function

int P2P_SetFreqArray (void* p2p_struct, int nb_freq, double *freq) 
{
    if (nb_freq < 1) return 0 ;
    
    if (p2p_struct == 0) return 0;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    int n = MIN (MAX_FREQ, nb_freq) ;
    for (int i = 0 ; i < n ; i++) path->freq[i] = freq[i] ;
    
    path->nbFreq = n ;
    path->InitImpedances() ;
    path->InitAirAbsorption ()  ;
    
    return n ;
}

// set/get the bandwidth

int P2P_SetBandwidth (void* p2p_struct, double bw) 
{
    if (p2p_struct == 0) return 0 ;
    if (bw <=0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    path->band_width = bw ;
    return 1 ;
}

double P2P_GetBandwidth (void* p2p_struct) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    return path->band_width ;
}


// set/get special calculations options

OPTIONS P2P_SetOptions (void* p2p_struct, OPTIONS mask) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    return (path->options = mask);
}

OPTIONS P2P_GetOptions (void* p2p_struct)
{
    if (p2p_struct == 0) return -1 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    return path->options ;
}

// set source and receiver heights
// source and receiver heights may be expressed as value +/- uncertainty
// the uncertainty will be used to reduce coherency between direct and ground reflected rays
//
// in normal mode, the source is assigned to the first segment point and the receiver to the last one
// when the "invRayTracing" option is set, the DLL will automatically assign the receiver to the first
// segment point and the source to the last one.

int P2P_SetSourceHeight (void* p2p_struct, double hs, double delta_hs) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    if (hs <= 0 || delta_hs < 0) return 0 ;
    
    if (path->options & invRayTracing) 
    {
        path->hReceiver = hs;
        path->delta_hReceiver = delta_hs;
    }
    else
    {
        path->hSource = hs;    
        path->delta_hSource = delta_hs;
    }
    return 1 ;
}

double P2P_GetSourceHeight (void* p2p_struct) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    return (path->options & invRayTracing) ? path->hReceiver : path->hSource ;
}

int P2P_SetReceiverHeight (void* p2p_struct, double hr, double delta_hr) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    if (hr <= 0 || delta_hr < 0) return 0 ;
    
    if (path->options & invRayTracing)
    {
        path->hSource = hr;
        path->delta_hSource = delta_hr ;
    }
    else
    {
        path->hReceiver = hr;    
        path->delta_hReceiver = delta_hr;    
    }
    return 1 ;
}

double P2P_GetReceiverHeight (void* p2p_struct) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    return (path->options & invRayTracing) ? path->hSource : path->hReceiver ;
}

// set/get the reference sound speed

int P2P_SetSoundSpeed (void* p2p_struct, double c0) 
{
    if (p2p_struct == 0) return 0 ;
    if (c0 < 300 || c0 > 350) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    path->c_sound = c0 ;
    return 1 ;
}

double P2P_GetSoundSpeed (void* p2p_struct) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    return path->c_sound ;
}

// setup air absorption

int P2P_SetupAirAbsorption (void* p2p_struct, double temp, double humidity)
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
 
    path->SetupAirAbsorption (temp, humidity) ;
    return 1 ;
}

int P2P_SetAirAbsorption (void *p2p_struct, double* abs_air)
{
    if (p2p_struct == 0) return 0;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    for (int i = 0 ; i < path->nbFreq ; i++) path->abs_air[i] = abs_air[i] ;
    
    return 1 ;
}

int P2P_GetAirAbsorption (void *p2p_struct, double* abs_air)
{
    if (p2p_struct == 0) return 0;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    for (int i = 0 ; i < path->nbFreq ; i++) abs_air[i] = path->abs_air[i] ;
    
    return 1 ;
}

// set/get sound speed profile parameters
//
//  A = lineair part of the sound speed profile (units : 1/s) 
//  B = logarithmic part of the sound speed profile (units : m/s)
//  C = turbulence strenght (Kolmogorov's structure parameter)
//  D = displacement height = height below which wind and temperature profiles vanish

int P2P_SetSoundSpeedProfile (void* p2p_struct, double A, double B, double C, double D) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    path->a_meteo = A ;
    path->b_meteo = B ;
    path->c_meteo = C ;
    path->d_meteo = D ;
    return 1 ;
}

int P2P_GetSoundSpeedProfile (void* p2p_struct, double *A, double *B, double *C, double *D) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    *A  = path->a_meteo  ;
    *B  = path->b_meteo  ;
    *C  = path->c_meteo  ;
    *D  = path->d_meteo  ;
    return 1 ;
}

// create/delete geometry of the ground and obstacles in the propagation plane

int P2P_Clear (void* p2p_struct) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    path->nbUserSegment = 0 ;
    path->nbSeg = 0 ;
    path->expand_distance = 1 ;
    return 1 ;
}

int P2P_AddSegment (void* p2p_struct, double x, double y, int impedance_model) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    int n = path->nbUserSegment ;
    
    double xmin = ((n == 0) ? 0 : path->userSegment[n-1].x) ;  
    xmin += DIST_MIN ;
    if (x < xmin) x = xmin ;
    
    path->userSegment[n].x = x ;
    path->userSegment[n].y = y ;
    path->userSegment[n].ground = MAX (0, MIN (impedance_model, path->maxImpedance));
    
    return (path->nbUserSegment = n+1) ;
}

// special function for line source integration (not for general use)

int P2P_SetDistanceFactor (void* p2p_struct, double factor) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    if (factor <= 0) return 0 ;
    
    path->expand_distance = factor ;
    return 1 ;
}

double P2P_GetDistanceFactor (void* p2p_struct) 
{
    if (p2p_struct == 0) return 0.0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;   
    return path->expand_distance ;
}

// user defined refinement of segment splitting

int P2P_SetSegmentSplitFactor (void* p2p_struct, int factor) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    if (factor < 1) return 0 ;
    
    path->userSplitSegment = factor ;
    return 1 ;
}

int P2P_GetSegmentSplitFactor (void* p2p_struct) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    return path->userSplitSegment ;
}
 
// setup user defined impedances using Delany and Bazley model

int P2P_SetImpedanceDB (void* p2p_struct, int index, double sigma, double thickness) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    if (index < groundUserDefined || index >= path->nbImpedance) return 0 ;
    
    int i = index ;
    
    path->impedance[i].model = true ;
    path->impedance[i].sigma = sigma ;
    path->impedance[i].thickness = thickness ;
        
    for (int j = 0 ; j < path->nbFreq ; j++) 
    {
       path->impedance[i].Z[j] = impedance_D_and_B (path->impedance[i].sigma, 
                                                   path->impedance[i].thickness, 
                                                   path->freq[j], 
                                                   path->c_sound) ;
    }
    
    return index ;
}

// setup user defined impedances (entering complex impedance values)

int P2P_SetImpedance (void* p2p_struct, int index, Complex *Z) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    if (index < groundUserDefined || index >= path->nbImpedance) return 0 ;
    
    int i = index ;
    
    path->impedance[i].model = false ;
    path->impedance[i].sigma = 0 ;
    path->impedance[i].thickness = 0 ;
        
    for (int j = 0 ; j < path->nbFreq ; j++) 
    {
       path->impedance[i].Z[j] = Z[j] ;
    }
    
    return index ;
}

// idem, using interlaced array of real values instead of complex

int P2P_SetImpedanceRI (void* p2p_struct, int index, double *Z) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    if (index < groundUserDefined || index >= path->nbImpedance) return 0 ;
    
    int i = index ;
    
    path->impedance[i].model = false ;
    path->impedance[i].sigma = 0 ;
    path->impedance[i].thickness = 0 ;
        
    for (int j = 0 ; j < path->nbFreq ; j++) 
    {
       path->impedance[i].Z[j] = Complex (Z[2*j], Z[2*j+1]) ;
    }
    
    return index ;
}

// read back impedances

int P2P_GetImpedanceDB (void* p2p_struct, int index, double *sigma, double *thickness) 
{
    *sigma = 0 ;
    *thickness = 0 ;
    
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    if (index < 0 || index > path->nbImpedance) return 0 ;
    
    if (!path->impedance[index].model) return 0 ;
    
    *sigma = path->impedance[index].sigma ;
    *thickness = path->impedance[index].thickness ;
        
    return 1 ;
}

int P2P_GetImpedance (void* p2p_struct, int index, Complex *Z) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    if (index < 0 || index > path->nbImpedance) return 0 ;
    
    for (int j = 0 ; j < path->nbFreq ; j++) 
    {
       Z[j] = path->impedance[index].Z[j] ;
    }
    
    return 1 ;
}

int P2P_GetImpedanceRI (void* p2p_struct, int index, double *Z) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    if (index < 0 || index > path->nbImpedance) return 0 ;
    
    for (int j = 0 ; j < path->nbFreq ; j++) 
    {
       Z[2*j  ] = path->impedance[index].Z[j].real() ;
       Z[2*j+1] = path->impedance[index].Z[j].imag() ;
    }
    
    return 1 ;
}

// do the calculations and get the results

int P2P_GetResults (void* p2p_struct, double *att) 
{  
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    time_t start = clock() ;  

    if (att != NULL)
    {
        for (int i = 0 ; i < path->nbFreq ; i++) att[i] = 0 ;
    }
    
    path->DoPointToPoint() ;
    
    if (att != NULL)
    {
        int npos = path->nbDetails-1 ;
        if (npos < 0) return 0 ;
       
        for (int i = 0 ; i < path->nbFreq ; i++)
        {
            att[i] = path->detail[npos].att[i] ;
        }
    }
    
    path->cpu_timer += (clock() - start) ;
    
    return 1 ;
}

// read back the equivalent ray curvature

double P2P_GetRayCurvature (void *p2p_struct)
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    return path->R_meteo ;    
}

// fix the equivalent ray curvature

double P2P_SetRayCurvature (void *p2p_struct, double invR)
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;
    
    path->R_meteo = invR ;
    path->a_meteo = path->c_sound * invR ;
    path->b_meteo = 0 ;
    
    return path->R_meteo ;    
}

// access to calculation details

int P2P_GetNbDetails (void *p2p_struct)
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    return path->nbDetails ;   
}

int P2P_GetDetails (void *p2p_struct, int index, int *model, int *pos_src, int *pos_dif, int *pos_rec, double *att) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    if (index < 0 || index >= path->nbDetails) return 0 ;
    
    if (model)    *model   = path->detail[index].model ;
    if (pos_src)  *pos_src = path->detail[index].pos_src ;
    if (pos_rec)  *pos_rec = path->detail[index].pos_rec ;
    if (pos_dif)  *pos_dif = path->detail[index].pos_dif ;
    
    if (att)
    {
        for (int i = 0 ; i < path->nbFreq ; i++) att[i] = path->detail[index].att[i] ;
    }
    
    return 1 ;
}

// info: get computation time of the calculation engine

double P2P_GetTimerCPU (void* p2p_struct, bool reset) 
{
    if (p2p_struct == 0) return 0.0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    double ret_val = (double) path->cpu_timer / (double) CLOCKS_PER_SEC ;
    
    if (reset) path->cpu_timer = 0 ;
    
    return ret_val ;
}

// ---------------------------------------------------------------------------------------------------------
// utility function: get averaged results over N meteorological conditions
//
// input :
//
//      nb_cond : number of conditions
//      rd      : value of D/R for each condition
//      wd      : weighting for each condition
//      
// output: averaged excess attenuation
//
// note: on output, GetDetails can be used to access each result separately
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// ---------------------------------------------------------------------------------------------------------

int P2P_GetAveragedResults (void* p2p_struct, int nb_cond, double *rd, double *wd, double *att) 
{
    int i,j,k  ;

    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    time_t start = clock() ;
    
 // default return value    
 
    if (att != NULL)
    {
        for (i = 0 ; i < path->nbFreq ; i++) att[i] = 0 ;
    }
    
 // check arguments
 
    if (nb_cond < 1 || rd == NULL || wd == NULL) return 0 ;
 
 // prepare for loop over propagation classes
 
    int n = path->nbUserSegment ;
    if (n < 1) return 0 ;
    double dist = path->userSegment[n-1].x ;

    double sum_wd = 0 ;
    for (i = 0 ; i < nb_cond ; i++) sum_wd += wd[i] ;
    if (sum_wd <= 0) return 0 ;
    
 // allocate result buffer
 
    double *res = (double *) calloc (nb_cond * path->nbFreq, sizeof(double)) ;
    if (res == NULL) return 0 ;
 
 // loop over equivalent ray curvaures
   
    for (i = 0, k = 0 ; i < nb_cond ; i++)
    {
        path->a_meteo = path->c_sound * rd[i] / dist ;
        path->b_meteo = 0 ;
    
        path->DoPointToPoint() ;
        
        int npos = path->nbDetails-1 ;
        if (npos < 0)
        {
            free (res) ;
            return 0 ;
        }
        
        for (j = 0 ; j < path->nbFreq ; j++)
        {
            res[k++] = path->detail[npos].att[j] ;
        }
    }        
 
 // copy results in detail buffer & free internal buffer
    
    path->nbDetails = nb_cond ;

    for (i = 0, k = 0 ; i < nb_cond ; i++)
    for (j = 0 ; j < path->nbFreq ; j++)
    {
       path->detail[i].att[j] = res[k++] ;
    }
    free (res) ;
 
 // calculate averaged result
 
    if (att != NULL)
    {
        for (i = 0 ; i < nb_cond ; i++)
        for (j = 0 ; j < path->nbFreq ; j++) 
        {
            att[j] += wd[i] * pow (10, 0.1 * path->detail[i].att[j]) ;
        }
        for (j = 0 ; j < path->nbFreq ; j++) 
        {
            att[j] = 10 * log10 (att[j] / sum_wd) ;
        }
    }

    path->cpu_timer += (clock() - start) ;
    
    return 1 ;
}

// ---------------------------------------------------------------------------------------------------------
// utility function ; convert wind speed, direction and stability class to sound speed profile parameters
//
// Copyright CSTB, 2002-2007, all rights reserved. Special conditions apply to members of the Harmonoise
// and Imagine consortium as stated in the Consortium's Agreements.
// ---------------------------------------------------------------------------------------------------------

static double Ustar[5] =
{
  0.00, 0.13, 0.30, 0.53, 0.87
} ;

static double Tstar[5][5] =
{
    { -0.40, -0.20, 0.00, 0.20, 0.30 },
    { -0.20, -0.10, 0.00, 0.10, 0.20 },
    { -0.10, -0.05, 0.00, 0.05, 0.10 },
    { -0.05,  0.00,	0.00, 0.00, 0.05 },
    {  0.00,  0.00, 0.00, 0.00,	0.00 }
} ;

static double invMO[5][5] =
{
   { -0.08, -0.05, 0.00, 0.04, 0.06 },
   { -0.05, -0.02, 0.00, 0.02, 0.04 },
   { -0.02, -0.01, 0.00, 0.01, 0.02 },
   { -0.01,	 0.00, 0.00, 0.00, 0.01 },
   {  0.00,  0.00, 0.00, 0.00, 0.00 }
} ;

int P2P_SetupMeteoParameters (void* p2p_struct, double windSpeed, double cosWind, int stabilityClass) 
{
    if (p2p_struct == 0) return 0 ;
    PropagationPath* path = (PropagationPath *) p2p_struct ;

    int iWind = 0 ;
    int iStab = stabilityClass ;
    
    if (windSpeed > 1) iWind = 1 ;
    if (windSpeed > 3) iWind = 2 ;
    if (windSpeed > 6) iWind = 3 ;
    if (windSpeed > 9) iWind = 4 ;

    double U = Ustar [iWind] ;
    double T = Tstar [iWind][iStab] ;
    double L = invMO [iWind][iStab] ;

    double k = 0.4 ;
    double g = 9.81 ;
    double Cp = 1005 ;
    double C0 = 331.4 ;
    double T0 = 273 ;
    double CC = 0.5 * C0 / T0 ;

    double CW = (iStab <= 2) ? 1.00 : 4.7 ;
    double CT = (iStab <= 2) ? 0.74 : 4.7 ;

    double AW = CW * U * cosWind * L / k ; 
    double BW = U * cosWind / k ;
    double AT = CC * (CT * T * L / k - g/Cp) ;
    double BT = CC * 0.74 * T / k ;

    path->a_meteo = AW + AT ;
    path->b_meteo = BW + BT ;
    
    return 1 ;
}

