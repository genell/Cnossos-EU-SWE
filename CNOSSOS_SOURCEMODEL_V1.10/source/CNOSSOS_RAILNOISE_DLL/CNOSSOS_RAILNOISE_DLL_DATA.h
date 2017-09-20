#pragma once

#include "stdafx.h"
#include "CNOSSOS_RAILNOISE_DLL_CONST.h"
#include "CNOSSOS_RAILNOISE_DLL_AUX.h"
#include "../tinyxml/tinyxml.h"

#include <string>
#include <iostream>
#include <list>
#include <vector>

using namespace std;

namespace CNOSSOS_RAILNOISE
{		
	typedef double freqs[MAX_FREQ_BAND];
	typedef double freqcentres[MAX_FREQ_BAND_CENTRE];
	typedef double wavelengths[MAX_WAVELENGTH];

	
	// XML helper functions
	TiXmlNode*		selectNode(TiXmlNode* base, string xPath);
	TiXmlElement*	findElementByAttribute(TiXmlNode* base, string xPath, string attributeName, string attributeValue);
	double			selectDouble(TiXmlNode* base, string xPath);
	bool			selectFreqs(TiXmlElement* element, const string valueAttribute, freqs& f);
	bool			selectFreqs(TiXmlNode* base, const string xPath, const string attributeName, const string attributeValue, const string valueAttribute, freqs& f);
	bool			selectWavelengths(TiXmlNode* base, string xPath, string attributeName, string attributeValue, string valueAttribute, wavelengths& wl);
	
	
	class RailCatalogue
	{
		public:
			TiXmlDocument	docTrack;
			TiXmlDocument	docVehicles;

			RailCatalogue();
			~RailCatalogue();
			bool	load_from_xml_files(string fnTrack, string fnVehicles);
			double	get_source_height(PhysicalSourceEnum p);
	};

	
	class RailTrack
	{
		private:
			RailCatalogue*	catalogue;
			TiXmlElement*	xmlInput;

			double	get_input_value(const string name, const double def = 0.0);

		public:
			// lookup functions
			double	get_section_length();
			double  get_angle_vertical();
			double  get_angle_horizontal();
			double	get_joint_density();
			double	get_curve_radius();

			bool	lookup_transfer_track(freqs& f);
			bool	lookup_transfer_superstructure(freqs& f);
			bool	lookup_roughness(const double v, freqs& f);				//	/TrackParameters/RailRoughness/Rail/@Values
			bool	lookup_single_impact_filter(const double v, freqs& f);	//	/TrackParameters/ImpactNoise/Impact[@ID=$input/@ImpactNoiseID]/@Values
			double  lookup_bridge();										//	/TrackParameters/BridgeConstant/Bridge[@ID=$input/@BridgeConstantID]/@Value

			RailTrack(RailCatalogue* catalogue);
			~RailTrack();

			void	setInput(TiXmlElement* track);
	};
	
	/// <summary>
	/// Stores all intermediate calculation results for a single vehicle category on a single track.
	/// </summary>
	class RailVehicle
	{
		private:
			RailCatalogue*	catalogue;
			TiXmlElement*	xmlInput;

		public:
			TiXmlElement*	xmlDefinition;

			// input data
			string					vehicle_id;	// ID of this vehicle type
			RunningConditionEnum	r;			// running condition
			double					Q;			// number of vehicles (of this type) per hour
			double					v;			// speed [km/h]
			double					Tidling;	// idling time
			SourceGeometryTypeEnum	source_type;	// point, line, area (depends on the running condition)

