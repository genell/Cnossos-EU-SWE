#include "roadnoisecatalog.h"
#include "roadnoisesegment.h"
#include "roadnoisevehiclecategory.h"
#include "roadnoiseconst.h"
#include "../tinyxml/tinyxml.h"

#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setw
#include <sstream>

RoadNoiseCatalog::RoadNoiseCatalog()
{
  refTemp = DEFAULT_REF_TEMP;
  refSpeed = DEFAULT_REF_SPEED;
  minSpeed = DEFAULT_MIN_SPEED;
  srcHeight = 0.05; // TODO: DEFAULT_SRC_HEIGHT

  numCategories = 0;
  surfaces = list<RoadSurface>();
}

RoadNoiseCatalog::~RoadNoiseCatalog()
{
  for (int i = numCategories; i >= 0; i--)
  {
    delete Category[i];
    numCategories--;
  }
}



RoadNoiseVehicleCategory* RoadNoiseCatalog::addCategory()
{
  RoadNoiseVehicleCategory* result = new RoadNoiseVehicleCategory();
  Category[numCategories++] = result;
  return result;
}

int RoadNoiseCatalog::indexOfCategory(const string id)
{
  for (int i = 0; i < numCategories; i++)
  {
    if (id.compare(Category[i]->id) == 0)
    {
      return i;
    }
  }
  return -1;
}

RoadNoiseVehicleCategory* RoadNoiseCatalog::getCategory(const string id)
{
  return getCategory(indexOfCategory(id));
}

RoadNoiseVehicleCategory* RoadNoiseCatalog::getCategory(const int index)
{
  if (index >= 0 && index < numCategories)
  {
    return Category[index];
  }
  else
  {
    return NULL;
  }
}

/////////////////////////////////


// --------------------------------------------------------------------------------------------------------
void setGradientCorrections(TiXmlElement *cat, const string childName, GradientCategory *gradientCorrection, const int index)
{
  TiXmlElement *e = cat->FirstChildElement(childName.c_str());
  if (e != NULL)
  {
    double value = 0.0;
    bool useSpeed = false;
    if (e->QueryDoubleAttribute("a1", &value) == TIXML_SUCCESS)
      gradientCorrection->Classes[index].a1 = value;
    if (e->QueryDoubleAttribute("a2", &value) == TIXML_SUCCESS)
      gradientCorrection->Classes[index].a2 = value;
    if (e->QueryBoolAttribute("UseSpeed", &useSpeed) == TIXML_SUCCESS)
      gradientCorrection->Classes[index].calcSpeedFactor = useSpeed;
    if (e->QueryDoubleAttribute("a3", &value) == TIXML_SUCCESS)
      gradientCorrection->Classes[index].a3 = value;
  }
}

// --------------------------------------------------------------------------------------------------------
// Auxiliary function to read a double from a string
//
// Parameters :
//				s		: string containing a double
//              def		: default value in case of an invalid string
// --------------------------------------------------------------------------------------------------------
double parseFloat(const string s, const double def = 0.0)
{
  double result = def;
  if (TIXML_SSCANF(s.c_str(), "%lf", &result) == 1)
    return result;
  else
    return def;
}

// --------------------------------------------------------------------------------------------------------
// Auxiliary function to read an integer from a string
//
// Parameters :
//				s		: string containing an integer
//              def		: default value in case of an invalid string
// --------------------------------------------------------------------------------------------------------
int parseInt(const string s, const int def = 0)
{
  int result = def;
  if (TIXML_SSCANF(s.c_str(), "%d", &result) == 1)
    return result;
  else
    return def;
}

// --------------------------------------------------------------------------------------------------------
int copyFloatsFromString(const string s, double *f, const int count)
{
  string token;
  istringstream iss(s);
  iss.imbue(locale("C", locale::numeric));
  int i = 0;
  while (iss >> token && i < count)
  {
    f[i] = parseFloat(token);
    i++;
  }
  return i + 1;
}

