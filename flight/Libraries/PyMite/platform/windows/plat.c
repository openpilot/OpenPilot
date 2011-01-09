/*
 * PyMite - A flyweight Python interpreter for 8-bit and larger microcontrollers.
 * Copyright 2002 Dean Hall.  All rights reserved.
 * PyMite is offered through one of two licenses: commercial or open-source.
 * See the LICENSE file at the root of this package for licensing details.
 *
 * Win32 host version, tweaked from original 'desktop' version
 */


#undef __FILE_ID__
#define __FILE_ID__ 0x70

//  determine how we handle time in Windows
//#define USE_SETTIMER 1   /* this requires an active message loop*/
#define USE_TIMESETEVENT 1 /* (Older) Windows multi-media function */
//#define USE_CREATETIMERQUEUETIMER 0 /* CreateTimerQueueTimer function */

/** PyMite platform-specific routines for Desktop target */

#include <stdio.h>
#include <string.h>

#include "pm.h"

//#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(USE_SETTIMER)
    /* local prototypes */
    VOID CALLBACK on_timer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
#endif

#if defined(USE_TIMESETEVENT)
    // shortcut so we do not need an explicit addition to the linker command line
    #pragma comment(lib, "winmm")

    void CALLBACK on_mm_timer(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
#endif


/* local variables*/
static UINT_PTR idEvent = 0;
static DWORD dwLastTime = 0;
static const UINT uTimeOut = 200;  /* # of ms for each timer tick */


/* Desktop target shall use stdio for I/O routines. */
PmReturn_t
plat_init(void)
{
    /*
     * The *nix version used the sigalrm to generate a timer. On Windows, we
     * use a standard Windows timer. Note we do not use an HWND so we do not
     * need to actually create any windows. But we do need a Windows event loop
     * to allow the timer callback to be called properly.
     */
    dwLastTime = GetTickCount();

#if defined(USE_SETTIMER)
    idEvent = SetTimer(NULL, 0, uTimeOut, on_timer);
    // use the return value in a KillTimer function. But we need a plat_uninit() call...
#endif

#if defined(USE_TIMESETEVENT)
    idEvent = timeSetEvent(uTimeOut, 0 /*change later, if system load too high*/, on_mm_timer, 0, TIME_PERIODIC|TIME_CALLBACK_FUNCTION);
#endif

    return PM_RET_OK;
}

#if defined(USE_SETTIMER)
/* Called from Windows every X ms */
VOID CALLBACK on_timer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    PmReturn_t retval = PM_RET_OK;
    DWORD delta = dwTime - dwLastTime;
    //printf("diff = %ld\n", delta);

    // update our system time
    pm_timerMsTicks = dwTime;

    retval = pm_vmPeriodic((uint16_t)delta);
    dwLastTime = dwTime;

    PM_REPORT_IF_ERROR(retval);
}
#endif

#if defined(USE_TIMESETEVENT)
void
CALLBACK on_mm_timer(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    PmReturn_t retval = PM_RET_OK;
    DWORD dwTime = GetTickCount();
    DWORD delta =  dwTime - dwLastTime;
    //printf("diff = %ld\n", delta);

    // update our system time
    pm_timerMsTicks = dwTime;

    retval = pm_vmPeriodic((uint16_t)delta);
    dwLastTime = dwTime;

    PM_REPORT_IF_ERROR(retval);
}
#endif

/*
 * Undo anything we may have done in the plat_init function
 */
PmReturn_t
plat_deinit(void)
{
#if defined(USE_SETTIMER)
    KillTimer(idEvent);
    idEvent = 0;
#endif

#if defined(USE_TIMESETEVENT)
    timeKillEvent(idEvent);
    idEvent = 0;
#endif
    return PM_RET_OK;
}

/*
 * Gives us time to update our clock, etc.
 */
