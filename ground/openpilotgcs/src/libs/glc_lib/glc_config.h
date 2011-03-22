/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_config.h the GLC_lib configuration file

#ifndef GLC_CONFIG_H
#define GLC_CONFIG_H

#include <QtGlobal>

// Dynamic library export macros
#ifndef GLC_LIB_STATIC
# ifdef CREATE_GLC_LIB_DLL
#  define GLC_LIB_EXPORT Q_DECL_EXPORT
# else
#  define GLC_LIB_EXPORT Q_DECL_IMPORT
# endif
#endif

// For static library, this macro is empty
#ifndef GLC_LIB_EXPORT
# define GLC_LIB_EXPORT
#endif

#endif // GLC_CONFIG_H
