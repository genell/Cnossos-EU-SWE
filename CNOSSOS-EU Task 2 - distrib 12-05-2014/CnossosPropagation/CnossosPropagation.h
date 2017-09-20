/*! 
 * \file CnossosPropagation.h 
 * \brief C-style API for the Cnossos-EU propagation modules
 * \author dirk.van-maercke@cstb.fr
 *
 * \mainpage CNOSSOS-EU Propagation Modules
 * \section main1 Public functions defined in the CnossosPropagation shared library
 * \subsection s1 Create, configure and delete the calculation engine
 * - \ref CNOSSOS_P2P_CreateEngine
 * - \ref CNOSSOS_P2P_DeleteEngine
 * - \ref CNOSSOS_P2P_SelectMethod
 * - \ref CNOSSOS_P2P_GetMethod
 * - \ref CNOSSOS_P2P_GetVersion
 * - \ref CNOSSOS_P2P_SetOptions
 * - \ref CNOSSOS_P2P_GetOptions
 * - \ref CNOSSOS_P2P_SetMeteo
 * - \ref CNOSSOS_P2P_GetMeteo
 * \subsection s2 Define the propagation path
 * - \ref CNOSSOS_P2P_ClearPath
 * - \ref CNOSSOS_P2P_AddToPath
 * - \ref CNOSSOS_P2P_SetSoundPower
 * \subsection s3 Create vertical extensions
 * - \ref CNOSSOS_P2P_CreatePointSource
 * - \ref CNOSSOS_P2P_CreateLineSource
 * - \ref CNOSSOS_P2P_CreateReceiver
 * - \ref CNOSSOS_P2P_CreateBarrier
 * - \ref CNOSSOS_P2P_CreateVerticalWall
 * - \ref CNOSSOS_P2P_CreateVerticalEdge
 * \subsection s4 Get the acoustical results 
 * - \ref CNOSSOS_P2P_GetResult
 * - \ref CNOSSOS_P2P_PrintPathData
 * - \ref CNOSSOS_P2P_PrintPathResults
 * \subsection s5 Manage material properties
 * - \ref CNOSSOS_P2P_GetMaterial
 * - \ref CNOSSOS_P2P_GetGValue
 * - \ref CNOSSOS_P2P_SetGValue
 * - \ref CNOSSOS_P2P_GetSigma
 * - \ref CNOSSOS_P2P_SetSigma
 * - \ref CNOSSOS_P2P_GetAlpha
 * - \ref CNOSSOS_P2P_SetAlpha
 * \subsection s6 Utility functions
 * - \ref CNOSSOS_P2P_GetVersionDLL
 * - \ref CNOSSOS_P2P_GetErrorMessage
 * - \ref CNOSSOS_P2P_ProcessPathFile
 * - \ref CNOSSOS_P2P_SetInfinityMode
 */
#ifndef _CNOSSOS_PROPAGATION_INCLUDED_
#define _CNOSSOS_PROPAGATION_INCLUDED_
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		CnossosPropagation.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: CnossosPropagation defines a standard C-style API to the CNOSSOS-EU propagation
 *	            module. Most popular programming languages can be bound to pure C functions 
 *				defined in DLL shared libraries, either at linker level, either at run time.
 * changes:
 *
 *	03/12/2013	initial version
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#if defined(WIN32) && !defined(__GNUC__)
	#ifdef _CNOSSOS_PROPAGATION_EXPORTS_
		#define _CNOSSOS_DLL_DECL_	extern "C" __declspec(dllexport)
	#else
		#define _CNOSSOS_DLL_DECL_	extern "C" __declspec(dllimport)
	#endif
#elif !defined (_CNOSSOS_DLL_DECL_)
	#if defined (__cplusplus)
		#define _CNOSSOS_DLL_DECL_	extern "C"
	#else
		#define _CNOSSOS_DLL_DECL_
	#endif
#endif

/**
 * \brief current version associated with this API
 */
#define CNOSSOS_P2P_VERSION_API "1.001"
/**
 * \brief Get the current version of the CnossosPropagation shared library.
 * \return String encoded version of the shared library.
 */