// --------------------------------------------------------------------------------------------------------
string stringFromFloats(const double *f, const int count, const char separator = ' ')
{
  ostringstream oss;
  oss.imbue(locale("C", locale::numeric));
  for (int i = 0; i < count; i++)
  {
    if (i > 0)
    {
      oss << separator;
    }
    oss << f[i];
  }
  return oss.str();
}

// --------------------------------------------------------------------------------------------------------
// Auxiliary function to read a double from a named child element
//
// Parameters :
//				e		: parent XML element
//              name	: name of child element
//              def		: default value in case of a missing element or an invalid string
// --------------------------------------------------------------------------------------------------------
double floatFromElement(TiXmlElement *e, const string name, const double def = 0.0)
{
  TiXmlElement *child = e->FirstChildElement(name.c_str());
  if (child != NULL)
    return parseFloat(child->GetText(), def);
  else
    return def;
}

// --------------------------------------------------------------------------------------------------------
// Auxiliary function to read an integer from a named child element
//
// Parameters :
//				e		: parent XML element
//              name	: name of child element
//              def		: default value in case of a missing element or an invalid string
// --------------------------------------------------------------------------------------------------------
int intFromElement(TiXmlElement *e, const string name, const int def = 0)
{
  TiXmlElement *child = e->FirstChildElement(name.c_str());
  if (child != NULL)
    return parseInt(child->GetText());
  else
    return def;
}

// --------------------------------------------------------------------------------------------------------
// Auxiliary function to write an error message to the error stream
//
// Parameters :
//				message	: error message
//              fn		: name of the file being processed when the error occurred. Defaults to empty.
//              xmlNode	: the XML node being being processed when the error occurred. Defaults to NULL.
// --------------------------------------------------------------------------------------------------------
void ReportError(const string message, const string fn = "", const TiXmlNode *xmlNode = NULL)
{
  cerr << message;
  if (!fn.empty())
  {
    cerr << " (" << fn;
    if (xmlNode != NULL)
    {
      cerr << "; " << xmlNode->Row() << "," << xmlNode->Column();
    }
    cerr << ")";
  }
  else if (xmlNode != NULL)
  {
    cerr << " (" << xmlNode->Row() << "," << xmlNode->Column() << ")";
  }
  cerr << endl;
}
#pragma endregion Helper_functions

#pragma region File_functions
// --------------------------------------------------------------------------------------------------------
// function to read the input xml file
//
// Parameter :
//				fn		: filename of the xml input file
//
//
// --------------------------------------------------------------------------------------------------------
bool  RoadNoiseSegment::loadFromXMLFile(const string fn)
{
  TiXmlDocument doc(fn.c_str());
  if (doc.LoadFile())
  {
    TiXmlElement *root = doc.FirstChildElement();

    TiXmlElement *segment = root->FirstChildElement("RoadSegment");
    if (segment == NULL)
    {
      ReportError("Error loading file: RoadSegment not found.", fn);
      return false;
    }

    TiXmlElement *e = segment->FirstChildElement("Test");
    if (e != NULL)
    {
      string test = e->GetText();
      this->doDebug = (test != "0") && (test != "false");
    }

    TempProp = new TempProperties(floatFromElement(segment, "Taverage"));
    GradProp = new GradientProperties(floatFromElement(segment, "Slope"));
    // TODO: SetStuddedMonths(int)?
    studdedMonths = intFromElement(segment, "Tstudded", 0);

    e = segment->FirstChildElement("SpeedVariations");
    if (e != NULL)
    {
      if (intFromElement(e, "Type") != 0)
      {
        AccProp = new AccelerationProperties(floatFromElement(e, "Distance"),
                                             intFromElement(e, "Type"));
      }
      else
      {
        if (AccProp != NULL)
          delete AccProp;
        AccProp = NULL;
      }
    }
    else
      ReportError("No SpeedVariations found", fn, segment);

    e = segment->FirstChildElement("Surface");
    if (e != NULL)
    {
      // setting the surface ID of the current segment will cause it to load the relevant road surface coefficients
      if (setSurfaceID(e->Attribute("Ref")) == NULL)
      {
        ReportError("Unknown surface", fn, e);
      }
    }

    TiXmlElement *cat = segment->FirstChildElement("Category");
    while (cat != NULL)
    {
      int m = catalog->indexOfCategory(cat->Attribute("Ref"));
      if (m >= 0)
      {
        // TODO: SetCategoryProperties(int categoryIndex, double Q, double V, double Fstud)
        this->Q[m] = floatFromElement(cat, "Q");
        this->v[m] = floatFromElement(cat, "V");
        this->Fstud[m] = floatFromElement(cat, "Fstud");
      }
      else
        ReportError("Unknown category", fn, cat);

      cat = cat->NextSiblingElement("Category");
    }
    return true;

  }
  else
  {
    ReportError(doc.ErrorDesc(), fn);
    return false;
    //throw doc.ErrorId();
  }
}

