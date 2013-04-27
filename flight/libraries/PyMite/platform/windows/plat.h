/*
# This file is Copyright 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
#
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING up one directory from this.
*/

#ifndef _PLAT_H_
#define _PLAT_H_

#include <stdio.h>

// MSVC does not define a stock snprintf, so we define one here
#if defined(_MSC_VER)
    //#define snprintf _snprintf
    #define snprintf(buf, count, format, ...) _snprintf_s(buf, count, _TRUNCATE, format, __VA_ARGS__)
#endif

#define PM_HEAP_SIZE 0x2000
#define PM_FLOAT_LITTLE_ENDIAN

#endif /* _PLAT_H_ */
