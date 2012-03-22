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

//! \file glc_objToworld.cpp implementation of the GLC_ObjToWorld class.


#include "glc_objtoworld.h"
#include "../sceneGraph/glc_world.h"
#include "glc_objmtlloader.h"
#include "../glc_fileformatexception.h"
#include "../maths/glc_geomtools.h"
#include "../sceneGraph/glc_structreference.h"
#include "../sceneGraph/glc_structinstance.h"
#include "../sceneGraph/glc_structoccurence.h"
#include <QTextStream>
#include <QFileInfo>
#include <QGLContext>

//////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////
GLC_ObjToWorld::GLC_ObjToWorld()
: m_pWorld(NULL)
, m_FileName()
, m_pMtlLoader(NULL)
, m_CurrentLineNumber(0)
, m_pCurrentObjMesh(NULL)
, m_FaceType(notSet)
, m_CurrentMeshMaterials()
, m_CurrentMaterialName("GLC_Default")
, m_ListOfAttachedFileName()
, m_Positions()
, m_Normals()
, m_Texels()
{
}

GLC_ObjToWorld::~GLC_ObjToWorld()
{
	clear();
}

/////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Create an GLC_World from an input OBJ File
GLC_World* GLC_ObjToWorld::CreateWorldFromObj(QFile &file)
{
	m_ListOfAttachedFileName.clear();
	m_FileName= file.fileName();
	//////////////////////////////////////////////////////////////////
	// Test if the file exist and can be opened
	//////////////////////////////////////////////////////////////////
	if (!file.open(QIODevice::ReadOnly))
	{
		QString message(QString("GLC_ObjToWorld::CreateWorldFromObj File ") + m_FileName + QString(" doesn't exist"));
		//qDebug() << message;
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotFound);
		throw(fileFormatException);
	}

	//////////////////////////////////////////////////////////////////
	// Init member
	//////////////////////////////////////////////////////////////////
	m_pWorld= new GLC_World;

	// Create Working variables
	int currentQuantumValue= 0;
	int previousQuantumValue= 0;
	int numberOfLine= 0;

	// Create the input file stream
	QTextStream objStream(&file);

	// QString buffer
	QString lineBuff;

	QString mtlLibLine;

	//////////////////////////////////////////////////////////////////
	// Searching mtllib attribute
	//////////////////////////////////////////////////////////////////
	while (!objStream.atEnd() && !lineBuff.contains("mtllib"))
	{
		++numberOfLine;
		lineBuff= objStream.readLine();
		if (lineBuff.contains("mtllib")) mtlLibLine= lineBuff;
	}

	//////////////////////////////////////////////////////////////////
	// Count the number of lines of the OBJ file
	//////////////////////////////////////////////////////////////////
	while (!objStream.atEnd())
	{
		++numberOfLine;
		objStream.readLine();
	}

	//////////////////////////////////////////////////////////////////
	// Reset the stream
	//////////////////////////////////////////////////////////////////
	objStream.resetStatus();
	objStream.seek(0);

	//////////////////////////////////////////////////////////////////
	// if mtl file found, load it
	//////////////////////////////////////////////////////////////////
	QString mtlLibFileName(getMtlLibFileName(mtlLibLine));
	if (!mtlLibFileName.isEmpty())
	{
		m_pMtlLoader= new GLC_ObjMtlLoader(mtlLibFileName);
		if (!m_pMtlLoader->loadMaterials())
		{
			delete m_pMtlLoader;
			m_pMtlLoader= NULL;
			if (!mtlLibLine.isEmpty())
			{
				QStringList stringList(m_FileName);
				stringList.append("Open Material File : " + mtlLibFileName + " failed");
				GLC_ErrorLog::addError(stringList);
			}
		}
		else
		{
			// Update Attached file name list
			m_ListOfAttachedFileName << mtlLibFileName;
			m_ListOfAttachedFileName << m_pMtlLoader->listOfAttachedFileName();
		}
	}
	else
	{
		//qDebug() << "GLC_ObjToWorld::CreateWorldFromObj: mtl file not found";
	}

	//////////////////////////////////////////////////////////////////
	// Read Buffer and create the world
	//////////////////////////////////////////////////////////////////
	emit currentQuantum(currentQuantumValue);
	m_CurrentLineNumber= 0;
	while (!objStream.atEnd())
	{
		++m_CurrentLineNumber;
		lineBuff= objStream.readLine();

		mergeLines(&lineBuff, &objStream);

		scanLigne(lineBuff);
		currentQuantumValue = static_cast<int>((static_cast<double>(m_CurrentLineNumber) / numberOfLine) * 100);
		if (currentQuantumValue > previousQuantumValue)
		{
			emit currentQuantum(currentQuantumValue);
		}
		previousQuantumValue= currentQuantumValue;

	}
	file.close();

	addCurrentObjMeshToWorld();

	//! Test if there is meshes in the world
	if (m_pWorld->rootOccurence()->childCount() == 0)
	{
		QString message= "GLC_ObjToWorld::CreateWorldFromObj " + m_FileName + " No mesh found!";
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::NoMeshFound);
		clear();
		throw(fileFormatException);
	}
	return m_pWorld;

}

