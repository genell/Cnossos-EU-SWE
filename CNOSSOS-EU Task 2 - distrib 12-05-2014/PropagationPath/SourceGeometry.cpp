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
 * ------------------------------------------------------------------------------------------------- 
 */
#include "SourceGeometry.h"
#include "PropagationPath.h"
#include "CalculationMethod.h"

using namespace CnossosEU ;
using namespace Geometry ;
/*
 * calculate geometrical spread for a point source
 */
double PointSource::GetGeometricalSpread (PropagationPath& path, CalculationMethod* method)
{
	return method->getGeometricalSpreadUnitSource (path, 1.0) ;
}
/*
 * calculate geometrical spread for a line source segment of known length
 */
double LineSource::GetGeometricalSpread (PropagationPath& path, CalculationMethod* method)
{
	return method->getGeometricalSpreadUnitSource (path, length) ;
}
/*
 * calculate geometrical spread for an area source of known surface area
 */
double AreaSource::GetGeometricalSpread (PropagationPath& path, CalculationMethod* method)
{
	return method->getGeometricalSpreadUnitSource (path, area) ;
}
/*
 * calculate geometrical spread for a line source segment known by its end-points. If the
 * angular resolution is set to zero, the receiver will see the whole segment, otherwise,
 * it is assumed that the receiver sees the segment under a fixed opening angle with the
 * propagation plane as its bisector.
 */
double LineSegment::GetGeometricalSpread (PropagationPath& path, CalculationMethod* method)
{
	return method->getGeometricalSpreadLineSource (path, p1, p2, fixed_angle) ;
}
/* construct the local coordinate system associated with a point source
 *
 * the X_axis is given by the user-defined vector orientation, the Y_axis is horizontal and perpendicular 
 * to the X_axis, the Z_axis is perpendicular  to both the X_axis and Y_axis and pointing upwards.
 */
bool PointSource::GetLocalCoordinateSystem (Geometry::Vector3D &x_axis, 
											Geometry::Vector3D &y_axis,
											Geometry::Vector3D &z_axis)
{
	double n = norm (orientation) ;
	if (n == 0) return false ;
	/*
	 * x_axis is aligned with user-defined orientation
	 */
	x_axis = orientation / n ;
	/*
	 * y_axis is horizontal and perpendicular to the x_axis
	 */
	z_axis = Vector3D (0.0, 0.0, 1.0) ;
	y_axis = z_axis ^ x_axis ;
	/*
	 * z_axis is perpendicular to both the x_axis and the y_axis
	 */
	z_axis = x_axis ^ y_axis ;
	assert (z_axis.z >= 0) ;
	return true ;
}
/*
 * construct the local coordinate system associated with a line source
 *
 * for a line source, the Y_axis is generally aligned with the line segment supporting the source, 
 * the X_axis is horizontal and perpendicular to the Y_axis,  the Z_axis is perpendicular to both 
 * the X_axis and Y_axis and pointing upwards.
 */
bool LineSource::GetLocalCoordinateSystem (Geometry::Vector3D &x_axis, 
										   Geometry::Vector3D &y_axis,
										   Geometry::Vector3D &z_axis)
{
	double n = norm (orientation) ;
	if (n == 0) return false ;
	/*
	 * y_axis is aligned with the source segment
	 */
	y_axis = orientation / n ;
	/*
	 * x_axis is horizontal and perpendicular to the x_axis
	 */
	z_axis = Vector3D (0.0, 0.0, 1.0) ;
	x_axis = y_axis ^ z_axis ;
	/*
	 * z_axis is perpendicular to both the x_axis and the y_axis
	 */
	z_axis = x_axis ^ y_axis ;
	assert (z_axis.z >= 0) ;
	return true ;
}
/*
 * construct the local coordinate system for a line segment source
 *
 * the Y_axis is aligned with the line segment, the X_axis is horizontal and  perpendicular to the Y_axis, 
 * the Z_axis is perpendicular to both the X_axis and Y_axis and pointing upwards.
 */
bool LineSegment::GetLocalCoordinateSystem (Geometry::Vector3D &x_axis, 
										    Geometry::Vector3D &y_axis,
										    Geometry::Vector3D &z_axis)
{
	LineSource src ;
	src.orientation = p2 - p1 ;
	return src.GetLocalCoordinateSystem (x_axis, y_axis, z_axis) ; 
}
