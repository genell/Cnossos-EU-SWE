/* 
 * ------------------------------------------------------------------------------------------------
 * file:		PathParseXML.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implement XML test file parser
 * changes:
 *
 *	18/10/2013	initial version
 *
 *  24/10/2013	added parsing of meteorological conditions
 *
 *  24/10/2013	added parsing of source related information ; for the moment being and until DGMR 
 *				provides final versions of output files, this information is directly encoded in 
 *				the path file.
 *
 *	02/12/2013	added parsing of extended source geometry, including point, line and area sources
 *
 *	05/12/2013	bug fixed: order of evaluation of internal/external source height fixed
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "./PathParseXML.h"
#include "./PropagationPath.h"
#include "./VerticalExt.h"
#include "./Material.h"
#include "./CalculationMethod.h"
#include "../system/environment.h"
using namespace CnossosEU ;

/*
 * check for matching tag name, note that XML tags are case sensitive
 */
static bool checkTagName (XMLNode* node, const char* name)
{
	if (node == 0) return false ;
	if (name == 0) return false ;
	return strcmp (node->GetName(), name) == 0 ;
}
/*
 * utility function: remove white space from the start of a string
 * returns false in case there is nothing left, true otherwise
 */
static bool SkipWhiteSpace (const char*& s)
{
	while (*s != 0 && *s <= ' ') s++ ;
	return (*s != 0) ;
}
/*
 * decode a floating-point number
 */
static bool DecodeValue (const char*& s, double& val)
{
	if (!SkipWhiteSpace (s)) return false ;
	val = strtod (s, (char**) &s) ;
	return true ;
}
/*
 * parse a tag containing a single floating-point number 
 */
static bool ParseValue (XMLNode* node, const char* name, double& value)
{
	if (!checkTagName (node, name)) return false ;
	const char* s = node->GetText() ;
	if (!DecodeValue (s, value))
	{
		signal_error (XMLParseError ("invalid number", node)) ;
		return false ;
	}
	if (SkipWhiteSpace (s))
	{
		signal_error (XMLParseError ("invalid number", node)) ;
		return false ;
	}
	print_debug ("ParseValue [%s] = %f \n", name, value) ;
	return true ;
}
/*
 * parse a tag containing a spectrum, note that spectra are stored as arrays of 
 * floating-point values where the size of the array is a constant defined in the
 * Spectrum class.
 */
static bool ParseSpectrum (XMLNode* node, const char* name, Spectrum& spec)
{
	if (!checkTagName (node, name)) return false ;

	const char*s = node->GetText() ;
	for (unsigned int i = 0 ; i < Spectrum::nbFreq ; i++)
	{
		if (!DecodeValue (s, spec[i]))
		{
			signal_error (XMLParseError ("missing value", node)) ;
			return false ;
		}
	}
	if (SkipWhiteSpace (s))
	{
		signal_error (XMLParseError ("too many values", node)) ;
		return false ;
	}
	print_debug ("ParseSpectrum OK \n") ;
	return true ;
}
/*
 * parse a position. A position is a triplet of (x,y,z) coordinates. 
 * Note that all three coordinates are optional. In case one of the coordinates is missing, 
 * its value will not be affected by this call.
 */
static bool ParsePosition (XMLNode* node, Position& pos, const char* tag = "pos")
{
	if (!checkTagName (node, tag))
	{
		signal_error (XMLMissingTag (tag, node)) ;
		return false ;
	}
	node = node->GetFirstChild() ;
	if (ParseValue (node, "x", pos.x)) node = node->GetNextEntity() ;
	if (ParseValue (node, "y", pos.y)) node = node->GetNextEntity() ;
	if (ParseValue (node, "z", pos.z)) node = node->GetNextEntity() ;
	if (node != 0)
	{
		throw XMLUnexpectedTag (node) ;
		return false ;
	}
	print_debug ("ParsePosition OK \n") ;
	return true ;
}
/*
 * Parse a reference to a material defined in project's database
 */
