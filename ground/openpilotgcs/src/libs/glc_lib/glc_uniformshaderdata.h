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
//! \file glc_uniformshaderdata.h interface for the GLC_UniformShaderData class.

#ifndef GLC_UNIFORMSHADERDATA_H_
#define GLC_UNIFORMSHADERDATA_H_

#include <QtOpenGL>

#include "maths/glc_matrix4x4.h"
#include "shading/glc_light.h"

#include "glc_config.h"

class GLC_Context;

class GLC_LIB_EXPORT GLC_UniformShaderData
{
public:
	GLC_UniformShaderData();
	virtual ~GLC_UniformShaderData();

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set Light values from the given light
	void setLightValues(const GLC_Light& light);

	//! Set lighting enbale state
	void setLightingState(bool enable);

	//! Set the model view matrix
	void setModelViewProjectionMatrix(const GLC_Matrix4x4& modelView, const GLC_Matrix4x4& projection);

	//! Update all uniform variables
	void updateAll(const GLC_Context* pContext);

//@}

//////////////////////////////////////////////////////////////////////
// private members
//////////////////////////////////////////////////////////////////////
private:

};

#endif /* GLC_UNIFORMSHADERDATA_H_ */