void
plat_tick(void)
{
    //BOOL bRet;
    MSG msg;

    //if ((bRet = GetMessage(&msg, 0, 0, 0)) != 0)
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/*
 * Gets a byte from the address in the designated memory space
 * Post-increments *paddr.
 */
uint8_t
plat_memGetByte(PmMemSpace_t memspace, uint8_t const **paddr)
{
    uint8_t b = 0;

    switch (memspace)
    {
        case MEMSPACE_RAM:
        case MEMSPACE_PROG:
            b = **paddr;
            *paddr += 1;
            return b;

        case MEMSPACE_EEPROM:
        case MEMSPACE_SEEPROM:
        case MEMSPACE_OTHER0:
        case MEMSPACE_OTHER1:
        case MEMSPACE_OTHER2:
        case MEMSPACE_OTHER3:
        default:
            return 0;
    }
}


/* Desktop target shall use stdio for I/O routines */
PmReturn_t
plat_getByte(uint8_t *b)
{
    int c;
    PmReturn_t retval = PM_RET_OK;

    c = getchar();
    *b = c & 0xFF;

    if (c == EOF)
    {
        PM_RAISE(retval, PM_RET_EX_IO);
    }

    return retval;
}


/* Desktop target shall use stdio for I/O routines */
PmReturn_t
plat_putByte(uint8_t b)
{
    int i;
    PmReturn_t retval = PM_RET_OK;

    i = putchar(b);
    fflush(stdout);

    if ((i != b) || (i == EOF))
    {
        PM_RAISE(retval, PM_RET_EX_IO);
    }

    return retval;
}


PmReturn_t
plat_getMsTicks(uint32_t *r_ticks)
{
    *r_ticks = pm_timerMsTicks;

    return PM_RET_OK;
}


void
plat_reportError(PmReturn_t result)
{

#ifdef HAVE_DEBUG_INFO
#define LEN_FNLOOKUP 26
#define LEN_EXNLOOKUP 17

    uint8_t res;
    pPmFrame_t pframe;
    pPmObj_t pstr;
    PmReturn_t retval;
    uint8_t bcindex;
    uint16_t bcsum;
    uint16_t linesum;
    uint16_t len_lnotab;
    uint8_t const *plnotab;
    uint16_t i;

    /* This table should match src/vm/fileid.txt */
    char const * const fnlookup[LEN_FNLOOKUP] = {
        "<no file>",
        "codeobj.c",
        "dict.c",
        "frame.c",
        "func.c",
        "global.c",
        "heap.c",
        "img.c",
        "int.c",
        "interp.c",
        "pmstdlib_nat.c",
        "list.c",
        "main.c",
        "mem.c",
        "module.c",
        "obj.c",
        "seglist.c",
        "sli.c",
        "strobj.c",
        "tuple.c",
        "seq.c",
        "pm.c",
        "thread.c",
        "float.c",
        "class.c",
        "bytearray.c",
    };

    /* This table should match src/vm/pm.h PmReturn_t */
    char const * const exnlookup[LEN_EXNLOOKUP] = {
        "Exception",
        "SystemExit",
        "IoError",
        "ZeroDivisionError",
        "AssertionError",
        "AttributeError",
        "ImportError",
        "IndexError",
        "KeyError",
        "MemoryError",
        "NameError",
        "SyntaxError",
        "SystemError",
        "TypeError",
        "ValueError",
        "StopIteration",
        "Warning",
    };

    /* Print traceback */
    printf("Traceback (most recent call first):\n");

    /* Get the top frame */
    pframe = gVmGlobal.pthread->pframe;

    /* If it's the native frame, print the native function name */
    if (pframe == (pPmFrame_t)&(gVmGlobal.nativeframe))
    {

        /* The last name in the names tuple of the code obj is the name */
        retval = tuple_getItem((pPmObj_t)gVmGlobal.nativeframe.nf_func->
                               f_co->co_names, -1, &pstr);
        if ((retval) != PM_RET_OK)
        {
            printf("  Unable to get native func name.\n");
            return;
        }
        else
        {
            printf("  %s() __NATIVE__\n", ((pPmString_t)pstr)->val);
        }

        /* Get the frame that called the native frame */
        pframe = (pPmFrame_t)gVmGlobal.nativeframe.nf_back;
    }

    /* Print the remaining frame stack */
    for (; pframe != C_NULL; pframe = pframe->fo_back)
    {
        /* The last name in the names tuple of the code obj is the name */
        retval = tuple_getItem((pPmObj_t)pframe->fo_func->f_co->co_names,
                               -1,
                               &pstr);
        if ((retval) != PM_RET_OK) break;

        /*
         * Get the line number of the current bytecode. Algorithm comes from:
         * http://svn.python.org/view/python/trunk/Objects/lnotab_notes.txt?view=markup
         */
        bcindex = pframe->fo_ip - pframe->fo_func->f_co->co_codeaddr;
        plnotab = pframe->fo_func->f_co->co_lnotab;
        len_lnotab = mem_getWord(MEMSPACE_PROG, &plnotab);
        bcsum = 0;
        linesum = pframe->fo_func->f_co->co_firstlineno;
        for (i = 0; i < len_lnotab; i += 2)
        {
            bcsum += mem_getByte(MEMSPACE_PROG, &plnotab);
            if (bcsum > bcindex) break;
            linesum += mem_getByte(MEMSPACE_PROG, &plnotab);
        }
        printf("  File \"%s\", line %d, in %s\n",
               ((pPmFrame_t)pframe)->fo_func->f_co->co_filename,
               linesum,
               ((pPmString_t)pstr)->val);
    }

    /* Print error */
    res = (uint8_t)result;
    if ((res > 0) && ((res - PM_RET_EX) < LEN_EXNLOOKUP))
    {
        printf("%s", exnlookup[res - PM_RET_EX]);
    }
    else
    {
        printf("Error code 0x%02X", result);
    }
    printf(" detected by ");

    if ((gVmGlobal.errFileId > 0) && (gVmGlobal.errFileId < LEN_FNLOOKUP))
    {
        printf("%s:", fnlookup[gVmGlobal.errFileId]);
    }
    else
    {
        printf("FileId 0x%02X line ", gVmGlobal.errFileId);
    }
    printf("%d\n", gVmGlobal.errLineNum);

#else /* HAVE_DEBUG_INFO */

    /* Print error */
    printf("Error:     0x%02X\n", result);
    printf("  Release: 0x%02X\n", gVmGlobal.errVmRelease);
    printf("  FileId:  0x%02X\n", gVmGlobal.errFileId);
    printf("  LineNum: %d\n", gVmGlobal.errLineNum);

    /* Print traceback */
    {
        pPmObj_t pframe;
        pPmObj_t pstr;
        PmReturn_t retval;

        printf("Traceback (top first):\n");

        /* Get the top frame */
        pframe = (pPmObj_t)gVmGlobal.pthread->pframe;

        /* If it's the native frame, print the native function name */
        if (pframe == (pPmObj_t)&(gVmGlobal.nativeframe))
        {

            /* The last name in the names tuple of the code obj is the name */
            retval = tuple_getItem((pPmObj_t)gVmGlobal.nativeframe.nf_func->
                                   f_co->co_names, -1, &pstr);
            if ((retval) != PM_RET_OK)
            {
                printf("  Unable to get native func name.\n");
                return;
            }
            else
            {
                printf("  %s() __NATIVE__\n", ((pPmString_t)pstr)->val);
            }

            /* Get the frame that called the native frame */
            pframe = (pPmObj_t)gVmGlobal.nativeframe.nf_back;
        }

        /* Print the remaining frame stack */
        for (;
             pframe != C_NULL;
             pframe = (pPmObj_t)((pPmFrame_t)pframe)->fo_back)
        {
            /* The last name in the names tuple of the code obj is the name */
            retval = tuple_getItem((pPmObj_t)((pPmFrame_t)pframe)->
                                   fo_func->f_co->co_names, -1, &pstr);
            if ((retval) != PM_RET_OK) break;

            printf("  %s()\n", ((pPmString_t)pstr)->val);
        }
        printf("  <module>.\n");
    }
#endif /* HAVE_DEBUG_INFO */
}
