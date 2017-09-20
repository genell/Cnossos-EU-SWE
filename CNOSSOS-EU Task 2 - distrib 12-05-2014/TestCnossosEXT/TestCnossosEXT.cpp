/* 
 * ------------------------------------------------------------------------------------------------
 * file:		TestCnossosEXT.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description:	test program for the user-defined ISOWithMeteo calculation method
 * changes:
 *
 *	18/01/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include "stdafx.h"
#include <conio.h>

#include "JRC-2012.h"
#include "JRC-draft-2010.h"
#include "ISO_WithMeteo.h"
#include "PathParseXML.h"

using namespace CnossosEU ;
using namespace System ;

#ifdef __GNUC__
int main(int argc, char* argv[])
#else
int _tmain(int argc, _TCHAR* argv[])
#endif
{
	const char* inputFile = "../data/flat ground - 200m.xml" ;
	XMLFileLoader xmlFile ;
	PropagationPath path ;
	PropagationPathOptions options ;
	PathResult result ;

	try
	{
		/*
		 * read the input file into memory
		 */
		printf ("Parse file: %s \n", inputFile) ;
		if (!xmlFile.ParseFile (inputFile))
		{
			signal_error (XMLSyntaxError (xmlFile)) ;
			return 0 ;
		}
		/*
		 * parse the XML-DOM structure into the application object/class schema
		 */
		ParsePathFromFile (xmlFile.GetRoot(), path, options) ;
		/*
		 * print out the original data
		 */
		printf ("Original data\n") ;
		path.print_input_data() ;
		/*
		 * loop over calculation methods
		 */
		for (int i = 0 ; i < 4 ; ++i)
		{
			/* 
			 * select one of the three predefined methods and compare with the
			 * modified (i.e. user defined) ISOWithMeteo calculation method
			 */
			ref_ptr<CalculationMethod> method ;
			switch (i)
			{
			case 0: method = new ISO_9613_2() ; break ;
			case 1: method = new JRC2012() ; break ;
			case 2: method = new JRCdraft2010() ; break ;
			case 3: method = new MyExtension::IsoWithMeteo() ; break ;
			}
			/*
			 * print out information about the actual propagation method
			 */
			printf ("Calculate path \n") ;
			printf (".Method:  %s \n", method->name()) ;
			printf (".Version: %s \n", method->version());
			/*
			 * force use of the NMPB meteorological model with 50% of favourable/homogeneous 
			 * propagation conditions
			 */
			options.meteo.model = MeteoCondition::JRC2012 ;
			options.meteo.pFav = 0.50 ;
			/*
			 * set calculation options and process the propagation path
			 */
			method->setOptions (options) ;
			method->doCalculation (path, result) ;
			/*
			 * print out the outcome of the geometrical analysis of the propagation path
			 */
			printf ("Unfolded propagation path\n") ;
			path.print_unfolded_path() ;

			/*
			 * print out results and details to the program's standard output
			 */
			printf ("Noise levels\n") ;
			print_results_to_stdout (result) ;
		}
	}
	/*
	 * handle any error that might have occurred while reading or processing the file
	 */
	catch (ErrorMessage& err)
	{
		/*
		 * print out diagnostic message
		 */
		err.print () ;
		printf ("No results available...\n") ;
	}
	printf ("Type any key to quit this program \n") ;
	_getch() ;

	return 0;
}

