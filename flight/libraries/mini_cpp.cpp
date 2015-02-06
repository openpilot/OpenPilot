/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 *
 * @file       mini_cpp.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      CPP support methods
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <pios.h>

// _init is called by __libc_init_array during invocation of static constructors
// __libc_init_array calls _init, which is defined in crti.o. _init calls functions that are in .init section.
// If you don't have meaningful stuff in .init section, just define an empty _init function.
extern "C" int _init(void)
{
    return 0;
}


// operator new
void *operator new(size_t size) throw()
{
    return pios_malloc(size);
}

// operator delete
void operator delete(void *p) throw()
{
    pios_free(p);
}

// The function __aeabi_atexit() handles the static destructors.  This can be empty
// because we have no operating system to return to, hence the static destructors
// will never be called.
extern "C" int __aeabi_atexit(__attribute__((unused)) void *object, __attribute__((unused)) void (*destructor)(void *), __attribute__((unused)) void *dso_handle)
{
    return 0;
}

// see https://answers.launchpad.net/gcc-arm-embedded/+question/221105
// and https://answers.launchpad.net/gcc-arm-embedded/+question/224709
__extension__ typedef int __guard __attribute__((mode(__DI__)));
extern "C" int __cxa_atexit(void (*f)(void *), void *p, void *d);
extern "C" int __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release(__guard *);
extern "C" void __cxa_guard_abort(__guard *);
extern "C" void __cxa_pure_virtual(void);

int __cxa_guard_acquire(__attribute__((unused)) __guard *g)
{
    return !*(char *)(g);
};
void __cxa_guard_release(__attribute__((unused)) __guard *g)
{
    *(char *)g = 1;
};
void __cxa_guard_abort(__attribute__((unused)) __guard *) {};
void __cxa_pure_virtual(void)
{
    while (1) {
        ;
    }
}
int __cxa_atexit(__attribute__((unused)) void (*f)(void *), __attribute__((unused)) void *p, __attribute__((unused)) void *d)
{
    return 0;
};
/**
 * @}
 */