static bool ParseMaterialRef (XMLNode* node, System::ref_ptr<Material>& mat)
{
	if (!checkTagName (node, "mat")) return false ;
	const char* id = node->GetAttribute("id") ;
	if (id == 0 || strlen(id) == 0)
	{
		signal_error (XMLParseError ("Missing identifier", node)) ;
		return false ;
	}
	mat = getMaterial (id) ;
	if (mat == 0) 
	{
		signal_error (XMLParseError ("Invalid material identifier", node)) ;
		return false ;
	}
	print_debug ("ParseMaterial [id=%s, G=%.2f] OK \n", id, mat->getGValue()) ;
	return true ;
}
/*
 * Parse sound power spectrum
 */
static bool ParseElementarySource (XMLNode* node, ElementarySource &source)
{
	if (!ParseSpectrum (node, "Lw", source.soundPower)) return false ;

	const char* attrib = node->GetAttribute ("measurementType") ;
	if (attrib)
	{
		if (strcmp(attrib, "FreeField") == 0)     source.measurementType = MeasurementType::FreeField ;
		if (strcmp(attrib, "HemiSpherical") == 0) source.measurementType = MeasurementType::HemiSpherical ;
	}

	attrib = node->GetAttribute ("sourceType") ;
	if (attrib)
	{
		if (strcmp(attrib, "PointSource") == 0) source.spectrumType = SpectrumType::PointSource ;
		if (strcmp(attrib, "LineSource") == 0)  source.spectrumType = SpectrumType::LineSource ;
		if (strcmp(attrib, "AreaSource") == 0)  source.spectrumType = SpectrumType::AreaSource ;
	}

	attrib = node->GetAttribute ("frequencyWeighting") ;
	if (attrib)
	{
		if (strcmp(attrib, "LIN") == 0) source.frequencyWeighting = FrequencyWeighting::dBLIN ;
		if (strcmp(attrib, "dBA") == 0) source.frequencyWeighting = FrequencyWeighting::dBA ;
	}

	return true ;
}
/*
 * Forward declaration
 */
static bool ParseSourceExt (XMLNode* node, SourceExt &source, bool is_import = false) ;
/*
 * Parse a Source extension record
 */
static bool ImportSourceDescription (XMLNode* node, SourceExt &source)
{
	if (!checkTagName (node, "import")) return false ;
	const char* filename = node->GetAttribute ("file") ;
	print_debug ("import external file [%s] \n", filename) ;
	/*
	 * create a new file parser and load the file
	 */
	XMLFileLoader* externalFile = new XMLFileLoader() ;
	if (!externalFile->ParseFile (filename))
	{
		signal_error (XMLSyntaxError (*externalFile)) ;
		return false ;
	}
	/*
	 * root node must be "CNOSSOS_SourcePower"
	 */
	node = externalFile->GetRoot() ;
	if (!checkTagName (node, "CNOSSOS_SourcePower"))
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}
	/*
	 * restart the parsing of the source record from the imported file 
	 */
	node = node->GetFirstChild() ;
	if (!ParseSourceExt(node, source, true))
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}
	/*
	 * cleanup and return
	 */
	delete externalFile ;
	print_debug ("import external file OK\n") ;
	return true ;
}
/*
 * Forward declaration
 */
static bool ParseSourceGeometry (XMLNode* node, SourceExt &source) ;
/*
 * Parse a Source extension record
 */