_CNOSSOS_DLL_DECL_ const char* CNOSSOS_P2P_GetVersionDLL (void) ;
/**
 * \brief Get the frequency range for the spectral data 
 * \param freq Array in which to return the frequency bands associated with spectral information 
 * passed to and from the shared library. If a NULL pointer is specified, the function returns
 * the number of values that would have been written on output but does not transfer any actual
 * values.
 * \return The number of frequency bands.
 * \note It is up to the caller to pass in an array of sufficient size. Applications can determine
 * the minimal size of this array by first calling the function with a NULL argument. 
 */
_CNOSSOS_DLL_DECL_ unsigned int CNOSSOS_P2P_GetFreq (double *freq) ;
/**
 * \brief Transparent pointer to the library's internal data structures for material properties
 */
struct CNOSSOS_P2P_MATERIAL ;
/**
 * \brief Transparent pointer to the library's internal data structures for vertical extensions 
 * associated with control points in the propagation path.
 */
struct CNOSSOS_P2P_EXTENSION ;
/**
 * \brief Transparent pointer to the library's internal data structures for the propagation 
 * calculation engine.
 */
struct CNOSSOS_P2P_ENGINE ;
/**
 * \brief Encoding of positions using 3 dimensional (X,Y,Z) coordinates
 */
typedef double CNOSSOS_P2P_POSITION[3] ;
/*!
 * \brief Sound power frequency weighting
 */
enum CNOSSOS_P2P_SPECTRUM_WEIGHTING
{
	CNOSSOS_P2P_SPECTRUM_LIN = 0, /**< Spectrum is unweighted */
	CNOSSOS_P2P_SPECTRUM_dBA = 1  /**< Spectrum is A-weighted */
};
/*!
 * \brief Sound power measurement conditions
 */
enum CNOSSOS_P2P_LW_MEASUREMENT_TYPE
{
	CNOSSOS_P2P_LW_UNDEFINED = 0,		/**< Measurement conditions undefined */
	CNOSSOS_P2P_LW_FREEFIELD = 1,       /**< Sound power measured under free field conditions */ 
	CNOSSOS_P2P_LW_HEMISPHERICAL = 2    /**< Sound power measured under hemispherical conditions*/
};
/**
 * \brief Choice of meteorological averaging
 */
enum CNOSSOS_P2P_METEO_MODEL
{
	CNOSSOS_P2P_METEO_DEFAULT = 0,		/**< Use default model corresponding to the selected calculation method */
	CNOSSOS_P2P_METEO_ISO9613 = 1,	    /**< Use the C0 correction as defined in the ISO 9613-2 standard */
	CNOSSOS_P2P_METEO_JRC2012 = 2       /**< Use the frequency of occurence of favorable propagation conditions */
};
/**
 * \brief Meteorological input data
 */
struct CNOSSOS_P2P_METEO
{
	CNOSSOS_P2P_METEO_MODEL model ;		/**< Choice of meteorological model */
	double C0 ;							/**< C0 correction term as defined in ISO 9613-2 */
	double pFav ;						/**< Frequency of occurence of favourable propagation conditions */
	double temperature ;				/**< Air temperature */
	double humidity ;					/**< Air relative humidity (expressed as %) */
};
/**
 * \brief Options modifying the behavior of the calculation engine
 */
