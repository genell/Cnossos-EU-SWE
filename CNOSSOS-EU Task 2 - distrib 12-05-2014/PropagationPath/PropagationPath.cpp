/* 
 * ------------------------------------------------------------------------------------------------
 * file:		PropagationPath.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implement geometrical analysis and validation of propagation path
 * note:		geometrical analysis and validation is a mandatory step before addressing the 
 *			    acoustical calculation engines
 * changes:
 *
 *	18/01/2013	initial version
  * ------------------------------------------------------------------------------------------------- 
 */
#include "PropagationPath.h"
#include "VerticalExt.h"
#include "Material.h"
#include "ErrorMessage.h"

#ifdef __GNUC__
#define sprintf_s sprintf
#endif

using namespace CnossosEU ;
using namespace Geometry ;
/*
 * utility: signal and/or return error conditions
 */
#define return_error(x) { signal_error (ErrorMessage(x)) ; return false ; }
/*
 * local buffer for creating error messages
 */
static char error_buffer[1024] ;
/*
 * information on vertical extensions associated with control points
 */
bool ControlPoint::isSource (void) 
{
	return ext && ext->isSource() ;
}
bool ControlPoint::isReceiver (void)
{
	return ext && ext->isReceiver() ;
}
bool ControlPoint::isVerticalWall (void)
{
	return ext && ext->isVerticalWall() ;
}
bool ControlPoint::isVerticalEdge (void)
{
	return ext && ext->isVerticalEdge() ;
}
bool ControlPoint::isBarrier (void)
{
	return ext && ext->isBarrier() ;
}
/*
 * geometrical analysis and validation of a propagation path
 */
bool PropagationPath::analyze_path (PropagationPathOptions const& options)
{
	if (! remove_barriers()) return false ;
	if (! check_source_receiver (options)) return false ;
	if (! setup_horizontal_path (options)) return false ;
	if (! setup_vertical_path (options)) return false ;
	return true ;
}
/*
 * utility function: remove barriers and replace them by additional control points
 */
bool PropagationPath::remove_barriers (void)
{
	for (unsigned int i = 0 ; i < size() ; ++i)
	{
		if (cp[i].isBarrier())
		{
			/*
			 * remove the barrier (but keep a local reference so it won't be deleted
			 */
			System::ref_ptr<BarrierExt> barrier = cp[i].ext.cast_to_ptr<BarrierExt>() ;
			assert (barrier != 0) ;
			cp[i].ext = 0 ;
			/*
			 * insert two extra control points
			 */
			int n = cp.size() ;
			cp.resize (n+2) ;
			for (unsigned int j = n+1 ; j > i+1 ; --j) cp[j] = cp[j-2] ; 
			/*
			 * insert top of barrier 
			 */
			ControlPoint& p0 = cp[i] ; 
			ControlPoint& p1 = cp[++i] = p0 ; 
			p1.pos.z += barrier->h ;
			p1.mat = barrier->mat ;
			/*
			 * insert ground point behind barrier
		     */
			ControlPoint& p2 = cp[++i] = p0 ;
			p2.mat = barrier->mat ;
		}
	}
	return true ;
}
/*
 * utility function: reverse the path 
 *
 * note that some calculation methods are not reciprocal and may require that the source 
 * is stored as the first segment point and the receiver as the last.
 */
bool PropagationPath::reverse_path (void)
{
	if (cp.size() < 2) return false ;

	for (unsigned int i = 0 ; i < size()/2 ; ++i)
	{
		unsigned int i1 = i ;
		unsigned int i2 = size()-1-i ;
		std::swap (cp[i1], cp[i2]) ;
	}

	for (unsigned int i = size()-1 ; i > 0 ; --i)
	{
		cp[i].mat = cp[i-1].mat ;
	}

	return true ;
}
/*
 * requirement: source and receiver must be associated with first, respectively last control point
 *
 * option: if ForceSourceToReceiver is specified, reverse the path if necessary
 */