static bool ParseSourceExt (XMLNode* node, SourceExt &source, bool is_import)
{
	if (!checkTagName (node, "source")) return false ;

	node = node->GetFirstChild () ;

	if (is_import)
	{
		/*
		 * source height is mandatory if the source is defined in an external file
		 */
		if (!ParseValue (node, "h", source.source.sourceHeight)) 
		{
			signal_error (XMLMissingTag ("h", node)) ;
			return false ;
		}
		node = node->GetNextEntity() ;
		print_debug (".model source height = %.2fm \n", source.source.sourceHeight) ;
	}
	else if (ImportSourceDescription(node, source))
	{
		node = node->GetNextEntity() ;
		/*
		 * set actual source height equal to height of the single elementary source;
		 * real applications would implement a loop over elementary source heights in
		 * the application code before calling the propagation path calculations.
		 */
		source.h = source.source.sourceHeight ;
		/*
		 * source height is optional in case of an imported elementary source
		 */
		if (ParseValue (node, "h", source.h)) 
		{
			node = node->GetNextEntity() ;
			print_debug (".user defined source height = %.2fm \n", source.h) ;
		}
	}
	else 
	{
		/*
		 * source height is mandatory in case there is no imported source model
		 */
		if (!ParseValue (node, "h", source.h)) 
		{
			signal_error (XMLMissingTag ("h", node)) ;
			return false ;
		}
		node = node->GetNextEntity() ;
		print_debug (".user defined source height = %.2fm \n", source.h) ;
	}
	/*
	 * source power is always optional
	 */
	if (ParseElementarySource (node, source.source)) 
	{
		node = node->GetNextEntity() ;
	}
	/*
	 * extended source geometry is optional but never defined in an external file
	 */
	if (!is_import)
	{
		if (ParseSourceGeometry (node, source)) node = node->GetNextEntity() ;
	}
	/* 
	 * no additional child items allowed
	 */
	if (node != 0)
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	print_debug ("ParseSourceExt OK \n") ;
	return true ;
}
/*
 * Parse point geometry extension
 */
static bool ParsePointSource (XMLNode* node, System::ref_ptr<SourceGeometry> &geo) 
{
	if (!checkTagName (node, "pointSource")) return false ;
	print_debug ("Parse PointSource \n") ;
	node = node->GetFirstChild() ;
	/*
	 * parse orientation (default is X_axis)
	 */
	Geometry::Vector3D orientation (1.0, 0.0, 0.0) ;
	if (ParsePosition(node, orientation, "orientation")) node = node->GetNextEntity() ;
	/*
	 * no other tags allowed
	 */
	if (node != 0)
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	/*
	 * return point source geometry
	 */
	geo = new PointSource (orientation) ;
	return true ;
}
/*
 * Parse source geometry extension
 */
static bool ParseLineSource (XMLNode* node, System::ref_ptr<SourceGeometry> &geo) 
{
	if (!checkTagName (node, "lineSource")) return false ;
	print_debug ("Parse LineSource \n") ;
	node = node->GetFirstChild() ;
	/*
	 * parse length (default = 1m)
	 */
	double length  = 1.0 ;
	if (ParseValue(node, "length", length)) 	node = node->GetNextEntity() ;
	/*
	 * parse orientation (default = Y_asis)
	 */
	Geometry::Vector3D orientation (0.0, 1.0, 0.0) ;
	if (ParsePosition(node, orientation, "orientation")) node = node->GetNextEntity() ;
	/*
	 * no other tags allowed
	 */
	if (node != 0)
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	/*
	 * return line source geometry
	 */
	geo = new LineSource (length, orientation) ;
	return true ;
}
/*
 * Parse source geometry extension
 */
static bool ParseAreaSource (XMLNode* node, System::ref_ptr<SourceGeometry> &geo) 
{
	if (!checkTagName (node, "areaSource")) return false ;
	print_debug ("Parse AreaSource \n") ;
	node = node->GetFirstChild() ;
	/*
	 * parse area, default = 1m�
	 */
	double area  = 1.0 ;
	if (ParseValue(node, "area", area)) node = node->GetNextEntity() ;
	/*
	 * parse orientation (default = Z_asis)
	 */
	Geometry::Vector3D orientation (0.0, 0.0, 1.0) ;
	if (ParsePosition(node, orientation, "orientation")) node = node->GetNextEntity() ;
	/*
	 * no other tags allowed
	 */
	if (node != 0)
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	/*
	 * return area source geometry
	 */
	geo = new AreaSource (area, orientation) ;
	return true ;
}
/*
 * Keep track of position of current control point
 */
static Geometry::Point3D current_cp ;
/*
 * Parse source geometry extension
 */
