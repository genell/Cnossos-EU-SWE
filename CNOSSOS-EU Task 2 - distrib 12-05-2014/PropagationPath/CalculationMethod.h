#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		CalculationMethod.h
 * version:		1.001
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: abstract base class for the propagation path noise calculations
 * changes:
 *
 *	18/10/2013	initial version 1.001
 *
 *  23/10/2013	decomposition of calculation in successive steps ; some steps are common to
 *				all methods, some are specific. Because all calculation steps are declared as
 *				virtual functions, each calculation method can override the default implementation 
 *				if required and described in the reference documents
 *
 *	24/10/2013	implemented atmospheric absorption and absorption due to reflections
 *
 *  24/10/2013  implemented source modeling, including sound power conversions and dB(A) weighting
 *
 *	24/10/2013	implemented different meteorological models (using C0 or pFav). Note that each
 *				propagation model can use either of these meteorological models and therefore
 *				the model is implemented in the common base class.
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "Spectrum.h"
#include "PropagationPath.h"
#include "PathResult.h"
#include "VerticalExt.h"
#include "ReferenceObject.h"

namespace CnossosEU
{
	/*
	 * abstract base class for calculation methods
	 */
	class CalculationMethod : public System::ReferenceObject
	{
	public:
		/*
		 * constructor
		 */
		CalculationMethod (void) : System::ReferenceObject()
		{
			nbCalls = 0 ;
			totalCPUTime = 0 ;
		}
		/*
		 * abstract base classes have virtual destructors
		 */
		virtual ~CalculationMethod (void) { }
		/*
		 * public interface defined by means of pure virtual functions
		 */
		virtual const char* name (void) = 0 ;
		virtual const char* version (void) = 0 ;
		/*
		 * set calculation options
		 */
		virtual void setOptions (PropagationPathOptions const& _options)
		{
			options = _options ;
		}
		/*
		 * get actual calculation options
		 */
		PropagationPathOptions const& getOptions (void) 
		{
			return options ;
		}
		/*
		 * calculate noise levels associated with propagation path
		 */
		virtual bool doCalculation (PropagationPath& path, PathResult& result) ;
		/*
		 * get performance counters
		 */
		void getPerformanceCounter (unsigned int& _nbCalls, double& _cpuTime)
		{
			_nbCalls = nbCalls ;
			_cpuTime = totalCPUTime ;
		}

		static SourceExt*   getSource (PropagationPath& path, unsigned int* pos = 0) ;
		static ReceiverExt* getReceiver (PropagationPath& path, unsigned int* pos = 0) ;

	protected:
		
		PropagationPathOptions options ;

		virtual bool	initCalculation (void) { return true ; } 
		virtual bool	exitCalculation (void) { return true ; } 

		virtual MeasurementType expectedMeasurementType (void) { return MeasurementType::Undefined ; }
		virtual MeteoCondition::MeteoModel getDefaultMeteoModel (void) { return MeteoCondition::DEFAULT ; }

		static double     getPropagationDistance (PropagationPath& path) ;
		static Spectrum   getAbsorption (Material* mat) ;
		virtual Spectrum  getSoundPower (PropagationPath& path) ;
		virtual Spectrum  getFrequencyWeighting (PropagationPath& path) ;
		virtual Spectrum  getSoundPowerAdaptation (PropagationPath& path) ;
		virtual Spectrum  getAirAbsorption (PropagationPath& path) ;
		virtual double	  getGeometricalSpread (PropagationPath& path) ;
		virtual Spectrum  getAbsorption (PropagationPath& path) ;
		virtual Spectrum  getFiniteSizeCorrection (PropagationPath& path) ;
		virtual Spectrum  getLateralDiffraction (PropagationPath& path) ;
		virtual Spectrum  getExcessAttenuation (PropagationPath& path, bool favorable_condition) ;
		
		virtual Spectrum  getNoiseLevel (PathResult& result, bool favourable_condition) ;
		virtual Spectrum  getNoiseLevel (PropagationPath& path, PathResult& result) ;
		virtual double	  getNoiseLevel (Spectrum& LpSpectrum) ;

		friend class CnossosEU::LineSegment ;
		friend class CnossosEU::LineSource ;
		friend class CnossosEU::PointSource ;
		friend class CnossosEU::AreaSource ;

		virtual  double	getGeometricalSpreadUnitSource (PropagationPath& path, double units) ;
		virtual  double	getGeometricalSpreadLineSource (PropagationPath& path, 
														Geometry::Point3D& p1,
														Geometry::Point3D& p2,	
														double fixed_angle) ;

	private:
		/*
		 * copy-construct and assignment operator are not implemented
		 */
		CalculationMethod (CalculationMethod const& other) ;
		CalculationMethod& operator= (CalculationMethod const&) ;	
		/*
		 * keep track of timings
		 */
		unsigned int nbCalls ;
		double  totalCPUTime ;
	};
	/*
	 * instantiate the appropriate calculation method
	 */
	CalculationMethod* getCalculationMethod (const char* id) ;
}