// --------------------------------------------------------------------------------------------------------
string floatToString(double n)
{
  ostringstream oss;
  oss.imbue(locale("C", LC_ALL));
  oss << n;
  return oss.str();
}

// --------------------------------------------------------------------------------------------------------
// private function to write the calculated values to a xml output file
//
// Parameter :
//				fn		: filename to contain the results of the roadsegment calculations
//
//
// --------------------------------------------------------------------------------------------------------
bool RoadNoiseSegment::saveResultsToXMLFile(const string fn)
{
  TiXmlDocument doc;
  TiXmlDeclaration *pi = new TiXmlDeclaration("1.0", "", "");
  doc.LinkEndChild(pi);
  TiXmlElement *root = new TiXmlElement("CNOSSOS_SourcePower");
  root->SetAttribute("version", XML_DATA_VERSION.c_str());
  doc.LinkEndChild(root);

  TiXmlElement *source = new TiXmlElement("source");
  root->LinkEndChild(source);

  TiXmlElement *e = new TiXmlElement("h");
  source->LinkEndChild(e);
  TiXmlText *text = new TiXmlText(floatToString(catalog->srcHeight).c_str());
  e->LinkEndChild(text);

  e = new TiXmlElement("Lw");
  source->LinkEndChild(e);
  e->SetAttribute("sourceType", "LineSource");
  e->SetAttribute("measurementType", "HemiSpherical");
  e->SetAttribute("frequencyWeighting", "LIN");

  text = new TiXmlText(stringFromFloats(TotalSpec, MAX_FREQ_BAND_CENTRE).c_str());
  e->LinkEndChild(text);

  return doc.SaveFile(fn.c_str());
}

// --------------------------------------------------------------------------------------------------------
// private auxilery function to for converting a integer to a string
//
// Parameter :
//				a		: integer value to be converted
//
// Return value :		  the converted value of a
// --------------------------------------------------------------------------------------------------------
string IntToString (int a)
{
  string str;
  ostringstream temp;
  temp<<a;
  return temp.str();
}

// --------------------------------------------------------------------------------------------------------
// private auxilery function to for converting a double to a string
//
// Parameter :
//				d		: double value to be converted
//
// Return value :		  the converted value of d
// --------------------------------------------------------------------------------------------------------
string DoubleToStr(double d)
{
  string str;
  ostringstream temp;
  temp << d;
  return temp.str();
}

//////////////////////////////