static bool ParseSegmentSource (XMLNode* node, System::ref_ptr<SourceGeometry> &geo) 
{
	if (!checkTagName (node, "lineSegment")) return false ;
	print_debug ("Parse LineSegment \n") ;
	node = node->GetFirstChild() ;

	Geometry::Point3D p1(current_cp) ;
	Geometry::Point3D p2(current_cp) ;
	double fixedAngle = 0 ;
	/*
	 * parse start position (default = position of current control point)
	 */
	if (ParsePosition (node, p1, "posStart")) node = node->GetNextEntity() ;
	/*
	 * parse end position (default = position of current control point)
	 */
	if (ParsePosition (node, p2, "posEnd"))   node = node->GetNextEntity() ;
	/*
	 * parse fixed viewing angle (default = 0)
	 */
	if (ParseValue (node, "fixedAngle", fixedAngle)) node = node->GetNextEntity() ;
	/*
	 * no further tags allowed
	 */
	if (node != 0) 
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	/*
	 * return source line segment geometry
	 */
	geo = new LineSegment(p1, p2, fixedAngle) ;
	
	print_debug (".start x=%.1f y=%.1f z=%.1f \n", p1.x, p1.y, p1.z) ;
	print_debug (".end   x=%.1f y=%.1f z=%.1f \n", p2.x, p2.y, p2.z) ;
	return true ;
}
/*
 * Parse source geometry extension
 */
static bool ParseSourceGeometry (XMLNode* node, SourceExt &source) 
{
	if (!checkTagName (node, "extGeometry")) return false ;
	print_debug ("Parse SourceGeometry\n") ;

	node = node->GetFirstChild() ;
	if (node == 0) 
	{
		signal_error (XMLParseError ("extended source geometry missing",node)) ;
		return false ;
	}

	if (!ParsePointSource(node, source.geo) && 
		!ParseLineSource(node, source.geo) && 
		!ParseAreaSource(node, source.geo) &&
		!ParseSegmentSource(node, source.geo))
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}

	node = node->GetNextEntity() ;
	if (node != 0) 
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	print_debug ("Parse SourceGeometry OK\n") ;
	return true ;
}
/*
 * Parse a Receiver extension record
 */
static bool ParseReceiverExt (XMLNode* node, ReceiverExt &receiver)
{
	if (!checkTagName (node, "receiver")) return false ;
	node = node->GetFirstChild () ;
	if (!ParseValue (node, "h", receiver.h))
	{
		signal_error (XMLMissingTag ("h", node)) ;
		return false ;
	}
	node = node->GetNextEntity() ;
	if (node != 0) 
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	print_debug ("ParseReceiverExt OK \n") ;
	return true ;
}
/*
 * Parse a Barrier extension record
 */
static bool ParseBarrierExt (XMLNode* node, BarrierExt &barrier)
{
	if (!checkTagName (node, "barrier")) return false ;
	node = node->GetFirstChild () ;
	if (!ParseValue (node, "h", barrier.h))
	{
		signal_error (XMLMissingTag ("h", node)) ;
		return false ;
	}
	node = node->GetNextEntity() ;
	if (ParseMaterialRef (node, barrier.mat))
	{
		node = node->GetNextEntity() ;
	}
	else
	{
		barrier.mat = getMaterial ("A0") ;
	}
	if (node != 0)
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	print_debug ("ParseBarrierExt OK \n") ;
	return true ;
}
/*
 * Parse a VerticalWall extension record
 */
static bool ParseVerticalWallExt (XMLNode* node, VerticalWallExt &wall)
{
	if (!checkTagName (node, "wall")) return false ;
	node = node->GetFirstChild () ;
	if (!ParseValue (node, "h", wall.h)) 
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	node = node->GetNextEntity() ;
	if (ParseMaterialRef (node, wall.mat))
	{
		node = node->GetNextEntity() ;
	}
	else
	{
		wall.mat = getMaterial ("A0") ;
	}
	if (node != 0)
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	print_debug ("ParseVerticalWallExt OK \n") ;
	return true ;
}
/*
 * Parse a Vertical Edge extension record
 */
