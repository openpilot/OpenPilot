/**
 * Super OSD, software revision 3
 * Copyright (C) 2010 Thomas Oldbury
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef FONTS_H
#define FONTS_H

// New fonts need a .c file (with the data) and a .h file with
// the definitions. Add the .h files here only.
#include "font_outlined8x14.h"
#include "font_outlined8x8.h"

// This number must also be incremented for each new font.
#define NUM_FONTS 4

// Flags for fonts.
#define FONT_LOWERCASE_ONLY	1
#define FONT_UPPERCASE_ONLY	2

// Font table. (Actual list of fonts in fonts.c.)
struct FontEntry
{
	int id;
	unsigned char width, height;
	const char *name;
	const char *lookup;
	const char *data;
	int flags;
};

extern struct FontEntry fonts[NUM_FONTS + 1];

#endif // FONTS_H