struct CNOSSOS_P2P_OPTIONS
{
	/*
	 * validation of input data with respect to geometrical requirements
	 */
	bool CheckHorizontalAlignment ;		//!< check validity of paths in the horizontal plane
	bool CheckLateralDiffraction ;		//!< check validity of laterally diffracted paths
	bool CheckHeightLowerBound ;		//!< check height of ray path with respect to upper limits of vertical obstacles
	bool CheckHeightUpperBound ;		//!< check height of ray path with respect to lower limits of vertical obstacles
	bool CheckSourceSegment ;           //!< check whether source line segments contains actual source position
	bool CheckSoundPowerUnits ;			//!< check conformity of sound power units and source geometry
	/*
		* reinforce requirements on the geometrical complexity of the path
		* note that some requirements will be reinforced by the calculation method independently of user settings
		*/
	bool ForceSourceToReceiver ;		//!< reverse the path in case it is ordered from Receiver to Source
	bool SimplifyPathGeometry ;			//!< remove non representative terrain points
	bool DisableReflections ;			//!< disable reflections
	bool DisableLateralDiffractions ;	//!< disable lateral diffractions
	bool IgnoreComplexPaths ;			//!< disable reflections and vertical diffraction for laterally diffracted paths
	/*
		* disable some parts of the acoustical calculations, e.g. because these parts are already taken care of
		* in other parts of the application software
		*/
	bool ExcludeSoundPower ;			//!< include sound power
	bool ExcludeGeometricalSpread ;		//!< include geometrical spread
	bool ExcludeAirAbsorption ;			//!< include atmospheric absorption
} ;
/**
 * \brief Select the result to be returned by the call to \ref CNOSSOS_P2P_GetResult
 */
enum CNOSSOS_P2P_RESULT
{
	CNOSSOS_P2P_RESULT_LP_AVG_dBA  = -1, //!< global, dB(A) weighted, long term averaged noise level
	CNOSSOS_P2P_RESULT_LP_FAV_dBA  = -2, //!< global, dB(A) weighted, noise level under favorable propagation conditions
	CNOSSOS_P2P_RESULT_LP_HOM_dBA  = -3, //!< global, dB(A) weighted, noise level under homogeneous propagation conditions
	CNOSSOS_P2P_RESULT_LP_AVG  = 0,      //!< long-time averaged noise level spectrum
	CNOSSOS_P2P_RESULT_LP_FAV  = 1,      //!< noise level spectrum under favorable propagation conditions
	CNOSSOS_P2P_RESULT_LP_HOM  = 2,      //!< noise level spectrum under homogeneous propagation conditions
	CNOSSOS_P2P_RESULT_ATT_FAV = 3,      //!< excess attenuation spectrum under favorable propagation conditions
	CNOSSOS_P2P_RESULT_ATT_HOM = 4,      //!< excess attenuation spectrum under homogeneous propagation conditions
	CNOSSOS_P2P_RESULT_LW_SOURCE = 5,    //!< sound power spectrum
	CNOSSOS_P2P_RESULT_DELTA_LW = 6,     //!< sound power conversion, free field versus hemispherical radiation conditions
	CNOSSOS_P2P_RESULT_ATT_GEO = 7,      //!< attenuation due to geometrical spreading
	CNOSSOS_P2P_RESULT_ATT_ATM = 8,      //!< attenuation due to atmospheric absorption
	CNOSSOS_P2P_RESULT_ATT_REF = 9,      //!< attenuation due to reflections by vertical obstacles
	CNOSSOS_P2P_RESULT_ATT_DIF = 10,     //!< attenuation due to lateral diffraction by vertical edges
	CNOSSOS_P2P_RESULT_ATT_SIZE = 11     //!< attenuation due to finite size of vertical obstacles
};
/**
 * \brief Get material pointer
 * \param id Unique textual identifier for the material
 * \param create_if_needed If a material with the given name does not exist and this flag is set to true,
 * the library will create a new material; otherwise this function returns a NULL pointer.
 * \return An opaque pointer to the material or NULL if no material with the given name exists and the
 * \ref create_if_needed flag is set to false.
 */
