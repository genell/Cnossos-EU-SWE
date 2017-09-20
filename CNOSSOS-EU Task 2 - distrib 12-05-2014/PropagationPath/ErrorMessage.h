#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		ErrorMessage.h
 * version:		1.001
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: generic support for handling error conditions
 * changes:
 *
 *	18/10/2013  initial version 1.001
 * 
 * ------------------------------------------------------------------------------------------------- 
 */
#include <stdio.h>
#include <assert.h>
#include <exception>
#include <string>
/*
 * error handling: base class
 */
namespace CnossosEU
{
	class ErrorMessage : public std::exception
	{
		std::string msg ;
	public:
		ErrorMessage (const char* what) : std::exception(), msg (what) { }
		ErrorMessage (void) : std::exception(), msg() { }
		~ErrorMessage (void) throw() {} 
		virtual const char* what(void) const throw()
		{
			return msg.c_str() ;
		}
		virtual void print (void)
		{
			printf ("ERROR: %s \n", what()) ;
		}
	} ;	
}
/*
 * abstract interface for error handling mechanism
 */
#define signal_error(msg) throw(msg) 
/*
 * abstract interface for warning / trace mechanism
 */
#
#ifdef _DEBUG
#define print_debug printf
#else
inline void print_debug (...) { }
#endif