//////////////////////////////////////////////////////////////////////
// Private services functions
//////////////////////////////////////////////////////////////////////

// Return the name of the mtl file
QString GLC_ObjToWorld::getMtlLibFileName(QString line)
{
	// Search mtl file with the same name than the OBJ file Name
	QString mtlFileName(m_FileName);
	mtlFileName.replace(m_FileName.size() - 3, 3, "mtl");
	QFile mtlFile(mtlFileName);
	if (!mtlFile.exists())// mtl file with same name not found
	{
		QTextStream stream(&line);
		QString header;
		if ((stream >> header >> mtlFileName).status() == QTextStream::Ok)
		{
			// If There is spaces in the string to extracts
			QString valueString2;
			while ((stream >> valueString2).status() == QTextStream::Ok)
			{
				mtlFileName.append(" ");
				mtlFileName.append(valueString2);
			}
			QFileInfo fileInfo(m_FileName);
			mtlFileName= fileInfo.absolutePath() + QDir::separator() + mtlFileName;
		}
		else
		{
			// There is no mtl file to load
			mtlFileName.clear();
		}
	}
	return mtlFileName;
}

// Scan a line previously extracted from OBJ file
void GLC_ObjToWorld::scanLigne(QString &line)
{
	line= line.trimmed();
	// Search Vertexs vectors
	if (line.startsWith("v ")|| line.startsWith(QString("v") + QString(QChar(9))))
	{
		line.remove(0,2); // Remove first 2 char
		m_Positions.append(extract3dVect(line));
		m_FaceType = notSet;
	}

	// Search texture coordinate vectors
	else if (line.startsWith("vt ")|| line.startsWith(QString("vt") + QString(QChar(9))))
	{
		line.remove(0,3); // Remove first 3 char
		m_Texels.append(extract2dVect(line));
		m_FaceType = notSet;
	}

	// Search normals vectors
	else if (line.startsWith("vn ") || line.startsWith(QString("vn") + QString(QChar(9))))
	{
		line.remove(0,3); // Remove first 3 char
		m_Normals.append(extract3dVect(line));
		m_FaceType = notSet;
	}

	// Search faces to update index
	else if (line.startsWith("f ") || line.startsWith(QString("f") + QString(QChar(9))))
	{
		// If there is no group or object in the OBJ file
		if (NULL == m_pCurrentObjMesh)
			{
				changeGroup("GLC_Default");
				//qDebug() << "Default group " << line;
			}
		line.remove(0,2); // Remove first 2 char
		extractFaceIndex(line);
	}

	// Search Material
	else if (line.startsWith("usemtl ") || line.startsWith(QString("usemtl") + QString(QChar(9))))
	{
		line.remove(0,7); // Remove first 7 char
		setCurrentMaterial(line);
		m_FaceType = notSet;
	}

	// Search Group
	else if (line.startsWith("g ") || line.startsWith("o ") || line.startsWith(QString("g") + QString(QChar(9)))
			|| line.startsWith(QString("o") + QString(QChar(9))))
	{
		m_FaceType = notSet;
		line.remove(0,2); // Remove first 2 char
		changeGroup(line);
	}

}
// Change current group
void GLC_ObjToWorld::changeGroup(QString line)
{
	//qDebug() << "GLC_ObjToWorld::changeGroup at Line :" << line;
	//////////////////////////////////////////////////////////////////
	// Parse the line containing the group name
	//////////////////////////////////////////////////////////////////
	QTextStream stream(&line);
	QString groupName;
	if ((stream >> groupName).status() == QTextStream::Ok)
	{
		// If There is an space in the string to extracts
		QString valueString2;
		while ((stream >> valueString2).status() == QTextStream::Ok)
		{
			groupName.append(" ");
			groupName.append(valueString2);
		}
		//////////////////////////////////////////////////////////////
		// If the groupName == "default" nothing to do
		//////////////////////////////////////////////////////////////
		if("default" != groupName)
		{
			addCurrentObjMeshToWorld();
			m_pCurrentObjMesh= new CurrentObjMesh(m_CurrentMaterialName);
			m_pCurrentObjMesh->m_pMesh->setName(groupName);

		}
	}
	else
	{
		QString message= "GLC_ObjToWorld::changeGroup " + m_FileName + " something is wrong!!";
		message.append("\nAt line : ");
		message.append(QString::number(m_CurrentLineNumber));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
		clear();
		throw(fileFormatException);
	}

}