bool PropagationPath::check_source_receiver (PropagationPathOptions const& options)
{
	/* 
	 * a valid path has at least two control points
	 */
	if (cp.size() < 2) 
	{
		return_error ("A propagation path must contain at least 2 control points") ; 
	}
	/*
	 * path must be either in source-to-receiver order or in receiver-to-source order
	 */
	unsigned int n1 = 0 ;
	unsigned int n2 = cp.size() - 1 ;
	if (cp[n1].isSource())
	{
		if (! cp[n2].isReceiver())
		{
			return_error ("Path starting at Source must end at Receiver\n")  ;
		}
	}
	else if (cp[n1].isReceiver())
	{
		if (! cp[n2].isSource()) 
		{
			return_error ("Path starting at Receiver must end at Source\n") ;
		}
		/*
		 * if path is in receiver-to-source order and the ForceSourceToReceiver 
		 * option is set, reverse the path
		 */
		if (options.ForceSourceToReceiver) 
		{
			reverse_path() ;
			assert (cp[n1].isSource()) ;
			assert (cp[n2].isReceiver()) ;
		}
	}
	else
	{
		return_error ("Path doesn't start with Source or Receiver\n") ;
	}
	/*
	 * additional checking on source geometry 
	 */
	unsigned int posSource = cp[n1].isSource() ? n1 : n2 ;
	SourceExt* src = dynamic_cast<SourceExt*> (cp[posSource].ext.get()) ;
	assert (src != 0) ;
	if (src->geo)
	{
		/*
		 * check units versus geometry, e.g. sound power must be in dB/m for a line source
		 * and in dB/m� for an area source.
		 */
		if (options.CheckSoundPowerUnits)
		{
			if (src->source.spectrumType != SpectrumType::Undefined &&
				src->source.spectrumType != src->geo->GetSpectrumType())
			{
				return_error ("Sound power units do not match source geometry") ;
			}
		}
		/*
		 * check position of the equivalent point source with respect to the source line segment
		 */
		LineSegment* seg = src->geo.cast_to_ptr<LineSegment> () ;
		if (seg != 0 && options.CheckSourceSegment)
		{
			Point3D p0 = cp[posSource].pos ;
			Vector3D p1 = seg->p1 - p0 ;
			Vector3D p2 = seg->p2 - p0 ;
			/*
				* the control point is almost aligned with and inside the segment if the triangle 
				* formed by the three position has a top angle of almost 180�
				*/
			double sin_alpha = norm (p1^p2) ;
			double cos_alpha = p1*p2 ;
			if (cos_alpha >= 0 || fabs(sin_alpha/cos_alpha) > 0.01)
			{
				return_error ("Source position not aligned with source segment") ;
			}
		}
	}
	/*
	 * mark source and receiver positions in the horizontal plane
	 */
	if (cp[n1].isSource())
	{
		cp[n1].mode2D = Action2D::Source ;
		cp[n2].mode2D = Action2D::Receiver ;
	}
	else
	{
		cp[n1].mode2D = Action2D::Receiver ;
		cp[n2].mode2D = Action2D::Source ;
	}
	return true ;
}
/*
 * check straight line condition, i.e. in between two successive critical points, the projection
 * of the propagation path on the horizontal plane should represent a straight line segment
 */
bool PropagationPath::check_horizontal_alignment (unsigned int n1, unsigned int n2)
{
	Geometry::Point2D  p_start (cp[n1].pos) ;
	Geometry::Point2D  p_end (cp[n2].pos) ;

	Geometry::Point2D  O = p_start ;
	Geometry::Vector2D N = p_end - p_start ;
	double d = norm(N) ; N = N/d ;
	double d_prev = 0 ;

	for (unsigned int i = n1+1 ; i < n2 ; ++i)
	{
		Geometry::Vector2D   P = cp[i].pos - O ;
		double u = P*N ;
		double v = P^N ;
		double c = fabs(v) / sqrt (u*(d-u)) ;

		if (c > 0.005)
		{
			sprintf_s (error_buffer, "Control point %d (x=%.2f, y=%.2f) is not aligned with segment\n", i, P.x, P.y) ;
			return_error (error_buffer) ;
		}

		if (u < d_prev || u > d)
		{
			sprintf_s (error_buffer, "Control point %d (x=%.2f, y=%.2f) is not in increasing order of distance\n", i, P.x, P.y) ;
			return_error (error_buffer) ;
		}
		d_prev = u ;
	}
	return true ;
}
/*
 * check straight line condition, i.e. in between two successive critical points, the projection
 * of the propagation path on the horizontal plane should represent a straight line segment
 */
