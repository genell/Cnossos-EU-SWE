#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		SourceGeometry.h
 * version:		1.001
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: defines point, line, area and segment sources used in the calculation of the free
 *              field sound pressure level of the different source geometries.
 * changes:
 *
 *	28/11/2013	initial version 1.001
 *
 *  28/11/2013	the calculation of geometrical spread depends on the source's geometry and on the
 *              calculation method. We use the visitor pattern to handle the double dependency; i.e.
 *				the calculation method first calls the source geometry model which in turn calls
 *				the appropriate method in the calculation method, passing in the correct parameters
 *				for the specific source type.
 *
 *  02/12/2013	added support for evaluation of directivity in local coordinates
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "Geometry3D.h"
#include "ElementarySource.h"
#include "ReferenceObject.h"

namespace CnossosEU
{
	struct PropagationPath ;
	class CalculationMethod ;

	/*
	 * abstract base class for different source geometry types
	 */
	class SourceGeometry : public System::ReferenceObject
	{
	public:
		/*
		 * default constructor
		 */
		SourceGeometry (void) : System::ReferenceObject() { }
		/*
		 * virtual copy operator is undefined for the base class
		 */
		virtual SourceGeometry* clone (void) const = 0 ;
		/*
		 * dispatch evaluation of geometrical spread depending on source geometry
		 */
		virtual double GetGeometricalSpread (PropagationPath& path, CalculationMethod* method) = 0 ;
		/*
		 * type of spectrum associated with this source geometry
		 */
		virtual SpectrumType GetSpectrumType (void) = 0 ;
		/*
		 * construct local coordinate system
		 */
		virtual bool GetLocalCoordinateSystem (Geometry::Vector3D &x_axis, 
							                   Geometry::Vector3D &y_axis,
											   Geometry::Vector3D &z_axis) 	{ return false ; }
	};
	/*
	 * point source geometry requires no additional parameters
	 */
	class PointSource : public SourceGeometry
	{
	public:
		/*
		 * default constructor
		 */
		PointSource (void) : SourceGeometry() { }
		/*
		 * construct oriented point source
		 */
		PointSource (Geometry::Vector3D const& _orientation) 
		: SourceGeometry(), orientation(_orientation) {}
		/*
		 * implement virtual copy operator
		 */
		virtual PointSource* clone (void) const { return new PointSource(*this) ; }
		/*
		 * dispatch evaluation of geometrical spread depending on source geometry
		 */
		virtual double GetGeometricalSpread (PropagationPath& path, CalculationMethod *method) ;
		/*
		 * type of sound power spectrum associated with this source geometry
		 */
		virtual SpectrumType GetSpectrumType (void) { return SpectrumType::PointSource ; }
		/*
		 * construct local coordinate system : the X_axis is given by the normalized vector 
		 * orientation, the Y_axis is horizontal and perpendicular to the X_axis, the Z_axis 
		 * is perpendicular to both the X_axis and Y_axis and pointing upwards.
		 */
		virtual bool GetLocalCoordinateSystem (Geometry::Vector3D &x_axis, 
											   Geometry::Vector3D &y_axis,
											   Geometry::Vector3D &z_axis) ;
		/*
		 * store orientation of the source
		 */
		Geometry::Vector3D orientation ;
	};
	/*
	 * area source: the equivalent point source represents a radiation surface. Sound power is defined
	 * per unit surface. The total sound power emitted is a function of the surface area of the source.
	 */
	class AreaSource : public PointSource
	{
	public:
		/* default constructor */
		AreaSource (double _area = 1.0) : PointSource(), area(_area) { }
		/*
		 * construct oriented area source. Note that for an area source, the orientation is generally 
		 * taken equal to the normal to the surface.
		 */
		AreaSource (double _area, Geometry::Vector3D const& _orientation) 
		: PointSource(_orientation), area(_area) { }
		/*
		 *  implement virtual copy operator
		 */
		virtual AreaSource* clone (void) const { return new AreaSource(*this) ; }
		/*
		 * dispatch evaluation of geometrical spread depending on source geometry
		 */
		virtual double GetGeometricalSpread (PropagationPath& path, CalculationMethod *method) ;
		/*
 		 * type of sound power spectrum associated with this source geometry
		 */
		virtual SpectrumType GetSpectrumType (void) { return SpectrumType::AreaSource ; }
		/*
		 * an area source is actually an equivalent point source representative of the radiation 
		 * from a surface area. The extent of this surface is stored as an area in squared meters.
		 */
		double area ;
	};
	/*
	 * line source: the equivalent point source represents an incoherent line of sources of finite length. 
	 * Sound power is defined per unit length. The total sound power emitted is a function of the length 
	 * of the segment.
	 */
	class LineSource : public PointSource
	{
	public:
		/*
		 * default constructor
		 */
		LineSource (double _length = 1.0) : PointSource(), length (_length) { }
		/*
		* construct oriented line source. Note that a line source is actually an equivalent point source 
		* representing of the radiation of a line source segment of finite length; the orientation is 
		* generally aligned with the straight line supporting the finite source line segment.
	    */
		LineSource (double _length, Geometry::Vector3D const& _orientation) 
		: PointSource (_orientation), length (_length) { }
		/*
		 * implement virtual copy operator
		 */
		virtual LineSource* clone (void) const { return new LineSource(*this) ; }
		/*
		 * dispatch evaluation of geometrical spread depending on source geometry
		 */
		virtual double GetGeometricalSpread (PropagationPath& path, CalculationMethod *method) ;
		/*
 		 * type of sound power spectrum associated with this source geometry
		 */
		virtual SpectrumType GetSpectrumType (void) { return SpectrumType::LineSource ; }
		/*
		 * local coordinate system : for a line source, the Y_axis is generally aligned with the line
		 * segment supporting the source, the X_axis is horizontal and perpendicular to the Y_axis, 
		 * the Z_axis is perpendicular to both the X_axis and Y_axis and pointing upwards.
		 */
		virtual bool GetLocalCoordinateSystem (Geometry::Vector3D &x_axis, 
											   Geometry::Vector3D &y_axis,
										       Geometry::Vector3D &z_axis) ;
		/*
		 * store the length of the segment
		 */
		double length ;
	};
	/*
	 * line segment source: the equivalent point source represents an incoherent line of source with 
	 * known geometry. Sound power is expressed per unit length. As the geometry is known, integration
	 * of the geometrical spread can be carried out analytically. This leads to higher accuracy, 
	 * especially if the angle of view of the segment from the receiver's point of view is larger than
	 * a few degrees. Note that this model supports the "implicit angle of view" typically used in
	 * case of discrete (inverse) ray-tracing with fixed angular resolution.
	 */
	class LineSegment : public SourceGeometry
	{
	public:
		/*
		 * default constructor
		 */
		LineSegment (void) : SourceGeometry(), p1(), p2(), fixed_angle(1.0) { } 
		/*
		 * construct a segment source defined by its endpoints
		 */
		LineSegment (Geometry::Point3D const& start, Geometry::Point3D const& end, double _fixedAngle = 0.0) 
		: SourceGeometry(), p1(start), p2(end), fixed_angle (_fixedAngle) { } 
		/*
		 * implement virtual copy operation
		 */
		virtual LineSegment* clone (void) const { return new LineSegment(*this) ; }
		/*
		 * dispatch evaluation of geometrical spread depending on source geometry
		 */
		virtual double GetGeometricalSpread (PropagationPath& path, CalculationMethod *method) ;
		/*
 		 * type of sound power spectrum associated with this source geometry
		 */
		virtual SpectrumType GetSpectrumType (void) { return SpectrumType::LineSource ; }
		/*
		 * local coordinate system : for a line source, the Y_axis is generally aligned with the line
		 * segment supporting the source, the X_axis is horizontal and perpendicular to the Y_axis, 
		 * the Z_axis is perpendicular to both the X_axis and Y_axis and pointing upwards.
		 */
		virtual bool GetLocalCoordinateSystem (Geometry::Vector3D &x_axis, 
								 			   Geometry::Vector3D &y_axis,
											   Geometry::Vector3D &z_axis) ;
		/*
		 * store the segment's end-points
		 */
		Geometry::Point3D p1 ;
		Geometry::Point3D p2 ;
		/* 
		 * fixed angle of view associated with the propagation path;  if zero, the angle of view
		 * is adjusted to correspond to the total length of the segment
		 */
		double  fixed_angle ;
	};
}