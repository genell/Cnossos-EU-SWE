#pragma once

// ----------------------------------------------------------------------------------------------------- 
//
// project : CNOSSOS_ROADNOISE.dll
// author  : Martijn Kirsten
// company : DGMR
//
// Calculation road traffic noise source emission
//
// Disclaimer + header text
//
// Version : 
// Release : 
//
// ----------------------------------------------------------------------------------------------------- 

#include "stdafx.h"
#include "CNOSSOS_ROADNOISE_DLL_CONST.h"
#include "CNOSSOS_ROADNOISE_DLL_DATA.h"
#include "CNOSSOS_ROADNOISE_DLL_AUX.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "../tinyxml/tinyxml.h"

using namespace std;


namespace CNOSSOS_ROADNOISE
{	

#pragma region Meta_Debug_functions
	// Constructor -> initializes all class members to default values
	RoadSegment::RoadSegment(RoadNoiseCatalog *catalog)
	{
		this->catalog = catalog;
		
		doDebug = false;

		for (int m=0;m < MAX_SRC_CAT; m++)			
		{
			VehicleCategory *cat = catalog->getCategory(m);
			if (cat != NULL)
			{
				this->calcPropNoise[m] = cat->calcNoise[ngPROPULSION];
				this->calcRollingNoise[m] = cat->calcNoise[ngROLLING];
			}
			else
			{
				this->calcPropNoise[m] = false;
				this->calcRollingNoise[m] = false;
			}

			this->v[m] = 0;
			this->Q[m] = 0;
			this->Fstud[m] = 0;

			for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
			{
				this->Spec[m][i] = 0;
				this->LwimTotal[m][i] = 0;
				this->LWeqlineimFactor[m][i] = 0;
				this->TotalRollingNoise[m][i] = 0;
				this->RollingNoiseFactor[m][i] =0;
				this->DeltaRollingNoiseFactor[m][i] = 0;
				this->DeltaRollingRoadCorrection[m][i] = 0;
				this->DeltaRollingTyreCorrection[m][i] =0;
				this->DeltaRollingAccCorrection[m][i] = 0;
				

				this->TotalPropulsionNoise[m][i] =0;
				this->PropulsionNoiseFactor[m][i] =0;
				this->DeltaPropulsionNoiseFactor[m][i] = 0;
				this->DeltaPropulsionRoadCorrection[m][i] = 0;
				this->DeltaPropulsionAccCorrection[m][i] =0;
				this->DeltaPropulsionGradientCorrection[m][i] = 0;
			}
			this->DeltaRollingTempCorrection[m] =0;
		}

		this->AccProp = NULL;
		this->TempProp = NULL;
		this->GradProp = NULL;
		this->SurfaceProp = NULL;
	}

	// Releases all created objects 
	RoadSegment::~RoadSegment()
	{
		if (AccProp != NULL)
			delete AccProp;
		if (TempProp != NULL)
			delete TempProp;
		if (GradProp != NULL)
			delete GradProp;
		this->SurfaceProp = NULL;
	}

	// Retrieves the relevant surface class
	RoadSurface* RoadSegment::setSurfaceID(const string id)
	{
		for (list<RoadSurface>::iterator rsi = this->catalog->surfaces.begin(); 
				rsi != this->catalog->surfaces.end(); ++rsi)
		{
			if (id.compare(rsi->id) == 0)
			{
				SurfaceProp = &*rsi;
				return SurfaceProp;
			}
		}
		return NULL;
	}

#pragma endregion Meta_Debug_functions

#pragma region RollingNoise_Calculation_functions
	
	// Calculate the road surface correction for vehicle category "cat" and frequency band "freq"
	// Formula III-19
	double RoadSegment::CalcRollingNoiseRoadSurfaceCorrection(const int cat, const int freq)
	{
		// Formula III-19
		double Alpha = this->SurfaceProp->coefficientA[cat][freq];
		double Beta = this->SurfaceProp->coefficientB[cat];
		DeltaRollingRoadCorrection[cat][freq] = Alpha + Beta * log10(this->v[cat] / catalog->refSpeed);
		return DeltaRollingRoadCorrection[cat][freq];
	}

