/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 1.2.0, packaged on September 2009.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_state.h interface for the GLC_State class.

#ifndef GLC_STATE_H_
#define GLC_STATE_H_

#include <QString>

//////////////////////////////////////////////////////////////////////
//! \class GLC_State
/*! \brief GLC_State store GLC_lib state*/

/*! GLC_State is used to set and get glabal GLC_lib state
 * */
//////////////////////////////////////////////////////////////////////
class GLC_State
{
private:
	GLC_State();
public:
	~GLC_State();

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true if VBO is supported
	inline static bool vboSupported()
	{return m_VboSupported;}

	//! Return true if VBO is used
	inline static bool vboUsed()
	{return m_UseVbo;}

	//! Return true if GLSL is supported
	inline static bool glslSupported()
	{return m_GlslSupported;}

	//! Return true if GLSL is used
	inline static bool glslUsed()
	{return m_UseShader;}

	//! Return true if Point Sprite is supported
	inline static bool pointSpriteSupported()
	{return m_PointSpriteSupported;}

	//! Return true if selection shader is used
	inline static bool selectionShaderUsed()
	{return m_UseSelectionShader;}

	//! Return true if is in selection mode
	inline static bool isInSelectionMode()
	{return m_IsInSelectionMode;}

	//! Return the Opengl version
	inline static QString version()
	{return m_Version;}

	//! Return the Opengl vendor
	inline static QString vendor()
	{return m_Vendor;}

	//! Return the Opengl renderer
	inline static QString renderer()
	{return m_Renderer;}

	//! Return true if OpenGL Vendor is NVIDIA
	inline static bool vendorIsNvidia()
	{return m_Vendor.contains("NVIDIA");}

	//! Return true if pixel culling is activate
	inline static bool isPixelCullingActivated()
	{return m_IsPixelCullingActivated;}
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Intialize the state
	static void init();

	//! Set VBO support
	static void setVboSupport();

	//! Set VBO usage
	static void setVboUsage(const bool);

	//! Set GLSL support
	static void setGlslSupport();

	//! Set Point Sprite support
	static void setPointSpriteSupport();

	//! Set GLSL usage
	static void setGlslUsage(const bool);

	//! Set selection shader usage
	static void setSelectionShaderUsage(const bool);

	//! Set selection mode
	inline static void setSelectionMode(const bool mode)
	{m_IsInSelectionMode= mode;}

	//! Set pixel culling state
	inline static void setPixelCullingUsage(const bool activation)
	{m_IsPixelCullingActivated= activation;}

//@}

//////////////////////////////////////////////////////////////////////
//Private attributes
//////////////////////////////////////////////////////////////////////
private:
	//! VBO supported flag
	static bool m_VboSupported;

	//! VBO used
	static bool m_UseVbo;

	//! GLSL supported flag
	static bool m_GlslSupported;

	//! Point Sprite supported flag
	static bool m_PointSpriteSupported;

	//! Use shader
	static bool m_UseShader;

	//! Use selectionShader flag
	static bool m_UseSelectionShader;

	//! In selection mode
	static bool m_IsInSelectionMode;

	//! Pixel culling activation
	static bool m_IsPixelCullingActivated;

	//! The Opengl card version
	static QString m_Version;

	//! The Opengl card vendor
	static QString m_Vendor;

	//! The Opengl card renderer
	static QString m_Renderer;

};

#endif /*GLC_STATE_H_*/
