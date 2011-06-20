/*
	FreeRTOS.org V4.2.1 (WIN32 port) - Copyright (C) 2007-2008 Dushara Jayasinghe.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS.org is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS.org; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS.org, without being obliged to provide
	the source code for any proprietary components.  See the licensing section
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

	***************************************************************************
	See http://www.FreeRTOS.org for documentation, latest information, license
	and contact details.  Please ensure to read the configuration and relevant
	port sections of the online documentation.

	Also see http://www.SafeRTOS.com for an IEC 61508 compliant version along
	with commercial development and support options.
	***************************************************************************
*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <shlobj.h>

extern DWORD * pxCurrentTCB;

/******************************************************************************
	Defines
******************************************************************************/
#define NMI	(1<<CPU_INTR_SWI)
#define MAX_KEY_LENGTH 255


/*
 * Task control block.  A task control block (TCB) is allocated to each task,
 * and stores the context of the task.
 */
typedef struct tskTaskControlBlock
{
	volatile portSTACK_TYPE	*pxTopOfStack;		/*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE STRUCT. */

	#if ( portUSING_MPU_WRAPPERS == 1 )
		xMPU_SETTINGS xMPUSettings;				/*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE STRUCT. */
	#endif

	xListItem				xGenericListItem;	/*< List item used to place the TCB in ready and blocked queues. */
	xListItem				xEventListItem;		/*< List item used to place the TCB in event lists. */
	unsigned portBASE_TYPE	uxPriority;			/*< The priority of the task where 0 is the lowest priority. */
	portSTACK_TYPE			*pxStack;			/*< Points to the start of the stack. */
	signed char				pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */

	#if ( portSTACK_GROWTH > 0 )
		portSTACK_TYPE *pxEndOfStack;			/*< Used for stack overflow checking on architectures where the stack grows up from low memory. */
	#endif

	#if ( portCRITICAL_NESTING_IN_TCB == 1 )
		unsigned portBASE_TYPE uxCriticalNesting;
	#endif

	#if ( configUSE_TRACE_FACILITY == 1 )
		unsigned portBASE_TYPE	uxTCBNumber;	/*< This is used for tracing the scheduler and making debugging easier only. */
	#endif

	#if ( configUSE_MUTEXES == 1 )
		unsigned portBASE_TYPE uxBasePriority;	/*< The priority last assigned to the task - used by the priority inheritance mechanism. */
	#endif

	#if ( configUSE_APPLICATION_TASK_TAG == 1 )
		pdTASK_HOOK_CODE pxTaskTag;
	#endif

	#if ( configGENERATE_RUN_TIME_STATS == 1 )
		unsigned long ulRunTimeCounter;		/*< Used for calculating how much CPU time each task is utilising. */
	#endif

} tskTCB;

tskTCB *debug_task_handle;

//MinGW doesn't include MMCSS ...
//Bummer!

typedef enum _AVRT_PRIORITY
{
	AVRT_PRIORITY_LOW = -1,
	AVRT_PRIORITY_NORMAL,
	AVRT_PRIORITY_HIGH,
	AVRT_PRIORITY_CRITICAL
} AVRT_PRIORITY, *PAVRT_PRIORITY;

typedef HANDLE WINAPI (*set_mm_thread_characteristics)(char *TaskName, LPDWORD TaskIndex);
typedef BOOL WINAPI (*set_mm_thread_priority)(HANDLE AvrtHandle, AVRT_PRIORITY priority);

set_mm_thread_characteristics AvSetMmThreadCharacteristics;
set_mm_thread_priority AvSetMmThreadPriority;

/*
	Win32 simulator doesn't really use a stack. Instead It just
	keep some task specific info in the pseudostack
*/
typedef struct
{
	void (*entry)(void *pvParameters);	/* Task Entry function */
	void *pvParameters;					/* parameters to Entry function */
	portSTACK_TYPE ulCriticalNesting;	/* Critical nesting count */
	HANDLE hThread;						/* handle of thread associated with task */
	HANDLE hSemaphore;					/* Semaphore thread (task) waits on at start and after yielding */
	portSTACK_TYPE dwGlobalIsr;			/* mask used to enable/disable interrupts */
	BOOL yielded;						/* Need to know how task went out of focus */
}SSIM_T;

