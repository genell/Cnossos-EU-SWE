#pragma once
#include "stdafx.h"

#include "CNOSSOS_RAILNOISE_DLL_CONST.h"
#include "CNOSSOS_RAILNOISE_DLL_DATA.h"
#include "CNOSSOS_RAILNOISE_DLL_AUX.h"
#include "..\CNOSSOS_DLL_CONSOLE\CNOSSOS_AUX.h"
#include "../tinyxml/tinyxml.h"

#include <iostream>
#include <math.h> 

using namespace std;
using namespace CNOSSOS;


namespace CNOSSOS_RAILNOISE
{	

#pragma region "Frequency/wavelength conversion functions"

	/// <summary>
	/// Returns the wavelength λ for the given speed and frequency.
	/// </summary>
	/// <param name="v">Speed [km/h]</param>
	/// <param name="f">Frequency [Hz]</param>
	/// <returns>Wavelength λ [cm]</returns>
	double wavelength(const double v, const double f)
	{
		return v / (0.036 * f);
	}

	/// <summary>
	/// Returns the frequency f for the given speed and wavelength.
	/// </summary>
	/// <param name="v">Speed [km/h]</param>
	/// <param name="λ">Wavelength [cm]</param>
	/// <returns>Frequency [Hz]</returns>
	double frequency(const double v, const double λ)
	{
		return v / (0.036 * λ);
	}

	/// <summary>
	/// Returns the frequencies freqs for the given speed and wavelengths.
	/// </summary>
	/// <param name="v">Speed [km/h]</param>
	/// <param name="wls">[in] array of wavelengths [cm]</param>
	/// <param name="freqs">[out] array of frequencies [Hz]</returns>
	void wavelengths_to_frequencies(double v, wavelengths &wls, freqs &freqs)
	{
		for (int i = 0; i < MAX_FREQ_BAND; i++)
		{
			double λ = wavelength(v, FreqBands[i]);
			// find out the boundary wavelengths
			int l1 = 0;
			int l2 = -1;
			for (int l = 0; l < MAX_WAVELENGTH; l++)
			{ 
				// The Wavelengths array is sorted in descending order, from long to short
				if (λ >= Wavelengths[l])
				{
					l2 = l;
					l1 = l - 1;
					if (l == 0) 
					{
						// λ is longer than the longest wavelength
						l1 = l;
					}
					break;
				}
			}
			if (l2 == -1) // λ is shorter than the shortest wavelength
			{
				l2 = MAX_WAVELENGTH - 1;
			}
			
			if (l1 == l2 || wls[l1] == wls[l2])
			{
				freqs[i] = wls[l1];
			}
			else
			{
				// Interpolate the values
				double λ1 = Wavelengths[l1];
				double λ2 = Wavelengths[l2];
				double fraction = (λ - λ1) / (λ2 - λ1);
				freqs[i] = wls[l1] + fraction * (wls[l2] - wls[l1]);
			}
		}
	}

	void clear(double value, freqs& f, int count = MAX_FREQ_BAND)
	{
		for (int i = 0; i < count; i++) {
			f[i] = value;
		}
	}
	inline void clear(freqs& f, int count = MAX_FREQ_BAND)
	{
		clear(0.0, f, count);
	}

	void clear(freqcentres& f, int count = MAX_FREQ_BAND_CENTRE)
	{
		for (int i = 0; i < count; i++) {
			f[i] = 0.0;
		}
	}

#pragma endregion

#pragma region "XML selection functions"
	TiXmlNode* selectNode(TiXmlNode* base, string xPath)
	{
		if (base == NULL) return NULL;
		if (xPath == "")
		{
			return base;
		}
		else if (xPath.substr(0, 1) == "/")
		{
			TiXmlNode* root = base;
			while (root != NULL && root->Parent() != NULL)
				root = root->Parent();
			return selectNode(root, xPath.substr(1));
		}
		else
		{
			string name = "";
			int sepPos = xPath.find('/');
			if (sepPos > 0)
			{
				name = xPath.substr(0, sepPos);
				xPath = xPath.substr(sepPos + 1);
			}
			else
			{
				name = xPath;
				xPath = "";
			}
			TiXmlNode* node;
			if (name == ".")
			{
				node = base;
			}
			else if (name == "..")
			{
				node = base->Parent();
			}
			else if (name == "*")
			{
				node = base->FirstChild();
			}
			else
			{
				node = base->FirstChild(name.c_str());
			}
			return selectNode(node, xPath);
		}
	}

	TiXmlElement* findElementByAttribute(TiXmlNode* base, string xPath, string attributeName, string attributeValue)
	{
		TiXmlNode* node = selectNode(base, xPath);
		while (node != NULL)
		{
			TiXmlElement* element = node->ToElement();
			if (element->Attribute(attributeName.c_str()) == attributeValue) {
				return element;
			}
			node = node->NextSibling(node->Value());
		}
		return NULL;
	}
	
