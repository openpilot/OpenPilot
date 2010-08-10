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

//! \file glc_shader.cpp implementation of the GLC_Shader class.

#include "glc_shader.h"
#include <QTextStream>
#include <QMutexLocker>
#include "../glc_exception.h"
#include "../glc_state.h"

// Static member initialization
QStack<GLuint> GLC_Shader::m_ProgrammStack;

GLuint GLC_Shader::m_CurrentProgramm= 0;

QMutex GLC_Shader::m_Mutex;


GLC_Shader::GLC_Shader()
: m_VertexByteArray()
, m_VertexShader(0)
, m_FragmentByteArray()
, m_FragmentShader(0)
, m_ProgramShader(0)
, m_Name("Empty Shader")
{
	qDebug() << "Create Shader";
}

// Construct shader with specifie vertex and fragment
GLC_Shader::GLC_Shader(QFile& vertex, QFile& fragment)
: m_VertexByteArray()
, m_VertexShader(0)
, m_FragmentByteArray()
, m_FragmentShader(0)
, m_ProgramShader(0)
, m_Name()
{
	qDebug() << "Create Shader";
	setVertexAndFragmentShader(vertex, fragment);
}

// Copy constructor
GLC_Shader::GLC_Shader(const GLC_Shader& shader)
: m_VertexByteArray(shader.m_VertexByteArray)
, m_VertexShader(0)
, m_FragmentByteArray(shader.m_FragmentByteArray)
, m_FragmentShader(0)
, m_ProgramShader(0)
, m_Name(shader.m_Name)
{
	qDebug() << "Create Shader";
	if (0 != shader.m_ProgramShader)
	{
		createAndCompileProgrammShader();
	}
}

GLC_Shader::~GLC_Shader()
{
	qDebug() << "GLC_Shader::~GLC_Shader";
	deleteShader();
}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return true if the shader can be deleted
bool GLC_Shader::canBeDeleted() const
{
	return m_CurrentProgramm != m_ProgramShader;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Use this shader programm
void GLC_Shader::use()
{
	if (GLC_State::isInSelectionMode()) return;
	// Program shader must be valid
	Q_ASSERT(0 != m_ProgramShader);
	// Test if it is a valid program shader
	if (glIsProgram(m_ProgramShader) != GL_TRUE)
	{
		QString message("GLC_Shader::use() m_ProgramShader is not a valid program shader ");
		GLC_Exception exception(message);
		throw(exception);
	}

	QMutexLocker locker(&m_Mutex);
	// Test if the program shader is not already the current one
	if (m_CurrentProgramm != m_ProgramShader)
	{
		if (m_CurrentProgramm != 0)
		{
			m_ProgrammStack.push(m_CurrentProgramm);
		}
		m_CurrentProgramm= m_ProgramShader;
		glUseProgram(m_CurrentProgramm);
	}

}

// Use specified program shader
void GLC_Shader::use(GLuint shaderId)
{
	if (GLC_State::isInSelectionMode()) return;
	//qDebug() << "GLC_Shader::use(GLuint shaderId)";
	// Test if the program shader is not already the current one
	if (m_CurrentProgramm != shaderId)
	{
		if (m_CurrentProgramm != 0)
		{
			m_ProgrammStack.push(shaderId);
		}
		m_CurrentProgramm= shaderId;
		glUseProgram(m_CurrentProgramm);
	}
	else if (glIsProgram(shaderId) != GL_TRUE)	// Test if it is a valid program shader
	{
		QString message("GLC_Shader::use(GLuint id) id is not a valid program shader ");
		GLC_Exception exception(message);
		throw(exception);
	}

}


// Use previous program shader
void GLC_Shader::unuse()
{
	if (GLC_State::isInSelectionMode()) return;

	QMutexLocker locker(&m_Mutex);
	if (m_ProgrammStack.isEmpty())
	{
		m_CurrentProgramm= 0;
	}
	else
	{
		m_CurrentProgramm= m_ProgrammStack.pop();
	}
	glUseProgram(m_CurrentProgramm);
}
// Compile and attach shader to a program shader
void GLC_Shader::createAndCompileProgrammShader()
{
	Q_ASSERT(0 == m_ProgramShader);

	createAndLinkVertexShader();
	createAndLinkFragmentShader();

	m_ProgramShader = glCreateProgram();
	glAttachShader(m_ProgramShader, m_VertexShader);
	glAttachShader(m_ProgramShader, m_FragmentShader);

	glLinkProgram(m_ProgramShader);

	// Check if the program as been linked successfully
	GLint params;
	glGetProgramiv(m_ProgramShader, GL_LINK_STATUS, &params);
	if (params != GL_TRUE)
	{
		QString message("GLC_Shader::setVertexAndFragmentShader Failed to link program ");
		GLC_Exception exception(message);
		throw(exception);
	}
}

// Create and compile vertex shader
void GLC_Shader::createAndLinkVertexShader()
{
	const char* pVertexShaderData= m_VertexByteArray.data();

	m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_VertexShader, 1, &pVertexShaderData, NULL);
	glCompileShader(m_VertexShader);

	// Check if the shader compilation is successful
	GLint params;
	glGetShaderiv(m_VertexShader, GL_COMPILE_STATUS, &params);
	if (params != GL_TRUE)
	{
		QString message("GLC_Shader::createAndLinkVertexShader Failed to compile Vertex shader");
		GLC_Exception exception(message);
		throw(exception);
	}
}
//Delete the shader
void GLC_Shader::deleteShader()
{
	qDebug() << "delete Shader";
	if (m_ProgramShader != 0)
	{
		// Test if the shader is the current one
		if (m_CurrentProgramm == m_ProgramShader)
		{
			qDebug() << "Warning deleting current shader";
		}
		//removing shader id from the stack
		if (m_ProgrammStack.contains(m_ProgramShader))
		{
			int indexToDelete= m_ProgrammStack.indexOf(m_ProgramShader);
			while (indexToDelete != -1)
			{
				m_ProgrammStack.remove(indexToDelete);
				indexToDelete= m_ProgrammStack.indexOf(m_ProgramShader);
			}
		}
			// Detach shader associated with the program
		glDetachShader(m_ProgramShader, m_VertexShader);
		glDetachShader(m_ProgramShader, m_FragmentShader);
		// Delete the shader
		glDeleteShader(m_VertexShader);
		m_VertexShader= 0;
		glDeleteShader(m_FragmentShader);
		m_FragmentShader= 0;
		// Delete the program
		glDeleteProgram(m_ProgramShader);
		m_ProgramShader= 0;
	}

}