_CNOSSOS_DLL_DECL_ CNOSSOS_P2P_MATERIAL* CNOSSOS_P2P_GetMaterial (const char* id, bool create_if_needed = false) ;
/**
 * \brief Set the flow resistivity for a given material
 * \param mat An opaque pointer returned by a call to \ref CNOSSOS_P2P_GetMaterial
 * \param sigma New value of the flow resistivity (in kPa.s/m²)
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_SetSigma  (CNOSSOS_P2P_MATERIAL* mat, double const& sigma) ;
/**
 * \brief Get the flow resistivity for a given material
 * \param mat An opaque pointer returned by a call to \ref CNOSSOS_P2P_GetMaterial
 * \param sigma Value of the flow resistivity (in kPa.s/m²)
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_GetSigma  (CNOSSOS_P2P_MATERIAL* mat, double & sigma) ;
/**
 * \brief Set the ground factor for a given material
 * \param mat An opaque pointer returned by a call to \ref CNOSSOS_P2P_GetMaterial
 * \param G New value of the ground factor
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_SetGValue (CNOSSOS_P2P_MATERIAL* mat, double const& G) ;
/**
 * \brief Get the ground factor for a given material
 * \param mat An opaque pointer returned by a call to \ref CNOSSOS_P2P_GetMaterial
 * \param G New value of the ground factor
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_GetGValue (CNOSSOS_P2P_MATERIAL* mat, double & G) ;
/**
 * \brief Set the spectral absorption factor for a given material
 * \param mat An opaque pointer returned by a call to \ref CNOSSOS_P2P_GetMaterial
 * \param alpha Array containing the values of the absorption factor spectrum 
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_SetAlpha  (CNOSSOS_P2P_MATERIAL* mat, double const* alpha) ;
/**
 * \brief Get the spectral absorption factor for a given material
 * \param mat An opaque pointer returned by a call to \ref CNOSSOS_P2P_GetMaterial
 * \param alpha Array containing the values of the absorption factor spectrum 
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_GetAlpha  (CNOSSOS_P2P_MATERIAL* mat, double * alpha) ;
/**
 * \brief Create a new calculation engine
 * \param name If specified, initializes the engine for the specified calculation method
 * \see CNOSSOS_P2P_SelectMethod
 * \return An opaque pointer to the newly created calculation engine
 */
_CNOSSOS_DLL_DECL_ CNOSSOS_P2P_ENGINE* CNOSSOS_P2P_CreateEngine (const char* name = 0) ;
/**
 * \brief Delete a calculation engine created by a call to \ref CNOSSOS_P2P_CreateEngine
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_DeleteEngine (CNOSSOS_P2P_ENGINE* p2p) ;
/**
 * \brief select the point to point method 
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine
 * \param name Textual identifier of the calculation method. Valid methods include 
 * "ISO-9613-2", "JRC-DRAFT-2010" and "JRC-2012"
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_SelectMethod (CNOSSOS_P2P_ENGINE* p2p, const char* name) ; 
/**
 * \brief Get the name of the currently selected propagation path calculator
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine
 * \return Pointer to a string containing the name of the method
 */
_CNOSSOS_DLL_DECL_ const char* CNOSSOS_P2P_GetMethod (CNOSSOS_P2P_ENGINE* p2p) ; 
/**
 * \brief Get the version of the currently selected propagation path calculator
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine
 * \return Pointer to a string containing the current version of the method
 */
_CNOSSOS_DLL_DECL_ const char* CNOSSOS_P2P_GetVersion (CNOSSOS_P2P_ENGINE* p2p) ; 
/**
 * \brief Clear the propagation path data 
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine
 * \return true if the call succeeded, false otherwise
 * \note A propagation path is created by successive calls to \ref CNOSSOS_P2P_AddToPath. 
 * A call to \ref CNOSSOS_P2P_GetResult will close the path and trigger the acoustical
 * calculations. In order to restart the calculation for a different path, applications 
 * must call the \ref CNOSSOS_P2P_ClearPath function before constructing a new one.
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_ClearPath (CNOSSOS_P2P_ENGINE* p2p) ; 
/**
 * \brief Add a new control point to the propagation path
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine
 * \param pos Position of the control point 
 * \param mat Transparent pointer to a material as returned by a previous call to \ref
 * CNOSSOS_P2P_GetMaterial
 * \param ext Transparent pointer to a vertical extension record as returned by a previous call to
 * \ref CNOSSOS_P2P_CreatePointSource,
 * \ref CNOSSOS_P2P_CreateLineSource,
 * \ref CNOSSOS_P2P_CreateReceiver,
 * \ref CNOSSOS_P2P_CreateBarrier,
 * \ref CNOSSOS_P2P_CreateVerticalWall or
 * \ref CNOSSOS_P2P_CreateVerticalEdge.
 * For intermediate boundary positions without vertical extensions, this argument is set to NULL.
 * \return The number of positions in the propagation path after appending the specified control point
 * \note A control point is part of the boundary separating the solid 2.5D model from the air above. 
 * Control points can be located on the ground or on top of man-made obstacles. Altitude of control
 * points is measured against a geo-spatial reference coordinate system. Vertical extensions encode
 * information related to vertical objects extending above the control point's position. The height of
 * a vertical extension is measured relative to the altitude of the control point.
 * \note A valid path should have exactly one source and one receiver vertical extension at its extreme 
 * points. Paths may start at the source and end with a receiver or vice-versa.
 */