#define portNO_CRITICAL_NESTING 		( ( unsigned portLONG ) 0 )

//#define DEBUG_OUTPUT
//#define ERROR_OUTPUT

#ifdef DEBUG_OUTPUT
/*	#define debug_printf(...) ( (WaitForSingleObject(hPrintfMutex, INFINITE)|1)?( \
	(  \
	(NULL != (debug_task_handle = (tskTCB *) pxCurrentTCB ))? \
	(fprintf( stderr, "%20s\t%20s\t%i: ",debug_task_handle->pcTaskName,__func__,__LINE__)): \
	(fprintf( stderr, "%20s\t%20s\t%i: ","__unknown__",__func__,__LINE__)) \
	|1)?( \
	((fprintf( stderr, __VA_ARGS__ )|1)?ReleaseMutex( hPrintfMutex ):0) \
	):0 ):0 ) */
#define debug_printf(...) ( ( (NULL != (debug_task_handle = (tskTCB *) pxCurrentTCB ))? \
		(fprintf( stderr, "%20s\t%20s\t%i: ",debug_task_handle->pcTaskName,__func__,__LINE__)): \
		(fprintf( stderr, "%20s\t%20s\t%i: ","__unknown__",__func__,__LINE__)) \
		|1)? \
		fprintf( stderr, __VA_ARGS__ ) : 0 \
		)

	#define debug_error debug_printf

#else
	#ifdef ERROR_OUTPUT
/*		#define debug_error(...) ( (WaitForSingleObject(hPrintfMutex, INFINITE)|1)?( \
		(  \
		(NULL != (debug_task_handle = (tskTCB *) pxCurrentTCB ))? \
        (fprintf( stderr, "%20s\t%20s\t%i: ",debug_task_handle->pcTaskName,__func__,__LINE__)): \
        (fprintf( stderr, "%20s\t%20s\t%i: ","__unknown__",__func__,__LINE__)) \
        |1)?( \
        ((fprintf( stderr, __VA_ARGS__ )|1)?ReleaseMutex( hPrintfMutex ):0) \
        ):0 ):0 )*/

#define debug_error(...) ( ( (NULL != (debug_task_handle = (tskTCB *) pxCurrentTCB ))? \
		(fprintf( stderr, "%20s\t%20s\t%i: ",debug_task_handle->pcTaskName,__func__,__LINE__)): \
		(fprintf( stderr, "%20s\t%20s\t%i: ","__unknown__",__func__,__LINE__)) \
		|1)? \
		fprintf( stderr, __VA_ARGS__ ) : 0 \
		)

		#define debug_printf(...)
	#else
		#define debug_printf(...)
		#define debug_error(...)
	#endif
#endif

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

/******************************************************************************
	National prototypes
******************************************************************************/
/******************************************************************************
	Global variables
******************************************************************************/
volatile DWORD dwPendingIsr;	// pending interrupts
HANDLE hIsrInvoke;	// event to signal an interrupt
HANDLE hIsrMutex;	// mutex to protect above 2

/******************************************************************************
	National variables
******************************************************************************/
static HANDLE hIsrDispatcher;	// handle of the main thread

static volatile portSTACK_TYPE dwGlobalIsr = NMI;	// interrupts disabled @ start
static volatile portSTACK_TYPE dwEnabledIsr = NMI;	// mask of enabled ISRs (individual)

static HANDLE hTickAck;		// acknolwledge tick interrupt
static HANDLE hTermAck;		// acknowledge termination

volatile SSIM_T *taskToDelete;

BOOL bIsWow64;
BOOL bUsingMMCSS;

static enum
{
	SWI_ID_YIELD,
	SWI_ID_TERMINATE,
	SWI_ID_ENDTASK
}
ESWI_ID;

unsigned portLONG ulCriticalNesting = ( unsigned portLONG ) 9999;
void (*vIsrHandler[CPU_INTR_COUNT])(void);
UINT msPerTick; //Returned from timeGetDevCaps()

#if portDEBUG == 1

#define MAX_TRACE	1024

const char * TraceStr[MAX_TRACE];
int TracePtr;

