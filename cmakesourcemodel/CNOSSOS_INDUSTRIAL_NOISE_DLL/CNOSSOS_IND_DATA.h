#pragma once

#include "stdafx.h"
#include "CNOSSOS_IND_CONST.h"

#include <vector>

using namespace std;

namespace CNOSSOS_INDUSTRIAL_NOISE
{
	// Forward declarations
	class IndustryCatalogue;
	class IndustrySourceDef;
	class IndustrySource;

	// --------------------------------------------------------------------------------------------
	// Definition of calculation class
	class IndustrySourceSet
	{
		private:
			IndustryCatalogue *catalogue;
			vector<IndustrySource> sources;

		public:
			// debugging and testing
			bool doDebug;

			//// Public input fields
			//double Height;
			//SourceType Type;
			//SourceDirectionality Directionality;
			//Frequencies Lw;
			//double horz_angle;
			//double vert_angle;

			// default constructor and destructor
			IndustrySourceSet(IndustryCatalogue *catalogue);
			~IndustrySourceSet();

			// Accessor methods
			IndustryCatalogue *get_catalogue();
			
			// I/O functions
			bool loadFromXmlFile(const string fn);
			bool saveResultsToXmlFile(const string fn);
			bool writeDebugData(const string fn);

			// Calculate the source power of the noise sources
			int Calculate();
	};

	static IndustrySourceSet *currentSourceSet;

	// --------------------------------------------------------------------------------------------
	// Definition of source
	class IndustrySource
	{
	private:
		IndustrySourceSet *parent;
		string id;
		IndustrySourceDef *definition;

	public:
		// Input fields
		string      weighting;
		double		height;
		double		period;
		double		source_time;
		bool		calcMovingVehicles;
		int			vehicle_count;
		double		vehicle_speed;
		double		length;
		double		horz_angle;
		double		vert_angle;

		// Result fields
		double deltaCw;
		Frequencies deltaDir;
		Frequencies Lw;

		// Constructor and destructor
		IndustrySource(IndustrySourceSet *parent);
		~IndustrySource();

		// Accessor methods
		void   set_id(const string id);
		string get_id();

		IndustrySourceDef* get_definition();

		// Calculate the source power of the noise sources
		bool Calculate();
		bool CalculateWorkinghourCorrection();
		bool CalculateDirectivity();
		bool CalculateSummation();

	};


	// --------------------------------------------------------------------------------------------
	// Definition of source definition
	class IndustrySourceDef
	{
		public:
			string id;
			string description;
			double height;
			SourceType type;
			SourceDirectionality directionality;
			Frequencies Lw;
			Frequencies directivity[36][36]; // per 10°
			//vector<vector<Frequencies>> directivity;
	};

	// --------------------------------------------------------------------------------------------
	// Definition of catalogue class
	class IndustryCatalogue
	{
		private:
			vector<IndustrySourceDef> sourceDefs;

		public:
			// Public values
			// ...

			// I/O functions
			bool loadFromXmlFile(const string fn);

			// default constructor and destructor
			IndustryCatalogue();
			~IndustryCatalogue();

			// Accessor methods
			IndustrySourceDef* getSourceDefById(const string id);
	};

	static IndustryCatalogue *catalogue;

}