// --------------------------------------------------------------------------------------------------------
// private function to read the calculation parameters file
//
// Parameter :
//				fn		: filename of the xml settings file
//
//
// --------------------------------------------------------------------------------------------------------
void RoadNoiseCatalog::LoadRoadParamsFromFile(const string fn)
{
  TiXmlDocument doc(fn.c_str());
  if (doc.LoadFile())
  {
    TiXmlElement *root = doc.FirstChildElement();

    // Static and reference values
    refSpeed	= floatFromElement(root, "Vref", DEFAULT_REF_SPEED);
    srcHeight	= floatFromElement(root, "Hsrc", DEFAULT_SRC_HEIGHT);
    refTemp		= intFromElement(root, "Tref", DEFAULT_REF_TEMP);

    // Vehicle definitions
    TiXmlElement *cats = root->FirstChildElement("VehicleDefinition");
    if (cats != NULL)
    {
      assert(numCategories == 0);
      TiXmlElement *cat = cats->FirstChildElement("Category");
      while (cat != NULL)
      {
        RoadNoiseVehicleCategory *oCat = addCategory();
        oCat->id = cat->Attribute("ID");
        oCat->description = cat->Attribute("Description");
        bool bCalc = false;
        if (cat->QueryBoolAttribute("RollingNoise", &bCalc) == TIXML_SUCCESS)
          oCat->calcNoise[ngROLLING] = bCalc;
        if (cat->QueryBoolAttribute("PropulsionNoise", &bCalc) == TIXML_SUCCESS)
          oCat->calcNoise[ngPROPULSION] = bCalc;
        if (cat->QueryBoolAttribute("Studded", &bCalc) == TIXML_SUCCESS)
          oCat->calcStudded = bCalc;
        if (oCat->calcStudded)
        {
          oCat->studdedProps = new StuddedCategory();
          copyFloatsFromString(cat->Attribute("Astudded"), oCat->studdedProps->studded[cfAlpha], MAX_FREQ_BAND_CENTRE);
          copyFloatsFromString(cat->Attribute("Bstudded"), oCat->studdedProps->studded[cfBeta], MAX_FREQ_BAND_CENTRE);
        }
        copyFloatsFromString(cat->Attribute("Ksurface"), oCat->Ksurface, MAX_FREQ_BAND_CENTRE);

        cat = cat->NextSiblingElement("Category");
      }
    }

    // Gradient calculation data
    cats = root->FirstChildElement("GradientCalculation");
    if (cats != NULL)
    {
      TiXmlElement *cat = cats->FirstChildElement("Category");
      while (cat != NULL)
      {
        RoadNoiseVehicleCategory *oCat = getCategory(cat->Attribute("Ref"));
        if (oCat != NULL)
        {
          if (oCat->gradientCorrection == NULL)
            oCat->gradientCorrection = new GradientCategory();

          bool bCalc = false;
          if (cat->QueryBoolAttribute("calc", &bCalc) == TIXML_SUCCESS)
            oCat->gradientCorrection->calc = bCalc;

          double value = 0.0;
          if (cat->QueryDoubleAttribute("Low", &value) == TIXML_SUCCESS)
            oCat->gradientCorrection->b1 = value;
          if (cat->QueryDoubleAttribute("High", &value) == TIXML_SUCCESS)
            oCat->gradientCorrection->b2 = value;

          setGradientCorrections(cat, "Low", oCat->gradientCorrection, 0);
          //							setGradientCorrections(cat, "Mid", oCat->gradientCorrection, 1);
          setGradientCorrections(cat, "High", oCat->gradientCorrection, 1);
        }
        else
        {
          ReportError("Unknown category", fn, cat);
        }
        cat = cat->NextSiblingElement("Category");
      }
    }

    // Vehicle emission coefficients
    cats = root->FirstChildElement("EmissionAB");
    if (cats != NULL)
    {
      TiXmlElement *cat = cats->FirstChildElement("Category");
      while (cat != NULL)
      {
        RoadNoiseVehicleCategory *oCat = getCategory(cat->Attribute("Ref"));
        if (oCat != NULL)
        {
          copyFloatsFromString(cat->Attribute("Ar"), oCat->coefficientA[ngROLLING], MAX_FREQ_BAND_CENTRE);
          copyFloatsFromString(cat->Attribute("Ap"), oCat->coefficientA[ngPROPULSION], MAX_FREQ_BAND_CENTRE);
          copyFloatsFromString(cat->Attribute("Br"), oCat->coefficientB[ngROLLING], MAX_FREQ_BAND_CENTRE);
          copyFloatsFromString(cat->Attribute("Bp"), oCat->coefficientB[ngPROPULSION], MAX_FREQ_BAND_CENTRE);
        }
        else
        {
          ReportError("Unknown category", fn, cat);
        }
        cat = cat->NextSiblingElement("Category");
      }
    }

    // Gradient calculation data
    cats = root->FirstChildElement("SpeedVariations");
    if (cats != NULL)
    {
      TiXmlElement *cat = cats->FirstChildElement("Category");
      while (cat != NULL)
      {
        RoadNoiseVehicleCategory *oCat = getCategory(cat->Attribute("Ref"));
        if (oCat != NULL)
        {
          TiXmlElement *type = cat->FirstChildElement("Type");
          while (type != NULL)
          {
            int k = 0;
            if (type->QueryIntAttribute("k", &k) == TIXML_SUCCESS)
            {
              double coefficient = 0.0;
              if (type->QueryDoubleAttribute("Cr", &coefficient) == TIXML_SUCCESS)
                oCat->speedVariationCoefficient[k][ngROLLING] = coefficient;
              if (type->QueryDoubleAttribute("Cp", &coefficient) == TIXML_SUCCESS)
                oCat->speedVariationCoefficient[k][ngPROPULSION] = coefficient;
            }
            else
            {
              ReportError("Unknown speed variation type", fn, type);
            }
            type = type->NextSiblingElement("Type");
          }
        }
        else
        {
          ReportError("Unknown category", fn, cat);
        }
        cat = cat->NextSiblingElement("Category");
      }
    }

  }
  else
  {
    ReportError(string("Error loading file: ") + doc.ErrorDesc() , fn);
    throw doc.ErrorId();
  }
}


