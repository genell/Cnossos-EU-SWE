#pragma once
/* ------------------------------------------------------------------------------------------------
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
#include "Geometry3D.h"

namespace CnossosEU
{
	using namespace Geometry ;

	struct MeanPlane
	{
		Point2D  origin ;
		Vector2D x_axis ;
		Vector2D y_axis ;

		MeanPlane (void) { } ; 
		/*
		 * construct mean plane from terrain profile positions in the vertical plane
		 */
		MeanPlane (std::vector<Point2D> const& profile) ;
		/*
		 * project a position on the mean plane, i.e. transforms a point given in (D,Z)
		 * coordinates relative to the propagation plane into (U,V) coordinates relative
		 * to the mean plane
		 */
		Vector2D project (Point2D const& p, bool no_negative_z = true)
		{
			Vector2D v = p - origin ;
			v = Vector2D (v*x_axis, v*y_axis) ;
			if (no_negative_z && v.y < 0) v.y = 0 ;
			return v ;
		}
		/*
		 * reverse projection of a position on the mean plane ; i.e. transforms back a point
		 * with (U,V) coordinates relative to the mean plane into coordinates (D,Z) relative
		 * to the propagation plane.
		 */
		Point2D unproject (Vector2D &v)
		{
			return origin + x_axis * v.x + y_axis * v.y ;
		}
		/* 
		 * take the image of a point relative to the mean plane
		 */
		Point2D image (Point2D const& p, bool no_negative_z = true)
		{
			Vector2D pi = project (p, no_negative_z) ;
			pi.y = - pi.y ;
			return unproject(pi) ;
		}
	};
}
