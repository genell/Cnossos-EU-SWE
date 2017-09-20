/* 
 * ------------------------------------------------------------------------------------------------
 * file:		ReferenceObject.cpp
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.CSTB.txt
 * description: implementation of intrusive smart pointers and automatic memory management
 * changes:
 *
 *	28/11/2013	this header and licensing conditions added
 * ------------------------------------------------------------------------------------------------- 
 */
#include "ReferenceObject.h"
/*
 * implementation of atomic operations depends on the operating system...
 * 
 * note that we prefer to put these very simple functions in a separate file instead of making them 
 * inline so that we can isolate the rest of the application from the (in)famous windows header file.
 *
 * note that this may be fixed in a future version of the C++ standard which will include generic 
 * support for atomic data types.
 */
#include <windows.h>

namespace System
{
	long atomic_increment (volatile long* x) { return ::InterlockedIncrement (x) ; }
	long atomic_decrement (volatile long* x) { return ::InterlockedDecrement (x) ; }
}