bool PropagationPath::check_horizontal_alignment (void)
{
	unsigned int prev = 0 ;
	
	for (unsigned int next = 1 ; next < cp.size() ; ++next)
	{
		if (cp[next].mode2D == Action2D::None) continue ;
		if (!check_horizontal_alignment (prev, next)) return false ;
		prev = next ;
	}
	return true ;
}
/*
 * check for valid paths containing at least one lateral diffraction
 */
bool PropagationPath::check_lateral_diffraction (void)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = cp.size()-1 ;
	std::vector<Geometry::Point2D> contour ;
	for (unsigned int i = n1 ; i <= n2 ; ++i)
	{
		if (i == n1 || i == n2 || cp[i].mode2D == Action2D::Diffraction) contour.push_back (cp[i].pos) ;
	}
	if (!is_convex (contour)) return_error ("Invalid laterally diffracted path (not convex)") ;
	return true ;
}
/*
 * setup and check the projection of the propagation path onto a horizontal plane
 */
bool PropagationPath::setup_horizontal_path (PropagationPathOptions const& options)
{
	unsigned int& nbRefl = info.nbReflections = 0 ;
	unsigned int& nbDiff = info.nbLateralDiffractions = 0 ;
	/*
	 * find reflections from vertical walls and lateral diffractions around vertical edges
	 */
	for (unsigned int i = 1 ; i < cp.size()-1 ; ++i)
	{
		if (cp[i].isVerticalWall())
		{
			cp[i].mode2D = Action2D::Reflection ;	
			nbRefl++ ;
		}
		else if (cp[i].isVerticalEdge())
		{
				cp[i].mode2D = Action2D::Diffraction ; 
				nbDiff++ ;
		}
		else if (cp[i].hasExtension())
		{
			/*
			 * no other extensions allowed in between source and receiver !
			 */
			sprintf_s (error_buffer, "unexpected extension type [%s] at position [%d]\n", typeid(*cp[i].ext).name(), i) ;
			return_error (error_buffer) ;
		}
		else
		{
			cp[i].mode2D = Action2D::None ;
		}
	}
	/*
	 * check number of reflections
	 */
	if (options.DisableReflections && nbRefl > 0)
	{
		return_error ("Invalid path, no reflections allowed") ; 
	}
	/*
	 * check number of lateral diffractions
	 */
	if (options.DisableLateralDiffractions && nbDiff > 0)
	{
		return_error ("Invalid path, no lateral diffractions allowed\n") ; 
	}
	/*
	 * each segment, when projected on the horizontal plane, should represent a straight line
	 */
	if (options.CheckHorizontalAlignment)
	{
		if (!check_horizontal_alignment()) return false ;
	}
	/*
	 * Laterally diffracted paths are supposed to be masked by the obstacle
	 * note that, given the information on input, we can only check this in case there are at 
	 * least 2 diffracting edges in the path...
	 */
	if (options.CheckLateralDiffraction && nbDiff > 1)
	{
		if (!check_lateral_diffraction()) return false ;
	}
	/*
	 * Some methods are based on a global approach for laterally diffracted paths ; in these methods
	 * lateral diffraction is allowed for simple paths only, not in combination with other effects.
	 * Alternative methods may be based on a local treatment of laterally diffracting edges, in which
	 * case, combination with reflection and diffraction over other obstacles is possible.
	 */
	if (options.IgnoreComplexPaths)
	{
		if (nbDiff > 0 && nbRefl > 0)
		{
			return_error ("Invalid path, contains lateral diffraction in combination with reflections") ;
		}
	}
	return true ;
}
/*
 * unfold the path in the vertical plane using (d_path, z) coordinates ; 
 * i.e. calculate d_path, the cumulative horizontal distance along the path
 */