static bool ParseVerticalEdgeExt (XMLNode* node, VerticalEdgeExt &edge)
{
	if (!checkTagName (node, "edge")) return false ;
	node = node->GetFirstChild () ;
	if (!ParseValue (node, "h", edge.h))
	{
		signal_error (XMLMissingTag ("h",node)) ;
		return false ;
	}
	node = node->GetNextEntity() ;
	if (node != 0) 
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	print_debug ("ParseVerticalEdgeExt OK \n") ;
	return true ;
}
/*
 * Parse an extension substitute group
 */
static bool ParseVerticalExtType (XMLNode* node, System::ref_ptr<VerticalExt>& ext)
{
	if (node == 0) return false ;

	print_debug ("parse extension type [%s] \n", node->GetName()) ;

	SourceExt source ;
	if (ParseSourceExt (node, source)) 
	{
		ext = new SourceExt (source) ; return true ;
	}

	ReceiverExt receiver ;
	if (ParseReceiverExt (node, receiver)) 
	{
		ext = new ReceiverExt (receiver) ; return true ;
	}

	BarrierExt barrier ;
	if (ParseBarrierExt (node, barrier)) 
	{
		ext = new BarrierExt (barrier) ; return true ;
	}

	VerticalWallExt wall ;
	if (ParseVerticalWallExt (node, wall)) 
	{
		ext = new VerticalWallExt (wall) ; return true ;
	}

	VerticalEdgeExt edge ;
	if (ParseVerticalEdgeExt (node, edge)) 
	{
		ext = new VerticalEdgeExt (edge) ; return true ;
	}

	signal_error (XMLUnexpectedTag (node)) ;
	return false ;
}
/*
 * Parse an extension. Note that an extension tag must contain exactly one element 
 * from the substitute group (Source, Receiver, Barrier, VerticalWall or VerticalEdge)
 */
static bool ParseVerticalExt (XMLNode* input_node, System::ref_ptr<VerticalExt>& ext)
{
	if (!checkTagName (input_node, "ext")) return false ;
	XMLNode* node = input_node->GetFirstChild() ;
	if (node == 0)
	{
		signal_error (XMLParseError ("Missing extension type", input_node)) ;
		return false ;
	}
	if (!ParseVerticalExtType (node, ext)) return false; 
	node = node->GetNextEntity() ;
	if (node != 0)
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	print_debug ("ParseVerticalExt OK (type = %s) \n", typeid(*ext).name()) ;
	return true ;
}
/*
 * Parse (and register) material properties
 */
static bool ParseMaterialProperties (XMLNode* node)
{
	if (!checkTagName (node, "mat")) return false ;
	
	const char* id = node->GetAttribute ("id") ;
	Material* mat = getMaterial(id, true) ;

	node = node->GetFirstChild() ;
	double G = 0 ;
	if (!ParseValue (node, "G", G)) return false ;
	mat->setG (G) ;
	node = node->GetNextEntity() ;
	double sigma = 0 ;
	if (ParseValue (node, "sigma", sigma)) 
	{
		mat->setSigma (sigma) ;
		node = node->GetNextEntity() ;
	}
	Spectrum alpha ;
	if (ParseSpectrum (node, "alpha", alpha)) 
	{
		mat->setAlpha (alpha) ;
		node = node->GetNextEntity() ;
	}
	if (node != 0) 
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	print_debug ("Parse Material Properties OK \n") ;
	return true ;
}
/*
 * Parse a control point
 */
bool ParseControlPoint (XMLNode* node, ControlPoint& cp)
{
	if (!checkTagName (node, "cp")) return false ;

	node = node->GetFirstChild() ;
	if (!ParsePosition (node, cp.pos)) return false ;
	current_cp = cp.pos ;

	node = node->GetNextEntity() ;
	if (ParseMaterialRef (node, cp.mat)) node = node->GetNextEntity() ;

	cp.ext = 0 ;
	if (ParseVerticalExt (node, cp.ext)) 
	{
		if (cp.ext == 0 || cp.ext->h <= 0)
		{
			signal_error (XMLParseError ("Parameter <h> must contain a value greater than zero", node)) ;
			return false ;
		}
		node = node->GetNextEntity() ;
	}

	if (node != 0) 
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	print_debug ("ParseControlPoint OK \n") ;
	print_debug (".pos = %.2f %.2f %.2f \n", cp.pos.x, cp.pos.y, cp.pos.z) ;
	return true ;
}
/*
 * Parse a propagation path
 */
