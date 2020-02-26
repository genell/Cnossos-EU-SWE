#include "PathParseOctave.h"
#include "../PropagationPath/VerticalExt.h"
#include "../PropagationPath/Material.h"
#include "../system/environment.h"
using namespace CnossosEU;

#ifdef DEBUG
#include <cstdio>
#include <iostream>
#define print_debug std::cout.flush();printf
#else
#define print_debug //
#endif

/*
 Utils
*/
bool iequals(const std::string& a, const std::string& b)
{
    unsigned int sz = a.size();
    if (b.size() != sz)
        return false;
    for (unsigned int i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

/*
 * parse a numeric option 
 */
static bool ParseValue(const std::string &attr, const octave_value &aValue, const std::string &name, double& value)
{
	if (!iequals (attr, name)) return false ;

	if (!aValue.is_double_type())
	{
		error("%s does not have a numeric value", aValue.string_value().c_str());
		return false;
	}
	value = aValue.double_value();
	print_debug("ParseValue [%s] = %.2f \n", name.c_str(), value) ;
	return true ;
}

static bool ApplyValue(const octave_value &aValue, double& value)
{
	if (!aValue.is_double_type())
	{
		return false;
	}
	value = aValue.double_value();
	return true ;
}

/*
 * parse a tag containing a spectrum, note that spectra are stored as arrays of 
 * floating-point values where the size of the array is a constant defined in the
 * Spectrum class.
 */
static bool ParseSpectrum (const std::string &attr, const octave_value &aValue, const std::string &name, Spectrum& spec)
{
	if (!iequals(attr, name)) return false;

	// Must be an array of size 8
	if (aValue.ndims() != 2 || aValue.dims().elem(1) < Spectrum::nbFreq)
		return false;

	auto arr = aValue.array_value();	
	for (unsigned int i = 0 ; i < Spectrum::nbFreq ; i++)
	{
		spec[i] = arr.data()[i];
	}

	print_debug ("ParseSpectrum OK \n") ;
	return true ;
}
/*
 * parse a position. A position is a triplet of (x,y,z) coordinates. 
 * Note that all three coordinates are optional. In case one of the coordinates is missing, 
 * its value will not be affected by this call.
 */
static bool ParsePosition (const octave_value &aVal, Position& pos)
{
	if (!aVal.is_map())
		return false;

	auto aMap = aVal.scalar_map_value();

	for (auto iOpt = aMap.begin(); iOpt!=aMap.end(); ++iOpt)
	{
		auto cKey = aMap.key(iOpt);
		auto cDef = aMap.contents(iOpt);

		auto found = ParseValue (cKey, cDef, "x", pos.x);
		if (!found)
			found = ParseValue (cKey, cDef, "y", pos.y);
		if (!found)
			found = ParseValue (cKey, cDef, "z", pos.z);
	}
	print_debug ("ParsePosition OK \n") ;
	return true ;
}
/*
 * Parse a reference to a material defined in project's database
 */
static bool ParseMaterialRef (const std::string &matRef, System::ref_ptr<Material>& mat)
{
	mat = getMaterial(matRef.c_str());
	if (mat == 0) 
	{
		error("Invalid material id reference %s\n", matRef.c_str());
		return false ;
	}
	print_debug ("ParseMaterial [id=%s, G=%.2f] OK \n", matRef.c_str(), mat->getGValue()) ;
	return true ;
}
/*
 * Parse sound power spectrum
 */
static bool ParseElementarySource (const octave_value &aVal, ElementarySource &source)
{
	if (!aVal.is_map())
		return false;
	auto aMap = aVal.scalar_map_value();

	auto vSpec = aMap.getfield("spectrum");
	if (vSpec.is_empty() || !ParseSpectrum("spectrum", vSpec, "Lw", source.soundPower)) return false ;

	auto vMType = aMap.contents("measurementType").string_value();
	if (iequals(vMType, "FreeField")) source.measurementType = MeasurementType::FreeField;
	else if (iequals(vMType, "HemiSpherical")) source.measurementType = MeasurementType::HemiSpherical;

	auto vSType = aMap.contents("sourceType").string_value();
	if (iequals(vSType, "PointSource")) source.spectrumType = SpectrumType::PointSource;
	else if (iequals(vSType, "LineSource")) source.spectrumType = SpectrumType::LineSource;
	else if (iequals(vSType, "AreaSource")) source.spectrumType = SpectrumType::AreaSource;

	auto vFWeight = aMap.contents("frequencyWeighting").string_value();
	if (iequals(vFWeight, "LIN")) source.frequencyWeighting = FrequencyWeighting::dBLIN;
	else if (iequals(vFWeight, "dBA")) source.frequencyWeighting = FrequencyWeighting::dBA;

	return true ;
}
/*
 * Parse point geometry extension
 */
static bool ParsePointSource (const octave_value &aVal, System::ref_ptr<SourceGeometry> &geo) 
{
	if (!aVal.is_map())
		return false;

	auto aMap = aVal.scalar_map_value();

	/*
	 * parse orientation (default is X_axis)
	 */
	Geometry::Vector3D orientation (1.0, 0.0, 0.0) ;
	if (!ParsePosition(aMap.getfield("orientation"), orientation))
		return false;

	/*
	 * return point source geometry
	 */
	geo = new PointSource (orientation) ;
	return true ;
}
/*
 * Parse source geometry extension
 */
static bool ParseLineSource (const octave_value &aVal, System::ref_ptr<SourceGeometry> &geo) 
{
	if (!aVal.is_map())
		return false;

	auto aMap = aVal.scalar_map_value();

	/*
	 * parse length (default = 1m)
	 */
	double length  = 1.0;
	ApplyValue(aMap.getfield("length"), length);

	/*
	 * parse orientation (default = Y_asis)
	 */
	Geometry::Vector3D orientation (0.0, 1.0, 0.0) ;
	ParsePosition(aMap.getfield("orientation"), orientation);
	
	/*
	 * return line source geometry
	 */
	geo = new LineSource (length, orientation) ;
	return true ;
}
/*
 * Parse source geometry extension
 */
static bool ParseAreaSource (const octave_value &aVal, System::ref_ptr<SourceGeometry> &geo) 
{
	if (!aVal.is_map())
		return false;

	auto aMap = aVal.scalar_map_value();

	/*
	 * parse area, default = 1m^2
	 */
	double area  = 1.0 ;
	ApplyValue(aMap.getfield("area"), area);
	/*
	 * parse orientation (default = Z_asis)
	 */
	Geometry::Vector3D orientation(0.0, 0.0, 1.0);
	ParsePosition(aMap.getfield("orientation"), orientation);

	/*
	 * return area source geometry
	 */
	geo = new AreaSource(area, orientation);
	return true;
}
/*
 * Keep track of position of current control point
 */
static Geometry::Point3D current_cp ;
/*
 * Parse source geometry extension
 */
static bool ParseSegmentSource (const octave_value &aVal, System::ref_ptr<SourceGeometry> &geo) 
{
	if (!aVal.is_map())
		return false;

	auto aMap = aVal.scalar_map_value();

	Geometry::Point3D p1(current_cp) ;
	Geometry::Point3D p2(current_cp) ;
	double fixedAngle = 0 ;
	/*
	 * parse start position (default = position of current control point)
	 */
	ParsePosition(aMap.getfield("posStart"), p1);
	/*
	 * parse end position (default = position of current control point)
	 */
	ParsePosition(aMap.getfield("posEnd"), p2);
	/*
	 * parse fixed viewing angle (default = 0)
	 */
	ApplyValue(aMap.getfield("fixedAngle"), fixedAngle);
	/*
	 * return source line segment geometry
	 */
	geo = new LineSegment(p1, p2, fixedAngle) ;
	
	print_debug (".start x=%.1f y=%.1f z=%.1f \n", p1.x, p1.y, p1.z) ;
	print_debug (".end   x=%.1f y=%.1f z=%.1f \n", p2.x, p2.y, p2.z) ;
	return true ;
}
/*
 * Parse a Source extension record
 */
static bool ParseSourceExt (const octave_scalar_map &aSource, SourceExt &source)
{
	auto hVal = aSource.getfield("h");
	/*
	* source height is mandatory
	*/
	if (!ApplyValue(aSource.getfield("h"), source.h))
	{
		error("source extension - h field is mandatory");
		return false ;
	}
	
	source.h = hVal.double_value();

	for (auto iOpt = aSource.begin(); iOpt!=aSource.end(); ++iOpt)
	{
		auto name = (*iOpt).first;
		auto value = aSource.getfield(name);

		/*
		* source power is always optional
		*/
		if (iequals(name, "Lw"))
			ParseElementarySource (value, source.source);

		/*
		* extended source geometry is optional but never defined in an external file
		*/
		else if (iequals(name,"pointSource"))
			ParsePointSource(value, source.geo);
		else if (iequals(name,"lineSource"))
			ParseLineSource(value, source.geo);
		else if (iequals(name,"areaSource"))
			ParseAreaSource(value, source.geo);
		else if (iequals(name,"lineSegment"))
			ParseSegmentSource(value, source.geo);
	}
	
	print_debug ("ParseSourceExt OK \n") ;
	return true ;
}
/*
 * Parse a Receiver extension record
 */
static bool ParseReceiverExt (const octave_scalar_map &aMap, ReceiverExt &receiver)
{
	if (!ApplyValue(aMap.getfield("h"), receiver.h))
	{
		error("receiver extension - h field is mandatory");
		return false ;
	}
	print_debug ("ParseReceiverExt OK \n") ;
	return true ;
}
/*
 * Parse a Barrier extension record
 */
static bool ParseBarrierExt (const octave_scalar_map &aMap, BarrierExt &barrier)
{
	if (!ApplyValue(aMap.getfield("h"), barrier.h))
	{
		error("barrier extension - h field is mandatory");
		return false;
	}

	auto vMat = aMap.contents("mat").string_value();
	if (!ParseMaterialRef(vMat, barrier.mat))
	{
		barrier.mat = getMaterial ("A0") ;
	}

	print_debug ("ParseBarrierExt OK \n") ;
	return true ;
}
/*
 * Parse a VerticalWall extension record
 */
static bool ParseVerticalWallExt (const octave_scalar_map &aMap, VerticalWallExt &wall)
{
	if (!ApplyValue(aMap.getfield("h"), wall.h))
	{
		error("wall extension - h field is mandatory");
		return false;
	}

	auto vMat = aMap.contents("mat").string_value();
	if (!ParseMaterialRef(vMat, wall.mat))
	{
		wall.mat = getMaterial ("A0") ;
	}

	print_debug ("ParseVerticalWallExt OK \n") ;
	return true ;
}
/*
 * Parse a Vertical Edge extension record
 */
static bool ParseVerticalEdgeExt (const octave_scalar_map &aMap, VerticalEdgeExt &edge)
{
	if (!ApplyValue(aMap.getfield("h"), edge.h))
	{
		error("edge extension - h field is mandatory");
		return false;
	}

	print_debug ("ParseVerticalEdgeExt OK \n") ;
	return true ;
}
/*
 * Parse an extension substitute group
 */
static bool ParseVerticalExtType (const std::string &extName, const octave_scalar_map &aExt, System::ref_ptr<VerticalExt>& ext)
{
	print_debug("parse extension type [%s] \n", extName.c_str());

	SourceExt source ;
	if (iequals(extName,"source") && ParseSourceExt(aExt, source)) 
	{
		ext = new SourceExt(source); return true;
	}

	ReceiverExt receiver ;
	if (iequals(extName,"receiver") && ParseReceiverExt(aExt, receiver)) 
	{
		ext = new ReceiverExt(receiver); return true;
	}

	BarrierExt barrier ;
	if (iequals(extName,"barrier") && ParseBarrierExt(aExt, barrier)) 
	{
		ext = new BarrierExt(barrier); return true;
	}

	VerticalWallExt wall ;
	if (iequals(extName,"wall") && ParseVerticalWallExt(aExt, wall)) 
	{
		ext = new VerticalWallExt(wall); return true;
	}

	VerticalEdgeExt edge ;
	if (iequals(extName,"edge") && ParseVerticalEdgeExt(aExt, edge)) 
	{
		ext = new VerticalEdgeExt(edge); return true;
	}

	return false ;
}

/*
 * Parse (and register) material properties
 */
static bool ParseMaterialProperties (const std::string &matId, const octave_scalar_map &matDef)
{
	const char* id = matId.c_str();
	Material* mat = getMaterial(id, true) ;

	for (auto iOpt = matDef.begin(); iOpt!=matDef.end(); ++iOpt)
	{
		auto mdKey = matDef.key(iOpt);
		auto mdValue = matDef.contents(iOpt);

		double G = 0 ;
		if (ParseValue (mdKey, mdValue, "G", G))
			mat->setG(G);

		double sigma = 0 ;
		if (ParseValue (mdKey, mdValue, "sigma", sigma)) 
			mat->setSigma(sigma);

		Spectrum alpha ;
		if (ParseSpectrum (mdKey, mdValue, "alpha", alpha)) 
			mat->setAlpha(alpha);
	}

	print_debug ("Parse Material Properties OK \n") ;
	return true ;
}
/*
 * Parse a control point
 */
bool ParseControlPoint (const octave_scalar_map &optCp, ControlPoint& cp)
{
	for (auto iOpt = optCp.begin(); iOpt!=optCp.end(); ++iOpt)
	{
		auto cpKey = optCp.key(iOpt);
		auto cpProp = optCp.contents(iOpt);

		if (iequals(cpKey,"pos")) {
			if (!cpProp.is_map())
				error("control point pos property must be a struct");

			if (!ParsePosition (cpProp.scalar_map_value(), cp.pos)) return false ;
				current_cp = cp.pos ;
		}
		else if (iequals(cpKey,"mat")) {
			if (!cpProp.is_string())
				error("control point mat property must be a material reference");
			ParseMaterialRef(cpProp.string_value(), cp.mat);
		}
		else {
			// Neither pos nor mat means ext type - try to parse
			cp.ext = 0 ;
			if (!cpProp.is_map())
				error("control point ext property must be a struct");
			if (ParseVerticalExtType(cpKey, cpProp.scalar_map_value(), cp.ext)) 
			{
				if (cp.ext == 0 || cp.ext->h <= 0)
				{
					error("Parameter (h) must contain a value greater than zero");
					return false ;
				}
			}
		}
	}
	print_debug ("ParseControlPoint OK \n") ;
	print_debug (".pos = %.2f %.2f %.2f \n", cp.pos.x, cp.pos.y, cp.pos.z) ;
	return true ;
}
/*
 * Parse a propagation path
 */
bool ParsePropagationPath (const octave_scalar_map &optPath, PropagationPath& path)
{
	path.clear() ;

	if (optPath.keys().numel() < 2) 
	{
		error("A valid path must contain at least 2 control points");
		return false ;
	}

	for (auto iOpt = optPath.begin(); iOpt!=optPath.end(); ++iOpt)
	{
		auto cpKey = optPath.key(iOpt);
		auto cpDef = optPath.contents(iOpt);
		print_debug("Parsing point '%s'", cpKey.c_str());

		if (!cpDef.is_empty() && !cpDef.is_map())
			error("expected path control point definition %s to be a struct", cpKey.c_str());

		ControlPoint cp ;
		cp.pos.x = 0 ;
		cp.pos.y = 0 ;
		cp.pos.z = 0 ;
		cp.mat = getMaterial ("H") ;
		cp.ext = 0 ;
		if (ParseControlPoint(cpDef.scalar_map_value(), cp))
			path.add (cp) ;
	}
	path[0].mat = path[1].mat ;

	print_debug ("ParsePath OK \n") ;
	return true ;
}

static bool ParseOption (const std::string &attrib, const std::string &name, bool& option, bool value)
{
	if (iequals(attrib, name) == 0)
	{
		option = value ;
		return true ;
	}
	return false ;
}
/*
 * Parse optional calculation parameters
 */
static bool ParseOptions (const octave_scalar_map &optargs, PropagationPathOptions& options)
{
	for (auto iOpt = optargs.begin(); iOpt!=optargs.end(); ++iOpt)
	{
		auto option = optargs.key(iOpt);
		bool option_value = false;
		if (!optargs.contents(iOpt).is_bool_scalar())
			error("option %s must have a boolean value\n", option.c_str());

		option_value = optargs.contents(iOpt).bool_value();

		if (ParseOption (option,"ForceSourceToReceiver", options.ForceSourceToReceiver, option_value)
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
			print_debug ("Parse option %s=%s\n", option.c_str(), option_value ? "true" : "false") ;
		}
		else
		{
			print_debug ("WARNING: unknown option \"%s\" ignored \n", option.c_str()) ;
		}
	}

	print_debug ("Parse options OK \n") ;
	return true ;
}
/*
 * Parse meteorological parameters
 */
static bool ParseMeteo (const octave_scalar_map &optmeteo, MeteoCondition &meteo)
{
	for (auto iOpt = optmeteo.begin(); iOpt!=optmeteo.end(); ++iOpt)
	{
		auto option = optmeteo.key(iOpt);
		auto oValue = optmeteo.contents(iOpt);

		if (iequals(option,"model")) {
			if (!oValue.is_string())
				error("option meteo.model must have a string value\n");
			auto model = oValue.string_value();
			// Match enum
			if (iequals(model, "ISO-9613-2")) meteo.model = MeteoCondition::ISO9613 ;
			else if (iequals(model, "JRC-2012")) meteo.model = MeteoCondition::JRC2012 ;
			else if (iequals(model, "NMPB-2008")) meteo.model = MeteoCondition::JRC2012 ;
		}

		bool found = ParseValue (option, oValue, "temperature", meteo.temperature);
		if (!found)
			found = ParseValue (option, oValue, "humidity",    meteo.humidity);
		if (!found)
			found = ParseValue (option, oValue, "pFav",        meteo.pFav);
		if (!found)
			found = ParseValue (option, oValue, "C0",          meteo.C0);
	}
	return true ;
}

/*
 * Parse optional calculation parameters
 */
bool ParseParameters (const octave_scalar_map &aOptions, const octave_scalar_map &aMeteo, PropagationPathOptions& options)
{	
	if (aOptions.nfields() > 0)
		ParseOptions(aOptions, options);

	if (aMeteo.nfields() > 0)
		ParseMeteo(aMeteo, options.meteo);

	print_debug ("Parse parameters OK\n") ;
	return true ;
}

/*
 * Parse optional material definitions
 */
bool ParseMaterials(const octave_scalar_map& materials)
{
	for (auto iOpt = materials.begin(); iOpt!=materials.end(); ++iOpt)
	{
		auto matKey = materials.key(iOpt);
		auto matDef = materials.contents(iOpt);

		if (!matDef.is_empty() && !matDef.is_map())
			error("expected material definition %s to be a struct", matKey.c_str());

		ParseMaterialProperties(matKey, matDef.scalar_map_value());
	}
	return true ;
}
