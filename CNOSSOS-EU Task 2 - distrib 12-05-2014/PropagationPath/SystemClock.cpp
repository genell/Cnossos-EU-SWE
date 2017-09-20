/* 
 * ------------------------------------------------------------------------------------------------
 * file:		SystemClock.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.CSTB.txt
 * description: monitor calculation times using high-resolution system clock
 * changes:
 *
 *	18/10/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include "SystemClock.h"
/* 
 * calls to the high-performance clock are specific to the Windows operating system
 * replace as appropriate for other operating systems...
 */
#include <windows.h>

SystemClock::COUNTER SystemClock::counter (void)
{
	LARGE_INTEGER counter ;
	QueryPerformanceCounter (&counter) ;
	return counter.QuadPart ;
}
SystemClock::COUNTER SystemClock::units_per_sec (void)
{
	LARGE_INTEGER counter ;
	QueryPerformanceFrequency (&counter) ;
	return counter.QuadPart ;
}