bool PropagationPath::unfold_path (void)
{
	cp[0].d_path = 0 ;
	for (unsigned int i = 1 ; i < cp.size() ; ++i)
	{
		/*
		 * calculate distance in 2D, i.e. using only the (x, y) coordinates of the control point's position
		 */
		Geometry::Point2D p1 (cp[i-1].pos) ;
		Geometry::Point2D p2 (cp[i].pos) ;
		cp[i].d_path = cp[i-1].d_path + dist(p1, p2) ;
	}
	return true ;
}
/*
 * calculate the path difference for a single diffraction edge in between the source and the
 * receiver, optionally returns the height of the direct ray path at the diffracting edge
 */
double PropagationPath::get_path_difference (Geometry::Point2D const& src, 
											 Geometry::Point2D const& dif,
											 Geometry::Point2D const& rec,
											double* z_intersection)
{
	double delta_dif = Geometry::dist (src, dif) 
					 + Geometry::dist(dif, rec) 
					 - Geometry::dist (src, rec) ;
	double y_direct  = src.y + (rec.y - src.y) * (dif.x - src.x) / (rec.x - src.x) ;
	if (y_direct > dif.y) delta_dif = -delta_dif ;
	if (z_intersection) *z_intersection = y_direct ;
	return delta_dif ;
}
/*
 * construct convex hull in the (d,z) plane (recursively)
 */
void PropagationPath::construct_convex_hull (unsigned int n1, unsigned int n2, unsigned int level)
{
	Geometry::Point2D p1 (cp[n1].d_path, cp[n1].z_path) ;
	Geometry::Point2D p2 (cp[n2].d_path, cp[n2].z_path) ;

	unsigned int pos_max = -1 ;
	double dif_max = 0 ;

	for (unsigned int i = n1+1 ; i < n2 ; ++i)
	{
		Geometry::Point2D pd (cp[i].d_path, cp[i].pos.z) ;
		double dif = get_path_difference (p1, pd, p2, &cp[i].z_path) ;
		if (pos_max == -1 || dif > dif_max)
		{
			pos_max = i ;
			dif_max = dif ;
		}
	}

	if (pos_max != -1)
	{
		if (dif_max >= 0)
		{
			assert (cp[pos_max].z_path <= cp[pos_max].pos.z) ;
			cp[pos_max].z_path = cp[pos_max].pos.z ;
			cp[pos_max].mode3D = Action3D::Diffraction ;
			construct_convex_hull (n1, pos_max, level+1) ;
			construct_convex_hull (pos_max, n2, level+1) ;
			info.pathType = PathInfo::DiffractedPath ;
			info.nbDiffractions++ ;
		}
		else if (level == 0)
		{
			assert (cp[pos_max].z_path >= cp[pos_max].pos.z) ;
			cp[pos_max].mode3D = Action3D::DiffractionBLOS ;
			info.pathType = PathInfo::PartialDiffractedPath ;
		}
	}
}
/*
 * construct convex hull in the (d,z) plane (recursively)
 */
bool PropagationPath::construct_convex_hull (void)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = cp.size()-1 ;
	/*
	 * chain straight line segments by means of prev/next indicator
	 */
	cp[n1].z_path = cp[n1].pos.z + cp[n1].ext->h ;
	cp[n2].z_path = cp[n2].pos.z + cp[n2].ext->h ;
	/*
	 * initialize type of control point 
	 */
	if (cp[n1].mode2D == Action2D::Source)
	{
		cp[n1].mode3D = Action3D::Source ;
		cp[n2].mode3D = Action3D::Receiver ;
	}
	else
	{
		cp[n1].mode3D = Action3D::Receiver ;
		cp[n2].mode3D = Action3D::Source ;
	}
	for (unsigned int i = n1+1 ; i < n2 ; ++i) cp[i].mode3D = Action3D::None ;
	/*
	 * initialize path type
	 */
	info.nbDiffractions = 0 ;
	info.pathType = PathInfo::UndefinedPath ;
	/*
	 * construct convex hull recursively
	 */
	construct_convex_hull (n1, n2, 0) ;
	/*
	 * check default path type
	 */
	if (info.pathType == PathInfo::UndefinedPath) info.pathType = PathInfo::DirectPath ;
	
	return true ;
}
/*
 * check heights of the ray path at reflecting walls / diffracting vertical edges
 */