	// Calculate the studded tyre correction category for vehicle category "cat" and frequency band "freq"
	// Formula III-7, III-8 and III-9
	double RoadSegment::CalcTyreCorrection(const int m, const int i)
	{
		VehicleCategory *cat = catalog->getCategory(m);
		if (this->studdedMonths > 0 && cat->calcStudded)
		{
			double Alpha = cat->studdedProps->studded[cfAlpha][i];
			double Beta = cat->studdedProps->studded[cfBeta][i];
			double delta_stud_im = 0.0;
			double v1 = this->v[m];

			// Formula III-7
			if (v1 < 50)			
				delta_stud_im = Alpha + Beta * log10(50 / catalog->refSpeed);
			else if (v1 >= 50 && v1 <= 90)
				delta_stud_im = Alpha + Beta * log10(v1 / catalog->refSpeed);
			else
				delta_stud_im = Alpha + Beta * log10(90 / catalog->refSpeed);

			// Formula III-8
			double ps = this->Fstud[m] * ((double)this->studdedMonths / 12);

			// Formula III-9
			double studded_tyres_im = 10 * log10((1 - ps) + ps * pow(10,(delta_stud_im / 10)) );
			DeltaRollingTyreCorrection[m][i] = studded_tyres_im;			
		}
		else		
			DeltaRollingTyreCorrection[m][i] = 0.0;
		return DeltaRollingTyreCorrection[m][i];
	}

	// Calculate the rolling noise acceleration correction for vehicle category "m" and frequency band "i"
	// Formula III-17
	double RoadSegment::CalcAccCorrectionRolling(const int m, const int i)
	{
		// if correction
		VehicleCategory *cat = catalog->getCategory(m);
		if (this->AccProp != NULL)
		{
			// Formula III-17
			double Cfactor = cat->speedVariationCoefficient[this->AccProp->K][ngROLLING];
			DeltaRollingAccCorrection[m][i] = Cfactor * max(1 - (abs(this->AccProp->distance) / 100),0.0);
		}
		else
			DeltaRollingAccCorrection[m][i] = 0;			
		return DeltaRollingAccCorrection[m][i];
	}

	// Calculate the rolling noise accelerationtemperature correction for each vehicle category "m"
	// Formula III-10
	double RoadSegment::CalcTempCorrection(const int m, const int i)
	{
		if (this->TempProp != NULL)
		{
			VehicleCategory *cat = catalog->getCategory(m);
			double K = cat->Ksurface[i];
			// Formula III-10
			DeltaRollingTempCorrection[m] = K * (catalog->refTemp - this->TempProp->t);
			
		}
		return DeltaRollingTempCorrection[m];
	}

	// Calculate the total rolling noise for each vehicle category "m" and frequency "i"
	// Formula III-5, III-6
	double RoadSegment::CalcRollingNoise(const int m, const int i)
	{
		VehicleCategory *cat = catalog->getCategory(m);
		// Formula III-5
		double alphaFactor = cat->coefficientA[ngROLLING][i];
		double betaFactor =  cat->coefficientB[ngROLLING][i];
		double logFactor = log10(this->v[m] / catalog->refSpeed);
		double LWR = alphaFactor + betaFactor * logFactor;

		RollingNoiseFactor[m][i] = LWR;
		
		// Formula III-6
		double deltaRoadSurface = CalcRollingNoiseRoadSurfaceCorrection(m, i);
		double deltaStudded = CalcTyreCorrection(m, i);
		double deltaAcc = CalcAccCorrectionRolling(m,i);
		double deltaTemp = CalcTempCorrection(m, i);
		//double deltaTemp = 
		 double deltaRollingNoise = deltaRoadSurface + deltaStudded + deltaAcc + deltaTemp;
		 DeltaRollingNoiseFactor[m][i] = deltaRollingNoise;
		 TotalRollingNoise[m][i] = LWR + deltaRollingNoise;
		return TotalRollingNoise[m][i];
	}
#pragma endregion RollingNoise_Calculation_functions

#pragma region Propagation_Calculation_functions	
	// Calculate the road surface correction for vehicle category "m" and frequency band "i"
	// Formula III-20
	double RoadSegment::CalcPropulsionNoiseRoadSurfaceCorrection(const int m, const int i)
	{
		// Formula III-20
		double Alpha = this->SurfaceProp->coefficientA[m][i];
		DeltaPropulsionRoadCorrection[m][i] = min(Alpha,0.0);
		return DeltaPropulsionRoadCorrection[m][i];
	}


	// Calculate the gradient correction for each vehicle category "m" and frequency "i"
	// Formula III-13, III-14, III-15, III-16
	double RoadSegment::CalcGradientCorrection(const int m,const int i)
	{
		double value = 0.0;
		if (this->GradProp != NULL)
		{
			double s = this->GradProp->s;
			double v = this->v[m];
			VehicleCategory *cat = catalog->getCategory(m);
			value = cat->gradientCorrection->CalcValue(s, v);
		}
		DeltaPropulsionGradientCorrection[m][i] = value;
		return DeltaPropulsionGradientCorrection[m][i];
	}

	// Calculate the propulsion noise acceleration correction for vehicle category "m" and frequency band "i"
	// Formula III-18
	double RoadSegment::CalcAccCorrectionPropulsion(const int m, const int i)
	{
		
		VehicleCategory *cat = catalog->getCategory(m);
		if (this->AccProp != NULL)
		{
			// Formula III-18
			double Cfactor = cat->speedVariationCoefficient[this->AccProp->K][ngPROPULSION];
			DeltaPropulsionAccCorrection[m][i] = Cfactor * max(1 - (abs(this->AccProp->distance) / 100),0.0);
		}
		else
			DeltaPropulsionAccCorrection[m][i] = 0;			
		return DeltaPropulsionAccCorrection[m][i];

	}


