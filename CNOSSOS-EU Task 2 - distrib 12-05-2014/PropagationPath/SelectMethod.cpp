/* 
 * ------------------------------------------------------------------------------------------------
 * file:		SelectMethod.cpp
 * version:		1.001
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: dynamically create a calculation method by name
 * note:		this function is defined in CalculationMethod.h but is implemented in a 
 *				separate file in order to avoid circular dependencies of header files
 * changes:
 *
 *	28/11/2013	initial version 1.001
 * ------------------------------------------------------------------------------------------------- 
 */
#include "JRC-draft-2010.h"
#include "JRC-2012.h"
#include "ISO-9613-2.h"
#include "CNOSSOS-2018.h"
#include "../system/environment.h"

using namespace CnossosEU ;

CalculationMethod* CnossosEU::getCalculationMethod (const char* id)
{
	if (id == 0) return NULL ;
	/*
	 * three methods are currently available
	 */
	if (_strcmpi (id, "CNOSSOS-2018") == 0) return new CNOSSOS_2018() ;
	if (_strcmpi (id, "ISO-9613-2") == 0) return new ISO_9613_2() ;
	if (_strcmpi (id, "JRC-2012") == 0) return new JRC2012() ;
	if (_strcmpi (id, "JRC-DRAFT-2010") == 0) return new JRCdraft2010() ;

	return NULL ;
}
