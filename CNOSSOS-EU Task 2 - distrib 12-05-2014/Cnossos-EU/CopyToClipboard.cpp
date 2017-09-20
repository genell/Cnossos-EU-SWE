#include <windows.h>
#include <string>
#include "ErrorMessage.h"
#include "PathResult.h"

using namespace CnossosEU ;
/*
 * copy text to clipboard
 */
bool CopyToClipboard (std::string text)
{
	BOOL ok = OpenClipboard (NULL) ;
	if (!ok) return false ;

	const char* ctext = text.c_str() ;
	unsigned int nb_bytes = strlen(ctext) + 1 ;
	
	HANDLE hMem = GlobalAlloc (GMEM_MOVEABLE, nb_bytes) ;
	if (!hMem) return false ;

	char* mtext = (char*) GlobalLock (hMem) ;
	if (!mtext) return false ;

	memcpy (mtext, ctext, nb_bytes) ;
	GlobalUnlock (hMem) ;

	EmptyClipboard () ;

	HANDLE hClip = SetClipboardData (CF_TEXT, hMem) ;
	if (!hClip) return false ;

	CloseClipboard () ;

	print_debug ("%d bytes written to the clipboard \n", nb_bytes) ;

	return true ;
}
/*
 * format spectral values in tabular format
 */
static void append (std::string& text, const char*name, Spectrum const& spec)
{
	text += name ;
	for (unsigned int i = 0 ; i < spec.size() ; ++i)
	{
		char buffer[80] ;
		if (_finite(spec.data(i)))
			sprintf (buffer, "\t%.1f", spec.data(i)) ;
		else
			sprintf (buffer, "\t%s", "=1/0.") ;
		text += buffer ;
	}
	text += "\r\n" ;
}
/*
 * copy PathResult table to the clipboard
 */
bool CopyToClipboard (PathResult& res)
{
	std::string text ;

	Spectrum freq ;
	for (unsigned int i = 0 ; i < freq.size() ; ++i) freq[i] = freq.freq(i) ;
	append (text, "Freq",    freq) ;
	append (text, "Lw",      res.Lw) ;
	append (text, "dB(A)",   res.dBA)  ;
	append (text, "deltaLw", res.delta_Lw)  ;
	append (text, "AttGeo",  Spectrum(res.AttGeo)) ;
	append (text, "AttAtm",  res.AttAir) ;
	append (text, "AttRef",  res.AttAbsMat) ;
	append (text, "AttDif",  res.AttLatDif) ;
	append (text, "AttSize", res.AttSize) ;
	append (text, "Att,F",   res.AttF) ;
	append (text, "Att,H",   res.AttH) ;
	append (text, "Lp,F",    res.LpF) ;
	append (text, "Lp,H",    res.LpH) ;
	append (text, "Leq",     res.Leq) ;

	return CopyToClipboard (text) ;
}

