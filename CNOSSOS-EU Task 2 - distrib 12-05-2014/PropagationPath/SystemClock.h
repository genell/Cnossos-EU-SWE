#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		SystemClock.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.CSTB.txt
 * description: monitor calculation times using high-resolution system clock
 * changes:
 *
 *	18/10/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include <stdint.h>

struct SystemClock 
{
	/* 
	 * large integer type for storing clock tick counts
	 */
	typedef uint64_t COUNTER ;
	/*
	 * implementation
	 */
	static COUNTER counter (void) ;
	static COUNTER units_per_sec (void) ;
	/*
	 * reset clock to zero
	 */
	void reset (void) { t_start = counter() ; }
	/*
	 * get elapsed time in seconds
	 */
	double get (bool reset_timer = false) 
	{ 
		double t = (double) (counter() - t_start) / (double) units_per_sec() ; 
		if (reset_timer) reset() ;
		return t ;
	}
	/*
	 * set counter to zero when created
	 */
	SystemClock (void) { reset() ; }

private:

	COUNTER t_start ;
};
