/* 
 * ------------------------------------------------------------------------------------------------
 * file:		MeanPlane.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: evaluate parametric representation of mean ground planes according to JRC-2012 
 * changes:
 *
 *	12/11/2013	initial version
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "MeanPlane.h"
#include "ErrorMessage.h"
#include <assert.h>
#include <stdio.h>

using namespace CnossosEU ;
/*
 * calculate the mean plane y = A.x + B for a sequence of terrain points projected on the 
 * unfolded propagation plane ; using (x,y) coordinates as in section  VI.2.2.c of the
 * JRC-2012 reference report.
 */
static void getMeanPLaneCoefficients (std::vector<Point2D> const& profile, double& A, double&B) 
{
	unsigned int n = profile.size()-1 ;
	double valA1 = 0;
	double valA2 = 0;
	double valB1 = 0;
	double valB2 = 0;
	/*
	 * equation VI-3
	 */
	for (int unsigned i = 0 ; i < n ; i++)
	{
		Point2D const& p1 = profile[i] ;
		Point2D const& p2 = profile[i+1] ;
		double dx = p2.x - p1.x ;
		if (dx != 0)
		{
			double ai = (p2.y - p1.y) / dx;
			double bi = p1.y - ai * p1.x;
			double vald2 = pow (p2.x, 2) - pow (p1.x, 2);
			double vald3 = pow (p2.x, 3) - pow (p1.x, 3);
			valA1 += ai * vald3 ;
			valA2 += bi * vald2;
			valB1 += ai * vald2;
			valB2 += bi * dx;
		}			
	}
	double valA = 2/3. * valA1 + valA2;
	double valB = valB1 + 2 * valB2;
	double dist3 = pow (profile[n].x - profile[0].x, 3) ;
	double dist4 = pow (profile[n].x - profile[0].x, 4) ;
	assert (dist3 > 0) ;
	assert (dist4 > 0) ;
	/*
	 * equation VI-4
	 */
	A = 3 * (2 * valA - valB * (profile[n].x + profile[0].x)) / dist3 ;
	B = 2 * valB * (pow(profile[n].x, 3) - pow(profile[0].x, 3)) / dist4 
	  - 3 * valA * (profile[n].x + profile[0].x) / dist3; 
}
/*
 * the reference document describes how to represent the mean plane by means of an equation 
 * z = A.d + B where d and z represent coordinates in the unfolded propagation plane.
 *
 * it is much easier however to carry out geometrical operations involving the mean plane 
 * by means of a parametric vector representation of the form P(d,z) = O(Od,Oz) + t.N(Nd,Nz) 
 * with Od = B, Oz = 0, Nd = A, Nz = 1.
 *
 * to make things even easier, we can normalize the vector N so that the parameter t becomes the 
 * Euclidean distance measured along the mean plane.
 */
MeanPlane::MeanPlane (std::vector<Point2D> const& profile) 
{
	double A, B ;
	getMeanPLaneCoefficients (profile, A, B) ;
	print_debug ("y = Ax + B with A=%.5f B=%.5f \n", A, B) ;
	origin = Point2D (0,B) ;
	x_axis = Vector2D (1,A) ; 
	x_axis = x_axis / norm (x_axis) ;
	y_axis = Vector2D (-x_axis.y, x_axis.x) ;
}