_CNOSSOS_DLL_DECL_ unsigned int CNOSSOS_P2P_AddToPath (
	CNOSSOS_P2P_ENGINE* p2p, 
	CNOSSOS_P2P_POSITION const& pos,
	CNOSSOS_P2P_MATERIAL* mat,
	CNOSSOS_P2P_EXTENSION* ext = 0) ;
/**
 * \brief Create a vertical extension representing a point source
 * \param h height of the point source above the boundary profile
 * \return A transparent pointer to a vertical extension record
 */
_CNOSSOS_DLL_DECL_ CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreatePointSource (double const& h) ;
/**
 * \brief Create a vertical extension representing a receiver point
 * \param h height of the receiver above the boundary profile
 * \return A transparent pointer to a vertical extension record
 */
_CNOSSOS_DLL_DECL_ CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateReceiver (double const& h) ;
/**
 * \brief Create a vertical extension for a thin barrier crossed by the propagation plane
 * \param h height of the barrier above the boundary profile
 * \param mat Pointer to the material covering the barrier, as returned by a call to \ref CNOSSOS_P2P_GetMaterial
 * \return A transparent pointer to a vertical extension record
 */
_CNOSSOS_DLL_DECL_ CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateBarrier (double const& h, CNOSSOS_P2P_MATERIAL* mat = 0) ;
/**
 * \brief Create an extension for a vertical wall acting as a reflector for the propagation path
 * \param h height of the reflecting wall above the boundary profile
 * \param mat Pointer to the material covering the reflecting surface, as returned by a call to \ref CNOSSOS_P2P_GetMaterial
 * \return A transparent pointer to a vertical extension record
 */
_CNOSSOS_DLL_DECL_ CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateVerticalWall (double const& h, CNOSSOS_P2P_MATERIAL* mat = 0) ;
/**
 * \brief Create an extension for a vertical edge causing lateral diffraction of the propagation path
 * \param h height of the diffracting edge above the boundary profile
 * \return A transparent pointer to a vertical extension record
 */
_CNOSSOS_DLL_DECL_ CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateVerticalEdge (double const& h) ;
/**
 * \brief Create a vertical extension representing a line source segment
 * \param h height of the source above the boundary profile
 * \param segment end-points of the source line segment as projected on the ground
 * \param fixedAngle explicit angle of view of the segment as seen from the receiver. 
 * \return A transparent pointer to a vertical extension record
 * \note Setting a fixed angle is necessary in case (inverse) ray-tracing is used for constructing the propagation 
 * path as each ray is considered representative of an implicit sector of propagation with fixed opening angle and 
 * having the propagation path as its bisector. If the fixed angle argument is set to zero, the calculation will 
 * explicitly calculate the angle of view of the segment as seen from the source, which is typically the case when 
 * using beam-tracing or image-source techniques for constructing the propagation paths.
 */
_CNOSSOS_DLL_DECL_ CNOSSOS_P2P_EXTENSION* CNOSSOS_P2P_CreateLineSource (
	double const& h, 
	CNOSSOS_P2P_POSITION const segment[2], 
	double const& fixedAngle = 0.0) ;