	double selectDouble(TiXmlNode* base, string xPath)
	{
		TiXmlNode* node = selectNode(base, xPath);
		if (node != NULL)
		{
			TiXmlNode* child = node->FirstChild();
			TiXmlText* text = child->ToText();
			return parseFloat(text->Value());
		}
		else
		{
			throw ERRMSG_MISSING_ELEMENT + ": " + xPath;
		}
	}
	bool selectFreqs(TiXmlElement* element, const string valueAttribute, freqs& f)
	{
		string value = element->Attribute(valueAttribute.c_str());
		return copyFloatsFromString(value, f, MAX_FREQ_BAND) == MAX_FREQ_BAND;
	}
	bool selectFreqs(TiXmlNode* base, const string xPath, const string attributeName, const string attributeValue, const string valueAttribute, freqs& f)
	{
		TiXmlElement* element = findElementByAttribute(base, xPath, attributeName, attributeValue);
		if (element != NULL) {
			return selectFreqs(element, valueAttribute, f);
		} else {
			report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": " + xPath + "[@"+attributeName+"='"+attributeValue+"']", "", base);
			return false;
		}
	}
	bool selectWavelengths(TiXmlNode* base, const string xPath, const string attributeName, const string attributeValue, const string valueAttribute, wavelengths& wl)
	{
		TiXmlElement* element = findElementByAttribute(base, xPath, attributeName, attributeValue);
		if (element != NULL) {
			string value = element->Attribute(valueAttribute.c_str());
			return copyFloatsFromString(value, wl, MAX_WAVELENGTH) == MAX_WAVELENGTH;
		} else {
			report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": " + xPath + "[@"+attributeName+"='"+attributeValue+"']", "", base);
			return false;
		}
	}
#pragma endregion