#define TRACE_INFO(s)	do {			\
	if(TracePtr < MAX_TRACE) {		\
		TraceStr[TracePtr++]=s;		\
	}					\
}while(0)

static char *TskName(DWORD *TCB)
{
	// HACK for debugging
	return ((char*)TCB + 56);
}

#else

#define TRACE_INFO(s)

#endif

/******************************************************************************
	National functions
******************************************************************************/
static DWORD WINAPI tick_generator(LPVOID lpParameter)
{
	HANDLE hTimer;
	LARGE_INTEGER liDueTime;
	HANDLE hObjList[2];
	float before, after;

	if(bUsingMMCSS)
	{
		DWORD taskIndex = 0;
		HANDLE AvRtHandle = AvSetMmThreadCharacteristics(TEXT("FreeRTOS"), &taskIndex);
		if(AvRtHandle == NULL)
		{
			printf("Error setting MMCSS on the tick generator.");
			getchar();
			exit(1);
		}
		AvSetMmThreadPriority(AvRtHandle, AVRT_PRIORITY_CRITICAL);
	}

	hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	liDueTime.QuadPart = -100000;

	hObjList[0] = hIsrMutex;
	hObjList[1] = hTimer;

	for(;;)
	{
		before = (float)clock()/CLOCKS_PER_SEC;
		debug_printf("tick before, %f\n", before);
		SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, FALSE);
		if(WaitForMultipleObjects(2, hObjList, TRUE, 2000) == WAIT_TIMEOUT)
		{
			printf("Tick generator: timed out at WaitForMultipleObjects\n");
			return 0;
		}
		after = (float)clock()/CLOCKS_PER_SEC;
		debug_printf("tick after, %f\n", after);
		debug_printf("diff: %f\n", after - before);

		// generate the tick interrupt
		dwPendingIsr |= (1<<CPU_INTR_TICK);
		SetEvent(hIsrInvoke);

		// wait till interrupt handler acknowledges the interrupt (avoids
		// overruns).
		if(SignalObjectAndWait(hIsrMutex, hTickAck, 2000, FALSE) == WAIT_TIMEOUT)
		{
			printf("Tick generator: timed out at SignalObjectAndWait\n");
			exit(1);
		}
	}
}

static DWORD WINAPI TaskSimThread( LPVOID lpParameter )
{
	SSIM_T *psSim=(SSIM_T*)lpParameter;
	ulCriticalNesting = psSim->ulCriticalNesting;

	if(bUsingMMCSS)
	{
		DWORD taskIndex = 0;
		HANDLE AvRtHandle = AvSetMmThreadCharacteristics("FreeRTOS", &taskIndex);
		if(AvRtHandle == NULL)
		{
			printf("Error setting MMCSS on a task.");
			getchar();
			exit(1);
		}
	}

	psSim->entry(psSim->pvParameters);

	return 0;
}

void create_mmcss_registry_entry()
{
	HKEY hKey;
	REGSAM flags = KEY_WRITE;
	if(bIsWow64)
		flags |= KEY_WOW64_64KEY;


	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
			TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile\\Tasks\\FreeRTOS"),
			0, NULL, 0, flags, NULL,
			&hKey, NULL) != ERROR_SUCCESS)
	{
		printf("Internal error creating MMCSS registry key.");
		getchar();
		exit(1);
	}

	DWORD affinity = 0;
	DWORD clock_rate = 10000;
	DWORD gpu_priority = 8;
	DWORD priority = 2;

	TCHAR *background_only = TEXT("True");
	TCHAR *scheduling_category = TEXT("High");
	TCHAR *sfio_priority = TEXT("Normal");

	RegSetValueEx(hKey, TEXT("Affinity"), 0, REG_DWORD,
			(BYTE *) &affinity, sizeof(DWORD));
	RegSetValueEx(hKey, TEXT("Clock Rate"), 0, REG_DWORD,
			(BYTE *) &clock_rate, sizeof(DWORD));
	RegSetValueEx(hKey, TEXT("GPU Priority"), 0, REG_DWORD,
			(BYTE *) &gpu_priority, sizeof(DWORD));
	RegSetValueEx(hKey, TEXT("Priority"), 0, REG_DWORD,
			(BYTE *) &priority, sizeof(DWORD));

	RegSetValueEx(hKey, TEXT("Background Only"), 0, REG_SZ,
			(BYTE *) background_only,
			(DWORD)(lstrlen(background_only)+1)*sizeof(TCHAR));
	RegSetValueEx(hKey, TEXT("Scheduling Category"), 0, REG_SZ,
			(BYTE *) scheduling_category,
			(DWORD)(lstrlen(scheduling_category)+1)*sizeof(TCHAR));
	RegSetValueEx(hKey, TEXT("SFIO Priority"), 0, REG_SZ,
			(BYTE *) sfio_priority,
			(DWORD)(lstrlen(sfio_priority)+1)*sizeof(TCHAR));

	RegCloseKey(hKey);
}