	// Calculate the total propulsion noise for each vehicle category "m" and frequency band "i"
	// Formula III-11, III-12
	double RoadSegment::CalcPropulsionNoise(const int m, const int i)
	{
		VehicleCategory *cat = catalog->getCategory(m);
		// Formula III-11
		double alphaFactor = cat->coefficientA[ngPROPULSION][i];
		double betaFactor =  cat->coefficientB[ngPROPULSION][i];
		double Factor = ((this->v[m] - catalog->refSpeed) / catalog->refSpeed);
		double LWR = alphaFactor + betaFactor * Factor;
		
		PropulsionNoiseFactor[m][i] = LWR;
		// Correction coefficients
		double deltaRoadSurface = CalcPropulsionNoiseRoadSurfaceCorrection(m, i);
		double deltaAcc = CalcAccCorrectionPropulsion(m,i);
		double delaGradient = CalcGradientCorrection(m,i); 
		
		double deltaPropagationNoise = deltaRoadSurface + deltaAcc + delaGradient;
		DeltaPropulsionNoiseFactor[m][i] = deltaPropagationNoise;

		TotalPropulsionNoise[m][i] = LWR + deltaPropagationNoise;
		return TotalPropulsionNoise[m][i];
	}

#pragma endregion Propagation_Calculation_functions		

#pragma region HighLevel_Calculation_functions	
	// Calculates the source power for each vehicle category and frequency
	// Formula III-1, III-3, III-4	
	int RoadSegment::CalcSegment()
	{
		for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
			TotalSpec[i] = 0.0;

		for (int m=0; m < catalog->numCategories; m++)
		{
			for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
			{
				double Lwim = 0.0;

				double PropNoise=0.0;
				double RollingNoise=0.0;
				if (this->calcPropNoise[m])
					PropNoise = CalcPropulsionNoise(m,i);
				if (this->calcRollingNoise[m])
					RollingNoise = CalcRollingNoise(m,i);

				Lwim = 0.0;
				if (this->calcPropNoise[m] && !this->calcRollingNoise[m])
					// Formula III-2
					Lwim = PropNoise;
				else if (!this->calcPropNoise[m] && this->calcRollingNoise[m])
					// Formula III-2
					Lwim = RollingNoise;
				else if (this->calcPropNoise[m] && this->calcRollingNoise[m])
					// Formula III-3
					Lwim = 10 * log10(pow(10,(RollingNoise / 10)) + pow(10,(PropNoise / 10)));
				LwimTotal[m][i] = Lwim;

				double Lw_eq_line_im = 0.0;
				if (this->Q[m] > 0.0 && this->v[m] > 0.0)
				{
					// Formula III-1
					Lw_eq_line_im = 10 * log10(this->Q[m] / (1000 * this->v[m]));
					LWeqlineimFactor[m][i] = Lw_eq_line_im;

					Spec[m][i] = Lwim  + Lw_eq_line_im;

					// Energetic summation
					TotalSpec[i] += pow(10, (Spec[m][i] / 10));
				}
				else
				{
					LWeqlineimFactor[m][i] = 0.0;
					Spec[m][i] = 0.0;
				}
			}
		}
		
		// Finalize the energetic summation of each band
		for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
			TotalSpec[i] = 10 * log10(TotalSpec[i]);

		return 0;
	}

#pragma endregion HighLevel_Calculation_functions

#pragma region Helper_functions
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
		iss.imbue(locale("C"));
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
		oss.imbue(locale("C"));
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
	bool  RoadSegment::loadFromXMLFile(const string fn)
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
		oss.imbue(locale("C"));
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
	bool RoadSegment::saveResultsToXMLFile(const string fn)
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