/**
 * \brief Set the sound power associated with the source.
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \param Lw  Pointer to an array containing the sound power spectrum.
 * \param spectrumWeighting Weighting of the sound power spectrum
 * \param measurementType Measurement conditions for the sound power spectrum
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_SetSoundPower (
	CNOSSOS_P2P_ENGINE* p2p, 
	double const* Lw,
	CNOSSOS_P2P_SPECTRUM_WEIGHTING spectrumWeighting = CNOSSOS_P2P_SPECTRUM_LIN,
	CNOSSOS_P2P_LW_MEASUREMENT_TYPE measurementType = CNOSSOS_P2P_LW_UNDEFINED) ;
/**
 * \brief Get the acoustical results for the current propagation path
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \param index Select the type of results to be returned
 * \param result Array in which to return the acoustical results. If a NULL pointer is specified, 
 * the function returns the number of values that would have been written on output but does not 
 * transfer any actual values.
 * \return The number of values written (or not) to the array of results or zero in case an error
 * occurred in the calculation.
 * \note It is up to the caller to pass in an array of sufficient size. Applications can determine
 * the minimal size of this array by first calling the function with a NULL argument. 
 */
_CNOSSOS_DLL_DECL_ unsigned int CNOSSOS_P2P_GetResult (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_RESULT index, double * result) ;
/**
 * \brief Get a description of the last error that occurred in the acoustical calculation.
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \return Pointer to a string containing the error message.
 */
_CNOSSOS_DLL_DECL_ const char* CNOSSOS_P2P_GetErrorMessage (CNOSSOS_P2P_ENGINE* p2p) ; 
/**
 * \brief Print the current propagation path to the application's default output device.
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_PrintPathData (CNOSSOS_P2P_ENGINE* p2p) ;
/**
 * \brief Print an overview of the acoustical results to the application's output device
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_PrintPathResults (CNOSSOS_P2P_ENGINE* p2p) ;
/**
 * \brief Get the current settings of the calculation options
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \param options Data structure used to return the current settings.
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_GetOptions (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_OPTIONS & options) ;
/**
 * \brief Set the current settings of the calculation options
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \param options Data structure containing the new values of the options.
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_SetOptions (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_OPTIONS const& options) ;
/**
 * \brief Set the meteorological condition parameters
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \param meteo Data structure containing the new values of the meteorological parameters.
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_SetMeteo (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_METEO const& meteo) ;
/**
 * \brief Get the meteorological condition parameters
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \param meteo Data structure used to return the current settings of the meteorological parameters
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_GetMeteo (CNOSSOS_P2P_ENGINE* p2p, CNOSSOS_P2P_METEO & meteo) ;
/**
 * \brief Process a XML input file and optionally produce an XML output file
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \param inputFile Name of an XML input file
 * \param outputFile Name of an (optional) XML output file
 * \return true if the call succeeded, false otherwise
 * \note The name of the input file can include a full path specification or be a path name 
 * relative to the current working directory. The name of the output file can be either a 
 * full path specification or a path name relative to the location of the input file. Even if no 
 * output file is generated, the application can use the \ref CNOSSOS_P2P_GetResult function 
 * to access the acoustical results associated with the path defined in the input file.
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_ProcessPathFile (
	CNOSSOS_P2P_ENGINE* p2p, 
	const char* inputFile,
	const char* outputFile = 0) ;
/**
 * \brief Retrieve performance counters 
 * \param p2p An opaque pointer returned by a previous call to CNOSSOS_P2P_CreateEngine.
 * \param nbCalls Number of paths processed by this calculation engine
 * \param cpuTime Total elapsed time passed inside the acoustical path calculations (in seconds)  
 * \return true if the call succeeded, false otherwise
 */
_CNOSSOS_DLL_DECL_ bool CNOSSOS_P2P_GetPerformanceCounters (
	CNOSSOS_P2P_ENGINE* p2p, 
	unsigned int & nbCalls,
	double & cpuTime) ;
/**
 * \brief Enables or disables IEEE representation of +/-infinity.
 * If disabled, the library will use an arbitrary large value to represent infinity
 */
_CNOSSOS_DLL_DECL_ void CNOSSOS_P2P_SetInfinityMode (bool on_off) ;

#endif