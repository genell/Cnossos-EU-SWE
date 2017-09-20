#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		PropagationPath.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description:	implementation of propagation path data structures and functionalities
 * changes:
 *
 *	18/01/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include <vector>
#include "Geometry3D.h"
#include "ErrorMessage.h"
#include "MeteoCondition.h"
#include "VerticalExt.h"
#include "Material.h"
/*
 * forward declarations, used to define transparent pointers
 */
namespace CnossosEU
{
	class  CalculationMethod ;
	/*
	 * position in 3D Cartesian coordinates
	 * operations on 3D points and vectors are imported from the Geometry3D package
	 */
	typedef Geometry::Point3D Position ;
	/*
	 * type of control point in the horizontal plane (used on output only)
	 */
	struct Action2D
	{
		enum Type
		{
			None        = 0,
			Source      = 1,
			Receiver    = 2,
			Reflection  = 3,
			Diffraction = 4		
		}  type ;

		Action2D (void) { type = None ; }

		Type operator= (Action2D::Type _type) { return type = _type ; }
		bool operator== (Action2D::Type _type) { return type == _type ; }
		bool operator!= (Action2D::Type _type) { return type != _type ; }
	} ;
	/*
	 * type of control point in the vertical plane (used on output only)
	 */
	struct Action3D
	{
		enum Type
		{
			None            = 0,
			Source          = 1,
			Receiver        = 2,
			Diffraction     = 3,
			DiffractionBLOS = 4
		} type ;

		Action3D (void) { type = None ; }

		Type operator= (Action3D::Type _type) { return type = _type ; }
		bool operator== (Action3D::Type _type) { return type == _type ; }
		bool operator!= (Action3D::Type _type) { return type != _type ; }
	} ;
	/*
	 * control point
	 */
	struct ControlPoint
	{
		/*
		 * position in 3D coordinates (input only)
		 */
		Position    pos ;
		/*
		 * material properties (input only)
		 */
		System::ref_ptr<Material>   mat ;
		/*
		 * vertical extension (input only)
		 */
		System::ref_ptr<VerticalExt> ext ;
		/*
		 * constructor
		 */
		ControlPoint() : pos(), mat(0), ext(0), mode2D(), mode3D(), d_path(0.0), z_path(0.0) { } 
		
		ControlPoint (ControlPoint const& other)
		{
			pos = other.pos ;
			mat = other.mat ; 
			ext = other.ext ? other.ext->clone() : 0 ;
			mode2D = other.mode2D ;
			mode3D = other.mode3D ;
			d_path = other.d_path ;
			z_path = other.z_path ; 
		}

		ControlPoint& operator= (ControlPoint const& other)
		{
			pos = other.pos ;
		  	mat = other.mat ; 
		  	ext = other.ext ? other.ext->clone() : 0 ;
			mode2D = other.mode2D ;
			mode3D = other.mode3D ;
			d_path = other.d_path ;
			z_path = other.z_path ; 
			return *this ;
		}
		/*
		 * indirect 
		 */
		bool hasExtension (void) { return ext != NULL ; }
		bool isSource (void) ;
		bool isReceiver (void) ;
		bool isBarrier (void) ;
		bool isVerticalWall (void) ;
		bool isVerticalEdge (void) ;
		/*
		 * type of control point in the horizontal plane (output only)
		 */	
		Action2D mode2D ;
		/*
		 * type of control point in the vertical plane (output only)
		 */	
		Action3D mode3D ;
		/*
		 * coordinates of the Fresnel ray path in the vertical plane (output only)
		 */
		double d_path ;
		double z_path ;
	};
	/*
	 * options for geometrical analysis of propagation paths
	 */
	struct PropagationPathOptions
	{
		/*
		 * if the path finder can guarantee conformity of the paths, additional checking may be disabled.
		 * do not remove checking in case the propagation path are constructed manually....
		 */
		bool CheckHorizontalAlignment ;		// check validity of paths in the horizontal plane
		bool CheckLateralDiffraction ;		// check validity of laterally diffracted paths
		bool CheckHeightLowerBound ;		// check height of ray path with respect to upper limits of vertical obstacles
		bool CheckHeightUpperBound ;		// check height of ray path with respect to lower limits of vertical obstacles
		bool CheckSourceSegment ;           // check whether source line segments contains actual source position
		bool CheckSoundPowerUnits ;			// check conformity of sound power units and source geometry
		/*
		 * reinforce requirements on the geometrical complexity of the path
		 * note that some requirements will be reinforced by the calculation method independently of user settings
		 */
		bool ForceSourceToReceiver ;		// reverse the path in case it is ordered from Receiver to Source
		bool SimplifyPathGeometry ;			// remove non representative terrain points
		bool DisableReflections ;			// disable reflections
		bool DisableLateralDiffractions ;	// disable lateral diffractions
		bool IgnoreComplexPaths ;		    // disable reflections and vertical diffraction for laterally diffracted paths
		/*
		 * disable some parts of the acoustical calculations, e.g. because these parts are already taken care of
		 * in other parts of the application software
		 */
		bool ExcludeSoundPower ;			// include sound power
		bool ExcludeGeometricalSpread ;		// include geometrical spread
		bool ExcludeAirAbsorption ;			// include atmospheric absorption
		