bool ParsePropagationPath (XMLNode* input_node, PropagationPath& path)
{
	static const char* tag = "path" ;

	if (!checkTagName (input_node, tag)) 
	{
		signal_error (XMLMissingTag (tag, input_node)) ;
		return false ;
	}

	path.clear() ;
	XMLNode* node = input_node->GetFirstChild() ;

	ControlPoint cp ;
	cp.pos.x = 0 ;
	cp.pos.y = 0 ;
	cp.pos.z = 0 ;
	cp.mat = getMaterial ("H") ;
	cp.ext = 0 ;
	while (ParseControlPoint (node, cp))
	{
		path.add (cp) ;
		node = node->GetNextEntity() ;
	}
	if (node != 0) 
	{
		throw XMLUnexpectedTag (node) ;
		return false ;
	}
	if (path.size() < 2) 
	{
		signal_error (XMLParseError ("A valid path must contain at least 2 control points", input_node)) ;
		return false ;
	}
	path[0].mat = path[1].mat ;
	print_debug ("ParsePath OK \n") ;
	return true ;
}
/*
 * Parse calculation method
 */
static bool ParseCalculationMethod (XMLNode* node, CalculationMethod*& method)
{
	if (!checkTagName (node, "select")) 
	{
		signal_error (XMLMissingTag ("select", node)) ;
		return false ;
	}
	const char* id = node->GetAttribute ("id") ;
	method = getCalculationMethod(id) ;
	if (method == 0)
	{
		signal_error (XMLParseError ("Unkown calculation method", node)) ;
		return false ;
	}
	print_debug ("Parse method = %s\n", method->name()) ;
	return true ;
}
/*
 *
 */
static bool ParseOption (const char* attrib, const char*name, bool& option, bool value)
{
	if (strcmp (attrib, name) == 0)
	{
		option = value ;
		return true ;
	}
	return false ;
}
/*
 * Parse optional calculation parameters
 */
static bool ParseOptions (XMLNode* node, PropagationPathOptions& options)
{
	if (!checkTagName (node, "options")) return false ;

	node = node->GetFirstChild() ;
	while (node != 0)
	{
		const char* value  = node->GetAttribute ("value") ;
		if (value == 0 || strlen(value) == 0)
		{
			signal_error (XMLParseError("Missing attribute (value)", node)) ;
		}
		bool option_value = false ;
		if (_strcmpi (value, "true") == 0) option_value = true ;

		const char* option = node->GetAttribute("id") ;

		if (option == 0 || strlen(option) == 0)
		{
			signal_error (XMLParseError("Missing attribute (id)", node)) ;
		}
		else if (ParseOption (option,"ForceSourceToReceiver", options.ForceSourceToReceiver, option_value)
		      || ParseOption (option,"CheckHorizontalAlignment",options.CheckHorizontalAlignment, option_value)
			  || ParseOption (option,"CheckLateralDiffraction",options.CheckLateralDiffraction, option_value)
			  || ParseOption (option,"DisableReflections", options.DisableReflections, option_value)
			  || ParseOption (option,"DisableLateralDiffractions", options.DisableLateralDiffractions, option_value)
			  || ParseOption (option,"CheckHeightLowerBound", options.CheckHeightLowerBound, option_value)
			  || ParseOption (option,"CheckHeightUpperBound", options.CheckHeightUpperBound, option_value)
			  || ParseOption (option,"CheckSourceSegment", options.CheckSourceSegment, option_value)
			  || ParseOption (option,"CheckSoundPowerUnits", options.CheckSoundPowerUnits, option_value)
			  || ParseOption (option,"SimplifyPathGeometry", options.SimplifyPathGeometry, option_value)
			  || ParseOption (option,"IgnoreComplexPaths", options.IgnoreComplexPaths, option_value)
			  || ParseOption (option,"ExcludeGeometricalSpread", options.ExcludeGeometricalSpread, option_value)
			  || ParseOption (option,"ExcludeAirAbsorption", options.ExcludeAirAbsorption, option_value)
			  || ParseOption (option,"ExcludeSoundPower", options.ExcludeSoundPower, option_value))
		{
			print_debug ("Parse option %s=%s\n", option, option_value ? "true" : "false") ;
		}
		else
		{
			print_debug ("WARNING: unknown option \"%s\" ignored \n", option) ;
		}
		node = node->GetNextEntity() ;
	}
	print_debug ("Parse options OK \n") ;
	return true ;
}
/*
 * Parse meteorological parameters
 */