bool PropagationPath::check_heights (PropagationPathOptions const& options)
{
	if (options.CheckHeightUpperBound)
	{
		for (unsigned int i = 1 ; i < cp.size()-1 ; ++i)
		{
			if (cp[i].mode2D == Action2D::Reflection ||	cp[i].mode2D == Action2D::Diffraction)
			{
				assert (cp[i].ext != 0) ;
				if (cp[i].z_path >= cp[i].pos.z + cp[i].ext->h)
				{
					sprintf_s (error_buffer, "Reflection/diffraction above upper bound at position %d", i) ;
					return_error (error_buffer) ;
				}
			}
		}
	}
	if (options.CheckHeightLowerBound)
	{
		for (unsigned int i = 1 ; i < cp.size()-1 ; ++i)
		{
			if (cp[i].mode2D == Action2D::Reflection || cp[i].mode2D == Action2D::Diffraction)
			{
				/*
				 * note that we cannot use z_path to check the lower limit because the construction 
				 * of the convex hull guarantees that z_path >= pos.z everywhere. We have equality 
				 * if this position corresponds to a point on the convex hull, i.e. if it is marked as 
				 * a vertical diffracting edge...
				 * From a purely formal point of view, all methods are able to calculate paths containing
				 * diffraction by the lower bound of the vertical obstacle, but some methods may explicitly 
				 * forbid this.
				 */
				if (cp[i].mode3D == Action3D::Diffraction)
				{
					sprintf_s (error_buffer, "Reflexion/diffraction below lower bound at position %d", i) ;
					return_error (error_buffer) ;
				}
			}
		}
	}
	return true ;
}
/*
 * setup and check the propagation path in the unfolded vertical plane
 */
bool PropagationPath::setup_vertical_path (PropagationPathOptions const& options)
{
	for (unsigned int i = 0 ; i < cp.size() ; ++i) cp[i].mode3D = Action3D::None ;
	/* 
	 * unfold the path into a single vertical plane, using D-Z coordinates
	 */
	if (!unfold_path()) return false ;
	/*
	 * simplify the boundary profile 
	 * note, this option can be used with any point-to-point calculation method
	 */
	if (options.SimplifyPathGeometry) simplify_path_geometry() ;
	/*
	 * construct the Fermat ray path in the vertical plane
	 * note that in case of reflections of lateral diffractions, the unfolding of the plane
	 * guarantees that the ray path conforms to the rules of specular reflections angles and/or 
	 * diffraction on the Keller cone
	 */
	if (!construct_convex_hull()) return false ;
	/*
	 * check height of the ray path with respect to the lower/upper bounds of the vertical 
	 * obstacles
	 */
	if (!check_heights (options)) return false ;
	/*
	 * Some methods are based on a global approach for laterally diffracted paths ; in these methods
	 * lateral diffraction is allowed for simple paths only, not in combination with other effects.
	 * Alternative methods may be based on a local treatment of laterally diffracting edges, in which
	 * case, combination with reflection and diffraction over other obstacles is possible.
	 */
	if (options.IgnoreComplexPaths && info.nbLateralDiffractions > 0)
	{
		if (info.nbDiffractions > 0)
		{
			return_error ("Invalid path, contains mixed diffraction by horizontal/vertical edges") ;
		}
		else
		{
			info.pathType = PathInfo::LateralDiffractedPath ;
		}
	}
	return true ;
}
/*
 * utility function: print out path description to stdout
 */