		PropagationPathOptions (void) : method(0), meteo()
		{
			CheckHorizontalAlignment	= true ;
			CheckLateralDiffraction		= true ;
			CheckHeightLowerBound		= false ;
			CheckHeightUpperBound		= false ;
			CheckSourceSegment	        = true ;
			CheckSoundPowerUnits        = true ;
			
			ForceSourceToReceiver		= false ;
			SimplifyPathGeometry		= false ;
			DisableReflections			= false ;
			DisableLateralDiffractions	= false ;
			IgnoreComplexPaths     = true ;

			ExcludeSoundPower			= false ;
			ExcludeGeometricalSpread	= false ;
			ExcludeAirAbsorption		= false ;
		}
		/*
		 * select method for acoustical calculations
		 */
		CalculationMethod* method ;	
		/*
		 * meteorological conditions
		 */
		MeteoCondition meteo ;
	};
	/*
	 * information about type of path
	 */
	struct PathInfo
	{
		enum PathType
		{
			UndefinedPath = 0,
			DirectPath = 1,
			DiffractedPath = 2,
			PartialDiffractedPath = 3,
			LateralDiffractedPath = 4
		};
		PathType pathType ;
		unsigned int nbReflections ;
		unsigned int nbDiffractions ;
		unsigned int nbLateralDiffractions ;

		PathInfo (void) : pathType (UndefinedPath) { } ;
	};
	/*
	 * data structure used to reconstruct the geometry of Fermat path as a sequence of 3D positions
	 */
	typedef std::vector<Position> RayPath ;
	/*
	 * a propagation path is encoded as an ordered list of control points
	 */
	struct PropagationPath 
	{
		/*
		 * store PropagationPath as an ordered sequence of control points
		 */
		std::vector<ControlPoint> cp ;
		/*
		 * store path information on output of the geometrical analysis
		 */
		PathInfo info ;
		/*
		 * constructor
		 */
		PropagationPath (void) : cp() { } ;
		/*
		 * destructor
		 */
		~PropagationPath (void) { } ;
		/*
		 * clear the path by removing all control points
		 */
		void clear (void) { cp.resize(0) ; }
		/*
		 * add a control point to the path
		 */
		void add (ControlPoint const& p) { cp.push_back (p) ; }
		/*
		 * return the number of control points
		 */
		size_t	size(void) { return cp.size() ; }
		/*
		 * return the number of control points
		 */
		void resize(size_t new_size) { cp.resize(new_size) ; }
		/*
		 * direct access to control points
		 */
		ControlPoint& operator[] (size_t index) { return cp[index] ;}
		/*
		 * utility function: print path definitions to stdout
		 */
		void print_input_data (void) ;
		void print_unfolded_path (void) ;
		/*
		 * before doing the noise calculation by means of the dedicated calculation method,
		 * carry out the geometrical analysis of the path which is common to all methods.
		 */
		bool analyze_path (PropagationPathOptions const& options) ;
		/*
		 * get the 3D ray path associated with the propagation path
		 *
		 * this information may be used e.g. to visualize the ray path in the 3D model of the site
		 *
		 * param math_pos_blos : if true, include positions below the line of sight, otherwise
		 * return the true (optical) Fermat path. Positions below the line correspond to ray path 
		 * partially diffracted by obstacles below the line of sight or by the upper limits of 
		 * reflecting obstacles and/or diffractions by vertical edges.
		 */
		RayPath get_ray_path (bool match_pos_blos = false) ;
		/*
		 * utility function: calculate path difference in the vertical plane
		*/
		static double get_path_difference (Geometry::Point2D const& src, 
										   Geometry::Point2D const& dif,
								           Geometry::Point2D const& rec,
								           double* z_intersection = 0) ;
		/*
		 * utility function: calculate Fresnel weight
		*/
		static double get_Fresnel_weighting (Geometry::Point2D const& src, 
										     Geometry::Point2D const& dif,
								             Geometry::Point2D const seg[2],
											 double delta_dif,
											 bool intersection_only = true) ;

		/*
		 *
		 */
		Geometry::Vector3D get_propagation_direction (void) ;

	private:
		/*
		 * reverse the path direction
		 */
		bool reverse_path (void) ;
		/*
		 * remove barriers from the path, replace hem by equivalent vertical segments
		 */
		bool remove_barriers (void) ;
		/*
		 * check source and receiver position
		 */
		bool check_source_receiver (PropagationPathOptions const& options) ;
		/*
		 * check alignment of control points in the horizontal plane
		 */
		bool check_horizontal_alignment (unsigned int n1, unsigned int n2) ;
		/*
		 * check alignment of control points in the horizontal plane
		 */
		bool check_horizontal_alignment (void) ;
		/*
		 * check alignment of control points in the horizontal plane
		 */
		bool check_lateral_diffraction (void) ;
		/*
		 * setup internal parameters related to the projection of the propagation path 
		 * on the horizontal plane
		 */
		bool setup_horizontal_path (PropagationPathOptions const& options) ;
		/*
		 * unfold the path, using D-Z coordinates in the vertical plane
		 */
		bool unfold_path (void) ;
		/*
		 * recursively construct the convex hull
		 */
		void construct_convex_hull (unsigned int n1, unsigned int n2, unsigned int level) ;
		/*
		 * construct the convex hull in the vertical plane
		 */
		bool construct_convex_hull (void) ;
		/*
		 * check height of specular reflection / Keller diffraction points with respect 
		 * to the upper and/or lower limits of the vertical obstacle
		 */
		bool check_heights (PropagationPathOptions const& options) ;
		/*
		 * setup internal parameters related to the construction of the principal ray 
		 * path (Fermat path) in the unfolded vertical plane.
		 */
		bool setup_vertical_path (PropagationPathOptions const& options) ;
		/*
		 *
		 */
		void simplify_path_geometry (void) ;
	};
}