static bool ParseMeteo (XMLNode* node, MeteoCondition &meteo)
{
	if (!checkTagName (node, "meteo")) return false ;

	const char*attrib = node->GetAttribute ("model") ;
	if (attrib)
	{
		if (strcmp(attrib, "ISO-9613-2")     == 0) meteo.model = MeteoCondition::ISO9613 ;
		if (strcmp(attrib, "JRC-2012")       == 0) meteo.model = MeteoCondition::JRC2012 ;
		if (strcmp(attrib, "NMPB-2008")      == 0) meteo.model = MeteoCondition::JRC2012 ;
	}
	node = node->GetFirstChild() ;
	if (ParseValue (node, "temperature", meteo.temperature)) node = node->GetNextEntity() ;
	if (ParseValue (node, "humidity",    meteo.humidity))	 node = node->GetNextEntity() ;
	if (ParseValue (node, "pFav",        meteo.pFav))		 node = node->GetNextEntity() ;
	if (ParseValue (node, "C0",          meteo.C0))			 node = node->GetNextEntity() ;
	if (node != 0)
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}
	return true ;
}
/*
 * Parse optional calculation parameters
 */
static bool ParseParameters (XMLNode* node, PropagationPathOptions& options)
{
	if (!checkTagName (node, "method"))  
	{
		signal_error (XMLMissingTag ("method", node)) ;
		return false ;
	}

	node = node->GetFirstChild() ;

	if (ParseCalculationMethod (node, options.method)) node = node->GetNextEntity () ;
	
	if (ParseOptions (node, options)) node = node->GetNextEntity() ;

	if (ParseMeteo(node, options.meteo)) node = node->GetNextEntity() ;

	if (node != 0)
	{
		signal_error (XMLUnexpectedTag (node)) ;
		return false ;
	}

	print_debug ("Parse parameters OK\n") ;
	return true ;
}
/*
 * Parse optional material definitions
 */
static bool ParseMaterials (XMLNode* node)
{
	if (! checkTagName (node, "materials")) return false ;

	node = node->GetFirstChild() ;
	while (node != 0)
	{
		if (!ParseMaterialProperties(node))
		{
			signal_error (XMLUnexpectedTag (node)) ;
			return false ;
		}
		node = node->GetNextEntity() ;
	}
	return true ;
}
/*
 * public API : parse a CnossosEU-XML file starting from the root node
 */
namespace CnossosEU
{
	bool ParsePathFromFile (XMLNode* node, PropagationPath& path, PropagationPathOptions& options) 
	{
		path.clear() ;

		static const char* root = "CNOSSOS-EU" ;
		if (!checkTagName (node, root)) 
		{
			signal_error (XMLMissingTag (root, node)) ;
			return false ;
		}

		node = node->GetFirstChild() ;

		if (ParseParameters (node, options)) node = node->GetNextEntity() ;

		if (ParseMaterials (node)) node = node->GetNextEntity() ;

		if (ParsePropagationPath (node, path)) node = node->GetNextEntity() ; 

		if (node != 0)
		{
			signal_error (XMLUnexpectedTag (node)) ;
			return false ;
		}
		return true ;
	}
}
