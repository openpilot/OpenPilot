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

//! \file glc_shader.h interface for the GLC_Shader class.

#ifndef GLC_SHADER_H_
#define GLC_SHADER_H_

#include "../glc_global.h"
#include <QGLShader>
#include <QGLShaderProgram>
#include <QStack>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QMap>

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Shader
/*! \brief GLC_Shader : OpenGL shader abstraction*/

/*! An GLC_Shader encapsulate vertex, fragment shader and programm\n
 *  GLC_Shader provide functionnality to load, compile and execute
 * 	GLSL vertex and fragment shader.
 */

//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Shader
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_Shader();

	//! Construct shader with specifie vertex and fragment
	GLC_Shader(QFile&, QFile&);

	//! Copy constructor
	GLC_Shader(const GLC_Shader&);

	//! Shader destructor
	~GLC_Shader();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the program shader id
	inline GLuint id() const
	{return m_ProgramShaderId;}

	//! Return true if the shader is usable
	inline bool isUsable() const
	{return m_ProgramShader.isLinked();}

	//! Return true if the shader can be deleted
	bool canBeDeleted() const;

	//! Return the shader's name
	inline QString name() const
	{return m_Name;}

	//! Return an handle to the QGLProgramShader of this shader
	inline QGLShaderProgram* programShaderHandle()
	{return &m_ProgramShader;}

	//! Return the position attribute id
	inline int positionAttributeId() const
	{return m_PositionAttributeId;}

	//! Return the texture coordinate attribute id
	inline int textureAttributeId() const
	{return m_TextcoordAttributeId;}

	//! Return the color attribute id
	inline int colorAttributeId() const
	{return m_ColorAttributeId;}

	//! Return the normal attribute id
	inline int normalAttributeId() const
	{return m_NormalAttributeId;}

	//! Return the number of shader
	static int shaderCount();

	//! Return true if the given shading group id as a shader
	static bool asShader(GLC_uint shadingGroupId);

	//! Return handle to the shader associated to the given group id
	/*! Return NULL if the given shading group as no associated shader*/
	static GLC_Shader* shaderHandle(GLC_uint shadingGroupId);

	//! Return true if there is an active shader
	static bool hasActiveShader();

	//! Return handle to the current active shader
	/*! Return NULL if there is no current active shader*/
	static GLC_Shader* currentShaderHandle();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set Vertex and fragment shaders
	void setVertexAndFragmentShader(QFile&, QFile&);

	//! Replace this shader by a copy of another shader
	/* If this shader is usable replacing shader must be usable*/
	void replaceShader(const GLC_Shader&);

	//! Assignement operator which use replace shader method
	inline GLC_Shader& operator=(const GLC_Shader& shader)
	{
		replaceShader(shader);
		return *this;
	}

	//! Set the Shader Name
	inline void setName(const QString& name)
	{m_Name= name;}

	//! Return the modelView location id
	inline int modelViewLocationId() const
	{return m_ModelViewLocationId;}

	//! Return the modelView Projection matrix id
	inline int mvpLocationId() const
	{return m_MvpLocationId;}

	//! Return the inverse modelView location id
	inline int invModelViewLocationId() const
	{return m_InvModelViewLocationId;}

	//! Return the enable lighting location id
	inline int enableLightingId() const
	{return m_EnableLightingId;}

	//! Return the lights enable state id
	inline int lightsEnableStateId() const
	{return m_LightsEnableStateId;}

	//! Return the light position id of the given light id
	inline int lightPositionId(GLenum lightId) const
	{return m_LightsPositionId.value(lightId);}

	//! Return the light ambient color id of the given light id
	inline int lightAmbientColorId(GLenum lightId) const
	{return m_LightsAmbientColorId.value(lightId);}

	//! Return the light diffuse color id of the given light id
	inline int lightDiffuseColorId(GLenum lightId) const
	{return m_LightsDiffuseColorId.value(lightId);}

	//! Return the light specular color id of the given light id
	inline int lightSpecularColorId(GLenum lightId) const
	{return m_LightsSpecularColorId.value(lightId);}

	//! Return the light spot direction id of the given light id
	inline int lightSpotDirectionId(GLenum lightId) const
	{return m_LightsSpotDirectionId.value(lightId);}

	//! Return the light attenuation factors id of the given light id
	inline int lightAttebuationFactorsId(GLenum lightId) const
	{return m_LightsAttenuationFactorsId.value(lightId);}

	//! Return the light spot exponent id of the given light id
	inline int lightSpotExponentId(GLenum lightId) const
	{return m_LightsSpotExponentId.value(lightId);}

	//! Return the light spot cutoff id of the given light id
	inline int lightSpotCutoffId(GLenum lightId) const
	{return m_LightsSpotCutoffAngleId.value(lightId);}

	//! Return the light compute distance attenuation id of the given light id
	inline int lightComputeDistanceAttenuationId(GLenum lightId) const
	{return m_LightsComputeDistanceAttenuationId.value(lightId);}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Use this shader program
	/*! Throw GLC_Exception if the program is not usable*/
	void use();

	//! Use specified program shader
	/*! Return true if the given shading group id is usable*/
	static bool use(GLC_uint ShadingGroupId);

	//! unuse programm shader
	static void unuse();

	//! Compile and attach shaders to a program shader
	/*! Throw GLC_Exception if vertex and fragment shader are not been set*/
	void createAndCompileProgrammShader();

	//!Delete the shader
	void deleteShader();