//Boldly taken from MSDN
BOOL IsUserAdmin(VOID)
/*++
Routine Description: This routine returns TRUE if the caller's
process is a member of the Administrators local group. Caller is NOT
expected to be impersonating anyone and is expected to be able to
open its own process and process token.
Arguments: None.
Return Value:
   TRUE - Caller has Administrators local group.
   FALSE - Caller does not have Administrators local group. --
*/
{
	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(
	    &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);
	if(b)
	{
		if (!CheckTokenMembership( NULL, AdministratorsGroup, &b))
		{
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}

	return b;
}

//Also boldly taken from MSDN
void CheckIsWow64()
{
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            printf("Error determining if FreeRTOS is running in WOW64 mode.");
            getchar();
            exit(1);
        }
    }
}

//Check whether we're an elevated process under UAC (only works with Vista and 7)
BOOL is_elevated_process()
{
	HANDLE tokenHandle;
	TOKEN_ELEVATION isElevated;
	DWORD returnLength;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
	{
		printf("Error opening process token while verifying elevated rights.");
		getchar();
		exit(1);
	}

	if(!GetTokenInformation(tokenHandle,
			TokenElevation,
			(void *) &isElevated, sizeof(isElevated), &returnLength))
	{
		printf("Error getting token information while verifying elevated rights.");
		CloseHandle(tokenHandle);
		getchar();
		exit(1);
	}

	CloseHandle(tokenHandle);

	return (isElevated.TokenIsElevated != 0);
}

