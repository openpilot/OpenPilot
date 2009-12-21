/* Gussy - 11/09 */

#include "integer.h"
#include "fattime.h"

DWORD get_fattime (void)
{
	DWORD res;
	
	/* For now proper timing is not implemented, the time below is the current time of writing, nothing special. */
	/* Is the need arises, proper RTC time should be implemented. */
	res =  (((DWORD)2009 - 1980) << 25)	//Year
			| ((DWORD)11 << 21)			//Month
			| ((DWORD)26 << 16)			//Day
			| (WORD)(03 << 11)			//Hour
			| (WORD)(46 << 5)			//Min
			| (WORD)(25 >> 1);			//Sec

	return res;
}