//@}

//////////////////////////////////////////////////////////////////////
// private services function
//////////////////////////////////////////////////////////////////////
private:
	//! Init light uniform id
	void initLightsUniformId();
//////////////////////////////////////////////////////////////////////
// private members
//////////////////////////////////////////////////////////////////////
private:
	//! The shading group Stack
	static QStack<GLC_uint> m_ShadingGroupStack;

	//! The current shading goup ID
	static GLC_uint m_CurrentShadingGroupId;

	//! Map between shading group id and program shader
	static QHash<GLC_uint, GLC_Shader*> m_ShaderProgramHash;

	//! Vertex shader
	QGLShader m_VertexShader;

	//! Fragment shader
	QGLShader m_FragmentShader;

	//! The programShader
	QGLShaderProgram m_ProgramShader;

	//! Programm shader ID
	GLC_uint m_ProgramShaderId;

	//! The Shader's name
	QString m_Name;

	//! The position attribute id
	int m_PositionAttributeId;

	//! The Texture coordinate attribute id
	int m_TextcoordAttributeId;

	//! The color attribute id
	int m_ColorAttributeId;

	//! The Normal attribute id
	int m_NormalAttributeId;

	//! The modelView location matrix id
	int m_ModelViewLocationId;

	//! The modelView Projection matrix id
	int m_MvpLocationId;

	//! The inverse modelView location id
	int m_InvModelViewLocationId;

	//! The enable lighting id
	int m_EnableLightingId;

	//! Lights enable states id
	int m_LightsEnableStateId;

	//! Lights positions id
	QMap<GLenum, int> m_LightsPositionId;

	//! Lights ambient color id
	QMap<GLenum, int> m_LightsAmbientColorId;

	//! Lights diffuse color id
	QMap<GLenum, int> m_LightsDiffuseColorId;

	//! Lights specular color id
	QMap<GLenum, int> m_LightsSpecularColorId;

	//! Lights spot direction id
	QMap<GLenum, int> m_LightsSpotDirectionId;

	//! Lights attenuation factors id
	QMap<GLenum, int> m_LightsAttenuationFactorsId;

	//! Lights spot exponent id
	QMap<GLenum, int> m_LightsSpotExponentId;

	//! Lights spot cutoff angle id
	QMap<GLenum, int> m_LightsSpotCutoffAngleId;

	//! Lights compute distance attenuation
	QMap<GLenum, int> m_LightsComputeDistanceAttenuationId;

};

#endif /*GLC_SHADER_H_*/
