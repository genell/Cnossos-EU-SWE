/* 
 * ------------------------------------------------------------------------------------------------
 * file:		TestCNOSSOS.cpp
 * version:		1.001
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: main program for testing the CNOSSOS-EU propagation methods
 * changes:
 *
 *	18/10/2013	initial version 1.001
 *
 *	05/11/2013	added command line options and display of usage message
 *
 *  15/11/2013	error handling unified, all errors are now handled over to the catch handler
 *
 *  20/11/2013	added support for changing/restoring the current working directory. All subsequent 
 *			    file names are therefore relative to  to the folder containing the input file. 
 *
 *  25/11/2013  added functionality for writing results to output file, including automatic
 *              generation of output file name
 *
 *	25/11/2013	added comments
 *
 *  25/11/2013	version 1.002 ready for distribution
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#ifndef __GNUC__
#include "stdafx.h"
#endif
#ifdef WIN32
#include <conio.h>
#include <direct.h>
#endif
#include "PathParseXML.h"
#include "PathResult.h"
#include "Material.h"
#ifdef __GNUC__
#ifndef WIN32
#include <curses.h>
#define _getcwd getcwd 
#define _strdup strdup
#define _access access
#define _chdir chdir
#define _getch getch
#endif
#endif

using namespace CnossosEU ;
using namespace System ;

static const char* usage =
"\n"
"Usage:\n"
"\n"
"  TestCnossos [-w] [-m=<method>] [-i=<input file>] [-c] [-o] [-o=<output file>]\n"
"\n" 
"  .if -w is specified, the program will halt and wait for some user input \n"
"   before exiting, otherwise exit is automatic at the end of the calculations \n"
"\n"
"  .if -c is specified, the program will copy the results to the clipboard in a\n"
"   tabular format ready for pasting in spreadsheet applications or text editors\n"
"\n"
"  .method = ISO-9613-2, JRC-draft-2010 or JRC-2012; if not specified, the \n"
"   calculation will use the default method as found in the input file.\n"
"\n"
"  .input file = the name of a valid XML file complying to the CNOSSOS-EU\n"
"   specifications.\n"
"\n"
"  .output file = the name of the output file. If the file already exists,\n"
"   if will be replaced. If no filename is specified, output will be sent to\n"
"   the standard output stream for the active process.\n"
"\n"
"   note that the output file will be searched and/or created relative to\n"
"   the folder containing the input file. If the file exists, it will be\n"
"   replaced or deleted (in case the program reports an error and no results\n"
"   can be produced).\n"
"\n"
"   if -o is specified without a file name, the program will automatically\n"
"   generate one, based on the name of the input file name.\n"
"\n"
;

static bool interactive_mode = false ;
static bool copy_to_clipboard = false ;

static char* getcwd (void)
{
	return _getcwd (NULL,0) ;
}

static char* getcwd (const char* fileName)
{
	char* new_cwd = _strdup (fileName) ;
	for (int i = strlen(new_cwd)-1 ; i > 0 ; --i)
	{
		if (new_cwd[i] == '/' || new_cwd[i] == '\\') 
		{
			new_cwd[i] = 0 ; 
			break ;
		}
	}
	return new_cwd ;
}

#ifdef _SHOW_LIBRARIES_
extern void show_libraries(unsigned int processID = -1) ;
#else
static void show_libraries(unsigned int processID = -1) { }
#endif

#ifdef WIN32
extern bool CopyToClipboard (CnossosEU::PathResult& result) ;
#endif

#ifdef __GNUC__
int  main (int argc, char* argv[])
#else
int _tmain (int argc, _TCHAR* argv[])
#endif
{
	//show_libraries () ;

	char* inputFile = 0 ;
	char* outputFile = 0 ;
	char* old_dir = 0 ;
	char* new_dir = 0 ;
	ref_ptr<CalculationMethod> method = 0 ;

	XMLFileLoader xmlFile ;
	PropagationPath path ;
	PropagationPathOptions options ;
	PathResult result ;

	/*
	 * parse command line options
	 */
	if (argc == 1)
	{
		/*
		 * no options specified: print usage message
		 */
		printf ("%s", usage) ;
		exit(0) ;
	}
	else
	{
		for (int i = 1 ; i < argc ; ++i)
		{
			/*
			 * option "-i=" : specify name of input file
			 */
			if (strncmp (argv[i],"-i=", 3) == 0)
			{
				inputFile = argv[i] + 3 ;
			}
			/*
			 * option "-m=" : specify calculation method
			 */
			else if (strncmp (argv[i], "-m=",3) == 0)
			{
				method = getCalculationMethod (argv[i]+3) ;
				if (method == 0)
				{
					printf ("WARNING: invalid method specified on command line, using input file instead\n") ;
				}
			}
			/*
			 * option "-w=" : specify interactive mode, i.e. wait for user confirmation before exiting the program
			 */
			else if (strcmp (argv[i], "-w") == 0)
			{
				interactive_mode = true ;
			}
			/*
			 * option "-o=" : specify name of output file
			 */
			else if (strncmp (argv[i],"-o=", 3) == 0)
			{
				outputFile = argv[i] + 3 ;
			}
			/*
			 * option "-o" : automatically generate the name of the output file
			 */
			else if (strncmp (argv[i],"-o", 2) == 0)
			{
				outputFile = (char*) "" ;
			}
			/*
			 * option "-c" : copy result to clipboard
			 */
			else if (strncmp (argv[i],"-c", 2) == 0)
			{
				copy_to_clipboard = true ;
			}
		}
	}
	/*
	 * automatically generate the name of the output file, using the name of
	 * the input file but adding the ".out.xml" file extension.
	 */
	if (outputFile != 0)
	{
		if (strlen(outputFile) == 0)
		{
			outputFile = (char*) malloc (strlen(inputFile)+10) ;
			strcpy (outputFile, inputFile) ;
			unsigned int pos = strlen(outputFile) ;
			while (pos > 0) 
			{
				if (outputFile[--pos] == '.') break ;
			}
			strcpy (outputFile+pos, ".out.xml") ;
			print_debug ("Generated output file name = %s \n", outputFile) ;
		}
	}
	/*
	 * try reading and processing the input file
	 */
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
		 * change the current working directory so that all other filenames
		 * will be considered relative to the folder containing the input file
		 */
		old_dir = getcwd() ;
		new_dir = getcwd(inputFile) ;
		_chdir (new_dir) ;
		print_debug ("Changing current working directory to %s \n", _getcwd(0,0))	;
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
		 * print out information about the actual propagation method
		 */
		if (method == 0) method = options.method ;
		printf ("Calculate path \n") ;
		printf (".Method:  %s \n", method->name()) ;
		printf (".Version: %s \n", method->version());
		/*
		 * set calculation options and process the propagation path
		 */