			// intermediate results
			freqs	LwEqLine[NUM_PHYSICAL_SOURCES];
			freqs	Lw0dir[NUM_PHYSICAL_SOURCES];
			freqs	Lw0dir_[NUM_SOURCE_TYPES][NUM_PHYSICAL_SOURCES];
			freqs	ΔLw0dirVert[NUM_PHYSICAL_SOURCES];
			freqs	ΔLw0dirHorz;
			freqs	Lw0[NUM_SOURCE_TYPES][NUM_PHYSICAL_SOURCES];
			freqs	LwTr;
			freqs	LwVeh;
			freqs	LwVehSup;
			freqs	ΔLsqueal;
			double	ΔLbridge;
			freqs	LRtot;
			freqs	LHTr;
			freqs	LHVeh;
			freqs	LHVehSup;
			freqs	LrRough;
			freqs	LrImpact;

			// lookup functions
			string	get_description();
			double	get_idling_time();
			double	get_number_of_axles();										//	/RailParameters/VehicleDefinition/Vehicle/@Axles

			bool	lookup_transfer_vehicle(freqs& f);							//	/RailParameters/VehicleTransfer/Transfer/@Values
			bool	lookup_traction(const PhysicalSourceEnum p, freqs& f);		//	/RailParameters/TractionNoise/Traction/Source[@Type={p}]/<@Constant|@Accelerating|@Decelerating|@Idling>
			bool	lookup_wheel_roughness(const double v, freqs& f);			//	/RailParameters/WheelRoughness/Roughness/@Values
			bool	lookup_contact_filter(const double v, freqs& f);			//	/RailParameters/ContactFilter/Contact/@Values
			bool	lookup_aerodynamic_noise(const PhysicalSourceEnum p,		// /RailParameters/AerodynamicNoise/Aerodynamic/Source[@Type={p}]/<@V0|@Values|@Alpha>
											freqs& Lw0v0, double& v0, double& α);

			// constructor / destructor
			RailVehicle(RailCatalogue *catalog, TiXmlElement *input_node);
	};
	
	
	/// <summary>
	/// Provides noise calculation for a single track section where one or more railway vehicles circulate. 
	/// </summary>
	class RailSection
	{
		private:
			TiXmlDocument	docInput;
			RailCatalogue*	catalogue;

			// Calculation indexes
			RailTrack*				track;		// current track
			RailVehicle*			vehicle;	// current vehicle
			PhysicalSourceEnum		p;			// physical source index

			// Intermediate results for each vehicle (type)
			vector<RailVehicle>		vehicles;
			
			// Calculation functions
			bool	calc_flow_per_running_condition(); // 3.2
			bool	calc_flow_moving(); // 3.2.1
			bool	calc_flow_idling(); // 3.2.2
			bool	calc_directional_sound_power(); // 3.3
			bool	calc_Δdirectivity_vertical(); // 3.3.1
			bool	calc_Δdirectivity_horizontal(); // 3.3.2
			bool	calc_rolling_noise(); // 3.4, 3.4.1, 3.4.2 -- h==0.5
			bool	calc_rolling_noise(const double v, freqs& Lw0v);
			bool	calc_roughness_impact(const double v); // 3.4.3
			bool	calc_squeal(); // 3.4.4
			bool	calc_bridge(); // 3.4.5
			bool	calc_traction_noise(); // 3.5 -- h==0.5|4.0
			bool	calc_aerodynamic_noise(); // 3.6 -- h==0.5|4.0

		public:
			bool					doDebug;
			double					Tref;				// Reference time
			PhysicalSourceEnum		physical_source;	// Physical source index
			bool					Idling;				// Whether to take only idling vehicles into account, or only moving.
			SourceGeometryTypeEnum	source_type;

			freqs		LwEqTdir[NUM_PHYSICAL_SOURCES][NUM_SOURCE_GEOMETRY_TYPES];	// Sound power in 1/3 octave bands
			freqcentres	Lw[NUM_PHYSICAL_SOURCES][NUM_SOURCE_GEOMETRY_TYPES];		// Sound power in octave bands
			
			RailSection(RailCatalogue *catalogue);
			~RailSection();

			// file i/o functions
			bool	load_from_xml_file(const string fn);
			bool	save_results_to_xml_file(const string fn);
			bool	writeDebugData(const string fn);

			bool	calculate(); // 3.1
	};

}