void check_mmcss_registry_entry()
{
	HKEY hKey;
	DWORD numSubKeys;
	TCHAR subKeyName[MAX_KEY_LENGTH];
	DWORD subKeyNameSize;
	BOOL exists = FALSE;
	REGSAM flags = KEY_READ;
	LONG enumError;
	if(bIsWow64)
		flags |= KEY_WOW64_64KEY;

	//Check for the registry entry

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile\\Tasks"),
			0, flags,
			&hKey) != ERROR_SUCCESS)
	{
		printf("Error opening MMCSS registry key.");
		getchar();
		exit(1);
	}

	//Enumerate through all subkeys
	RegQueryInfoKey(hKey, NULL, NULL, NULL,
			&numSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	//printf("Num subkeys: %d\n", (int) numSubKeys);
	for(DWORD i = 0; i < numSubKeys; i++)
	{
		subKeyNameSize = MAX_KEY_LENGTH;
		if((enumError = RegEnumKeyEx(hKey, i, subKeyName, &subKeyNameSize,
				NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
		{
			//printf("Subkey name: %s\n", subKeyName);
			if(lstrcmp(subKeyName, TEXT("FreeRTOS")) == 0)
			{
				exists = TRUE;
			}
		}
		else
			printf("Error enumerating subkeys: %d\n", (int) enumError);
	}

	RegCloseKey(hKey);

	if(exists)
	{
		return;
	}

	//it doesn't exist
	//Create it

	//Check for sufficient privileges
	if(!is_elevated_process())
	{
		printf("Error: this program needs elevated privileges to run properly the first time only.\n");
		printf("Please click 'run as administrator'.\n");
		getchar();
		exit(1);
	}

	create_mmcss_registry_entry();
}

void load_mmcss()
{
	HMODULE hAvrt = LoadLibrary(TEXT("Avrt.dll"));
	if(hAvrt == NULL)
	{
		printf("Error: could not find avrt.dll.");
		getchar();
		exit(1);
	}

	AvSetMmThreadCharacteristics = (set_mm_thread_characteristics)
			GetProcAddress(hAvrt, TEXT("AvSetMmThreadCharacteristicsA"));
	AvSetMmThreadPriority = (set_mm_thread_priority)
			GetProcAddress(hAvrt, TEXT("AvSetMmThreadPriority"));
}

static void create_system_objects(void)
{
	OSVERSIONINFO version;

	version.dwOSVersionInfoSize = sizeof(version);
	GetVersionEx(&version);

	//For operating systems up to XP, increase the clock rate
	//and increase the priority as much as possible
	if(version.dwMajorVersion < 6)
	{
		if(IsUserAdmin())
			SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		else
			SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

		//Set timer

		TIMECAPS tc;
		timeGetDevCaps(&tc, sizeof(tc));
		msPerTick = tc.wPeriodMin;
		debug_printf("Ms per tick: %i\n", msPerTick);
		if(msPerTick > 2)
		{
			printf("Warning: your system timer has a low resolution.\n");
			printf("Either decrease the tick rate, or get a better PC!\n");
		}
		timeBeginPeriod(tc.wPeriodMin);
		bUsingMMCSS = FALSE;
	}
	else
	{
		//From Vista onward, use MMCSS
		CheckIsWow64();
		check_mmcss_registry_entry();
		load_mmcss();
		bUsingMMCSS = TRUE;
	}

	DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&hIsrDispatcher,
		0,
		FALSE,
		DUPLICATE_SAME_ACCESS);

	if(bUsingMMCSS)
	{
		DWORD taskIndex = 0;
		HANDLE AvRtHandle = AvSetMmThreadCharacteristics("FreeRTOS", &taskIndex);
		if(AvRtHandle == NULL)
		{
			printf("Error setting MMCSS on the scheduler.");
			getchar();
			exit(1);
		}
		AvSetMmThreadPriority(AvRtHandle, AVRT_PRIORITY_LOW);
	}
	else
		SetThreadPriority(hIsrDispatcher, THREAD_PRIORITY_BELOW_NORMAL);

	hIsrMutex = CreateMutex(NULL, FALSE, NULL);
	hIsrInvoke = CreateEvent(NULL, FALSE, FALSE, NULL);
	hTickAck = CreateEvent(NULL, FALSE, FALSE, NULL);
	hTermAck = CreateEvent(NULL, FALSE, FALSE, NULL);

	dwEnabledIsr |= (1<<CPU_INTR_TICK);

#if configUSE_PREEMPTION != 0
	if(bUsingMMCSS)
		CreateThread(NULL, 0, tick_generator, NULL, 0, NULL); //It uses MMCSS too
	else
		SetThreadPriority(CreateThread(NULL, 0, tick_generator, NULL, 0, NULL),
				THREAD_PRIORITY_ABOVE_NORMAL);
#endif
}

/******************************************************************************
	Global functions
******************************************************************************/
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
	BOOL ok;
	SSIM_T *psSim;

	psSim=(SSIM_T*)(pxTopOfStack - sizeof(SSIM_T));
	psSim->entry = ( void (*)(void *)) pxCode;
	psSim->pvParameters=pvParameters;
	psSim->ulCriticalNesting=portNO_CRITICAL_NESTING;
	psSim->dwGlobalIsr = (DWORD)-1;

	// semaphore that's used to handle yielding
	psSim->hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
	psSim->yielded = FALSE;
	psSim->hThread = CreateThread(NULL, 0, TaskSimThread, psSim, CREATE_SUSPENDED, NULL);
	ok=SetThreadPriority(psSim->hThread, THREAD_PRIORITY_IDLE);
	return (portSTACK_TYPE *) psSim;
}

portBASE_TYPE xPortStartScheduler( void )
{
	BOOL bSwitch, bTaskDelete;
	SSIM_T *psSim;
	DWORD dwIntr;
	int i, iIsrCount;
	HANDLE hObjList[2]; 

	create_system_objects();

	/* Start the first task. */
	psSim=(SSIM_T *)*pxCurrentTCB;
	ulCriticalNesting = portNO_CRITICAL_NESTING;
	ResumeThread(psSim->hThread);

	hObjList[0] = hIsrMutex;
	hObjList[1] = hIsrInvoke;

	WaitForSingleObject(hIsrMutex, INFINITE);
	dwGlobalIsr = psSim->dwGlobalIsr;
	ReleaseMutex(hIsrMutex);

	for(;;)
	{
		if(WaitForMultipleObjects(2, hObjList, TRUE, 2000) == WAIT_TIMEOUT)
		{
			printf("vPortStartScheduler: timed out at WaitForMultipleObjects\n");
			exit(1);
		}

		psSim=(SSIM_T *)*pxCurrentTCB;

		psSim->ulCriticalNesting = ulCriticalNesting ;
		psSim->dwGlobalIsr = dwGlobalIsr;

		dwIntr = ((dwGlobalIsr & dwEnabledIsr) & dwPendingIsr);

		bSwitch = (dwIntr != 0);	// switch only for a real interrupt
		iIsrCount = 0;			// no suspensions after handling first int
		bTaskDelete = FALSE;

		for(i=0; dwIntr && i<CPU_INTR_COUNT; i++)
		{
			if(dwIntr & (1<<i))
			{
				dwIntr &= ~(1<<i);

				switch(i)
				{
				case CPU_INTR_SWI:
					dwPendingIsr &= ~(1<<CPU_INTR_SWI);

					if(ESWI_ID == SWI_ID_TERMINATE)
					{
						SetEvent(hTermAck);
						ReleaseMutex(hIsrMutex);
						return 0;
					}
					else if(ESWI_ID == SWI_ID_ENDTASK)
					{
						TerminateThread(taskToDelete->hThread, 0);
						CloseHandle(taskToDelete->hSemaphore);
					}

					psSim->yielded = TRUE;
					iIsrCount++;
					break;

				case CPU_INTR_TICK:
					dwPendingIsr &= ~(1<<CPU_INTR_TICK);

					if(!iIsrCount++)
						SuspendThread(psSim->hThread);

					vTaskIncrementTick();
					//debug_printf("Sending tick ack...\n");
					SetEvent(hTickAck);
					break;

				default:
					if(!iIsrCount++)
						SuspendThread(psSim->hThread);

					vIsrHandler[i]();
					break;
				}
			}
		}

		if(bSwitch)
		{
			vTaskSwitchContext();
			debug_printf("switching context\n");
			psSim=(SSIM_T *)*pxCurrentTCB;
			ulCriticalNesting = psSim->ulCriticalNesting;
			dwGlobalIsr = psSim->dwGlobalIsr;
			
			if(psSim->yielded) {
				psSim->yielded = FALSE;
				ReleaseSemaphore(psSim->hSemaphore, 1, NULL);		// awake next task
			} else {
				ResumeThread(psSim->hThread);
			}
		}

		if((dwGlobalIsr & dwEnabledIsr) & dwPendingIsr)
		{
			// we just enabled interrupts (by task switching in) and an interrupt is
			// pending. Reinvoke ourselves
			SetEvent(hIsrInvoke);
		}

		ReleaseMutex(hIsrMutex);
	}
}

void vPortEndScheduler( void )
{
	if(hIsrDispatcher)
	{
		WaitForSingleObject(hIsrMutex, INFINITE);
		dwPendingIsr |= (1<<CPU_INTR_SWI);
		ESWI_ID = SWI_ID_TERMINATE;
		SetEvent(hIsrInvoke);
		SignalObjectAndWait(hIsrMutex, hTermAck, INFINITE, FALSE);
	}
}

void __disable_interrupt(void)
{
	if(hIsrDispatcher)
	{
		WaitForSingleObject(hIsrMutex, INFINITE);
		dwGlobalIsr = NMI;
		ReleaseMutex(hIsrMutex);
	}
	else
	{
		dwGlobalIsr = NMI;
	}
}

void __enable_interrupt(void)
{
	if(hIsrDispatcher)
	{
		WaitForSingleObject(hIsrMutex, INFINITE);
		dwGlobalIsr = (DWORD)-1;
		
		if(dwPendingIsr)
		{
			SetEvent(hIsrInvoke);
		}
		ReleaseMutex(hIsrMutex);
	}
	else
	{
		dwGlobalIsr = (DWORD)-1;
	}
}

void __swi(void)
{
	SSIM_T *psSim=(SSIM_T *)*pxCurrentTCB;

	if(hIsrDispatcher)
	{
		WaitForSingleObject(hIsrMutex, INFINITE);
		dwPendingIsr |= (1<<CPU_INTR_SWI);
		ESWI_ID = SWI_ID_YIELD;
		SetEvent(hIsrInvoke);
		SignalObjectAndWait(hIsrMutex, psSim->hSemaphore, INFINITE, FALSE);
	}
}

void __delete_task(void *tcb)
{
	taskToDelete = *((SSIM_T **)tcb);

	if(hIsrDispatcher)
	{
		WaitForSingleObject(hIsrMutex, INFINITE);
		dwPendingIsr |= (1<<CPU_INTR_SWI);
		ESWI_ID = SWI_ID_ENDTASK;
		SetEvent(hIsrInvoke);
		ReleaseMutex(hIsrMutex);

		if(pxCurrentTCB == tcb)
			Sleep(100000); //This task is suicidal, wait until it dies
	}
}

void __generate_interrupt(int iNo)
{
	if(hIsrDispatcher)
	{
		WaitForSingleObject(hIsrMutex, INFINITE);
		dwPendingIsr |= (1<<iNo);
		SetEvent(hIsrInvoke);
		ReleaseMutex(hIsrMutex);
	}
}

int iPortSetIsrHandler(int iNo, void (*handler)(void))
{
	if(iNo < CPU_INTR_COUNT) {
		if(hIsrDispatcher) {
			//SSIM_T *psSim=(SSIM_T *)*pxCurrentTCB;

			WaitForSingleObject(hIsrMutex, INFINITE);
			vIsrHandler[iNo]=handler;
			ReleaseMutex(hIsrMutex);
		}
		else
		{
			vIsrHandler[iNo]=handler;
		}

		return 0;
	}

	return -1;
}

void vPortEnableInt(int iNo)
{
	if(hIsrDispatcher)
	{
		WaitForSingleObject(hIsrMutex, INFINITE);
		dwEnabledIsr |= (1<<iNo);

		if((dwGlobalIsr & dwEnabledIsr) & dwPendingIsr)
		{
			SetEvent(hIsrInvoke);
		}

		ReleaseMutex(hIsrMutex);
	}
	else
	{
		dwEnabledIsr |= (1<<iNo);
	}
}

void vPortDisableInt(int iNo)
{
	if(hIsrDispatcher)
	{
		WaitForSingleObject(hIsrMutex, INFINITE);
		dwEnabledIsr &= ~(1<<iNo);
		ReleaseMutex(hIsrMutex);
	}
	else
	{
		dwEnabledIsr &= ~(1<<iNo);
	}
}

BOOL bPortIsEnabledInt(int iNo)
{
	DWORD dwEnabled;

	if(hIsrDispatcher)
	{
		WaitForSingleObject(hIsrMutex, INFINITE);
		dwEnabled = dwEnabledIsr;
		ReleaseMutex(hIsrMutex);
	}
	else
	{
		dwEnabled = dwEnabledIsr;
	}

	return (dwEnabled & (1<<iNo)) != 0;
}

void vPortEnterCritical( void )
{
	/* Disable interrupts first! */
	__disable_interrupt();

	/* Now interrupts are disabled ulCriticalNesting can be accessed
	directly.  Increment ulCriticalNesting to keep a count of how many times
	portENTER_CRITICAL() has been called. */
	ulCriticalNesting++;
}

void vPortExitCritical( void )
{
	if( ulCriticalNesting > portNO_CRITICAL_NESTING )
	{
		/* Decrement the nesting count as we are leaving a critical section. */
		ulCriticalNesting--;

		/* If the nesting level has reached zero then interrupts should be
		re-enabled. */
		if( ulCriticalNesting == portNO_CRITICAL_NESTING )
		{
			__enable_interrupt();
		}
	}
}

unsigned long ulPortGetTimerValue( void )
{
	return (unsigned long) clock();
}