#ifdef _TEST_CPU_TIME_
		for (int i = 0 ; i < 1000 ; ++i)
		{
			method->setOptions (options) ;
			method->doCalculation (path, result) ;
		}
#else
		method->setOptions (options) ;
		method->doCalculation (path, result) ;
#endif
		/*
		 * print out information about the actual meteorological data used in the evaluation
		 * of the long-time averaged noise level. Note that this information is available on
		 * output of the calculation (because each calculation method may select the most
		 * appropriate meteorological model).
		 */
		options = method->getOptions() ;
		if (options.meteo.model == MeteoCondition::ISO9613)
		{
			printf (".Meteo:   ISO-9613-2, C0=%.1f dB\n", options.meteo.C0) ;
		}
		else 
		{
			printf (".Meteo:   JRC-2012, pFav=%.1f%% \n", 100 * options.meteo.pFav) ;
		}
		/*
		 * print out the outcome of the geometrical analysis of the propagation path
		 */
		printf ("Unfolded propagation path\n") ;
		path.print_unfolded_path() ;

#ifdef _DEBUG
		RayPath ray = path.get_ray_path (false) ;
		printf ("Fermat path\n") ;
		for (int unsigned i = 0 ; i < ray.size() ; ++i)
		{
			printf ("X=%.1f Y=%.1f Z=%.2f \n", ray[i].x, ray[i].y, ray[i].z) ;
		}
#endif
		/*
		 * print out results and details to the program's standard output (usually the console 
		 * window or terminal from which the end-user typed the command line)
		 */
		unsigned int nb_calls ;
		double cpu_time ;
		method->getPerformanceCounter (nb_calls, cpu_time) ;
		printf (".%d calls to P2P calculation in %.6fms \n", nb_calls, cpu_time*1000.) ;
		printf ("Noise levels\n") ;
		print_results_to_stdout (result) ;
		/*
		 * write results to output file
		 */
		if (outputFile != 0)
		{
			printf ("Creating output file %s \n", outputFile) ;
			output_results_to_XML (outputFile, result) ;
		}
		/*
		 * copy results to clipboard
		 */
		if (copy_to_clipboard)
		{
#ifdef WIN32			
			bool ok = CopyToClipboard (result) ;
			printf ("Copy results to clipboard : %s \n", ok ? "OK" : "failed") ;
#else
			printf ("Clipboard output only supported on Windows builds\n") ;
#endif
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
		/*
		 * delete output file (if specified and existing)
		 */
		if (outputFile != 0 && _access (outputFile,0) == 0)
		{
			remove (outputFile) ;
		}
		printf ("No results available...\n") ;
		printf ("No output file generated\n") ;
	}
	/*
	 * reset current working directory to initial value
	 */
	if (old_dir) 
	{
		_chdir(old_dir) ;
		print_debug ("Reset current working directory to %s \n", old_dir)	;
		free (old_dir) ;
		free (new_dir) ;
	}
	
	/*
	 * in interactive mode, wait for user confirmation before exiting the program
	 */
	if (interactive_mode)
	{
		printf ("Type any key to quit this program \n") ;
		_getch() ;
	}

	return 0;
}