// --------------------------------------------------------------------------------------------------------
// private function to read the road surfaces file
//
// Parameter :
//				fn		: filename of the xml settings file
//
//
// --------------------------------------------------------------------------------------------------------
void RoadNoiseCatalog::LoadRoadSurfacesFromFile(const string fn)
{
  TiXmlDocument doc(fn.c_str());
  if (doc.LoadFile())
  {
    TiXmlElement *root = doc.FirstChildElement();

    // Surface definitions
    TiXmlElement *surfs = root->FirstChildElement("RoadSurfaces");
    if (surfs != NULL)
    {
      assert(surfaces.size() == 0);
      TiXmlElement *surf = surfs->FirstChildElement("Surface");
      while (surf != NULL)
      {
        RoadSurface *oSurface = new RoadSurface();
        oSurface->id = surf->Attribute("ID");
        oSurface->description = surf->Attribute("Description");
        double value = 0.0;
        if (surf->QueryDoubleAttribute("Vmin", &value) == TIXML_SUCCESS)
          oSurface->Vmin = value;
        if (surf->QueryDoubleAttribute("Vmax", &value) == TIXML_SUCCESS)
          oSurface->Vmax = value;

        TiXmlElement *cat = surf->FirstChildElement("Category");
        while (cat != NULL)
        {
          int m = indexOfCategory(cat->Attribute("Ref"));
          if (m >= 0)
          {
            copyFloatsFromString(cat->Attribute("A"), oSurface->coefficientA[m], MAX_FREQ_BAND_CENTRE);
            double coefficient = 0.0;
            if (cat->QueryDoubleAttribute("B", &coefficient) == TIXML_SUCCESS)
              oSurface->coefficientB[m] = coefficient;
          }
          else
          {
            ReportError("Unknown category reference", fn, cat);
          }

          cat = cat->NextSiblingElement("Category");
        }

        surfaces.push_back(*oSurface);
        surf = surf->NextSiblingElement("Surface");
      }
    }
    else
    {
      ReportError("No RoadSurfaces found", fn, root);
      throw -1;
    }

  }
  else
  {
    ReportError(string("Error loading file: ") + doc.ErrorDesc(), fn);
    throw doc.ErrorId();
  }
}

