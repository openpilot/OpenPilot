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

//! \file glc_objmtlloader.cpp implementation of the GLC_ObjMtlLoader class.

#include "glc_objmtlloader.h"
#include "../glc_fileformatexception.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QtDebug>
#include <QGLContext>

GLC_ObjMtlLoader::GLC_ObjMtlLoader(const QString& fileName)
: m_FileName(fileName)
, m_pCurrentMaterial(NULL)
, m_Materials()
, m_LoadStatus()
, m_ListOfAttachedFileName()
{
}

GLC_ObjMtlLoader::~GLC_ObjMtlLoader()
{
	// Remove unused material
	QHash<QString, GLC_Material*>::iterator i;
	for (i= m_Materials.begin(); i != m_Materials.end(); ++i)
	{
		if (i.value()->isUnused()) delete i.value();
	}
	m_Materials.clear();
	m_ListOfAttachedFileName.clear();
}
/////////////////////////////////////////////////////////////////////
// Get functions
//////////////////////////////////////////////////////////////////////
// Get a material from is name
GLC_Material* GLC_ObjMtlLoader::material(const QString& materialName)
{
	if (m_Materials.contains(materialName))
	{
		return m_Materials[materialName];
	}
	else
	{
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////
// Set functions
//////////////////////////////////////////////////////////////////////
// Load the materials
bool GLC_ObjMtlLoader::loadMaterials()
{

	// Create the input file stream
	QFile mtlFile(m_FileName);

	if (!mtlFile.open(QIODevice::ReadOnly))
	{
		qDebug() << "GLC_ObjMtlLoader::LoadMaterial File " << m_FileName << " doesn't exist";
		return false;
	}
	else
	{
		//qDebug() << "GLC_ObjMtlLoader::LoadMaterial OK File " << m_FileName << " exist";
	}

	QTextStream mtlStream(&mtlFile);

	QString lineBuff;
	QString header;

	while (!mtlStream.atEnd())
	{
		lineBuff= mtlStream.readLine();
		//qDebug() << lineBuff;
		QTextStream streamLine(lineBuff.toAscii());

		if ((streamLine >> header).status() ==QTextStream::Ok)
		{

			// Search New material
			if (header =="newmtl")
			{
				//qDebug() << "New material find";

				if (NULL != m_pCurrentMaterial)
				{	// It's not the first material
					//qDebug() << "Add material : " << m_pCurrentMaterial->name();
					processMayaSpecific();
					m_Materials.insert(m_pCurrentMaterial->name(), m_pCurrentMaterial);
					m_pCurrentMaterial= NULL;
				}

				m_pCurrentMaterial= new GLC_Material;
				if (!extractMaterialName(lineBuff)) return false;
				//qDebug() << "New Material " << m_pCurrentMaterial->name();

			}
			else if ((header == "Ka") || (header == "Kd") || (header == "Ks")) // ambiant, diffuse and specular color
			{
				if (!extractRGBValue(lineBuff)) return false;
			}

			else if ((header == "Ns") || (header == "d"))	// shiness Or transparency
			{
				if (!extractOneValue(lineBuff)) return false;
			}
			else if ((header == "map_Kd") || (header == "map_Ka"))	// Texture
			{
				//qDebug() << "Texture detected";
				extractTextureFileName(lineBuff);
			}

		}
	}

	if (NULL != m_pCurrentMaterial)
	{
		//qDebug() << "Add material : " << m_pCurrentMaterial->name();
		m_Materials.insert(m_pCurrentMaterial->name(), m_pCurrentMaterial);
		m_pCurrentMaterial= NULL;
	}

	mtlFile.close();
	return true;

}
/////////////////////////////////////////////////////////////////////
// Private services functions
//////////////////////////////////////////////////////////////////////

// Extract the material name
bool GLC_ObjMtlLoader::extractMaterialName(QString &ligne)
{
	bool result= false;
	QTextStream stream(&ligne);
	QString valueString;
	QString header;
	if ((stream >> header >> valueString).status() == QTextStream::Ok)
	{
		// If There is an space in the string to extracts
		QString valueString2;
		while ((stream >> valueString2).status() == QTextStream::Ok)
		{
			valueString.append(" ");
			valueString.append(valueString2);
		}
		m_pCurrentMaterial->setName(valueString);
		//qDebug() << "Material name is : " << valueString;
		result= true;
	}
	else
	{
		m_LoadStatus= "GLC_ObjMtlLoader::extractMaterialName : something is wrong!!";
		result= false;
	}
	return result;
}
// Extract the texture file name
void GLC_ObjMtlLoader::extractTextureFileName(QString &ligne)
{
	QTextStream stream(&ligne);
	QString valueString;
	QString header;
	if ((stream >> header >> valueString).status() == QTextStream::Ok)
	{
		// Retrieve the .obj file path
		QFileInfo fileInfo(m_FileName);

		QString textureFileName(fileInfo.absolutePath() + QDir::separator());
		// concatenate File Path with texture filename
		textureFileName.append(getTextureName(stream, valueString));

		QFile textureFile(textureFileName);

		if (!textureFile.open(QIODevice::ReadOnly))
		{
			QStringList stringList(m_FileName);
			stringList.append("Open File : " + textureFileName + " failed");
			GLC_ErrorLog::addError(stringList);
		}
		else if ((textureFileName.right(3).contains("TGA", Qt::CaseInsensitive)))
		{
			QStringList stringList(m_FileName);
			stringList.append("Image : " + textureFileName + " not suported");
			GLC_ErrorLog::addError(stringList);
		}
		else
		{
			m_ListOfAttachedFileName << textureFileName;
			// Create the texture and assign it to current material
			GLC_Texture *pTexture = new GLC_Texture(textureFile);
			m_pCurrentMaterial->setTexture(pTexture);
			//qDebug() << "Texture File is : " << valueString;
		}
		textureFile.close();
	}
}

// Extract RGB value
bool GLC_ObjMtlLoader::extractRGBValue(QString &ligne)
{
	bool result= false;
	QTextStream stream(&ligne);
	QString header;
	QString rColor, gColor, bColor;
	QColor color(Qt::white);

	if ((stream >> header >> rColor >> gColor >> bColor).status() == QTextStream::Ok)
	{
		bool okr, okg, okb;
		color.setRedF(rColor.toDouble(&okr));
		color.setGreenF(gColor.toDouble(&okg));
		color.setBlueF(bColor.toDouble(&okb));
		if (!(okr && okg && okb))
		{
			m_LoadStatus= "GLC_ObjMtlLoader::ExtractRGBValue : Wrong format of rgb color value!!";
			qDebug() << m_LoadStatus;
			result= false;
		}
		else
		{
			color.setAlphaF(1.0);
			if (header == "Ka") // Ambiant Color
			{
				m_pCurrentMaterial->setAmbientColor(color);
				//qDebug() << "Ambiant Color : " <<  color.redF() << " " << color.greenF() << " " << color.blueF();
				result= true;
			}

			else if (header == "Kd") // Diffuse Color
			{
				m_pCurrentMaterial->setDiffuseColor(color);
				//qDebug() << "Diffuse Color : " <<  color.redF() << " " << color.greenF() << " " << color.blueF();
				result= true;
			}

			else if (header == "Ks") // Specular Color
			{
				m_pCurrentMaterial->setSpecularColor(color);
				//qDebug() << "Specular Color : " <<  color.redF() << " " << color.greenF() << " " << color.blueF();
				result= true;
			}

			else
			{
				m_LoadStatus= "GLC_ObjMtlLoader::ExtractRGBValue : something is wrong!!";
				result= false;
			}
		}

	}else
	{
		m_LoadStatus= "GLC_ObjMtlLoader::ExtractRGBValue : something is wrong!!";
		qDebug() << m_LoadStatus;
		result= false;
	}

	return result;

}

// Extract One value
bool GLC_ObjMtlLoader::extractOneValue(QString &ligne)
{
	QTextStream stream(&ligne);
	QString valueString;
	QString header;
	GLfloat value;

	if ((stream >> header >> valueString).status() == QTextStream::Ok)
	{
		if (header == "Ns") // Ambient color
		{
			bool ok;
			value= valueString.toFloat(&ok);
			if (!ok)
			{
				m_LoadStatus= "GLC_ObjMtlLoader::ExtractOneValue : Wrong format of Shiness !";
				qDebug() << m_LoadStatus;
				return false;
			}
			m_pCurrentMaterial->setShininess(value);
			return true;
		}
		else if (header == "d") // Transparancy
		{
			bool ok;
			value= valueString.toFloat(&ok);
			if (!ok)
			{
				m_LoadStatus= "GLC_ObjMtlLoader::ExtractOneValue : Wrong format Transparency!";
				qDebug() << m_LoadStatus;
				return false;
			}
			m_pCurrentMaterial->setOpacity(static_cast<qreal>(value));
			return true;
		}

		else
		{
			m_LoadStatus= "GLC_ObjMtlLoader::ExtractOneValue : Ambient Color not found!!";
			qDebug() << m_LoadStatus;
			return false;
		}
	}
	else
	{
		m_LoadStatus= "GLC_ObjMtlLoader::ExtractOneValue : something is wrong!!";
		qDebug() << m_LoadStatus;
		GLC_FileFormatException fileFormatException(m_LoadStatus, m_FileName, GLC_FileFormatException::WrongFileFormat);
		return false;
	}

}

// Get texture file name without parameters
QString GLC_ObjMtlLoader::getTextureName(QTextStream &inputStream, const QString &input)
{
	QString textureName(input);
	int numberOfStringToSkip= 0;
	// Check if there is a map parameter and count
	if ((input == "-o") || (input == "-s") || (input == "-t"))
	{
		numberOfStringToSkip= 3;
	}
	else if (input == "-mm")
	{
		numberOfStringToSkip= 2;
	}
	else if ((input == "-blendu") || (input == "-blendv") || (input == "-cc")
			|| (input == "-clamp") || (input == "-texres"))
	{
		numberOfStringToSkip= 1;
	}

	if (numberOfStringToSkip != 0)
	{
		// Skip unread map parameters
		for (int i= 0; i < numberOfStringToSkip; ++i)
		{
			inputStream >> textureName;
		}

		if ((inputStream >> textureName).status() == QTextStream::Ok)
		{
			textureName= getTextureName(inputStream, textureName);
		}
		else
		{
			m_LoadStatus== "GLC_ObjToMesh2::extractString : Error occur when trying to decode map option";
			GLC_FileFormatException fileFormatException(m_LoadStatus, m_FileName, GLC_FileFormatException::WrongFileFormat);
			throw(fileFormatException);
		}
	}
	return textureName;
}
// Process Maya specific obj
void GLC_ObjMtlLoader::processMayaSpecific()
{
	// Test if the current material have a texture
	if (m_pCurrentMaterial->hasTexture())
	{
		// Test if the diffuse color of material is black
		if (m_pCurrentMaterial->diffuseColor() == Qt::black)
		{
			// Change the material's diffuse color in order to see the texture
			m_pCurrentMaterial->setDiffuseColor(Qt::lightGray);
		}
	}
}