// Create and compile fragment shader
void GLC_Shader::createAndLinkFragmentShader()
{
	const char* pFragmentShaderData= m_FragmentByteArray.data();

	m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_FragmentShader, 1, &pFragmentShaderData, NULL);
	glCompileShader(m_FragmentShader);

	// Check if the shader compilation is successful
	GLint params;
	glGetShaderiv(m_FragmentShader, GL_COMPILE_STATUS, &params);
	if (params != GL_TRUE)
	{
		QString message("GLC_Shader::createAndLinkFragmentShader Failed to compile fragment shader");
		GLC_Exception exception(message);
		throw(exception);
	}
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set Vertex and fragment shader
void GLC_Shader::setVertexAndFragmentShader(QFile& vertexFile, QFile& fragmentFile)
{
	m_Name= QFileInfo(vertexFile).baseName();
	setVertexShader(vertexFile);
	setFragmentShader(fragmentFile);
}

// Replace this shader by a copy of another shader
void GLC_Shader::replaceShader(const GLC_Shader& sourceShader)
{
	Q_ASSERT(isUsable() == sourceShader.isUsable());

	// Test if the source shader is the same than this shader
	if (this == &sourceShader)
	{
		return;
	}
	m_VertexByteArray= sourceShader.m_VertexByteArray;
	m_FragmentByteArray= sourceShader.m_FragmentByteArray;

	if (isUsable())
	{
		const GLuint oldShaderId= m_ProgramShader;

		// Detach shader associated with the program
		glDetachShader(m_ProgramShader, m_VertexShader);
		glDetachShader(m_ProgramShader, m_FragmentShader);
		// Delete the shader
		glDeleteShader(m_VertexShader);
		glDeleteShader(m_FragmentShader);
		// Delete the program
		glDeleteProgram(m_ProgramShader);

		// Init shader ID
		m_ProgramShader= 0;
		m_VertexShader= 0;
		m_FragmentShader= 0;

		// Rebuilt shader
		createAndCompileProgrammShader();

		// Update the shader program stack
		if (m_ProgrammStack.contains(oldShaderId))
		{
			int indexToReplace= m_ProgrammStack.indexOf(oldShaderId);
			while (indexToReplace != -1)
			{
				m_ProgrammStack.replace(indexToReplace, m_ProgramShader);
				indexToReplace= m_ProgrammStack.indexOf(oldShaderId);
			}
		}
		// Check the value of current shader
		if (oldShaderId == m_CurrentProgramm)
		{
			m_CurrentProgramm= m_ProgramShader;
			// Use the new One
			glUseProgram(m_CurrentProgramm);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
// Set Vertex shader
void GLC_Shader::setVertexShader(QFile& vertexFile)
{
	Q_ASSERT(0 == m_ProgramShader);
	m_VertexByteArray= readShaderFile(vertexFile);
}

// Set fragment shader
void GLC_Shader::setFragmentShader(QFile& fragmentFile)
{
	Q_ASSERT(0 == m_ProgramShader);
	m_FragmentByteArray= readShaderFile(fragmentFile);
}

// Return char* of an Ascii file
QByteArray GLC_Shader::readShaderFile(QFile& shaderFile)
{
	if (!shaderFile.open(QIODevice::ReadOnly))
	{
		QString message(QString("GLC_Shader::readShaderFile Failed to open file : ") + shaderFile.fileName());
		GLC_Exception exception(message);
		throw(exception);
	}
	QByteArray result(shaderFile.readAll());
	result.append('\0');
	return result;
}

