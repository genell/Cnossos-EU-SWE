#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "CNOSSOS_IND_DATA.h"
#include "../tinyxml/tinyxml.h"
#include "../CNOSSOS_DLL_CONSOLE/CNOSSOS_AUX.h"


using namespace std;

namespace CNOSSOS_INDUSTRIAL_NOISE
{

	// --------------------------------------------------------------------------------------------
	IndustryCatalogue::IndustryCatalogue()
	{
		sourceDefs = vector<IndustrySourceDef>();
	}
	// --------------------------------------------------------------------------------------------
	IndustryCatalogue::~IndustryCatalogue()
	{
		sourceDefs.clear();
	}
	// --------------------------------------------------------------------------------------------
	// Load the catalogue from XML
	bool IndustryCatalogue::loadFromXmlFile( const string fn )
	{
		TiXmlDocument doc(fn.c_str());
		if (doc.LoadFile())
		{
			TiXmlElement *root = doc.FirstChildElement("CNOSSOS_Industry_Catalogue");
			
			if (root == NULL)
			{
				CNOSSOS::report_error(ERRMSG_WRONG_ROOT_ELEMENT, fn, &doc);
				return false;
			}

			TiXmlElement *firstDirectivity = root->FirstChildElement("Directivity");

			TiXmlElement *def = root->FirstChildElement("SourceDefinition");
			if (def == NULL)
			{
				CNOSSOS::report_error(ERRMSG_MISSING_ELEMENT + " 'SourceDefinition'", fn, root);
				return false;
			}
			while (def != NULL)
			{
				IndustrySourceDef* sourceDef = new IndustrySourceDef();
				sourceDef->id = def->Attribute("ID");
				sourceDef->description = CNOSSOS::stringFromElement(def, "Description");
				sourceDef->height = CNOSSOS::floatFromElement(def, "Height");
				int enumValue;
				if (CNOSSOS::mapStringToEnum(CNOSSOS::stringFromElement(def, "Type"), SourceTypeNames, NUM_SOURCE_TYPES, &enumValue))
				{
					sourceDef->type = static_cast<SourceType>(enumValue);
				}
				else
				{
					CNOSSOS::report_error("Unexpected Type", fn, def);
				}
				if (CNOSSOS::mapStringToEnum(CNOSSOS::stringFromElement(def, "MeasurementType"), SourceDirectionalityNames, NUM_SOURCE_DIRECTIONALITIES, &enumValue))
				{
					sourceDef->directionality = static_cast<SourceDirectionality>(enumValue);
				}
				else
				{
					CNOSSOS::report_error("Unexpected MeasurementType", fn, def);
				}
				CNOSSOS::copyFloatsFromString(CNOSSOS::stringFromElement(def, "Lw"), sourceDef->Lw, MAX_FREQ_BAND_CENTRE);
				
				// Initialize the directivity
				for(int hi = 0; hi < 35; hi++)
				{
					for (int vi = 0; vi < 35; vi++)
					{
						for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
						{
							sourceDef->directivity[hi][vi][i] = 0.0;
						}
					}
				}
				
				TiXmlElement *e = def->FirstChildElement("DirectivityRef");
				if (e != NULL)
				{
					// Look up the directivity definition
					string	id = e->GetText();
					TiXmlElement *d = firstDirectivity;
					while (d != NULL)
					{
						if (d->Attribute("ID") == id)
							break;
						d = d->NextSiblingElement(d->Value());
					}

					if (d == NULL)
					{
						e = NULL;
						if (id != "")
						{
							CNOSSOS::report_error("Directivity with ID '" + id + "' not found", NULL, def);
						}
					}
					else
					{
						TiXmlElement *a = d->FirstChildElement("Angle");
						while (a != NULL)
						{
							int horz_angle, vert_angle;
							int horz_index, vert_index;
							string values;
							if (a->QueryIntAttribute("horz", &horz_angle) == TIXML_SUCCESS
								&& a->QueryIntAttribute("vert", &vert_angle) == TIXML_SUCCESS)
							{
								horz_index = ((360 + horz_angle) % 360) / 10;
								vert_index = ((360 + vert_angle) % 360) / 10;
								if (horz_index < 0 || horz_index > 35 || vert_index < 0 || vert_index > 35)
								{
									CNOSSOS::report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES, fn, a);
								}
								else
								{
									CNOSSOS::copyFloatsFromString(a->Attribute("values"), sourceDef->directivity[horz_index][vert_index], MAX_FREQ_BAND_CENTRE);
								}
							}
							else
							{
								CNOSSOS::report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES, fn, a);
							}
							// Next!
							a = a->NextSiblingElement(a->Value());
						}
					}
				}
				
				sourceDefs.push_back(*sourceDef);
				// Next!
				def = def->NextSiblingElement(def->Value());
			} // definitions