void PropagationPath::print_input_data (void)
{
	printf ("------------------------------------------------------------------------\n") ;
	printf ("ID |       X |       Y |     Z |    G | Extension \n") ;
	printf ("------------------------------------------------------------------------\n") ;
	for (unsigned int i = 0 ; i < size() ; ++i)
	{
		ControlPoint& p = cp[i] ;
		Position& pos = p.pos ;
		printf ("%2.2d | %7.2f | %7.2f | %5.2f | %3.2f | ", i, pos.x, pos.y, pos.z, p.mat->getGValue()) ;
		if (p.isSource())
		{
			SourceExt* source = p.ext.cast_to_ptr<SourceExt> () ; 
			assert (source != 0) ;
			printf ("SRC h=%.2f", source->h) ;
		}
		else if (p.isReceiver())
		{
			ReceiverExt* receiver = p.ext.cast_to_ptr<ReceiverExt>() ;
			assert (receiver != 0) ;
			printf ("REC h=%.2f", receiver->h) ;
		}
		else if (p.isBarrier())
		{
			BarrierExt* barrier = p.ext.cast_to_ptr<BarrierExt>() ; 
			assert (barrier != 0) ;
			printf ("SCR h=%.2f G=%.2f", barrier->h, barrier->mat->getGValue()) ;
		}
		else if (p.isVerticalWall())
		{
			VerticalWallExt* wall = p.ext.cast_to_ptr<VerticalWallExt>() ;
			assert (wall != 0) ;
			printf ("REF h=%.2f G=%.2f", wall->h, wall->mat->getGValue()) ;
		}
		else if (p.isVerticalEdge())
		{
			VerticalEdgeExt* edge = p.ext.cast_to_ptr<VerticalEdgeExt>() ;
			assert (edge != 0) ;
			printf ("DIF h=%.2f", edge->h) ;
		}
		else if (p.hasExtension ())
		{
			printf ("Unknown extension, type = %s", typeid(*p.ext).name()) ;
		}
		printf ("\n") ;
	}
	printf ("------------------------------------------------------------------------\n") ;
}
/*
 * utility function: print out path description to stdout
 */
void PropagationPath::print_unfolded_path (void)
{
	printf ("------------------------------------------------------------------------\n") ;
	printf ("ID |       D |      Z |      H |  Z_ray |    G | Mode \n") ;
	printf ("------------------------------------------------------------------------\n") ;
	for (unsigned int i = 0 ; i < size() ; ++i)
	{
		ControlPoint& p = cp[i] ;
		printf ("%2.2d | %7.2f | %6.2f | %6.2f | %6.2f | %3.2f | ", 
			     i, cp[i].d_path, cp[i].pos.z, cp[i].ext ? cp[i].ext->h : 0.0, cp[i].z_path, cp[i].mat->getGValue()) ;
		
		if (cp[i].mode2D == Action2D::Source)
			printf ("SRC") ;
		else if (cp[i].mode2D == Action2D::Receiver)
			printf ("REC") ;
		else if (cp[i].mode2D == Action2D::Diffraction)
			printf ("DIF.LAT") ;
		else if (cp[i].mode2D == Action2D::Reflection)
		{
			if (cp[i].mode3D == Action3D::Diffraction || cp[i].z_path > cp[i].pos.z + cp[i].ext->h)	
				printf ("REF+DIF") ;
			else
				printf ("REF") ;
		}
		else if (cp[i].mode3D == Action3D::Diffraction)
			printf ("DIF") ;
		else if (cp[i].mode3D == Action3D::DiffractionBLOS)
			printf ("DIF.BLOS") ;

		printf ("\n") ;
	}
	printf ("------------------------------------------------------------------------\n") ;
}
/*
 * return the ray path associated with the propagation path. This information can be used to 
 * visualize the ray path in the 3D geometrical model of the site.
 *
 * if math_pos_blos is true, return the ray path including positions below the actual line of
 * sight, otherwise return the true Fermat path as used in the acoustical calculations.
 *
 * positions below the line of sight are generated in case of partial diffraction in the vertical
 * plane (i.e. paths influenced but not blocked by vertical edges) or when the specular reflection
 * and/or Keller diffraction occurs outside the limits of a vertical obstacle.
 */
RayPath PropagationPath::get_ray_path (bool match_pos_blos)
{
	RayPath ray ;
	Position pos ;

	for (unsigned int i = 0 ; i < cp.size(); ++i)
	{
		if (cp[i].mode2D == Action2D::None && cp[i].mode3D == Action3D::None) continue ;

		pos   = cp[i].pos ;
		pos.z = cp[i].z_path ;

		if (match_pos_blos)
		{
			if (cp[i].mode2D == Action2D::Diffraction || cp[i].mode2D == Action2D::Reflection)
			{
				assert (cp[i].ext != 0) ;
				pos.z = std::min (cp[i].z_path, cp[i].pos.z + cp[i].ext->h) ;
			}
			else if (cp[i].mode3D == Action3D::DiffractionBLOS)
			{
				pos.z = std::min (cp[i].pos.z, cp[i].z_path) ;
			}
		}
		else
		{
			if (cp[i].mode2D == Action2D::None && cp[i].mode3D == Action3D::DiffractionBLOS) continue ;
		}

		ray.push_back (pos) ;
	}
	return ray ;
}