	// --------------------------------------------------------------------------------------------------------
	// private function to dump all available (partial) results in a output file (for debug purposes)
	//
	// Parameter :
	//				debugfile		: the file to contained all output
	//				
	// 
	// --------------------------------------------------------------------------------------------------------
	bool RoadSegment::writeDebugData(const string debugfile)
	{
		// DEBUG DATA UITSPUGEN 

		ofstream myfile;
		myfile.open(debugfile.c_str());
		if (!myfile)
		{
			cerr << "Unable to write to file " << debugfile << endl;
			return false;
		}
		myfile.imbue(locale(""));

		// Input data
		myfile << "Sep=\t" << endl;
		myfile << "Cat\tID\tQuantity\tSpeed\tFstud" << endl;
		for (int i=0; i < catalog->numCategories; i++)
			myfile << "Cat" << i << "\t" << catalog->getCategory(i)->id << "\t" << this->Q[i] << "\t" << this->v[i] << "\t" << this->Fstud[i] << endl;

		if (this->TempProp == NULL)
			myfile << "No temperature data found" << endl;
		else
			myfile << "Use temperature: " << this->TempProp->t << "°C" << endl;

		if (this->AccProp == NULL)
			myfile << "No acceleration properties found" << endl;
		else
		{
			if (this->AccProp->K == 1)
				myfile << "Acceleration: CROSSING at distance of: " << this->AccProp->distance << " m" << endl;
			else
				myfile << "Acceleration: ROUNDABOUT at distance of: " << this->AccProp->distance << " m" << endl ;
		}

		if (this->GradProp == NULL)
			myfile << "No gradient properties" << endl;
		else
			myfile << "Roadsegment has gradient of: " << this->GradProp->s << "%" << endl << endl;
		
		if (this->SurfaceProp == NULL)
			myfile << "Road surface reference: <none>" << endl;
		else
			myfile << "Road surface reference: " << this->SurfaceProp->id << endl;

		myfile << "Roadsegment has the following studded tyre properties:" << endl;
		myfile << "Months per year: " << this->studdedMonths << endl;
		PrintDataTable		(myfile, this->Fstud, "Fraction of vehicles with studded tyres:");
		myfile << endl;

		// Rolling Noise
		PrintDoubleDataTable(myfile, this->DeltaRollingAccCorrection, "Rolling noise acceleration correction"); // ΔLwr,acc
		PrintDoubleDataTable(myfile, this->DeltaRollingTyreCorrection, "Rolling noise tyre correction"); // ΔLstudded
		PrintDataTable		(myfile, this->DeltaRollingTempCorrection, "Temperature correction"); // ΔLw,temp
		PrintDoubleDataTable(myfile, this->DeltaRollingRoadCorrection, "Rolling noise road correction"); // ΔLwr,road
		PrintDoubleDataTable(myfile, this->DeltaRollingNoiseFactor, "Total rolling noise correction"); // ΔLwr
		PrintDoubleDataTable(myfile, this->RollingNoiseFactor, "Total rolling noise factor");
		PrintDoubleDataTable(myfile, this->TotalRollingNoise, "Total rolling noise"); // Lwr

		// Propulsion noise
		PrintDoubleDataTable(myfile, this->DeltaPropulsionAccCorrection, "Propulsion noise acceleration correction"); // ΔLwp,acc
		PrintDoubleDataTable(myfile, this->DeltaPropulsionGradientCorrection, "Propulsion noise gradient correction"); // ΔLwp,grad
		PrintDoubleDataTable(myfile, this->DeltaPropulsionRoadCorrection, "Propulsion noise road correction"); // ΔLwp,road
		PrintDoubleDataTable(myfile, this->DeltaPropulsionNoiseFactor, "Total propulsion noise correction"); // ΔLwp
		PrintDoubleDataTable(myfile, this->PropulsionNoiseFactor, "Total propulsion noise factor");
		PrintDoubleDataTable(myfile, this->TotalPropulsionNoise, "Total propulsion noise"); // Lwp

		// totals
		PrintDoubleDataTable(myfile, this->LwimTotal, "Lw;i,m - Total noise");
		PrintDoubleDataTable(myfile, this->LWeqlineimFactor, "Total Lweqline_im factor");

		// Output
		PrintDoubleDataTable(myfile, this->Spec, "Lw;eq,i,m - Total result");

		myfile << endl << "Cumulated total result" << endl;
		myfile << "\t\t";
		for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
			myfile << setw(8) << FreqBands[i] << "\t";	
		myfile << endl << endl;
		myfile << "\t\t";
		for (int i=0; i < MAX_FREQ_BAND_CENTRE; i++)
			myfile << setw(8) << this->TotalSpec[i] << "\t";	
		myfile << endl;
				
		myfile.close();
		return true;
				
	}



#pragma endregion File_IO_functions



#pragma region RoadNoiseCatalog_methods

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

	
	
	VehicleCategory* RoadNoiseCatalog::addCategory()
	{
		VehicleCategory* result = new VehicleCategory();
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

	VehicleCategory* RoadNoiseCatalog::getCategory(const string id)
	{
		return getCategory(indexOfCategory(id));
	}

	VehicleCategory* RoadNoiseCatalog::getCategory(const int index)
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
					VehicleCategory *oCat = addCategory();
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
					VehicleCategory *oCat = getCategory(cat->Attribute("Ref"));
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
					VehicleCategory *oCat = getCategory(cat->Attribute("Ref"));
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
					VehicleCategory *oCat = getCategory(cat->Attribute("Ref"));
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

#pragma endregion RoadNoiseCatalog_methods

}
