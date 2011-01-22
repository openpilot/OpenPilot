/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 2.0.0, packaged on July 2010.

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
//! \file glc_selectionmaterial.h interface for the GLC_SelectionMaterial class.

#ifndef GLC_SELECTIONMATERIAL_H_
#define GLC_SELECTIONMATERIAL_H_

#include <QColor>
#include <QtOpenGL>
#include "../glc_ext.h"
#include "glc_shader.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_SelectionMaterial
/*! \brief GLC_SelectionMaterial : Material used for selection feedback*/

//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_SelectionMaterial
{
private:
	GLC_SelectionMaterial();

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Execute OpenGL Material
	static void glExecute();
	//! Init shader
	inline static void initShader() {m_SelectionShader.createAndCompileProgrammShader();}
	//! delete shader
	static void deleteShader();
	//! Set shader
	inline static void setShaders(QFile& vertex, QFile& fragment)
	{m_SelectionShader.setVertexAndFragmentShader(vertex, fragment);}
	//! Use shader
	inline static void useShader() {m_SelectionShader.use();}
	//! Unused shader
	inline static void unUseShader() {m_SelectionShader.unuse();}

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////

private:
		//! Selection Shader
		static GLC_Shader m_SelectionShader;

};

#endif /*GLC_SELECTIONMATERIAL_H_*/
