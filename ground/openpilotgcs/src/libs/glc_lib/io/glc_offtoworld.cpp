
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

//! \file glc_offtoworld.cpp implementation of the GLC_OffToWorld class.

#include "glc_offtoworld.h"
#include "../sceneGraph/glc_world.h"
#include "../glc_fileformatexception.h"
#include "../sceneGraph/glc_structreference.h"
#include "../sceneGraph/glc_structinstance.h"
#include "../sceneGraph/glc_structoccurence.h"

#include <QTextStream>
#include <QFileInfo>
#include <QGLContext>

GLC_OffToWorld::GLC_OffToWorld()
: m_pWorld(NULL)
, m_FileName()
, m_CurrentLineNumber(0)
, m_pCurrentMesh(NULL)
, m_CurVertexIndex(0)
, m_NbrOfVertexs(0)
, m_NbrOfFaces(0)
, m_IsCoff(false)
, m_Is4off(false)
, m_PositionBulk()
, m_NormalBulk()
, m_ColorBulk()
, m_IndexList()
{

}

GLC_OffToWorld::~GLC_OffToWorld()
{
	clear();
}

/////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Create an GLC_World from an input OFF File
GLC_World* GLC_OffToWorld::CreateWorldFromOff(QFile &file)
{
	clear();
	m_FileName= file.fileName();
	//////////////////////////////////////////////////////////////////
	// Test if the file exist and can be opened
	//////////////////////////////////////////////////////////////////
	if (!file.open(QIODevice::ReadOnly))
	{
		QString message(QString("GLC_OffToWorld::CreateWorldFromOff File ") + m_FileName + QString(" doesn't exist"));
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

	// Create the input file stream
	QTextStream offStream(&file);

	// QString buffer
	QString lineBuff;

	//////////////////////////////////////////////////////////////////
	// Check the OFF Header
	//////////////////////////////////////////////////////////////////
	// Check if the file begin with "OFF" or "COFF"
	++m_CurrentLineNumber;
	lineBuff= offStream.readLine();
	lineBuff= lineBuff.trimmed();
	if(offStream.atEnd() || (!lineBuff.startsWith("OFF") && !lineBuff.startsWith("COFF") && !lineBuff.startsWith("4OFF")))
	{
		QString message= "GLC_OffToWorld::CreateWorldFromOff : OFF or COFF header not found";
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
		clear();
		throw(fileFormatException);
	}

	// Set the COFF flag
	m_IsCoff= lineBuff.startsWith("COFF");

	// Set the 4OFF flag
	m_Is4off= lineBuff.startsWith("4OFF");

	// Create the mesh
	m_pCurrentMesh= new GLC_Mesh();

	// Set mesh color per vertex if needed
	if (m_IsCoff)
	{
		m_pCurrentMesh->setColorPearVertex(true);
	}
	// Get the number of vertex and faces and skip comments
	++m_CurrentLineNumber;
	lineBuff= offStream.readLine();
	lineBuff= lineBuff.trimmed();
	while(!offStream.atEnd() && lineBuff.startsWith(QChar('#')))
	{
		++m_CurrentLineNumber;
		lineBuff= offStream.readLine();
		lineBuff= lineBuff.trimmed();
	}
	extractNbrVertexsAndNbrFaces(lineBuff);

	//////////////////////////////////////////////////////////////////
	// Read Buffer and load vertexs
	//////////////////////////////////////////////////////////////////
	emit currentQuantum(currentQuantumValue);

	for (int currentVertex= 0; currentVertex < m_NbrOfVertexs; ++currentVertex)
	{
		// Check it the end of file has been reached
		if(offStream.atEnd())
		{
			QString message= "GLC_OffToWorld::CreateWorldFromOff : This file seems to be incomplete";
			message.append("\nAt ligne : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
			clear();
			throw(fileFormatException);
		}

		++m_CurrentLineNumber;
		lineBuff= offStream.readLine();
		// Skip empty line
		while (lineBuff.isEmpty())
		{
			++m_CurrentLineNumber;
			lineBuff= offStream.readLine();
		}
		// Add current vertex and color if needed to the mesh
		extractVertex(lineBuff);

		// Update Current Quantum for progress bar usage.
		currentQuantumValue = static_cast<int>((static_cast<double>(currentVertex) / (m_NbrOfVertexs + m_NbrOfFaces)) * 100);
		if (currentQuantumValue > previousQuantumValue)
		{
			emit currentQuantum(currentQuantumValue);
		}
		previousQuantumValue= currentQuantumValue;
	}

	//////////////////////////////////////////////////////////////////
	// Read Buffer and load faces
	//////////////////////////////////////////////////////////////////
	for (int currentFace= 0; currentFace < m_NbrOfFaces; ++currentFace)
	{
		// Check it the end of file has been reached
		if(offStream.atEnd())
		{
			QString message= "GLC_OffToWorld::CreateWorldFromOff : This file seems to be incomplete";
			message.append("\nAt ligne : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
			clear();
			throw(fileFormatException);
		}

		++m_CurrentLineNumber;
		lineBuff= offStream.readLine();
		while (lineBuff.isEmpty())
		{
			++m_CurrentLineNumber;
			lineBuff= offStream.readLine();
		}

		// Add current Face to the mesh
		extractFaceIndex(lineBuff);

		// Update Current Quantum for progress bar usage.
		currentQuantumValue = static_cast<int>((static_cast<double>(currentFace + m_NbrOfVertexs + 1) / (m_NbrOfVertexs + m_NbrOfFaces)) * 100);
		if (currentQuantumValue > previousQuantumValue)
		{
			emit currentQuantum(currentQuantumValue);
		}
		previousQuantumValue= currentQuantumValue;

	}

	file.close();
	// Compute mesh normals
	computeNormal();

	m_pCurrentMesh->addVertice(m_PositionBulk.toVector());
	m_pCurrentMesh->addNormals(m_NormalBulk.toVector());
	if (!m_ColorBulk.isEmpty())
	{
		m_pCurrentMesh->addColors(m_ColorBulk.toVector());
	}
	m_pCurrentMesh->addTriangles(NULL, m_IndexList);

	m_pCurrentMesh->finish();
	GLC_3DRep* pRep= new GLC_3DRep(m_pCurrentMesh);
	m_pCurrentMesh= NULL;
	m_pWorld->rootOccurence()->addChild(new GLC_StructOccurence(pRep));

	return m_pWorld;
}

/////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

// clear offToWorld allocate memmory and reset member
void GLC_OffToWorld::clear()
{
	if (NULL != m_pCurrentMesh)
	{
		delete m_pCurrentMesh;
		m_pCurrentMesh= NULL;
	}
	m_pWorld= NULL;
	m_FileName.clear();
	m_CurrentLineNumber= 0;
	m_pCurrentMesh= NULL;
	m_CurVertexIndex= 0;
	m_NbrOfVertexs= 0;
	m_NbrOfFaces= 0;
	m_IsCoff= false;
	m_Is4off= false;
	m_PositionBulk.clear();
	m_NormalBulk.clear();
	m_ColorBulk.clear();
}

// Extract a Vertex from a string and add color component if needed
void GLC_OffToWorld::extractVertex(QString &line)
{

	QTextStream stringVecteur(&line);

	QString xString, yString, zString;

	if (((stringVecteur >> xString >> yString >> zString).status() == QTextStream::Ok))
	{
		bool xOk, yOk, zOk;
		m_PositionBulk.append(xString.toFloat(&xOk));
		m_PositionBulk.append(yString.toFloat(&yOk));
		m_PositionBulk.append(zString.toFloat(&zOk));
		if (!(xOk && yOk && zOk))
		{
			QString message= "GLC_OffToWorld::extractVertex : failed to convert vertex component to float";
			message.append("\nAt ligne : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
			clear();
			throw(fileFormatException);
		}
		if (m_Is4off)
		{
			QString wString;
			if (((stringVecteur >> wString).status() == QTextStream::Ok))
			{
				float w;
				bool wOk;
				w= wString.toFloat(&wOk);
				if (!wOk)
				{
					QString message= "GLC_OffToWorld::extractVertex : failed to convert vertex fourth component to float";
					message.append("\nAt ligne : ");
					message.append(QString::number(m_CurrentLineNumber));
					GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
					clear();
					throw(fileFormatException);
				}
				const int lastValue= m_PositionBulk.size() - 1;
				m_PositionBulk[lastValue - 2]= m_PositionBulk.at(lastValue - 2) / w;
				m_PositionBulk[lastValue - 1]= m_PositionBulk.at(lastValue - 1) / w;
				m_PositionBulk[lastValue]= m_PositionBulk.at(lastValue) / w;
			}
			else
			{
				QString message= "GLC_OffToWorld::extractVertex : failed to read vector fourth component";
				message.append("\nAt ligne : ");
				message.append(QString::number(m_CurrentLineNumber));
				GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
				clear();
				throw(fileFormatException);
			}
		}

		// Test if the file is a COFF
		if (m_IsCoff)
		{
			QString rString, gString, bString, aString;

			if (((stringVecteur >> rString >> gString >> bString >> aString).status() == QTextStream::Ok))
			{
				bool rOk, gOk, bOk, aOk;
				m_ColorBulk.append(rString.toFloat(&rOk));
				m_ColorBulk.append(gString.toFloat(&gOk));
				m_ColorBulk.append(bString.toFloat(&bOk));
				m_ColorBulk.append(aString.toFloat(&aOk));
				if (!(rOk && gOk && bOk && aOk))
				{
					QString message= "GLC_OffToWorld::extractVertex : failed to convert color component to float";
					message.append("\nAt ligne : ");
					message.append(QString::number(m_CurrentLineNumber));
					GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
					clear();
					throw(fileFormatException);
				}
			}
			else
			{
				QString message= "GLC_OffToWorld::extractVertex : failed to read vertex color";
				message.append("\nAt ligne : ");
				message.append(QString::number(m_CurrentLineNumber));
				GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
				clear();
				throw(fileFormatException);
			}
		}

	}
	else
	{
		QString message= "GLC_OffToWorld::extractVertex : failed to read vector component";
		message.append("\nAt ligne : ");
		message.append(QString::number(m_CurrentLineNumber));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}

}

// Extract Number off Vertex and faces
void GLC_OffToWorld::extractNbrVertexsAndNbrFaces(QString &line)
{
	QTextStream stringVecteur(&line);

	QString xString, yString;

	if (((stringVecteur >> xString >> yString).status() == QTextStream::Ok))
	{
		bool xOk, yOk;
		m_NbrOfVertexs= xString.toInt(&xOk);
		m_NbrOfFaces= yString.toInt(&yOk);
		if (!(xOk && yOk))
		{
			QString message= "GLC_OffToWorld::extractNbrVertexsAndNbrFaces : failed to convert text to int";
			message.append("\nAt ligne : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
			clear();
			throw(fileFormatException);
		}
	}
	else
	{
		QString message= "GLC_OffToWorld::extractNbrVertexsAndNbrFaces : failed to extract nbr of vertexs/faces";
		message.append("\nAt ligne : ");
		message.append(QString::number(m_CurrentLineNumber));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);

	}
}

// Extract a face from a string
void GLC_OffToWorld::extractFaceIndex(QString &line)
{
	QString buff;

	QList<GLuint> indexList;

	//////////////////////////////////////////////////////////////////
	// Parse the line containing face index
	//////////////////////////////////////////////////////////////////
	QTextStream streamFace(&line);
	// Get the number of vertex
	if((streamFace >> buff).status() != QTextStream::Ok)
	{
		QString message= "GLC_OffToWorld::extractFaceIndex failed to extract number of vertex index";
		message.append("\nAt line : ");
		message.append(QString::number(m_CurrentLineNumber));
		message.append(QString("\n") + line);
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}
	bool conversionOk;
	// Convert the QString Number of vertex to int
	int numberOfVertex= buff.toInt(&conversionOk);
	if (!conversionOk)
	{
		QString message= "GLC_OffToWorld::extractFaceIndex failed to convert String to int";
		message.append("\nAt line : ");
		message.append(QString::number(m_CurrentLineNumber));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}
	// Extract the face's vertexs index
	for (int i= 0; i < numberOfVertex; ++i)
	{
		// Get a vertex index
		if((streamFace >> buff).status() != QTextStream::Ok)
		{
			QString message= "GLC_OffToWorld::extractFaceIndex failed to extract vertex index";
			message.append("\nAt line : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
			clear();
			throw(fileFormatException);
		}
		// Convert the QString vertex index into int
		int index= buff.toInt(&conversionOk);
		if (conversionOk)
		{
			indexList.append(index);
		}
		else
		{
			QString message= "GLC_ObjToWorld::extractFaceIndex failed to convert String to int";
			message.append("\nAt line : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
			clear();
			throw(fileFormatException);
		}
	}

	// Trying to read face color
	QString rString, gString, bString;
	if((streamFace >> rString >> gString >> bString).status() == QTextStream::Ok)
	{
		// Fill color bulk data if needed
		if (m_ColorBulk.isEmpty())
		{
			const int size= m_PositionBulk.size() / 3 * 4;
			for (int i= 0; i < size; ++i)
			{
				m_PositionBulk.append(0.0);
			}
		}
		float r, g, b;
		bool rOk, gOk, bOk;
		r= rString.toFloat(&rOk);
		g= gString.toFloat(&gOk);
		b= bString.toFloat(&bOk);
		if (!rOk || !gOk || !bOk)
		{
			QString message= "GLC_ObjToWorld::extractFaceIndex failed to convert String to float";
			message.append("\nAt line : ");
			message.append(QString::number(m_CurrentLineNumber));
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
			clear();
			throw(fileFormatException);
		}
		for (int i= 0; i < numberOfVertex; ++i)
		{
			m_ColorBulk[indexList[i] * 4]= r;
			m_ColorBulk[indexList[i] * 4 + 1]= g;
			m_ColorBulk[indexList[i] * 4 + 2]= b;
			m_ColorBulk[indexList[i] * 4 + 3]= 1.0f;
		}
	}

	// Add the face to index List
	m_IndexList.append(indexList);
}

// compute face normal
void GLC_OffToWorld::computeNormal()
{
	//qDebug() << "GLC_OffToWorld::computeNormal";
	//qDebug() << "Position size= " << m_PositionBulk.size();
	//qDebug() << "Normal size= " << m_NormalBulk.size();
	const QList<float>* pData= &m_PositionBulk;
	// Fill the list of normal
	QList<float>* pNormal= &m_NormalBulk;
	const int normalCount= pData->size();
	for (int i= 0; i < normalCount; ++i)
	{
		pNormal->append(0.0f);
	}
	// Compute the normals and add them to the current mesh info
	const int size= m_IndexList.size();
	double xn, yn, zn;


	for (int i= 0; i < size; i+=3)
	{
		// Vertex 1
		xn= pData->at(m_IndexList.at(i) * 3);
		yn= pData->at(m_IndexList.at(i) * 3 + 1);
		zn= pData->at(m_IndexList.at(i) * 3 + 2);
		const GLC_Vector3d vect1(xn, yn, zn);

		// Vertex 2
		xn= pData->at(m_IndexList.at(i + 1) * 3);
		yn= pData->at(m_IndexList.at(i + 1) * 3  + 1);
		zn= pData->at(m_IndexList.at(i + 1) * 3 + 2);
		const GLC_Vector3d vect2(xn, yn, zn);

		// Vertex 3
		xn= pData->at(m_IndexList.at(i + 2) * 3);
		yn= pData->at(m_IndexList.at(i + 2) * 3 + 1);
		zn= pData->at(m_IndexList.at(i + 2) * 3 + 2);
		const GLC_Vector3d vect3(xn, yn, zn);

		const GLC_Vector3d edge1(vect3 - vect2);
		const GLC_Vector3d edge2(vect1 - vect2);

		GLC_Vector3d normal(edge1 ^ edge2);
		normal.normalize();

		GLC_Vector3df curNormal= normal.toVector3df();
		for (int curVertex= 0; curVertex < 3; ++curVertex)
		{
			(*pNormal)[m_IndexList.at(i + curVertex) * 3]= curNormal.x();
			(*pNormal)[m_IndexList.at(i + curVertex) * 3 + 1]= curNormal.y();
			(*pNormal)[m_IndexList.at(i + curVertex) * 3 + 2]= curNormal.z();
		}
	}

}

