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
//! \file glc_renderstatistics.h interface for the GLC_RenderStatistics class.

#ifndef GLC_RENDERSTATISTICS_H_
#define GLC_RENDERSTATISTICS_H_

#include "glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_RenderStatistics
/*! \brief GLC_RenderStatistics is use to collect render statistics*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_RenderStatistics
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Private constructor. This class is static only
	GLC_RenderStatistics();
	virtual ~GLC_RenderStatistics();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true statistics are activated
	static bool activated();

	//! Return current body count
	static unsigned int bodyCount();

	//! Return current triangles count
	static unsigned long triangleCount();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set activation flag to the given flag
	static void setActivationFlag(bool flag);

	//! Reset all count
	static void reset();

	//! Add bodies to the current body count
	static void addBodies(unsigned int bodies);

	//! Add Triangles to the current tringle count
	static void addTriangles(unsigned int triangles);

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! Flag to know if statistics are activated
	static bool m_IsActivated;

	//! Last render geometry count
	static unsigned int m_LastRenderGeometryCount;

	//! Last render polygon count
	static unsigned long m_LastRenderPolygonCount;
};

#endif /* GLC_RENDERSTATISTICS_H_ */
