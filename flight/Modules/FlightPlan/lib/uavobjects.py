"""__NATIVE__
#include "openpilot.h"
#include "flightplanstatus.h"
"""

def FlightPlanStatusUpdate(val):
	"""__NATIVE__
	pPmObj_t pobj;
	FlightPlanStatusData status;
	
	pobj = NATIVE_GET_LOCAL(0);
	if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_INT )
		status.Debug = ((pPmInt_t) pobj)->val;
	else if ( OBJ_GET_TYPE(pobj) == OBJ_TYPE_FLT )
		status.Debug = ((pPmFloat_t) pobj)->val;
	 
	FlightPlanStatusSet(&status);
 
	return PM_RET_OK;
	"""
	pass
	
	

	
