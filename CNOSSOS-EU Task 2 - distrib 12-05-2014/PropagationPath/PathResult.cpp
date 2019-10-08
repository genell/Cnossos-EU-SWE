/* 
 * ------------------------------------------------------------------------------------------------
 * file:		PathResult.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: implements global functions defined in PathResult.h
 * changes:
 *
 *	23/10/2013	initial version
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "PathResult.h"
#include "Spectrum.h"
#include <stdio.h>
#ifdef __GNUC__
#define _finite finite
#endif
using namespace CnossosEU ;

static void print_value (double x)
{
	if (_finite(x))
		printf (" | %5.1f", x) ;
	else
		printf (" |  -inf") ;

}
static void print_spec (const char* name, Spectrum const& spec)
{
	printf ("%-8.8s", name) ;
	for (unsigned int i = 0 ; i < spec.size() ; ++i) 
	{
		print_value (spec.data(i)) ;
	}
	printf ("\n") ;
}

static void print_freq (void)
{
	Spectrum any ;
	printf ("%-8.8s", "Freq(Hz)") ;
	for (unsigned int i = 0 ; i < any.size() ; ++i) printf (" | %5.0f", any.freq(i)) ;
	printf ("\n") ;
}

static void output_spectrum_to_XML (FILE* fp, unsigned int level, const char* tag, Spectrum const& spec)
{
	for (unsigned int i = 0 ; i < level ; ++i) fprintf (fp,"  ") ;
	fprintf (fp, "<%s>\n", tag) ;
	for (unsigned int i = 0 ; i < level ; ++i) fprintf (fp,"  ") ;
	for (unsigned int i = 0 ; i < spec.size() ; ++i) fprintf (fp, " %7.1f", spec.data(i)) ;
	fprintf (fp, "\n") ;
	for (unsigned int i = 0 ; i < level ; ++i) fprintf (fp,"  ") ;
	fprintf (fp, "</%s>\n", tag) ;
}

namespace CnossosEU
{
	void print_results_to_stdout (PathResult& res)
	{
		printf ("------------------------------------------------------------------------\n") ;
		print_freq() ;
		printf ("------------------------------------------------------------------------\n") ;
		print_spec ("Lw",  res.Lw) ;
		print_spec ("dB(A)", res.dBA)  ;
		print_spec ("deltaLw",  res.delta_Lw)  ;
		print_spec ("AttGeo", Spectrum(res.AttGeo)) ;
		print_spec ("AttAtm", res.AttAir) ;
		print_spec ("AttRef", res.AttAbsMat) ;
		print_spec ("AttDif", res.AttLatDif) ;
		print_spec ("AttSize", res.AttSize) ;
		print_spec ("Att,F", res.AttF) ;
		print_spec ("Att,H", res.AttH) ;
		print_spec ("Lp,F",  res.LpF) ;
		print_spec ("Lp,H",  res.LpH) ;
		print_spec ("Leq",   res.Leq) ;
		printf ("------------------------------------------------------------------------\n") ;
		printf ("Lp,F    ") ; print_value (res.LpF_dBA) ; printf (" dB(A)\n") ;
		printf ("Lp,H    ") ; print_value (res.LpH_dBA) ; printf (" dB(A)\n") ;
		printf ("Leq     ") ; print_value (res.Leq_dBA) ; printf (" dB(A)\n") ;
		printf ("------------------------------------------------------------------------\n") ;
	}

	bool output_results_to_XML (const char* filename, PathResult& result)
	{
		FILE* fp = fopen (filename, "wt") ;
		if (!fp) return false ;

		fprintf (fp, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n") ;
		fprintf (fp, "<CNOSSOS-EU>\n") ;
		fprintf (fp, "  <pathResult>\n") ;
		fprintf (fp, "    <LeqA> %.1f </LeqA>\n", result.Leq_dBA) ;
		output_spectrum_to_XML (fp, 2, "Leq", result.Leq) ;
		fprintf (fp, "    <details>\n") ;
		output_spectrum_to_XML (fp, 3, "Lw", result.Lw) ;
		output_spectrum_to_XML (fp, 3, "dBA", result.dBA) ;
		output_spectrum_to_XML (fp, 3, "deltaLw", result.delta_Lw) ;
		output_spectrum_to_XML (fp, 3, "attGeo", Spectrum(result.AttGeo)) ;
		output_spectrum_to_XML (fp, 3, "attAir", result.AttAir) ;
		output_spectrum_to_XML (fp, 3, "attRef", result.AttAbsMat) ;
		output_spectrum_to_XML (fp, 3, "attDif", result.AttLatDif) ;
		output_spectrum_to_XML (fp, 3, "attSize", result.AttSize) ;
		output_spectrum_to_XML (fp, 3, "attF", result.AttF) ;
		output_spectrum_to_XML (fp, 3, "attH", result.AttH) ;
		output_spectrum_to_XML (fp, 3, "LpF", result.LpF) ;
		output_spectrum_to_XML (fp, 3, "LpH", result.LpH) ;
		fprintf (fp, "      <LpFA> %.1f </LpFA>\n", result.LpF_dBA) ;
		fprintf (fp, "      <LpHA> %.1f </LpHA>\n", result.LpH_dBA) ;
		fprintf (fp, "    </details>\n") ;
		fprintf (fp, "  </pathResult>\n") ;
		fprintf (fp, "</CNOSSOS-EU>\n") ;
	
		fclose (fp) ;
		return true ;
	}
}
