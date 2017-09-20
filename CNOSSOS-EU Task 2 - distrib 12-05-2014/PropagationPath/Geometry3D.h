#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		Geometry3D.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.CSTB.txt
 * description: implements basic operations on 2,3 and 4-d positions and vectors  
 * changes:
 *
 *	18/01/2013	this notice added to original file from CSTB
 * ------------------------------------------------------------------------------------------------- 
 */
#include <cmath>
#include <vector>

namespace Geometry
{
	// forward declarations
	struct Point4D ;
	struct EulerAngles ;

	// position in 2D
	struct Point2D
	{
		union
		{
			double coord[2] ;
			struct  
			{
				double x ;
				double y ;
			};
		};
		Point2D (void) : x(0), y(0) { }
		Point2D (double _x, double _y) : x(_x) , y(_y) { }
	};

	// position in 3D
	struct Point3D : public Point2D
	{
		double z ;
		Point3D (void) : Point2D(), z(0) { }
		Point3D (double _x, double _y, double _z) : Point2D (_x, _y), z(_z) { }
		Point3D (Point4D const& p, bool z_plus_h = false) ;
	};

	// position in 4D
	struct Point4D : public Point3D
	{
		double h ;
		Point4D (void) : Point3D(), h(0) { }
		Point4D (double _x, double _y, double _z, double _h) : Point3D (_x, _y, _z), h(_h) { }
	};

	// construct a 3D position from 4D data, use z or z+h as third coordinate
	inline Point3D::Point3D (Point4D const& p, bool z_plus_h)
	{
		x = p.x ;
		y = p.y ;
		z = p.z ;
		if (z_plus_h) z += p.h ;
	}

	// vector in 3D
	// note that operations such as sum, scalar, dot and cross product are defined for vectors, 
	// not for affine positions
	struct Vector3D : public Point3D
	{
		Vector3D () { }
		Vector3D (Point3D const& p) : Point3D(p) { }
		Vector3D (Point3D const& p1, Point3D const& p2)
		{
			x = p2.x - p1.x ;
			y = p2.y - p1.y ;
			z = p2.z - p1.z ;
		}
		Vector3D (double _x, double _y, double _z) : Point3D (_x,_y,_z)	{ }
		Vector3D (EulerAngles const& euler) ;
	};

	// construct vector from affine positions
	inline Vector3D operator- (Point3D const& p, Point3D const& q)
	{
		return Vector3D(q, p) ;
	}

	// sum of of two 3D vectors
	inline Vector3D operator+ (Vector3D const& p, Vector3D const& q)
	{
		return Vector3D (p.x + q.x, p.y + q.y, p.z + q.z) ;
	}

	// difference of two 3D vectors
	inline Vector3D operator- (Vector3D const& p, Vector3D const& q)
	{
		return Vector3D (q, p) ;
	}

	// Position + Vector (displacement) = Position
	inline Point3D operator+ (Point3D const& p, Vector3D const& q)
	{
		return Vector3D (p.x + q.x, p.y + q.y, p.z + q.z) ;
	}
	
	// dot-product of two 3D vectors
	inline double operator* (Vector3D const& p, Vector3D const& q)
	{
		return p.x * q.x + p.y * q.y + p.z * q.z ;
	}

	// cross-product of two 3D vectors
	inline Vector3D operator^ (Vector3D const& p, Vector3D const& q)
	{
		double x = p.y * q.z - p.z * q.y ;
		double y = p.z * q.x - p.x * q.z ;
		double z = p.x * q.y - p.y * q.x ;
		return Vector3D (x, y, z) ;
	}

	// scalar-product of a 3D vectors and a real number
	inline Vector3D operator* (Vector3D const& p, double const& q)
	{
		return Vector3D (q*p.x, q*p.y, q*p.z) ;
	}

	// scalar-product of a 3D vectors and a real number
	inline Vector3D operator* (double const& q, Vector3D const& p)
	{
		return Vector3D (q*p.x, q*p.y, q*p.z) ;
	}

	// scalar-product of a 3D vectors and a real number
	inline Vector3D operator/ (Vector3D const& p, double const& q)
	{
		return p * (1/q) ;
	}

	inline double norm2 (Vector3D const& p)
	{
		return p * p ;
	}

	inline double norm (Vector3D const& p)
	{
		return sqrt (norm2(p)) ;
	}

	inline double dist2 (Point3D const& p, Point3D const& q)
	{
		return norm2 (p - q) ;
	}

	inline double dist (Point3D const& p, Point3D const& q)
	{
		return norm (p - q) ;
	}

	// polyline in 3D
	typedef std::vector<Point3D> Polyline ;

	// vector in 2D
	struct Vector2D : public Point2D
	{
		Vector2D () { }
		Vector2D (Point2D const& p) : Point2D(p) { }
		Vector2D (Point2D const& p1, Point2D const& p2)
		{
			x = p2.x - p1.x ;
			y = p2.y - p1.y ;
		}
		Vector2D (double _x, double _y) : Point2D (_x,_y)	{ }
	};