using namespace Geometry ;

double PropagationPath::get_Fresnel_weighting (Point2D const& src, 
											   Point2D const& rec,
											   Point2D const seg[2],
											   double delta_dif,
											   bool intersection_only) 
{
	const double PI = 3.1415926 ;
	/*
	 * local coordinate system positioned at the midpoint of source and receiver
	 */
	Point2D origin = (src + rec)/2 ;
	/*
	 * X-axis aligned with the line from source to receiver
	 */
	Vector2D SR = rec - src ; 
	double dSR = norm (SR) ;
	assert (dSR > 0) ;
	Vector2D x_axis = SR / dSR ;
	/*
	 * Y-axis is +90� with respect to the X-axis
	 */
	Vector2D y_axis (-x_axis.y, x_axis.x) ;
	/*
	 * parameters of the Fresnel ellipse
	 * in local coordinates, the ellipse is given by equation (x/A)� + (y/B)� = 1
	 * and passes through points (A,0), (0,B), (-A,0) and (0,-B)
	 */
	double A = (dSR + delta_dif) / 2 ;
	double B = sqrt ((dSR + delta_dif) * (dSR + delta_dif) - dSR * dSR) / 2 ;
	/*
	 * transform segment to local coordinates
	 */
	Vector2D q1 (origin, seg[0]) ;
	Vector2D q2 (origin, seg[1]) ;
	/*
	 * projection on local coordinate system
	 */
	Vector2D p1 (q1 * x_axis, q1 * y_axis) ;
	Vector2D p2 (q2 * x_axis, q2 * y_axis) ;
	/*
	 * anisotropic scaling 
	 * this transforms the Fresnel ellipse into a circle centered at the origin and unit radius
	 * note that anisotropic scaling does not change ratios measured along the segment
	 */
	p1.x /= A ;
	p1.y /= B ;
	p2.x /= A ;
	p2.y /= B ;
	/*
	 * intersect segment [p1, p2] with circle x� + y� = 1
	 * this is equivalent to solving the equation |p1 + t (p2 - p1)| = 1
	 */
	Vector2D p (p1) ;
	Vector2D n (p1, p2) ;
	/*
	 * write equation as a.t� + 2.b.t + c = 0
	 */
	double a = n * n ;
	double b = p * n ;
	double c = p * p - 1 ;
	double d = b * b - a * c ;
	/*
	 * determinant < 0 : line segment does not intersect the ellipse
	 */
	if (d <= 0) return 0 ;
	/*
	 * calculate two solutions
	 */
	double t1 = (-b - sqrt(d)) / a ;
	double t2 = (-b + sqrt(d)) / a ;
	/*
	 * return relative size of the segment compared to Frensel zone
	 */
	if (intersection_only)
	{
		/*
		 * return size of the intersection of the segment [t=0, t=1] and the Fresnel zone [t1, t2]
		 * relative to the size of the Frensel zone.
		 */
		if (t1 > 1) return 0 ;
		if (t2 < 0) return 0 ;
		double u1 = std::max (t1, 0.0) ;
		double u2 = std::min (t2, 1.0) ;
		double w = (u2 - u1) / (t2 - t1) ;
		/*
		 * this value is smaller or equal to 1 by definition
		 */
		assert (w <= 1) ;
		return w ;
	}
	else
	{
		/*
		 * return size of the segment [t=0, t=1] relative to size of the Frensel zone [t1, t2]
		 */
		return 1 / (t2 - t1) ;
	}
}
/*
 * local function: recursively mark control points that should be preserved because they
 * represent a significant deviation from flat terrain.
 */
