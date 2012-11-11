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

#include "fonts.h"

// Font table. Add new fonts here. The table must end with a -1 for the id.
struct FontEntry fonts[NUM_FONTS + 1] = {
	{ 0, 8, 14, "Outlined8x14",
			font_lookup_outlined8x14,
			font_data_outlined8x14,
			0 },
	{ 1, 8, 8, "Outlined8x8",
			font_lookup_outlined8x8,
			font_data_outlined8x8,
			FONT_UPPERCASE_ONLY },
	{ 2, 8, 10, "font8x10", 0, 0, 0 },
	{ 3, 12, 18, "font12x18", 0, 0, 0 },
	{ -1 } // ends font table
};
