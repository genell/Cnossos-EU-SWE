/* 
 * ------------------------------------------------------------------------------------------------
 * file:		TestCNnossosCPP.cpp
 * version:		1.001
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: demonstration program using C++ API defined in PropagationPath.lib
 * ------------------------------------------------------------------------------------------------- 
 */
#ifdef WIN32
#include "stdafx.h"
#endif
#include "PropagationPath.h"
#include "Material.h"
#include "VerticalExt.h"
#include "CalculationMethod.h"
#include "PathResult.h"
#ifdef __GNUC__
#ifndef WIN32
#include <curses.h>
#define _getch getch
#endif
#endif


using namespace CnossosEU ;
using namespace Geometry ;
/*
 * show calculation details
 */
static void show_details (PropagationPath& path, PathResult& result)
{
	path.print_input_data() ;
	print_results_to_stdout (result) ;
}
/*
 * create the source
 */
static SourceExt* getSourceModel (void)
{
	double LwValues[8] = { 80., 85., 90., 95., 100., 100., 95.0, 90.0 } ;
	/*
	 * create the elementary source
	 */
	ElementarySource Lw ;
	Lw.sourceHeight = 0.3 ;
	Lw.frequencyWeighting = FrequencyWeighting::dBLIN ;
	Lw.measurementType = MeasurementType::HemiSpherical ;
	Lw.spectrumType = SpectrumType::LineSource ;
	Lw.soundPower = Spectrum (LwValues) ;
	/*
	 * create the source extension adapter
	 */
	return new SourceExt (Lw) ;
}
/*
 * main
 */
#ifdef __GNUC__
int main(int argc, char* argv[])
#else
int _tmain(int argc, _TCHAR* argv[])
#endif
{
	try
	{
		/*
		 * create a user-defined material and specify properties
		 */
		Material* mat = getMaterial ("UserDefinedGround", true) ;
		mat->setG (0.6) ;
		mat->setSigma (600) ;
		/*
		 * construct the geometry of the boundary beneath the path
		 */
		PropagationPath path ;
		path.resize(4) ;
		path[0].pos = Point3D ( 0.0,  0.0, 0.0) ;
		path[1].pos = Point3D (10.0,  5.0, 0.0) ;
		path[2].pos = Point3D (16.0,  8.0, 2.0) ;
		path[3].pos = Point3D (50.0, 25.0, 2.0) ;
		/* 
		 * specify materials associated with the boundary
		 * note that materials are associated with the end points of the segments and 
		 * that specifying the material for the first position in the path is optional
		 */
		path[0].mat = path[1].mat = getMaterial ("F") ;
		path[2].mat = path[3].mat = getMaterial ("UserDefinedGround") ;
		/*
		 * create the elementary source 
		 */
		SourceExt* source = getSourceModel() ;
		/*
		 * add source geometry
		 */
		Point3D p1 (0.0, -2.5, 0.0) ;
		Point3D p2 (0.0,  2.5, 0.0) ;
		source->geo = new LineSegment (p1, p2) ;
		/*
		 * create the source at position 0
		 */
		path[0].ext = source ;
		/*
		 * create the receiver at position 3
		 */
		path[3].ext = new ReceiverExt (2.50) ;
		/*
		 * create the propagation module
		 */
		CalculationMethod* method = getCalculationMethod ("JRC-draft-2010") ;
		/*
		 * set calculation options
		 */
		PropagationPathOptions options ;
		options.meteo.C0 = 0 ;
		options.meteo.model = MeteoCondition::ISO9613 ;
		method->setOptions (options) ;
		/*
		 * run the calculation
		 */
		PathResult result ;
		method->doCalculation (path, result) ;
		/*
		 * print details (in debug mode only)
		 */
		show_details (path, result) ;
		/*
		 * initial noise level
		 */
		double Leq_no_barrier = result.Leq_dBA ;
		/*
		 * add a 2m high absorbing barrier at position 2, i.e. on top of the berm
		 *
		 * IMPORTANT NOTE: the propagation path structure is used both as input and output in 
		 * the noise calculation, therefore, it may contain things you didn't except after the 
		 * call to the doCalculation method.h, e.g. it is not certain that you will find the
		 * original data you wrote to the path in exactly the same locations... Therefore, here 
		 * it's done for the purpose of demonstration. Please don't do this in your end-user 
		 * software based on automatic construction of propagation paths.
		 */
		path[2].ext = new BarrierExt (2.0, getMaterial ("A4")) ;
		/*
		 * redo the calculation
		 */
		method->doCalculation (path, result) ;
		/*
		 * print details (in debug mode only)
		 */
		show_details (path, result) ;
		/*
		 * noise level with the barrier
		 */
		double Leq_with_barrier = result.Leq_dBA ;
		/*
		 * show effect of barrier
		 */
		printf ("Noise level without barrier = %.1f dB(A) \n", Leq_no_barrier) ;
		printf ("Noise level with    barrier = %.1f dB(A) \n", Leq_with_barrier) ;
	}
	catch (ErrorMessage& err)
	{
		err.print () ;
	}

	_getch() ;
	return 0;
}