void mark_path_geometry (PropagationPath& path, unsigned int n1, unsigned int n2)
{
	if ((n2 - n1) < 2) return ;

	Point2D p1 (path[n1].d_path, path[n1].pos.z) ;
	Point2D p2 (path[n2].d_path, path[n2].pos.z) ;
	double  dp = dist (p1, p2) ;
	double dmax ;
	unsigned int imax = 0 ;
	for (unsigned int i = n1+1 ; i < n2 ; ++i)
	{
		Point2D pi (path[i].d_path, path[i].pos.z) ;
		double  di = dist (p1, pi)  + dist (pi, p2) - dp ;
		if (imax == 0 || di > dmax)
		{
			imax = i ;
			dmax = di ;
		}
	}
	/*
	 * keep terrain point if height greater than 1% of the distance ; note that the
	 * path difference increases as (h/d)� where h is the height of the obstacle...
	 */
	if (dmax > 0.00025 * dp)
	{
		path[imax].mode3D = Action3D::DiffractionBLOS ;
		/*
		 * recursively continue on the left and the right side of this position
		 */
		mark_path_geometry (path, n1, imax) ;
		mark_path_geometry (path, imax, n2) ;
	}
}

void PropagationPath::simplify_path_geometry (void)
{
	print_debug (".simplify path \n") ;

	if (cp.size() < 2) return ;
	unsigned int n1 = 0 ;
	unsigned int n2 = cp.size()-1 ;
	/*
	 * use the mode3D indicator to mark positions in the vertical plane, mark positions
	 * to be preserved with the DiffractionBlos indicator.
	 */
	for (unsigned int i = n1 ; i <= n2 ; ++i) cp[i].mode3D = Action3D::None ;
	/*
	 * do not remove source, receiver, reflections and lateral diffractions
	 */
	for (unsigned int i = n1 ; i <= n2 ; ++i) 
	{
		if (cp[i].mode2D != Action2D::None) cp[i].mode3D = Action3D::DiffractionBLOS ;
	}
	/*
	 * do not remove impedance jumps
	 */
	for (unsigned int i = n1+1 ; i < n2 ; ++i) 
	{
		if (cp[i].mat != cp[i+1].mat) cp[i].mode3D = Action3D::DiffractionBLOS ;
	}
	/*
	 * do not remove positions that deviate significantly from flat terrain
	 */
	unsigned int istart = n1 ;
	for (unsigned int i = n1+1 ; i <= n2 ; ++i) 
	{
		if (cp[i].mode3D == Action3D::None) continue ;
		mark_path_geometry (*this, istart, i) ;
		istart = i ;
	}
	/*
	 * remove unmarked positions
	 */
	unsigned int nout = n1 ;
	for (unsigned int i = n1 ; i <= n2 ; ++i) 
	{
		if (cp[i].mode3D != Action3D::None)
		{
			if (nout != i) cp[nout] = cp[i] ;
			nout++ ;
		}
		else
		{
			print_debug (" terrain point %d removed \n", i) ;
		}
	}
	if (nout != n2) cp.resize (n2 = nout) ;
	/*
	 * reset the mode3D indicator to its default value before starting the geometrical 
	 * analysis in the vertical plane
	 */
	for (unsigned int i = n1 ; i < n2 ; ++i) cp[i].mode3D = Action3D::None ;
}

Geometry::Vector3D PropagationPath::get_propagation_direction (void)
{
	unsigned int n1 = 0 ;
	unsigned int n2 = cp.size()-1 ;
	/*
	 * get source and receiver indexes
	 */
	assert (cp[n1].ext != 0) ;
	assert (cp[n2].ext != 0) ;
	bool source_to_receiver = cp[n1].ext->isSource() ;
	unsigned int isrc  = source_to_receiver ? n1 : n2 ;
	unsigned int irec  = source_to_receiver ? n2 : n1 ;
	unsigned int istep = source_to_receiver ?  1 : -1 ;
	unsigned int iref ;
	assert (cp[isrc].ext->isSource()) ;
	assert (cp[irec].ext->isReceiver()) ;
	/*
	 * move from the source to the first reflection or diffraction point
	 */
	for (iref = isrc + istep ; iref != irec ; iref += istep)
	{
		if (cp[iref].mode2D != Action2D::None || cp[iref].mode3D != Action3D::None) break ;
	}
	/*
	 * return vector from (real) source to (virtual) receiver
	 */
	Point3D src = cp[isrc].pos ; src.z = cp[isrc].z_path ;
	Point3D rec = cp[iref].pos ; rec.z = cp[iref].z_path ;
	return (rec - src) ;
}