// Extract a Vector from a string
QList<float> GLC_ObjToWorld::extract3dVect(QString &line)
{
	float x=0.0f;
	float y=0.0f;
	float z=0.0f;

	QList<float> vectResult;
	QTextStream stringVecteur(&line);

	QString xString, yString, zString;

	if (((stringVecteur >> xString >> yString >> zString).status() == QTextStream::Ok))
	{
		bool xOk, yOk, zOk;
		x= xString.toFloat(&xOk);
		y= yString.toFloat(&yOk);
		z= zString.toFloat(&zOk);
		if (!(xOk && yOk && zOk))
		{
			QString message= "GLC_ObjToWorld::extract3dVect " + m_FileName + " failed to convert vector component to float";
			message.append("\nAt ligne : ");
			message.append(QString::number(m_CurrentLineNumber));
			QStringList stringList(m_FileName);
			stringList.append(message);
			GLC_ErrorLog::addError(stringList);

			//GLC_FileFormatException fileFormatException(message, m_FileName);
			//clear();
			//throw(fileFormatException);
		}
		else
		{
			vectResult << x << y << z;
		}
	}

	return vectResult;

}

// Extract a Vector from a string
QList<float> GLC_ObjToWorld::extract2dVect(QString &line)
{
	float x=0.0f;
	float y=0.0f;
	QList<float> vectResult;
	QTextStream stringVecteur(&line);

	QString xString, yString;

	if (((stringVecteur >> xString >> yString).status() == QTextStream::Ok))
	{
		bool xOk, yOk;
		x= xString.toFloat(&xOk);
		y= yString.toFloat(&yOk);
		if (!(xOk && yOk))
		{
			QString message= "GLC_ObjToWorld::extract2dVect " + m_FileName + " failed to convert vector component to double";
			message.append("\nAt ligne : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
			clear();
			throw(fileFormatException);
		}
		vectResult << x << y;
	}

	return vectResult;
}

// Extract a face from a string
void GLC_ObjToWorld::extractFaceIndex(QString &line)
{
	QString buff;

	int coordinateIndex;
	int normalIndex;
	int textureCoordinateIndex;

	QList<GLuint> currentFaceIndex;
	//////////////////////////////////////////////////////////////////
	// Parse the line containing face index
	//////////////////////////////////////////////////////////////////
	QTextStream streamFace(&line);
	while ((!streamFace.atEnd()))
	{
		streamFace >> buff;
		extractVertexIndex(buff, coordinateIndex, normalIndex, textureCoordinateIndex);

		ObjVertice currentVertice(coordinateIndex, normalIndex, textureCoordinateIndex);
		if (m_pCurrentObjMesh->m_ObjVerticeIndexMap.contains(currentVertice))
		{
			currentFaceIndex.append(m_pCurrentObjMesh->m_ObjVerticeIndexMap.value(currentVertice));
		}
		else
		{
			// Add Vertex to the mesh bulk data
			m_pCurrentObjMesh->m_Positions.append(m_Positions.value(coordinateIndex * 3));
			m_pCurrentObjMesh->m_Positions.append(m_Positions.value(coordinateIndex * 3 + 1));
			m_pCurrentObjMesh->m_Positions.append(m_Positions.value(coordinateIndex * 3 + 2));
			if (-1 != normalIndex)
			{
				// Add Normal to the mesh bulk data
				m_pCurrentObjMesh->m_Normals.append(m_Normals.value(normalIndex * 3));
				m_pCurrentObjMesh->m_Normals.append(m_Normals.value(normalIndex * 3 + 1));
				m_pCurrentObjMesh->m_Normals.append(m_Normals.value(normalIndex * 3 + 2));
			}
			else
			{
				// Add Null Normal to the mesh bulk data
				m_pCurrentObjMesh->m_Normals.append(0.0f);
				m_pCurrentObjMesh->m_Normals.append(0.0f);
				m_pCurrentObjMesh->m_Normals.append(0.0f);
			}
			if (-1 != textureCoordinateIndex)
			{
				// Add texture coordinate to the mesh bulk data
				m_pCurrentObjMesh->m_Texels.append(m_Texels.value(textureCoordinateIndex * 2));
				m_pCurrentObjMesh->m_Texels.append(m_Texels.value(textureCoordinateIndex * 2 + 1));
			}
			else if (!m_pCurrentObjMesh->m_Texels.isEmpty())
			{
				// Add epmty texture coordinate
				m_pCurrentObjMesh->m_Texels.append(0.0f);
				m_pCurrentObjMesh->m_Texels.append(0.0f);
			}
			// Add the index to current face index
			currentFaceIndex.append(m_pCurrentObjMesh->m_NextFreeIndex);
			// Add ObjVertice to ObjVertice Map
			m_pCurrentObjMesh->m_ObjVerticeIndexMap.insert(currentVertice, m_pCurrentObjMesh->m_NextFreeIndex);
			// Increment next free index
			++(m_pCurrentObjMesh->m_NextFreeIndex);
		}

	}
	//////////////////////////////////////////////////////////////////
	// Check the number of face's vertex
	//////////////////////////////////////////////////////////////////
	const int size= currentFaceIndex.size();
	if (size < 3)
	{
		QStringList stringList(m_FileName);
		stringList.append("GLC_ObjToWorld::extractFaceIndex Face with less than 3 vertex found");
		GLC_ErrorLog::addError(stringList);
		return;
	}
	//////////////////////////////////////////////////////////////////
	// Add the face to the current mesh
	//////////////////////////////////////////////////////////////////
	if ((m_FaceType == coordinateAndNormal) || (m_FaceType == coordinateAndTextureAndNormal))
	{
		if (size > 3)
		{
			glc::triangulatePolygon(&currentFaceIndex, m_pCurrentObjMesh->m_Positions);
		}
		m_pCurrentObjMesh->m_Index.append(currentFaceIndex);
	}
	else if (m_FaceType != notSet)
	{
		if (size > 3)
		{
			glc::triangulatePolygon(&currentFaceIndex, m_pCurrentObjMesh->m_Positions);
		}
		// Comput the face normal
		if (currentFaceIndex.size() < 3) return;
		GLC_Vector3df normal= computeNormal(currentFaceIndex.at(0), currentFaceIndex.at(1), currentFaceIndex.at(2));

		// Add Face normal to bulk data
		QSet<GLuint> indexSet= currentFaceIndex.toSet();
		QSet<GLuint>::iterator iIndexSet= indexSet.begin();
		while (indexSet.constEnd() != iIndexSet)
		{
			m_pCurrentObjMesh->m_Normals[*iIndexSet * 3]= normal.x();
			m_pCurrentObjMesh->m_Normals[*iIndexSet * 3 + 1]= normal.y();
			m_pCurrentObjMesh->m_Normals[*iIndexSet * 3 + 2]= normal.z();

			++iIndexSet;
		}

		m_pCurrentObjMesh->m_Index.append(currentFaceIndex);

	}
	else
	{
		QString message= "GLC_ObjToWorld::extractFaceIndex " + m_FileName + " unknow face type";
		message.append("\nAt line : ");
		message.append(QString::number(m_CurrentLineNumber));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
		clear();
		throw(fileFormatException);
	}
}
//! Set Current material index
void GLC_ObjToWorld::setCurrentMaterial(QString &line)
{
	QTextStream streamString(&line);
	QString materialName;

	if (!((streamString >> materialName).status() == QTextStream::Ok))
	{
		QString message= "GLC_ObjToWorld::SetCurrentMaterial " + m_FileName + " : failed to extract materialName";
		message.append("\nAt line : ");
		message.append(QString::number(m_CurrentLineNumber));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}
	//////////////////////////////////////////////////////////////////
	// Check if the material is already loaded from the current mesh
	//////////////////////////////////////////////////////////////////
	if ((NULL != m_pMtlLoader) && m_pMtlLoader->contains(materialName))
	{
		if (NULL == m_pCurrentObjMesh)
		{
			changeGroup("GLC_Default");
		}
		Q_ASSERT(NULL != m_pCurrentObjMesh->m_pLastOffsetSize);

		if (m_pCurrentObjMesh->m_Index.size() != m_pCurrentObjMesh->m_pLastOffsetSize->m_Offset)
		{
			// Update last material offsetSize
			m_pCurrentObjMesh->m_pLastOffsetSize->m_size= m_pCurrentObjMesh->m_Index.size() - m_pCurrentObjMesh->m_pLastOffsetSize->m_Offset;
		}
		else
		{
			QHash<QString, MatOffsetSize*>::iterator iMat= m_pCurrentObjMesh->m_Materials.begin();
			while (m_pCurrentObjMesh->m_Materials.constEnd() != iMat)
			{
				if (iMat.value() == m_pCurrentObjMesh->m_pLastOffsetSize)
				{
					iMat= m_pCurrentObjMesh->m_Materials.erase(iMat);
				}
				else
				{
					++iMat;
				}
			}
		}
		// Create this material offsetSize
		MatOffsetSize* pMatOffsetSize= new MatOffsetSize();
		pMatOffsetSize->m_Offset= m_pCurrentObjMesh->m_Index.size();
		// Update current Obj mesh
		m_pCurrentObjMesh->m_pLastOffsetSize= pMatOffsetSize;
		m_pCurrentObjMesh->m_Materials.insertMulti(materialName, pMatOffsetSize);
		// Update current material name
		m_CurrentMaterialName= materialName;
	}

}
// Extract a vertex from a string
void GLC_ObjToWorld::extractVertexIndex(QString line, int &Coordinate, int &Normal, int &TextureCoordinate)
{
 	if (m_FaceType == notSet)
 	{
 		setObjType(line);
 	}

 	if (m_FaceType == coordinateAndTextureAndNormal)
 	{
		// Replace "/" with " "
		line.replace('/', ' ');
		QTextStream streamVertex(&line);
		QString coordinateString, textureCoordinateString, normalString;
	 	if ((streamVertex >> coordinateString >> textureCoordinateString >> normalString).status() == QTextStream::Ok)
		{
			bool coordinateOk, textureCoordinateOk, normalOk;
			Coordinate= coordinateString.toInt(&coordinateOk);
			--Coordinate;
			TextureCoordinate= textureCoordinateString.toInt(&textureCoordinateOk);
			--TextureCoordinate;
			Normal= normalString.toInt(&normalOk);
			--Normal;
			if (!(coordinateOk && textureCoordinateOk && normalOk))
			{
				QString message= "GLC_ObjToWorld::extractVertexIndex " + m_FileName + " failed to convert String to int";
				message.append("\nAt line : ");
				message.append(QString::number(m_CurrentLineNumber));
				GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
				clear();
				throw(fileFormatException);
			}
		}
		else
		{
			QString message= "GLC_ObjToWorld::extractVertexIndex Obj file " + m_FileName + " type is not supported";
			message.append("\nAt line : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
			clear();
			throw(fileFormatException);
		}

 	}
 	else if (m_FaceType == coordinateAndTexture)
 	{
		// Replace "/" with " "
		line.replace('/', ' ');
		QTextStream streamVertex(&line);
		QString coordinateString, textureCoordinateString;
	 	if ((streamVertex >> coordinateString >> textureCoordinateString).status() == QTextStream::Ok)
		{
			bool coordinateOk, textureCoordinateOk;
			Coordinate= coordinateString.toInt(&coordinateOk);
			--Coordinate;
			TextureCoordinate= textureCoordinateString.toInt(&textureCoordinateOk);
			--TextureCoordinate;
			Normal= -1;
			if (!(coordinateOk && textureCoordinateOk))
			{
				QString message= "GLC_ObjToWorld::extractVertexIndex " + m_FileName +  "failed to convert String to int";
				message.append("\nAt line : ");
				message.append(QString::number(m_CurrentLineNumber));
				GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
				clear();
				throw(fileFormatException);
			}
		}
		else
		{
			QString message= "GLC_ObjToWorld::extractVertexIndex " + m_FileName + " this Obj file type is not supported";
			message.append("\nAt line : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
			clear();
			throw(fileFormatException);
		}
 	}
 	else if (m_FaceType == coordinateAndNormal)
 	{
		// Replace "/" with " "
		line.replace('/', ' ');
		QTextStream streamVertex(&line);
		QString coordinateString, normalString;
	 	if ((streamVertex >> coordinateString >> normalString).status() == QTextStream::Ok)
		{
			bool coordinateOk, normalOk;
			Coordinate= coordinateString.toInt(&coordinateOk);
			--Coordinate;
			TextureCoordinate= -1;
			Normal= normalString.toInt(&normalOk);
			--Normal;
			if (!(coordinateOk && normalOk))
			{
				QString message= "GLC_ObjToWorld::extractVertexIndex " + m_FileName + " failed to convert String to int";
				message.append("\nAt line : ");
				message.append(QString::number(m_CurrentLineNumber));
				GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
				clear();
				throw(fileFormatException);
			}
		}
		else
		{
			QString message= "GLC_ObjToWorld::extractVertexIndex " + m_FileName + " this Obj file type is not supported";
			message.append("\nAt line : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
			clear();
			throw(fileFormatException);
		}
 	}
  	else if (m_FaceType == coordinate)
 	{
 		QTextStream streamVertex(&line);
 		QString coordinateString;
	 	if ((streamVertex >> coordinateString).status() == QTextStream::Ok)
		{
			bool coordinateOk;
			Coordinate= coordinateString.toInt(&coordinateOk);
			--Coordinate;
			TextureCoordinate= -1;
			Normal= -1;
			if (!coordinateOk)
			{
				QString message= "GLC_ObjToWorld::extractVertexIndex "  + m_FileName + " failed to convert String to int";
				message.append("\nAt line : ");
				message.append(QString::number(m_CurrentLineNumber));
				GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
				clear();
				throw(fileFormatException);
			}
		}
		else
		{
			QString message= "GLC_ObjToWorld::extractVertexIndex Obj " + m_FileName + " file type is not supported";
			message.append("\nAt line : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
			clear();
			throw(fileFormatException);
		}
 	}
 	else
 	{
		QString message= "GLC_ObjToWorld::extractVertexIndex OBJ file " + m_FileName + " not reconize";
		message.append("\nAt line : ");
		message.append(QString::number(m_CurrentLineNumber));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
		clear();
		throw(fileFormatException);
 	}
}

// set the OBJ File type
void GLC_ObjToWorld::setObjType(QString& ligne)
{
	const QRegExp coordinateOnlyRegExp("^\\d{1,}$"); // ex. 10
 	const QRegExp coordinateTextureNormalRegExp("^\\d{1,}/\\d{1,}/\\d{1,}$"); // ex. 10/30/54
 	const QRegExp coordinateNormalRegExp("^\\d{1,}//\\d{1,}$"); // ex. 10//54
 	const QRegExp coordinateTextureRegExp("^\\d{1,}/\\d{1,}$"); // ex. 10/56

 	if (coordinateTextureNormalRegExp.exactMatch(ligne))
 	{
 		m_FaceType= coordinateAndTextureAndNormal;
 	}
 	else if (coordinateTextureRegExp.exactMatch(ligne))
 	{
 		m_FaceType= coordinateAndTexture;
 	}
 	else if (coordinateNormalRegExp.exactMatch(ligne))
 	{
 		m_FaceType= coordinateAndNormal;
 	}
  	else if (coordinateOnlyRegExp.exactMatch(ligne))
 	{
 		m_FaceType= coordinate;
 	}
 	else
 	{
		QString message= "GLC_ObjToWorld::setObjType OBJ file " + m_FileName + " not reconize";
		message.append("\nAt line : ");
		message.append(QString::number(m_CurrentLineNumber));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
		clear();
		throw(fileFormatException);
 	}
}

// compute face normal
GLC_Vector3df GLC_ObjToWorld::computeNormal(GLuint index1, GLuint index2, GLuint index3)
{
	double xn, yn, zn;

	// Vertex 1
	xn= m_pCurrentObjMesh->m_Positions.at(index1 * 3);
	yn= m_pCurrentObjMesh->m_Positions.at(index1 * 3 + 1);
	zn= m_pCurrentObjMesh->m_Positions.at(index1 * 3 + 2);
	const GLC_Vector3d vect1(xn, yn, zn);

	// Vertex 2
	xn= m_pCurrentObjMesh->m_Positions.at(index2 * 3);
	yn= m_pCurrentObjMesh->m_Positions.at(index2 * 3 + 1);
	zn= m_pCurrentObjMesh->m_Positions.at(index2 * 3 + 2);
	const GLC_Vector3d vect2(xn, yn, zn);

	// Vertex 3
	xn= m_pCurrentObjMesh->m_Positions.at(index3 * 3);
	yn= m_pCurrentObjMesh->m_Positions.at(index3 * 3 + 1);
	zn= m_pCurrentObjMesh->m_Positions.at(index3 * 3 + 2);
	const GLC_Vector3d vect3(xn, yn, zn);

	const GLC_Vector3d edge1(vect3 - vect2);
	const GLC_Vector3d edge2(vect1 - vect2);

	GLC_Vector3d normal(edge1 ^ edge2);
	normal.normalize();

	return normal.toVector3df();
}

// clear objToWorld allocate memmory
void GLC_ObjToWorld::clear()
{
	m_CurrentMeshMaterials.clear();
	m_ListOfAttachedFileName.clear();

	if (NULL != m_pMtlLoader)
	{
		delete m_pMtlLoader;
		m_pMtlLoader= NULL;
	}
	if (NULL != m_pCurrentObjMesh)
	{
		delete m_pCurrentObjMesh;
		m_pCurrentObjMesh= NULL;
	}

}
// Merge Mutli line in one
void GLC_ObjToWorld::mergeLines(QString* pLineBuff, QTextStream* p0bjStream)
{
	if (pLineBuff->endsWith(QChar('\\')))
	{
		pLineBuff->replace(QChar('\\'), QChar(' '));
		pLineBuff->append(p0bjStream->readLine());
		++m_CurrentLineNumber;
		mergeLines(pLineBuff, p0bjStream);
	}
}

// Add the current Obj mesh to the world
void GLC_ObjToWorld::addCurrentObjMeshToWorld()
{
	if (NULL != m_pCurrentObjMesh)
	{
		if (!m_pCurrentObjMesh->m_Positions.isEmpty())
		{
			m_pCurrentObjMesh->m_pMesh->addVertice(m_pCurrentObjMesh->m_Positions.toVector());
			m_pCurrentObjMesh->m_Positions.clear();
			m_pCurrentObjMesh->m_pMesh->addNormals(m_pCurrentObjMesh->m_Normals.toVector());
			m_pCurrentObjMesh->m_Normals.clear();
			if (!m_pCurrentObjMesh->m_Texels.isEmpty())
			{
				m_pCurrentObjMesh->m_pMesh->addTexels(m_pCurrentObjMesh->m_Texels.toVector());
				m_pCurrentObjMesh->m_Texels.clear();
			}
			QHash<QString, MatOffsetSize*>::iterator iMat= m_pCurrentObjMesh->m_Materials.begin();
			while (m_pCurrentObjMesh->m_Materials.constEnd() != iMat)
			{
				GLC_Material* pCurrentMaterial= NULL;
				if ((NULL != m_pMtlLoader) && (m_pMtlLoader->contains(iMat.key())))
				{
					pCurrentMaterial= m_pMtlLoader->material(iMat.key());
				}
				// Create the list of triangles to add to the mesh
				const int offset= iMat.value()->m_Offset;
				int size= iMat.value()->m_size;
				if (0 == size)
				{
					size= m_pCurrentObjMesh->m_Index.size() - offset;
				}
				//qDebug() << "Offset : " << offset << " size : " << size;
				QList<GLuint> triangles;
				for (int i= offset; i < (offset + size); ++i)
				{
					triangles.append(m_pCurrentObjMesh->m_Index.at(i));
				}
				// Add the list of triangle to the mesh
				if (!triangles.isEmpty())
				{
					m_pCurrentObjMesh->m_pMesh->addTriangles(pCurrentMaterial, triangles);
				}

				++iMat;
			}
			if (m_pCurrentObjMesh->m_pMesh->faceCount(0) > 0)
			{
				m_pCurrentObjMesh->m_pMesh->finish();
				GLC_3DRep* pRep= new GLC_3DRep(m_pCurrentObjMesh->m_pMesh);
				m_pWorld->rootOccurence()->addChild((new GLC_StructInstance(pRep)));
			}
			else
			{
				delete m_pCurrentObjMesh->m_pMesh;
			}

		}
		else
		{
			delete m_pCurrentObjMesh->m_pMesh;
		}

		delete m_pCurrentObjMesh;
		m_pCurrentObjMesh= NULL;
	}
}