			return true;
		}
		else
		{
			CNOSSOS::report_error("Unable to load file: " + string(doc.ErrorDesc()), fn);
			return false;
		}
	}
	// --------------------------------------------------------------------------------------------
	IndustrySourceDef* IndustryCatalogue::getSourceDefById( const string id )
	{
		for (vector<IndustrySourceDef>::iterator sourceDef = sourceDefs.begin(); sourceDef != sourceDefs.end(); ++sourceDef)
		{
			if (sourceDef->id.compare(id) == 0)
			{
				return &*sourceDef;
			}
		}
		return NULL;
	}

	// ============================================================================================
	// IndustrySourceSet implementation

	// --------------------------------------------------------------------------------------------
	IndustrySourceSet::IndustrySourceSet( IndustryCatalogue *catalogue )
	{
		this->catalogue = catalogue;
		sources = vector<IndustrySource>();
	}
	// --------------------------------------------------------------------------------------------
	IndustrySourceSet::~IndustrySourceSet()
	{
		this->catalogue = NULL;
		sources.clear();
	}

	// --------------------------------------------------------------------------------------------
	int round(const double x)
	{
		if (x < 0)
			return static_cast<int>(ceil(x + 0.5));
		else
			return static_cast<int>(floor(x + 0.5));
	}

	// --------------------------------------------------------------------------------------------
	bool IndustrySourceSet::loadFromXmlFile( const string fn )
	{
		TiXmlDocument doc(fn.c_str());
		if (doc.LoadFile())
		{
			TiXmlElement *root = doc.FirstChildElement("CNOSSOS_Industry_Input");
			if (root == NULL)
			{
				CNOSSOS::report_error(ERRMSG_MISSING_ELEMENT + " 'CNOSSOS_Industry_Input'", fn);
				return false;
			}

			this->doDebug = CNOSSOS::boolFromElement(root, "Test");

			TiXmlElement *src = root->FirstChildElement("Source");
			if (src == NULL)
			{
				CNOSSOS::report_error(ERRMSG_MISSING_ELEMENT + " 'Source'", fn);
				return false;
			}
			while (src != NULL)
			{
				IndustrySource source = IndustrySource(this);
				source.set_id(src->Attribute("Ref"));
				if (source.get_definition() == NULL)
					CNOSSOS::report_error("Unknown source reference", fn, src);
				else
				{
					source.weighting = CNOSSOS::stringFromElement(src, "Weighting", "A");
					source.height = CNOSSOS::floatFromElement(src, "Height", source.get_definition()->height);
					source.period = CNOSSOS::floatFromElement(src, "Period");
					if (abs(source.period) < numeric_limits<double>::epsilon())
						CNOSSOS::report_error("Period is zero or not specified", fn, src);
					source.source_time = CNOSSOS::floatFromElement(src, "SourceTime");

					source.calcMovingVehicles = false;
					TiXmlElement *e = src->FirstChildElement("Vehicles");
					if (e == NULL)
					{
						CNOSSOS::report_error(ERRMSG_MISSING_ELEMENT + " 'Vehicles'", fn, src);
					}
					else if (e->QueryBoolAttribute("moving", &source.calcMovingVehicles) != TIXML_SUCCESS)
					{
						CNOSSOS::report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES, fn, e);
					}
					if (source.calcMovingVehicles)
					{
						source.vehicle_count = CNOSSOS::intFromElement(e, "Count");
						source.vehicle_speed = CNOSSOS::floatFromElement(e, "Speed");
						if (abs(source.period) < numeric_limits<double>::epsilon())
							CNOSSOS::report_error("Speed is zero or not specified", fn, e);
						source.length = CNOSSOS::floatFromElement(e, "Length");
					}
					e = src->FirstChildElement("Directivity");
					if (e != NULL)
					{
						TiXmlElement *a = e->FirstChildElement("Angle");
						TiXmlElement *v = e->FirstChildElement("Vector");
						if ((a == NULL) == (v == NULL))
						{
							CNOSSOS::report_error(string(e->Value()) + " must contain an Angle or a Vector (but not both)", fn, e);
							return false;
						}
						if (a != NULL)
						{
							if (a->QueryDoubleAttribute("horz", &source.horz_angle) != TIXML_SUCCESS
								|| a->QueryDoubleAttribute("vert", &source.vert_angle) != TIXML_SUCCESS)
							{
								CNOSSOS::report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES, fn, a);
								return false;
							}
							a = a->NextSiblingElement(a->Value());
						}
						else if (v != NULL)
						{
							double x, y, z;
							double horz, vert, radius;
							if (v->QueryDoubleAttribute("x", &x) == TIXML_SUCCESS
								&& v->QueryDoubleAttribute("y", &y) == TIXML_SUCCESS
								&& v->QueryDoubleAttribute("z", &z) == TIXML_SUCCESS)
							{
								// read vector values, convert them to Euler angles, and store those in degrees
								CNOSSOS::vector_to_angles(x, y, z, &horz, &vert, &radius);
								source.horz_angle = CNOSSOS::rad_to_deg(horz);
								source.vert_angle = CNOSSOS::rad_to_deg(vert);
							}
							else
							{
								CNOSSOS::report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES, fn, v);
								return false;
							}
							v = v->NextSiblingElement(v->Value());
						}
					}
					else
					{
						CNOSSOS::report_error(ERRMSG_MISSING_ELEMENT + " 'Directivity'", fn, src);
						return false;
					}
					
					sources.push_back(source);
				}
				// Next!
				src = src->NextSiblingElement(src->Value());
			} // definitions

			return true;
		}
		else
		{
			CNOSSOS::report_error("Unable to load file: " + string(doc.ErrorDesc()), fn);
			return false;
		}
	}
	// --------------------------------------------------------------------------------------------
	bool IndustrySourceSet::saveResultsToXmlFile( const string fn )
	{
		TiXmlDocument doc;
		TiXmlDeclaration *pi = new TiXmlDeclaration("1.0", "", "");
		doc.LinkEndChild(pi);
		TiXmlElement *root = new TiXmlElement("CNOSSOS_SourcePower");
		root->SetAttribute("version", XML_DATA_VERSION.c_str());
		doc.LinkEndChild(root);

		for(vector<IndustrySource>::iterator source = sources.begin(); source != sources.end(); source++)
		{
			TiXmlElement *src = new TiXmlElement("source");
			root->LinkEndChild(src);

			TiXmlElement *e = new TiXmlElement("h");
			src->LinkEndChild(e);
			TiXmlText *text = new TiXmlText(CNOSSOS::stringFromFloat(source->height).c_str());
			e->LinkEndChild(text);

			e = new TiXmlElement("Lw");
			src->LinkEndChild(e);
			e->SetAttribute("sourceType", SourceTypeNames[source->get_definition()->type].c_str());
			e->SetAttribute("measurementType", SourceDirectionalityNames[source->get_definition()->directionality].c_str());
			e->SetAttribute("frequencyWeighting", source->weighting.c_str());

			text = new TiXmlText(CNOSSOS::stringFromFloats(source->Lw, MAX_FREQ_BAND_CENTRE).c_str());
			e->LinkEndChild(text);
		}

		return doc.SaveFile(fn.c_str());
	}
	// --------------------------------------------------------------------------------------------
	int IndustrySourceSet::Calculate()
	{
		int result = 0;
		for(vector<IndustrySource>::iterator source = sources.begin(); source != sources.end(); source++)
		{
			if (!source->Calculate())
				result = 1;
		}
		return result;
	}
	// --------------------------------------------------------------------------------------------
	bool IndustrySourceSet::writeDebugData( const string fn )
	{
		ofstream csv;
		csv.open(fn, ios_base::out);
		if (!csv)
		{
			cerr << "!!! Unable to write to file " << fn << endl;
			return false;
		}
		csv.imbue(locale("")); // use the user's default locale
		csv << "Sep=\t" << endl; // indicate we're going to use tab as separator
		
		// Write the input data
		csv << "source_def\theight";
		for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
			csv << "\tLw;" << FreqBands[i];
		csv << "\tperiod\tsrc_time\tmoving_vehicles\tcount\tspeed\tlength\thorz_angle\tvert_angle\tdef_height\ttype\tradiation" << endl;
		for(vector<IndustrySource>::iterator source = sources.begin(); source != sources.end(); source++)
		{
			IndustrySourceDef *def = source->get_definition();
			csv << source->get_id() << "\t";
			csv << source->height << "\t";
			for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
			{
				if (def)
					csv << def->Lw[i] << "\t";
				else
					csv << "-\t";
			}
			csv << source->period << "\t";
			csv << source->source_time << "\t";
			if (source->calcMovingVehicles)
			{
				csv << "yes\t";
				csv << source->vehicle_count << "\t";
				csv << source->vehicle_speed << "\t";
				csv << source->length << "\t";
			}
			else
			{
				csv << "no\t";
				csv << "-\t";
				csv << "-\t";
				csv << "-\t";
			}
			csv << source->horz_angle << "\t";
			csv << source->vert_angle << "\t";
			if (def)
			{
				csv << def->height << "\t";
				csv << SourceTypeNames[def->type] << "\t";
				csv << SourceDirectionalityNames[def->directionality] << "\t";
			}
			csv << endl;
		}
		csv << endl;

		// Write the intermediate data and end results
		csv << "source_def\tname";
		for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
		{
			csv << "\t" << FreqBands[i] << "Hz";
		}
		csv << endl;
		for(vector<IndustrySource>::iterator source = sources.begin(); source != sources.end(); source++)
		{
			csv << source->get_id() << "\t";
			csv << "delta_Cw\t";
			csv << source->deltaCw;
			csv << endl;

			csv << source->get_id() << "\t";
			csv << "delta_Dir\t";
			for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
				csv << source->deltaDir[i] << "\t";
			csv << endl;

			csv << source->get_id() << "\t";
			csv << "Lw\t";
			for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
				csv << source->Lw[i] << "\t";
			csv << endl;
		}
		csv << endl;
		
		csv.close();
		return true;
	}

	// --------------------------------------------------------------------------------------------
	IndustryCatalogue * IndustrySourceSet::get_catalogue()
	{
		return this->catalogue;
	}


	// ============================================================================================
	// IndustrySource implementation

	// --------------------------------------------------------------------------------------------
	IndustrySource::IndustrySource( IndustrySourceSet *parent )
	{
		this->parent = parent;
	}
	// --------------------------------------------------------------------------------------------
	IndustrySource::~IndustrySource()
	{
		this->parent = NULL;
	}
	// --------------------------------------------------------------------------------------------
	void IndustrySource::set_id( const string id )
	{
		this->id = id;
		this->definition = parent->get_catalogue()->getSourceDefById(id);
	}
	// --------------------------------------------------------------------------------------------
	string IndustrySource::get_id()
	{
		return this->id;
	}
	// --------------------------------------------------------------------------------------------
	IndustrySourceDef* IndustrySource::get_definition()
	{
		return this->definition;
	}
	// --------------------------------------------------------------------------------------------
	bool IndustrySource::Calculate()
	{
		bool result = CalculateWorkinghourCorrection()
						&& CalculateDirectivity()
						&& CalculateSummation();
		if (result == false)
		{
			for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
			{
				Lw[i] = numeric_limits<double>::quiet_NaN();
			}
		}
		return result;
	}
	// --------------------------------------------------------------------------------------------
	bool IndustrySource::CalculateWorkinghourCorrection()
	{
		if (abs(period) < numeric_limits<double>::epsilon())
		{
			CNOSSOS::report_error("Period cannot be zero");
			deltaCw = numeric_limits<double>::quiet_NaN();
			return false;
		}

		if (this->calcMovingVehicles)
		{
			if (abs(vehicle_speed) < numeric_limits<double>::epsilon())
			{
				CNOSSOS::report_error("Vehicles.Speed cannot be zero");
				deltaCw = numeric_limits<double>::quiet_NaN();
				return false;
			}
			deltaCw = 10 * log10((length * vehicle_count) / (1000 * vehicle_speed * period));
		}
		else
		{
			deltaCw = 10 * log10(source_time / period); // V-1
		}
		return true;
	}
	// --------------------------------------------------------------------------------------------
	bool IndustrySource::CalculateDirectivity()
	{
		int horz_index = round(horz_angle / 10);
		int vert_index = round(vert_angle / 10);
		if (horz_index < 0 || horz_index > 35 || vert_index < 0 || vert_index > 35)
		{
			CNOSSOS::report_error("Invalid angles (" + CNOSSOS::stringFromFloat(horz_angle) + "," + CNOSSOS::stringFromFloat(vert_angle)  + ")");
			for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
			{
				deltaDir[i] = numeric_limits<double>::quiet_NaN();
			}
			return false;
		}
		for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
		{
			deltaDir[i] = definition->directivity[horz_index][vert_index][i];
		}
		return true;
	}
	// --------------------------------------------------------------------------------------------
	bool IndustrySource::CalculateSummation()
	{
		for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
		{
			Lw[i] = definition->Lw[i] + deltaCw - deltaDir[i];
		}
		return true;
	}

}