#pragma region "RailSection class"
	RailSection::RailSection(RailCatalogue *catalogue)
	{
		this->catalogue = catalogue;
		this->track = new RailTrack(catalogue);
	}
	RailSection::~RailSection()
	{
		delete this->track;
	}


	/// <summary>
	/// Calculates this instance.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calculate() // 3.1, 3
	{
		bool result = true;
		// r = running conditions (constant, accelerating, decelerating, idling)
		// p = physical source (A [h==0.5], B [h==4.0])
		// i = frequency band

		for (int pi = A; pi < NUM_PHYSICAL_SOURCES; pi++) {
			this->p = static_cast<PhysicalSourceEnum>(pi);
			
			int calculatedResults[NUM_SOURCE_GEOMETRY_TYPES];
			for (int sgi = A; sgi < NUM_SOURCE_GEOMETRY_TYPES; sgi++) {
				SourceGeometryTypeEnum source_type = static_cast<SourceGeometryTypeEnum>(sgi);
				clear(LwEqTdir[p][source_type]);
				clear(Lw[p][source_type]);
				calculatedResults[source_type] = 0;
			}
			for(std::vector<RailVehicle>::iterator it = vehicles.begin(); it != vehicles.end(); ++it) {
				this->vehicle = &*it;
				
				cout << "Calculating vehicle '" << vehicle->get_description() << "', ref=" << vehicle->vehicle_id << endl;
				if (calc_flow_per_running_condition())	// 3.2
				{
					
					for (int i = 0; i < MAX_FREQ_BAND; i++) {
						LwEqTdir[p][vehicle->source_type][i] += erg(vehicle->LwEqLine[p][i]);
					}
					calculatedResults[vehicle->source_type]++;
				}
				else
				{
					result = false;
				}
			}
			for (int sgi = A; sgi < NUM_SOURCE_GEOMETRY_TYPES; sgi++) {
				SourceGeometryTypeEnum source_type = static_cast<SourceGeometryTypeEnum>(sgi);
				if (calculatedResults[source_type] > 0)
				{
					for (int i = 0; i < MAX_FREQ_BAND; i++)
					{
						LwEqTdir[p][source_type][i] = dB(LwEqTdir[p][source_type][i]);
					}
					// convert 1/3 octaves to octaves
					for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++)
					{
						Lw[p][source_type][i] = dBsum(LwEqTdir[p][source_type][i * 3 + 0], LwEqTdir[p][source_type][i * 3 + 1], LwEqTdir[p][source_type][i * 3 + 2]);
					}
				}
				else if (p == this->physical_source && source_type == this->source_type)
				{
					report_error("No vehicles match the Idling parameter.", "", selectNode(&docInput, "/*/Idling"));
				}
			}

		} // for p

		return result;
	}

	/// <summary>
	/// Calc_flow_per_running_conditions this instance.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_flow_per_running_condition() // 3.2
	{
		clear(vehicle->LwEqLine[p]);
		
		// Calculate Lw0dir[p];
		bool result = calc_directional_sound_power(); // 3.3
		if (result) {
			// Calculate LwEqLine[p]
			switch(vehicle->r) {
				case constant: 
				case accelerating: 
				case decelerating: // moving vehicles
					result = calc_flow_moving(); // 3.2.1
					break;
				case idling: // idling vehicles
					result = calc_flow_idling(); // 3.2.2
					break;
			}
		}

		return result;
	}

	/// <summary>
	/// Calculate the traffic flow for the current vehicle when moving
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_flow_moving() // 3.2.1; calculates Lw0eqLine[t][s]
	{
		bool result = true;

		double Q = vehicle->Q;
		double v = vehicle->v;
		
		for(int i = 0; i < MAX_FREQ_BAND; i++) {
			double Lw0dir = vehicle->Lw0dir[p][i];
			vehicle->LwEqLine[p][i] = Lw0dir + dB(Q / (1000 * v));
		}

		return result;
	}

	/// <summary>
	/// Calculate the traffic flow for the current vehicle when idling
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_flow_idling() // 3.2.2; calculates Lw0eqLine[t][s][r]
	{
		bool result = true;

		double T = vehicle->get_idling_time();
		double L = track->get_section_length();

		for(int i = 0; i < MAX_FREQ_BAND; i++) {
			double Lw0dir = vehicle->Lw0dir[p][i];
			vehicle->LwEqLine[p][i] = Lw0dir + dB(T / (this->Tref * L));
		}

		return result;
	}

	/// <summary>
	/// Calc_directional_sound_powers this instance.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_directional_sound_power() // 3.3
	{
		bool result = true;

		result = calc_Δdirectivity_vertical() && result; // => 3.3.1
		result = calc_Δdirectivity_horizontal() && result; // => 3.3.2
		if (!result)
			return result;

		// Initialization
		clear(vehicle->Lw0dir[p]);
		
		result = false;
		for (int sti = rolling; sti < NUM_SOURCE_TYPES; sti++) {
			SourceTypeEnum source_type = static_cast<SourceTypeEnum>(sti);
			
			bool sub_result = false;
			switch (source_type) {
				case rolling:
					sub_result = calc_rolling_noise();
					break;
				case traction:
					sub_result = calc_traction_noise();
					break;
				case aerodynamic:
					sub_result = calc_aerodynamic_noise();
					break;
			}
			result = result || sub_result;

			if (sub_result) {
				for(int i = 0; i < MAX_FREQ_BAND; i++) {
					double ΔLw0dir;
					if (p == B && source_type != aerodynamic) { // vertical directivity correction at 4.0m only applies to aerodynamic noise
						ΔLw0dir = vehicle->ΔLw0dirHorz[i];
					} else {
						ΔLw0dir = vehicle->ΔLw0dirVert[p][i] + vehicle->ΔLw0dirHorz[i];
					}
					vehicle->Lw0dir_[source_type][p][i] = vehicle->Lw0[source_type][p][i] + ΔLw0dir;	// IV-4

					vehicle->Lw0dir[p][i] += erg(vehicle->Lw0dir_[source_type][p][i]);
				}
			} else {
				clear(vehicle->Lw0dir_[source_type][p]);
			}
		} // for source_type
		
		// Finalize cumulation of the sources types (rolling, traction and aerodynamic)
		if (result)
		{
			for(int i = 0; i < MAX_FREQ_BAND; i++) {
				vehicle->Lw0dir[p][i] = dB(vehicle->Lw0dir[p][i]);
			}
		}

		return result;
	}

	/// <summary>
	/// Calculates the Δdirectivity_vertical.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_Δdirectivity_vertical() // 3.3.1
	{
		double ψ = track->get_angle_vertical() * PI / 180;
		for(int i = 0; i < MAX_FREQ_BAND; i++)
		{
			double fci;
			double t1;
			double t2;
			double t3;
			switch (p) {
				case A: // h == 0.5
					fci = FreqBands[i];
					t1 = 40.0 / 3.0;
					t2 = (2.0/3.0) * sin(2.0 * ψ) - sin(ψ);
					t3 = log10((fci+600.0) / 200.0);

					vehicle->ΔLw0dirVert[p][i] = abs( t1 * t2 * t3 ); // IV-15
					break; 
				case B: // h == 4.0
					if (ψ < 0) {
						vehicle->ΔLw0dirVert[p][i] = 10 * log10(pow(cos(ψ), 2)); // IV-16
					} else {
						vehicle->ΔLw0dirVert[p][i] = 0; // IV-17
					}
					break; 
			}
		}
		return true;
	}

	/// <summary>
	/// Calculates the Δdirectivity_horizontal.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_Δdirectivity_horizontal() // 3.3.2
	{
		double φ = track->get_angle_horizontal() * PI / 180;
		for (int i = 0; i < MAX_FREQ_BAND; i++)
		{
			vehicle->ΔLw0dirHorz[i] = dB(0.01 + 0.99 * pow(sin(φ), 2));
		}
		return true;
	}

	/// <summary>
	/// Calculate the rolling speed of the current vehicle
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_rolling_noise() // 3.4, 3.4.1, 3.4.2 -- h==0.5
	{
		clear(vehicle->Lw0[rolling][p]);
		
		if (vehicle->r == idling) 
		{
			return false;
		}

		if (p == B)
		{
			return false;
		}
		
		bool result = true;
		
		double	v = vehicle->v;
		double	Na = vehicle->get_number_of_axles();
		result = track->lookup_transfer_track(vehicle->LHTr) 
				&& vehicle->lookup_transfer_vehicle(vehicle->LHVeh) 
				&& track->lookup_transfer_superstructure(vehicle->LHVehSup);
		if (!result)
			return result;
		
		// Calculate LrTot
		result = calc_roughness_impact(v) && result;
		result = calc_squeal() && result;
		result = calc_bridge() && result;
		
		for(int i = 0; i < MAX_FREQ_BAND; i++) {
			double	LrTot = vehicle->LRtot[i];
			
			vehicle->LwTr[i] = LrTot + vehicle->LHTr[i] + dB(Na);
			vehicle->LwVeh[i] = LrTot + vehicle->LHVeh[i] + dB(Na);
			vehicle->LwVehSup[i] = LrTot + vehicle->LHVehSup[i] + dB(Na);
			
			vehicle->Lw0[rolling][p][i] = dBsum(vehicle->LwTr[i], vehicle->LwVeh[i], vehicle->LwVehSup[i])
											+ vehicle->ΔLsqueal[i] + vehicle->ΔLbridge;
		}

		return result;
	}

	/// <summary>
	/// Calculate the rolling speed of the current vehicle at the given speed
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_rolling_noise(const double v, freqs& Lw0v) // 3.4, 3.4.1, 3.4.2 -- h==0.5
	{
		// Create a temporary vehicle object to calculate the rolling noise at the given speed
		// (We do this so that the intermediate results of the original object are not affected)
		RailVehicle* tmp_vehicle = new RailVehicle(*this->vehicle);
		// Replace the vehicle's speed by the requested speed
		tmp_vehicle->v = v;

		// Replace our current vehicle with the temporary one (and keep a backup)
		RailVehicle* bak_vehicle = this->vehicle;
		this->vehicle = tmp_vehicle;
		
		// Calculate the rolling noise using the temporary vehicle object
		bool result = calc_rolling_noise();

		if (result) {
			// copy the rolling noise results
			for (int i = 0; i < MAX_FREQ_BAND; i++) {
				Lw0v[i] = vehicle->Lw0[rolling][p][i];
			}
		}

		// Restore the original vehicle
		this->vehicle = bak_vehicle;

		// clean up the temporary object
		delete tmp_vehicle;

		return result;
	}

	/// <summary>
	/// Calc_roughness_impacts this instance.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_roughness_impact(const double v) // 3.4.3
	{
		double	n = track->get_joint_density();
		n = 0.01 * n; // density n is given per 100 metres;

		freqs LrTr = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		freqs LrVeh = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		freqs A3 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		freqs LrImpactSingle = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		if (track->lookup_roughness(v, LrTr)
			&& vehicle->lookup_wheel_roughness(v, LrVeh) 
			&& vehicle->lookup_contact_filter(v, A3) 
			&& track->lookup_single_impact_filter(v, LrImpactSingle))
		{
			for(int i = 0; i < MAX_FREQ_BAND; i++) {
				vehicle->LrRough[i] = dBsum(LrTr[i], LrVeh[i]) + A3[i];
				vehicle->LrImpact[i] = LrImpactSingle[i] + dB(n / 0.01);
				vehicle->LRtot[i] = dBsum(vehicle->LrRough[i], vehicle->LrImpact[i]);
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	/// <summary>
	/// Calculate the squeal noise of the current vehicle.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_squeal() // 3.4.4
	{
		double R = track->get_curve_radius(); // radius of the curve [m]
		double ΔLsqueal; // [dB]
		if (R < 300) {
			ΔLsqueal = 8;
		} else if (R < 500) {
			ΔLsqueal = 5;
		} else {
			ΔLsqueal = 0;
		}
		for (int i = 0; i < MAX_FREQ_BAND; i++) {
			vehicle->ΔLsqueal[i] = ΔLsqueal;
		}
		return true;
	}

	/// <summary>
	/// Calc_bridges this instance.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_bridge() // 3.4.5
	{
		vehicle->ΔLbridge = track->lookup_bridge();
		return true;
	}

	/// <summary>
	/// Calc_traction_noises this instance.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_traction_noise() // 3.5 -- h==0.5|4.0
	{
		clear(vehicle->Lw0[traction][p]);

		freqs Ltraction = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		if (vehicle->lookup_traction(p, Ltraction)) {
			for(int i = 0; i < MAX_FREQ_BAND; i++) {
				vehicle->Lw0[traction][p][i] = Ltraction[i];
			}
			return true;
		} else {
			return false;
		}
	}

	/// <summary>
	/// Calc_aerodynamic_noises this instance.
	/// </summary>
	/// <returns>true if successful</returns>
	bool RailSection::calc_aerodynamic_noise() // 3.6 -- h==0.5|4.0
	{
		clear(vehicle->Lw0[aerodynamic][p]);
		if (vehicle->r == idling) 
		{
			return false;
		}

		bool result = true;

		freqs Lw0 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		if (vehicle->v > 200) {
			double	v0, α;
			freqs Lw0v0;
			result = vehicle->lookup_aerodynamic_noise(this->p, Lw0v0, v0, α) && (v0 != 0);
			if (result)
			{
				for (int i = 0; i < MAX_FREQ_BAND; i++)
				{
					Lw0[i] = Lw0v0[i] + α * log10(vehicle->v / v0);
				}
			}
		}

		for (int i = 0; i < MAX_FREQ_BAND; i++)
		{
			vehicle->Lw0[aerodynamic][p][i] = Lw0[i];
		}

		return result;
	}

	bool RailSection::load_from_xml_file(const string fn)
	{
		if (docInput.LoadFile(fn.c_str())) 
		{
			TiXmlElement* root = docInput.FirstChildElement("CNOSSOS_Rail_Input");
			if (root == NULL) {
				CNOSSOS::report_error(ERRMSG_WRONG_ROOT_ELEMENT, fn, &docInput);
				return false;
			} else if (root->Attribute("version") != XML_DATA_VERSION) {
				CNOSSOS::report_error(ERRMSG_WRONG_VERSION, fn, root);
			}
			TiXmlElement* track = root->FirstChildElement("Track");
			if (track == NULL)
			{
				CNOSSOS::report_error(ERRMSG_MISSING_ELEMENT + ": Track", fn, root);
				return false;
			}
			TiXmlElement* vehicles = root->FirstChildElement("Vehicles");
			if (vehicles == NULL)
			{
				CNOSSOS::report_error(ERRMSG_MISSING_ELEMENT + ": Vehicles", fn, root);
				return false;
			}
			
			// Read the calculation parameters
			this->doDebug = CNOSSOS::boolFromElement(root, "Test");
			this->Tref = CNOSSOS::floatFromElement(root, "Tref");
			this->Idling = CNOSSOS::boolFromElement(root, "Idling");
			this->source_type = (this->Idling ? stPoint : stLine);
			string source = CNOSSOS::stringFromElement(root, "Source");
			if (source == "A")
				this->physical_source = A;
			else if(source == "B")
				this->physical_source = B;
			else
			{
				physical_source = NUM_PHYSICAL_SOURCES;
				report_error(ERRMSG_MISSING_OR_INVALID_ELEMENT + ": Source", fn, root->FirstChild("Source"));
			}
			
			// load the vehicle definitions
			TiXmlElement* vehicle = vehicles->FirstChildElement("Vehicle");
			while (vehicle != NULL)
			{
				RailVehicle* oVehicle = new RailVehicle(catalogue, vehicle);
				if (oVehicle->xmlDefinition == NULL)
				{
					delete oVehicle;
					oVehicle = NULL;
				}
				this->vehicles.push_back(*oVehicle);
				
				vehicle = vehicle->NextSiblingElement(vehicle->Value());
			}

			// load the track definition into our track object
			this->track->setInput(track);

			return true;
		}
		else
		{
			CNOSSOS::report_error("Unable to load file: " + string(docInput.ErrorDesc()), fn);
			return false;
		}
	}

	void writeFrequencies(ofstream& csv, const freqs &f, const int bands = MAX_FREQ_BAND) {
		for (int i = 0; i < bands; i++) {
			csv << f[i] << "\t";
		}
		csv << endl;
	}
	void writeFrequencies(ofstream& csv, const freqcentres &f, const int bands = MAX_FREQ_BAND_CENTRE) {
		for (int i = 0; i < bands; i++) {
			csv << f[i] << "\t";
		}
		csv << endl;
	}
	
	bool RailSection::writeDebugData( const string fn ) {
		ofstream csv;
		csv.open(fn, ios_base::out);
		if (!csv)
		{
			cerr << "!!! Unable to write to file " << fn << endl;
			return false;
		}
		csv.imbue(locale("")); // use the user's default locale
		csv << "Sep=\t" << endl; // indicate we're going to use tab as separator

		// Write the general input data
		csv << "Tref:\t" << this->Tref << endl;
		string ps = PhysicalSourceNames[physical_source];
		csv << "Source:\t" << ps << endl;
		csv << "Idling:\t" << (this->Idling ? "yes" : "no") << "\t" << SourceGeometryTypeNames[this->source_type] << endl;
		csv << endl;

		// Write the input data for the track
		csv << "Track data:" << endl;
		csv << "Vertical angle\t" << track->get_angle_vertical() << endl;
		csv << "Horizontal angle\t" << track->get_angle_horizontal() << endl;
		csv << "Section length\t" << track->get_section_length() << endl;
		csv << "Curve radius\t" << track->get_curve_radius() << endl;
		csv << endl;

		// Write the end results
		csv << "[Hz]\t\t\t";
		for (int i = 0; i < MAX_FREQ_BAND_CENTRE; i++) {
			csv << FreqBands[i * 3 + 1] << "\t";
		}
		csv << endl;
		csv << "Lw\tA\tPointSource\t"; writeFrequencies(csv, Lw[A][stPoint]);
		csv << "Lw\tA\tLineSource\t"; writeFrequencies(csv, Lw[A][stLine]);
		csv << "Lw\tB\tPointSource\t"; writeFrequencies(csv, Lw[B][stPoint]);
		csv << "Lw\tB\tLineSource\t"; writeFrequencies(csv, Lw[B][stLine]);
		csv << endl;
		csv << "[Hz]\t\t\t"; writeFrequencies(csv, FreqBands);
		csv << "LwEqTdir\tA\tPointSource\t"; writeFrequencies(csv, LwEqTdir[A][stPoint]);
		csv << "LwEqTdir\tA\tLineSource\t"; writeFrequencies(csv, LwEqTdir[A][stLine]);
		csv << "LwEqTdir\tB\tPointSource\t"; writeFrequencies(csv, LwEqTdir[B][stPoint]);
		csv << "LwEqTdir\tB\tLineSource\t"; writeFrequencies(csv, LwEqTdir[B][stLine]);

		// Write the intermediate results
		for(std::vector<RailVehicle>::iterator it = vehicles.begin(); it != vehicles.end(); ++it) {
			this->vehicle = &*it;

			// output vehicle identifier and corresponding input data
			csv << endl;
			csv << " ---------- VEHICLE\tRef = " << vehicle->vehicle_id << "\t" << vehicle->get_description() << endl;
			csv << "Running condition:\t" << RunningConditionNames[vehicle->r] << "\t" << SourceGeometryTypeNames[vehicle->source_type] << endl;
			if (vehicle->r == idling) {
				csv << "Tidling:\t" << vehicle->Tidling << endl;
				csv << "Section length:\t" << track->get_section_length() << endl;
			} else {
				csv << "Q:\t" << vehicle->Q << endl;
				csv << "v:\t" << vehicle->v << endl;
			}
			csv << endl;

			for (int pi = A; pi < NUM_PHYSICAL_SOURCES; pi++) {
				this->p = static_cast<PhysicalSourceEnum>(pi);
				ps = PhysicalSourceNames[p];

				// Intermediate results
				csv << "LwEqLine\t"+ps+"\t\t"; writeFrequencies(csv, vehicle->LwEqLine[p]);
				csv << "Lw0dir\t"+ps+"\t\t"; writeFrequencies(csv, vehicle->Lw0dir[p]);
				for (int sti = rolling; sti < NUM_SOURCE_TYPES; sti++) {
					SourceTypeEnum source_type = static_cast<SourceTypeEnum>(sti);
					string st = SourceTypeNames[source_type];
					csv << "Lw0dir\t"+ps+"\t"+st+"\t"; writeFrequencies(csv, vehicle->Lw0dir_[source_type][p]);
					csv << "Lw0\t"+ps+"\t"+st+"\t"; writeFrequencies(csv, vehicle->Lw0[source_type][p]);
				}
				csv << "deltaLw0dirVert\t"+ps+"\t\t"; writeFrequencies(csv, vehicle->ΔLw0dirVert[p]);
				csv << endl;
			} // for p
			csv << "deltaLw0dirHorz\t\t\t"; writeFrequencies(csv, vehicle->ΔLw0dirHorz);
			csv << endl;
			
			// Intermediate results independent of physical source
			csv << "LwTr\t\t\t"; writeFrequencies(csv, vehicle->LwTr);
			csv << "LwVeh\t\t\t"; writeFrequencies(csv, vehicle->LwVeh);
			csv << "LwVehSup\t\t\t"; writeFrequencies(csv, vehicle->LwVehSup);
			csv << "deltaLsqueal\t\t\t"; writeFrequencies(csv, vehicle->ΔLsqueal);
			csv << "deltaLbridge\t\t\t" << vehicle->ΔLbridge << endl;
			csv << "LRtot\t\t\t"; writeFrequencies(csv, vehicle->LRtot);
			csv << "LHTr\t\t\t"; writeFrequencies(csv, vehicle->LHTr);
			csv << "LHVeh\t\t\t"; writeFrequencies(csv, vehicle->LHVeh);
			csv << "LHVehSup\t\t\t"; writeFrequencies(csv, vehicle->LHVehSup);
			csv << "LrRough\t\t\t"; writeFrequencies(csv, vehicle->LrRough);
			csv << "LrImpact\t\t\t"; writeFrequencies(csv, vehicle->LrImpact);
		}

		csv.close();
		return true;
	}

	bool RailSection::save_results_to_xml_file(const string fn)
	{
		TiXmlDocument doc;
		TiXmlDeclaration *pi = new TiXmlDeclaration("1.0", "", "");
		doc.LinkEndChild(pi);
		TiXmlElement *root = new TiXmlElement("CNOSSOS_SourcePower");
		root->SetAttribute("version", XML_DATA_VERSION.c_str());
		doc.LinkEndChild(root);

		TiXmlElement *src = new TiXmlElement("source");
		root->LinkEndChild(src);

		TiXmlElement *e = new TiXmlElement("h");
		src->LinkEndChild(e);
			
		TiXmlText *text = new TiXmlText(CNOSSOS::stringFromFloat(catalogue->get_source_height(physical_source)).c_str());
		e->LinkEndChild(text);

		e = new TiXmlElement("Lw");
		src->LinkEndChild(e);

		e->SetAttribute("sourceType", SourceGeometryTypeNames[source_type].c_str());
		e->SetAttribute("measurementType", "FreeField");
		e->SetAttribute("frequencyWeighting", "LIN");

		text = new TiXmlText(CNOSSOS::stringFromFloats(Lw[physical_source][source_type], MAX_FREQ_BAND_CENTRE).c_str());
		e->LinkEndChild(text);

		return doc.SaveFile(fn.c_str());
	}
#pragma endregion RailSection;


#pragma region "RailCatalogue class"

	RailCatalogue::RailCatalogue()
	{
	}
	RailCatalogue::~RailCatalogue()
	{
	}

	double RailCatalogue::get_source_height(PhysicalSourceEnum p)
	{
		switch (p)
		{
			case A:
				return selectDouble(&docVehicles, "/RailParameters/h1");
			case B:
				return selectDouble(&docVehicles, "/RailParameters/h2");
			default:
				return 0; // ?!?
		}
	}

	bool RailCatalogue::load_from_xml_files(string fnTrack, string fnVehicles) {
		bool result = true;
		
		if (!docTrack.LoadFile(fnTrack.c_str())) {
			CNOSSOS::report_error("Unable to load file: " + string(docTrack.ErrorDesc()), fnTrack);
			return false;
		} else {
			TiXmlElement *root = docTrack.FirstChildElement("TrackParameters");
			if (root == NULL)
			{
				CNOSSOS::report_error(ERRMSG_WRONG_ROOT_ELEMENT, fnTrack, &docTrack);
				return false;
			} else if (root->Attribute("version") != XML_DATA_VERSION)
			{
				CNOSSOS::report_error(ERRMSG_WRONG_VERSION, fnTrack, root);
				return false;
			}
		}

		if (!docVehicles.LoadFile(fnVehicles.c_str())) {
			CNOSSOS::report_error("Unable to load file: " + string(docVehicles.ErrorDesc()), fnVehicles);
			return false;
		} else {
			TiXmlElement *root = docVehicles.FirstChildElement("RailParameters");
			if (root == NULL)
			{
				CNOSSOS::report_error(ERRMSG_WRONG_ROOT_ELEMENT, fnVehicles, &docVehicles);
				return false;
			} else if (root->Attribute("version") != XML_DATA_VERSION)
			{
				CNOSSOS::report_error(ERRMSG_WRONG_VERSION, fnVehicles, root);
				return false;
			}
		}

		return result;
	}

#pragma endregion RailCatalogue;


#pragma region "RailVehicle class"
	RailVehicle::RailVehicle(RailCatalogue *catalog, TiXmlElement *input_node)
	{

		clear(LwEqLine[A]);
		clear(LwEqLine[B]);
		clear(Lw0dir[A]);
		clear(Lw0dir[B]);
 		clear(Lw0dir_[rolling][A]);
		clear(Lw0dir_[rolling][B]);
		clear(Lw0dir_[traction][A]);
		clear(Lw0dir_[traction][B]);
		clear(Lw0dir_[aerodynamic][A]);
		clear(Lw0dir_[aerodynamic][B]);
		clear(ΔLw0dirVert[A]);
		clear(ΔLw0dirVert[B]);
		clear(ΔLw0dirHorz);
 		clear(Lw0[rolling][A]);
		clear(Lw0[rolling][B]);
		clear(Lw0[traction][A]);
		clear(Lw0[traction][B]);
		clear(Lw0[aerodynamic][A]);
		clear(Lw0[aerodynamic][B]);
		clear(LwTr);
		clear(LwVeh);
		clear(LwVehSup);
		clear(ΔLsqueal);
		ΔLbridge = 0.0;
		clear(LRtot);
		clear(LHTr);
		clear(LHVeh);
		clear(LHVehSup);
		clear(LrRough);
		clear(LrImpact);

		this->catalogue = catalog;
		this->xmlInput = input_node;
		this->vehicle_id = xmlInput->Attribute("Ref");
		this->xmlDefinition = findElementByAttribute(&catalog->docVehicles, "/RailParameters/VehicleDefinition/Vehicle", "ID", vehicle_id);
		if (xmlDefinition == NULL) {
			report_error("Unrecognized vehicle ID: " + vehicle_id, "", xmlInput);
		}

		string	rc = xmlInput->Attribute("RunningCondition");
		int		value = 0;
		if (CNOSSOS::mapStringToEnum(rc, RunningConditionNames, NUM_RUNNING_CONDITIONS, &value))
		{
			this->r = static_cast<RunningConditionEnum>(value);
			this->source_type = (this->r == idling ? stPoint : stLine);
		}
		else
		{
			CNOSSOS::report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": RunningCondition (" + rc + ")", "", xmlInput);
		}
		if (xmlInput->QueryDoubleAttribute("Q", &this->Q) != TIXML_SUCCESS)
		{
			report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": Q", "", xmlInput);
		}
		if (xmlInput->QueryDoubleAttribute("v", &this->v) != TIXML_SUCCESS)
		{
			report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": v", "", xmlInput);
		}
		if (xmlInput->QueryDoubleAttribute("IdlingTime", &this->Tidling) != TIXML_SUCCESS)
		{
			report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": IdlingTime", "", xmlInput);
		}
	}


	double RailVehicle::get_idling_time()
	{
		return Tidling;
	}

	string RailVehicle::get_description()
	{
		return xmlInput->Attribute("Description");
	}

	double RailVehicle::get_number_of_axles()
	{
		double result = 0;
		if (xmlDefinition->QueryDoubleAttribute("Axles", &result) != TIXML_SUCCESS)
		{
			report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": Axles", "", xmlDefinition);
		}
		return result;
	}

	bool RailVehicle::lookup_aerodynamic_noise(const PhysicalSourceEnum p, freqs& Lw0v0, double& v0, double& α) {
		const string	baseXPath = "/RailParameters/AerodynamicNoise/Aerodynamic";
		string			id = xmlDefinition->Attribute("RefAerodynamic");
		TiXmlElement*	xmlVehicle = findElementByAttribute(&catalogue->docVehicles, baseXPath, "ID", id);
		if (xmlVehicle == NULL) {
			report_error(ERRMSG_MISSING_ELEMENT + ": "+baseXPath+"[@ID='"+ id +"']", "", &catalogue->docVehicles);
			return false;
		}
		TiXmlElement*	xmlSource = findElementByAttribute(xmlVehicle, "Source", "Type", PhysicalSourceNames[p]);
		if (xmlSource == NULL) {
			report_error(ERRMSG_MISSING_ELEMENT + ": "+baseXPath+"[@ID='"+ id +"']/Source[@Type='" + PhysicalSourceNames[p] + "']", "", xmlVehicle);
			return false;
		}
		return (xmlSource->QueryDoubleAttribute("V0", &v0) == TIXML_SUCCESS
				&& xmlSource->QueryDoubleAttribute("Alpha", &α) == TIXML_SUCCESS
				&& selectFreqs(xmlSource, "Values", Lw0v0));
	}

	bool RailVehicle::lookup_transfer_vehicle(freqs& f)
	{
		string	id = xmlDefinition->Attribute("RefTransfer");
		return selectFreqs(&catalogue->docVehicles, "/RailParameters/VehicleTransfer/Transfer", "ID", id, "Values", f);
	}

	bool RailVehicle::lookup_traction(const PhysicalSourceEnum p, freqs& f)
	{
		string			id = xmlDefinition->Attribute("RefTraction");
		const string	baseXPath = "/RailParameters/TractionNoise/Traction";
		TiXmlElement*	xmlTraction = findElementByAttribute(&catalogue->docVehicles, baseXPath, "ID", id);
		if (xmlTraction == NULL) {
			report_error(ERRMSG_MISSING_ELEMENT + ": "+baseXPath+"[@ID='"+ id +"']", "", &catalogue->docVehicles);
			return false;
		}
		string	attr_name = "";
		switch(this->r) {
			case constant:
				attr_name = "Constant";
				break;
			case decelerating:
				attr_name = "Decelerating";
				break;
			case accelerating:
				attr_name = "Accelerating";
				break;
			case idling:
				attr_name = "Idling";
				break;
		}
		return selectFreqs(xmlTraction, "Source", "Type", PhysicalSourceNames[p], attr_name, f);
	}

	bool RailVehicle::lookup_wheel_roughness(const double v, freqs& f)
	{
		string		id = xmlDefinition->Attribute("RefRoughness");
		wavelengths wl;
		if (selectWavelengths(&catalogue->docVehicles, "/RailParameters/WheelRoughness/Roughness", "ID", id, "Values", wl))
		{
			wavelengths_to_frequencies(v, wl, f);
			return true;
		}
		else
			return false;
	}

	bool RailVehicle::lookup_contact_filter(const double v, freqs& f)
	{
		string	id = xmlDefinition->Attribute("RefContact");
		wavelengths wl;
		if (selectWavelengths(&catalogue->docVehicles, "/RailParameters/ContactFilter/Contact", "ID", id, "Values", wl))
		{
			wavelengths_to_frequencies(v, wl, f);
			return true;
		}
		else
			return false;
	}

