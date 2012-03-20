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

//! \file glc_state.h interface for the GLC_State class.

#ifndef GLC_STATE_H_
#define GLC_STATE_H_

#include <QString>

#include "glc_cachemanager.h"

#include "glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_State
/*! \brief GLC_State store GLC_lib state*/

/*! GLC_State is used to set and get glabal GLC_lib state
 * */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_State
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
	static bool vboSupported();

	//! Return true if VBO is used
	static bool vboUsed();

	//! Return true if GLSL is supported
	static bool glslSupported();

	//! Return true if frameBuffer is supported
	static bool frameBufferSupported();

	//! Return true if GLSL is used
	static bool glslUsed();

	//! Return true if Point Sprite is supported
	static bool pointSpriteSupported();

	//! Return true if selection shader is used
	static bool selectionShaderUsed();

	//! Return true if is in selection mode
	static bool isInSelectionMode();

	//! Return the Opengl version
	static QString version();

	//! Return the Opengl vendor
	static QString vendor();

	//! Return the Opengl renderer
	static QString renderer();

	//! Return true if OpenGL Vendor is NVIDIA
	static bool vendorIsNvidia();

	//! Return true if pixel culling is activate
	static bool isPixelCullingActivated();

	//! Return true if the cache is used
	static bool cacheIsUsed();

	//! Return the current cache manager
	static GLC_CacheManager& currentCacheManager();

	//! Return true if space partitionning is used
	static bool isSpacePartitionningActivated();

	//! Return the default octree depth
	static int defaultOctreeDepth();

	//! Return true if frustum culling is activated
	static bool isFrustumCullingActivated();

	//! Return true valid
	static bool isValid();
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

	//! Set the frame buffer support
	static void setFrameBufferSupport();

	//! Set GLSL usage
	static void setGlslUsage(const bool);

	//! Set selection shader usage
	static void setSelectionShaderUsage(const bool);

	//! Set selection mode
	static void setSelectionMode(const bool);

	//! Set pixel culling state
	static void setPixelCullingUsage(const bool);

	//! Set the cache usage
	static void setCacheUsage(const bool);

	//! Set the current cache manager
	static void setCurrentCacheManager(const GLC_CacheManager&);

	//! Set space partionning usage
	static void setSpacePartionningUsage(const bool);

	//! Set the default octree depth
	static void setDefaultOctreeDepth(int);

	//! Set the frustum culling usage
	static void setFrustumCullingUsage(bool);

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

	//! Cache usage
	static bool m_UseCache;

	//! The current cache manager
	static GLC_CacheManager m_CacheManager;

	//! Space partitionning activation
	static bool m_IsSpacePartitionningActivated;

	//! Frustum culling activated
	static bool m_IsFrustumCullingActivated;

	//! Frame buffer supported
	static bool m_IsFrameBufferSupported;

	//! State valid flag
	static bool m_IsValid;

};

#endif /*GLC_STATE_H_*/
