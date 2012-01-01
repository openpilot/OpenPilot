/* -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*-
 * PIOS interface shims for MSHEAP
 *
 * Copyright 2011 Michael Smith. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    1. Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "msheap.h"
#include "pios_config.h"
#include "pios.h"

/*
 * Symbols exported by the linker script telling us where the heap is.
 */
extern char	_sheap;
extern char	_eheap;

#if defined(PIOS_INCLUDE_FREERTOS)
/*
 * Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
 * all the API functions to use the MPU wrappers.  That should only be done when
 * task.h is included from an application file.
 * */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
# include "FreeRTOS.h"
# include "task.h"
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/*
 * Optional callback for allocation failures.
 */
extern void vApplicationMallocFailedHook(void) __attribute__((weak));

void *
pvPortMalloc(size_t s)
{
	void *p;

	vPortEnterCritical();
	p = msheap_alloc(s);
	vPortExitCritical();

	if (p == NULL && &vApplicationMallocFailedHook != NULL)
		vApplicationMallocFailedHook();

	return p;
}

void
vPortFree(void *p)
{
	vPortEnterCritical();
	msheap_free(p);
	vPortExitCritical();
}

size_t
xPortGetFreeHeapSize(void)
{
	return msheap_free_space();
}

void
vPortInitialiseBlocks(void)
{
	msheap_init(&_sheap, &_eheap);
}

void
xPortIncreaseHeapSize(size_t bytes)
{
	msheap_extend(bytes);
}

void *
malloc(size_t size)
{
	return pvPortMalloc(size);
}

void
free(void *p)
{
	return vPortFree(p);
}

#else /* !PIOS_INCLUDE_FREERTOS */
int heap_init_done;
void *
malloc(size_t size)
{
//	static 

	if (!heap_init_done) {
		msheap_init(&_sheap, &_eheap);
		heap_init_done = 1;
	}

	return msheap_alloc(size);
}

void
free(void *p)
{
	return msheap_free(p);
}

#endif /* PIOS_INCLUDE_FREERTOS */

void
msheap_panic(const char *reason)
{
	//PIOS_DEBUG_Panic(reason);
}