#pragma endregion RailVehicle;


#pragma region "RailTrack class"

	RailTrack::RailTrack(RailCatalogue* catalogue)
	{
		this->catalogue = catalogue;
		this->xmlInput = NULL;
	}
	RailTrack::~RailTrack()
	{
		this->xmlInput = NULL;
		this->catalogue = NULL;
	}

	void RailTrack::setInput(TiXmlElement* track)
	{
		this->xmlInput = track;
	}

	double RailTrack::get_input_value(const string name, const double def)
	{
		double result = def;
		if (xmlInput->QueryDoubleAttribute(name.c_str(), &result) != TIXML_SUCCESS)
		{
			report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": " + name, "", xmlInput);
		}
		return result;
	}
	
	double RailTrack::get_section_length()
	{
		return get_input_value("SectionLength");
	}

	double RailTrack::get_angle_vertical()
	{
		return get_input_value("VerticalAngle");
	}

	double RailTrack::get_angle_horizontal()
	{
		return get_input_value("HorizontalAngle");
	}

	double RailTrack::get_curve_radius()
	{
		return get_input_value("CurveRadius");
	}

	double RailTrack::get_joint_density()
	{
		string ID = xmlInput->Attribute("ImpactNoiseID");
		TiXmlElement *element = findElementByAttribute(&catalogue->docTrack, "/TrackParameters/ImpactNoise/Impact", "ID", ID);
		double result = 0;
		if (element == NULL || element->QueryDoubleAttribute("JointDensity", &result) != TIXML_SUCCESS)
		{
			report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": JointDensity", catalogue->docTrack.Value(), element);
		}
		return result;
	}

	bool RailTrack::lookup_transfer_track(freqs& f)
	{
		string ID = xmlInput->Attribute("TrackTransferID");
		return selectFreqs(&catalogue->docTrack, "/TrackParameters/TrackTransfer/Track", "ID", ID, "Values", f);
	}

	bool RailTrack::lookup_transfer_superstructure(freqs& f)
	{
		string ID = xmlInput->Attribute("StructureTransferID");
		return selectFreqs(&catalogue->docTrack, "/TrackParameters/StructureTransfer/Structure", "ID", ID, "Values", f);
	}

	bool RailTrack::lookup_roughness(const double v, freqs& f)
	{
		string ID = xmlInput->Attribute("RailRoughnessID");
		wavelengths wl = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		if (selectWavelengths(&catalogue->docTrack, "/TrackParameters/RailRoughness/Rail", "ID", ID, "Values", wl))
		{
			wavelengths_to_frequencies(v, wl, f);
			return true;
		}
		else
			return false;
	}

	bool RailTrack::lookup_single_impact_filter(const double v, freqs& f)
	{
		string ID = xmlInput->Attribute("ImpactNoiseID");
		wavelengths wl = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		if (selectWavelengths(&catalogue->docTrack, "/TrackParameters/ImpactNoise/Impact", "ID", ID, "Values", wl))
		{
			wavelengths_to_frequencies(v, wl, f);
			return true;
		}
		else
			return false;
	}

	double RailTrack::lookup_bridge()
	{
		string ID = xmlInput->Attribute("BridgeConstantID");
		TiXmlElement* element = findElementByAttribute(&catalogue->docTrack, "/TrackParameters/BridgeConstant/Bridge", "ID", ID);
		double result = 0;
		if (element != NULL) {
			element->QueryDoubleAttribute("Value", &result);
		} else {
			report_error(ERRMSG_MISSING_OR_INVALID_ATTRIBUTES + ": Bridge", catalogue->docTrack.Value(), element);
		}
		return result;
	}


#pragma endregion RailTrack;
	
}
