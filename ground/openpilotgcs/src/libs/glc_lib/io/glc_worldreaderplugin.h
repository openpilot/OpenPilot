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

//! \file glc_worldreaderplugin.h interface for reading world from 3D model

#ifndef GLC_WORLDREADERPLUGIN_H_
#define GLC_WORLDREADERPLUGIN_H_

#include <QtPlugin>
#include <QStringList>

#include "glc_worldreaderhandler.h"

class GLC_WorldReaderPlugin
{
public:
	virtual ~GLC_WorldReaderPlugin() {}

	//! Return the list of 3D model keys this plugin support
	virtual QStringList keys() const =0;

	//! Return a reader handler
	virtual GLC_WorldReaderHandler* readerHandler()= 0;
};

Q_DECLARE_INTERFACE(GLC_WorldReaderPlugin, "com.GLC_lib.GLC_WorldReaderPlugin")

#endif /* GLC_WORLDREADERPLUGIN_H_ */