	// construct vector from affine positions
	inline Vector2D operator- (Point2D const& p, Point2D const& q)
	{
		return Vector2D(q, p) ;
	}

	// sum of of two 3D vectors
	inline Vector2D operator+ (Vector2D const& p, Vector2D const& q)
	{
		return Vector2D (p.x + q.x, p.y + q.y) ;
	}

	// difference of two 3D vectors
	inline Vector2D operator- (Vector2D const& p, Vector2D const& q)
	{
		return Vector2D (q, p) ;
	}

	// Position + Vector (displacement) = Position
	inline Point2D operator+ (Point2D const& p, Vector2D const& q)
	{
		return Vector2D (p.x + q.x, p.y + q.y) ;
	}

	// dot-product of two 2D vectors
	inline double operator* (Vector2D const& p, Vector2D const& q)
	{
		return p.x * q.x + p.y * q.y ;
	}

	// cross-product of two 2D vectors
	inline double operator^ (Vector2D const& p, Vector2D const& q)
	{
		return p.x * q.y - p.y * q.x ;
	}

	// scalar-product of a 2D vectors and a real number
	inline Vector2D operator* (Vector2D const& p, double const& q)
	{
		return Vector2D (q*p.x, q*p.y) ;
	}

	// scalar-product of a 2D vectors and a real number
	inline Vector2D operator* (double const& q, Vector2D const& p)
	{
		return Vector2D (q*p.x, q*p.y) ;
	}

	// scalar-product of a 3D vectors and a real number
	inline Vector2D operator/ (Vector2D const& p, double const& q)
	{
		return p * (1/q) ;
	}

	// squared norm of a 2D vector
	inline double norm2 (Vector2D const& p)
	{
		return p * p ;
	}

	// norm of a 2D vector
	inline double norm (Vector2D const& p)
	{
		return sqrt (norm2(p)) ;
	}

	// distance between 2 points in 2D
	inline double dist2 (Point2D const& p, Point2D const& q)
	{
		return norm2 (p - q) ;
	}

	inline double dist (Point2D const& p, Point2D const& q)
	{
		return norm (p - q) ;
	}

	// representation of a normalized 3D vector by means of Euler angles
	// the azimuth angle is measured in the XY plane with zero on the positive X axis, positive in the 
	// direction of the positive Y axis ;the elevation is the angle between the vector and the XY plane 
	struct EulerAngles 
	{
		double azimuth ;
		double elevation ;

		EulerAngles (double _azimuth = 0, double _elevation = 0) : azimuth (_azimuth), elevation(_elevation) {} 
		EulerAngles (Vector3D const& v) ;
	};

	inline Vector3D::Vector3D (EulerAngles const& euler)
	{
		x = cos (euler.azimuth) * cos (euler.elevation) ;
		y = sin (euler.azimuth) * cos (euler.elevation) ;
		z = sin (euler.elevation) ;
	}

	inline EulerAngles::EulerAngles (Vector3D const& v)
	{
		double n = norm(v) ;
		/*
		 * atan2 return the angle defined by the vector (x,y) in 2D and the positive X axis ; 
		 * returned values are in the range [-PI, +PI]
		 * note that atan2 is undefined if x = y = 0 and that behavior may depend on the control
		 * parameters of the floating-point calculation unit (see fp_control function).
		 */
		azimuth = atan2 (v.y, v.x) ;
		/*
		 * asin returns the angle defined by the value of sinus ; returned values are  in the 
		 * range [-PI/2, +PI/2]
		 * note that the results is undefined if n = 0 ; i.e. if x = y = z = 0 and that behavior
		 * may depend on the control parameters of the floating point calculation unit
		 */
		elevation = asin (v.z/n) ;
	}
	/*
	 * Check whether a polygon is convex or not.
	 * Method: the surface of any three successive points on the contour must either 
	 * be zero or all surfaces have the same sign (positive in case the contour is
	 * counter-clockwise, negative in case the polygon is oriented clockwise).
	 * Note that the polygon does not need to be explicitly closed...
	 */
	inline bool is_convex (std::vector<Point2D> const& pos)
	{
		if (pos.size() <= 3) return true ;
		unsigned int n1 = 0 ;
		unsigned int n2 = pos.size()-1 ;
		double cos_sign = 0 ;
		for (unsigned int i = n1 ; i <= n2 ; ++i)
		{
			unsigned int i1 = (i==n1) ? n2 : i-1 ;
			unsigned int i2 = (i==n2) ? n1 : i+1 ;
			double cos_angle = (pos[i1] - pos[i]) ^ (pos[i2] - pos[i]) ;
			if (cos_sign == 0 && cos_angle != 0)
			{
				cos_sign = cos_angle ;
			}
			else
			{
				if (cos_angle * cos_sign < 0) return false ;
			}
		}
		return true ;
	}
}
