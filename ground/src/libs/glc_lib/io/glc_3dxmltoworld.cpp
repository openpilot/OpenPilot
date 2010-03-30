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

//! \file glc_3dxmltoworld.cpp implementation of the GLC_3dxmlToWorld class.

#include "glc_3dxmltoworld.h"
#include "../sceneGraph/glc_world.h"
#include "../glc_fileformatexception.h"
#include "../geometry/glc_extendedmesh.h"

// Quazip library
#include "../quazip/quazip.h"
#include "../quazip/quazipfile.h"

#include <QString>
#include <QGLContext>
#include <QFileInfo>

GLC_3dxmlToWorld::GLC_3dxmlToWorld(const QGLContext* pContext)
: QObject()
, m_pQGLContext(pContext)
, m_pStreamReader(NULL)
, m_FileName()
, m_p3dxmlArchive(NULL)
, m_p3dxmlFile(NULL)
, m_pCurrentFile(NULL)
, m_RootName()
, m_pWorld(NULL)
, m_ReferenceHash()
, m_AssyLinkList()
, m_InstanceOf()
, m_SetOfExtRef()
, m_InstanceOfExtRefHash()
, m_ExternalReferenceHash()
, m_MaterialHash()
, m_IsInArchive(false)
, m_ReferenceRepHash()
, m_LocalRepLinkList()
, m_ExternRepLinkList()
, m_SetOfExtRep()
, m_pCurrentMaterial(NULL)
, m_TextureImagesHash()
, m_LoadStructureOnly(false)
, m_ListOfAttachedFileName()
{

}

GLC_3dxmlToWorld::~GLC_3dxmlToWorld()
{
	delete m_pStreamReader;
	m_pStreamReader= NULL;

	// Clear current file
	if (NULL != m_pCurrentFile)
	{
		m_pCurrentFile->close();
		delete m_pCurrentFile;
		m_pCurrentFile= NULL;
	}

	// Clear the 3dxml file
	if (NULL != m_p3dxmlFile)
	{
		m_p3dxmlFile->close();
		delete m_p3dxmlFile;
		m_p3dxmlFile= NULL;
	}

	// Clear the 3dxml archive
	if (NULL != m_p3dxmlArchive)
	{
		m_p3dxmlArchive->close();
		delete m_p3dxmlArchive;
		m_p3dxmlArchive= NULL;
	}
	clearMaterialHash();
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Create an GLC_World from an input 3DXML File
GLC_World* GLC_3dxmlToWorld::CreateWorldFrom3dxml(QFile &file, bool structureOnly)
{
	m_LoadStructureOnly= structureOnly;
	m_FileName= file.fileName();

	// Create the 3dxml Zip archive
	m_p3dxmlArchive= new QuaZip(m_FileName);
	// Trying to load archive
	if(not m_p3dxmlArchive->open(QuaZip::mdUnzip))
	{
	  // In this case, the 3dxml is not compressed or is not valid
	  m_RootName= m_FileName;
	  delete m_p3dxmlArchive;
	  m_p3dxmlArchive= NULL;
	}
	else
	{
		m_IsInArchive= true;
		// Set the file Name Codec
		m_p3dxmlArchive->setFileNameCodec("IBM866");

		// Load the manifest
		loadManifest();
	}

	if (not m_LoadStructureOnly)
	{
		// Trying to Load CATRepImage file
		loadCatRepImage();

		// Trying to Load CATRefMaterial File
		loadCatMaterialRef();
	}

	// Load the product structure
	loadProductStructure();


	emit currentQuantum(100);
	return m_pWorld;
}

// Create 3DRep from an 3DXML rep
GLC_3DRep GLC_3dxmlToWorld::Create3DrepFrom3dxmlRep(const QString& fileName)
{
	GLC_3DRep resultRep;
	QString entryFileName;
	if (fileName.left(5) == QString("Zip::"))
	{
		m_FileName= fileName;
		m_FileName.remove("Zip::");
		m_FileName= m_FileName.left(m_FileName.indexOf("::"));

		// Create the 3dxml Zip archive
		m_p3dxmlArchive= new QuaZip(m_FileName);
		// Trying to load archive
		if(not m_p3dxmlArchive->open(QuaZip::mdUnzip))
		{
		  delete m_p3dxmlArchive;
		  return GLC_3DRep();
		}
		else
		{
			m_IsInArchive= true;
			// Set the file Name Codec
			m_p3dxmlArchive->setFileNameCodec("IBM866");
		}
		entryFileName= fileName;
		entryFileName.remove(QString("Zip::") + m_FileName + "::");

	}
	else
	{
		entryFileName= fileName;
	}
	setStreamReaderToFile(entryFileName);
	if (QFileInfo(entryFileName).suffix().toLower() == "3dxml")
	{
		GLC_StructReference* pStructRef= createReferenceRep(QString());
		GLC_3DRep* pRep= NULL;
		if (( NULL != pStructRef) and pStructRef->hasRepresentation())
		{
			pRep= dynamic_cast<GLC_3DRep*>(pStructRef->representationHandle());
		}
		if (NULL != pRep)
		{
			resultRep= GLC_3DRep(*pRep);
			resultRep.setName(pStructRef->name());
		}
	}
	else if (QFileInfo(entryFileName).suffix().toLower() == "3drep")
	{
		resultRep= loadCurrentExtRep();
	}
	resultRep.removeEmptyGeometry();
	return resultRep;
}

//////////////////////////////////////////////////////////////////////
// Private services functions
//////////////////////////////////////////////////////////////////////
// Load the 3dxml's manifest
void GLC_3dxmlToWorld::loadManifest()
{
	setStreamReaderToFile("Manifest.xml");
	m_RootName= getContent("Root");

	if (m_pStreamReader->atEnd() or m_pStreamReader->hasError())
	{
		QString message(QString("GLC_3dxmlToWorld::loadManifest Manifest file ") + m_FileName + " doesn't contains Root Element");
		qDebug() << message;
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}

	delete m_pStreamReader;
	m_pStreamReader= NULL;

	m_p3dxmlFile->close();
}

//! Close all files and clear memmory
void GLC_3dxmlToWorld::clear()
{
	delete m_pWorld;
	m_pWorld= NULL;

	delete m_pStreamReader;
	m_pStreamReader= NULL;

	// Clear current file
	if (NULL != m_pCurrentFile)
	{
		m_pCurrentFile->close();
		delete m_pCurrentFile;
		m_pCurrentFile= NULL;
	}

	// Clear the 3dxml file
	if (NULL != m_p3dxmlFile)
	{
		m_p3dxmlFile->close();
		delete m_p3dxmlFile;
		m_p3dxmlFile= NULL;
	}

	// Clear the 3dxml archive
	if (NULL != m_p3dxmlArchive)
	{
		m_p3dxmlArchive->close();
		delete m_p3dxmlArchive;
		m_p3dxmlArchive= NULL;
	}

	clearMaterialHash();
}

// Go to Element
void GLC_3dxmlToWorld::goToElement(const QString& elementName)
{
	while(startElementNotReached(elementName))
	{
		m_pStreamReader->readNext();
	}
}

// Go to a Rep of a xml
void GLC_3dxmlToWorld::goToRepId(const QString& id)
{
	while(not m_pStreamReader->atEnd() and not ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "Representation")
			and (m_pStreamReader->attributes().value("id").toString() == id)))
	{
		m_pStreamReader->readNext();
	}

}

// Go to Polygonal Rep Type
void GLC_3dxmlToWorld::gotToPolygonalRepType()
{
	while(not m_pStreamReader->atEnd() and not ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "Rep")
			and (m_pStreamReader->attributes().value("xsi:type").toString() == "PolygonalRepType")))
	{
		//qDebug() << m_pStreamReader->name();
		//qDebug() << m_pStreamReader->attributes().value("xsi:type").toString();
		m_pStreamReader->readNext();
	}

}

// Return the content of an element
QString GLC_3dxmlToWorld::getContent(const QString& element)
{
	QString Content;
	while(endElementNotReached(element))
	{
		m_pStreamReader->readNext();
		if (m_pStreamReader->isCharacters() and not m_pStreamReader->text().isEmpty())
		{
			Content+= m_pStreamReader->text().toString();
		}
	}

	return Content.trimmed();
}

// Read the specified attribute
QString GLC_3dxmlToWorld::readAttribute(const QString& name, bool required)
{
	QString attributeValue;
	if (required and not m_pStreamReader->attributes().hasAttribute(name))
	{
		QString message(QString("required attribute ") + name + QString(" Not found"));
		qDebug() << message;
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}
	else
	{
		attributeValue= m_pStreamReader->attributes().value(name).toString();
	}
	return attributeValue;
}

// Load the product structure
void GLC_3dxmlToWorld::loadProductStructure()
{
	setStreamReaderToFile(m_RootName);
	goToElement("ProductStructure");
	if (m_pStreamReader->atEnd() or m_pStreamReader->hasError())
	{
		QString message(QString("GLC_3dxmlToWorld::loadProductStructure Element ProctStructure Not found in ") + m_FileName);
		qDebug() << message;
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}

	// Load the structure
	while(endElementNotReached("ProductStructure"))
	{
		if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
				and ((m_pStreamReader->name() == "Reference3D") or (m_pStreamReader->name() == "Instance3D")
						or (m_pStreamReader->name() == "ReferenceRep") or (m_pStreamReader->name() == "InstanceRep")))
		{
			if (m_pStreamReader->name() == "Reference3D") loadReference3D();
			else if (m_pStreamReader->name() == "Instance3D") loadInstance3D();
			else if (m_pStreamReader->name() == "ReferenceRep") loadReferenceRep();
			else loadInstanceRep();
		}

		m_pStreamReader->readNext();
	}

	// Check if an error Occur
	if (m_pStreamReader->hasError())
	{
		QString message(QString("GLC_3dxmlToWorld::loadProductStructure An error occur in ") + m_FileName);
		qDebug() << message;
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}

	// Load local representation ref
	loadLocalRepresentations();

	// Load external ref
	loadExternalRef3D();

	// Load extern representations
	loadExternRepresentations();

	{ // Link locals instance with reference
		InstanceOfHash::iterator iInstance= m_InstanceOf.begin();
		while (iInstance != m_InstanceOf.constEnd())
		{
			GLC_StructInstance* pInstance= iInstance.key();
			GLC_StructReference* pRef= m_ReferenceHash.value(iInstance.value());
			pInstance->setReference(pRef);

			++iInstance;
		}
		m_InstanceOf.clear();

	}
	//qDebug() << "Local instance linked with reference";

	{ // Link external instance with reference

		InstanceOfExtRefHash::iterator iInstance= m_InstanceOfExtRefHash.begin();
		while (iInstance != m_InstanceOfExtRefHash.constEnd())
		{
			GLC_StructInstance* pInstance= iInstance.key();
			GLC_StructReference* pRef;
			if (m_ExternalReferenceHash.contains(iInstance.value()))
			{
				pRef= m_ExternalReferenceHash.value(iInstance.value());
			}
			else
			{
				QString referenceName= pInstance->name();
				referenceName= referenceName.left(pInstance->name().lastIndexOf('.'));
				//qDebug() << referenceName;
				pRef= new GLC_StructReference(referenceName);
			}

			pInstance->setReference(pRef);

			++iInstance;
		}
		m_InstanceOfExtRefHash.clear();
		m_ExternalReferenceHash.clear();

	}
	//qDebug() << "external instance linked with reference";

	// Create the unfolded tree
	createUnfoldedTree();

	//qDebug() << "Unfolded tree created";

}

// Load a Reference3D
void GLC_3dxmlToWorld::loadReference3D()
{
	const unsigned int id= readAttribute("id", true).toUInt();
	const QString refName(readAttribute("name", true));
	GLC_StructReference* pStructReference;

	if (id == 1) // This is the root reference.
	{
		m_pWorld= new GLC_World();
		m_pWorld->setRootName(refName);
		pStructReference= m_pWorld->rootOccurence()->structInstance()->structReference();
		pStructReference->setName(refName);
	}
	else
	{
		pStructReference= new GLC_StructReference(refName);
	}
	// Try to find extension
	GLC_Attributes userAttributes;
	while (endElementNotReached("Reference3D"))
	{
		if (m_pStreamReader->isStartElement() and (m_pStreamReader->name() == "Reference3DExtensionType"))
		{
			while (endElementNotReached("Reference3DExtensionType"))
			{
				if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "Attribute"))
				{
					QString name= readAttribute("name", true);
					QString value= readAttribute("value", true);
					userAttributes.insert(name, value);
				}
				m_pStreamReader->readNext();
			}
		}
		m_pStreamReader->readNext();
	}
	if (not userAttributes.isEmpty())
	{
		pStructReference->setAttributes(userAttributes);
	}

	m_ReferenceHash.insert(id, pStructReference);
}

// Load a Instance3D
void GLC_3dxmlToWorld::loadInstance3D()
{
	const QString local= "urn:3DXML:Reference:loc:";
	const QString externRef= "urn:3DXML:Reference:ext:";

	const QString instName(readAttribute("name", false));
	const unsigned int aggregatedById= getContent("IsAggregatedBy").toUInt();
	QString instanceOf= getContent("IsInstanceOf");
	const QString matrixString= getContent("RelativeMatrix");
	GLC_Matrix4x4 instanceMatrix= loadMatrix(matrixString);


	GLC_StructInstance* pStructInstance= new GLC_StructInstance(instName);
	pStructInstance->move(instanceMatrix);

	// Try to find extension
	GLC_Attributes userAttributes;
	while (endElementNotReached("Instance3D"))
	{
		if (m_pStreamReader->isStartElement() and (m_pStreamReader->name() == "Instance3DExtensionType"))
		{
			while (endElementNotReached("Instance3DExtensionType"))
			{
				if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "Attribute"))
				{
					QString name= readAttribute("name", true);
					QString value= readAttribute("value", true);
					userAttributes.insert(name, value);
				}
				m_pStreamReader->readNext();
			}
		}
		m_pStreamReader->readNext();
	}
	if (not userAttributes.isEmpty())
	{
		pStructInstance->setAttributes(userAttributes);
	}

	if (instanceOf.contains(externRef))
	{
		const QString extRefId= instanceOf.remove(externRef).remove("#1");
		m_SetOfExtRef << extRefId;
		m_InstanceOfExtRefHash.insert(pStructInstance, extRefId);
	}
	else if (instanceOf.contains(local))
	{
		const unsigned int refId= instanceOf.remove(local).toUInt();
		m_InstanceOf.insert(pStructInstance, refId);
	}
	else
	{
		// 3dvia 3dxml
		const unsigned int refId= instanceOf.toUInt();
		m_InstanceOf.insert(pStructInstance, refId);
	}

	AssyLink assyLink;
	assyLink.m_ParentRefId= aggregatedById;
	assyLink.m_pChildInstance= pStructInstance;

	m_AssyLinkList.append(assyLink);
}

// Load a Reference representation
void GLC_3dxmlToWorld::loadReferenceRep()
{
	const QString local= "urn:3DXML:Representation:loc:";
	const QString externName= "urn:3DXML:";

	const unsigned int id= readAttribute("id", true).toUInt();
	const QString refName(readAttribute("name", true));
	QString associatedFile(readAttribute("associatedFile", true));

	if (associatedFile.contains(local))
	{
		const QString repId= associatedFile.remove(local);
		m_ReferenceRepHash.insert(id, repId);
	}
	else if (associatedFile.contains(externName))
	{
		const QString repId= associatedFile.remove(externName);
		m_ReferenceRepHash.insert(id, repId);
	}
}

// Load a Instance representation
void GLC_3dxmlToWorld::loadInstanceRep()
{
	const QString local= "urn:3DXML:Reference:loc:";

	const QString instName(readAttribute("name", true));
	const unsigned int aggregatedById= getContent("IsAggregatedBy").toUInt();
	QString instanceOf= getContent("IsInstanceOf");

	if (instanceOf.contains(local))
	{
		// The 3dxml is a 3dxml rep from CATIA V5
		const unsigned int refId= instanceOf.remove(local).toUInt();

		RepLink repLink;
		repLink.m_ReferenceId= aggregatedById;
		repLink.m_RepId= refId;

		m_LocalRepLinkList.append(repLink);
	}
	else
	{
		// The 3dxml is a 3dvia 3dxml
		const unsigned int refId= instanceOf.toUInt();
		RepLink repLink;
		repLink.m_ReferenceId= aggregatedById;
		repLink.m_RepId= refId;

		m_ExternRepLinkList.append(repLink);
	}
}

// Load External Ref
void GLC_3dxmlToWorld::loadExternalRef3D()
{
	if (m_SetOfExtRef.isEmpty()) return;

	const int size= m_SetOfExtRef.size();
	int previousQuantumValue= 0;
	int currentQuantumValue= 0;
	int currentFileIndex= 0;
	emit currentQuantum(currentQuantumValue);

	SetOfExtRef::iterator iExtRef= m_SetOfExtRef.begin();
	while (iExtRef != m_SetOfExtRef.constEnd())
	{

		const QString currentRefFileName= (*iExtRef);
		//qDebug() << "Current File name : " << currentRefFileName;
		// Get the refFile of the 3dxml
		if (not m_LoadStructureOnly and setStreamReaderToFile(currentRefFileName))
		{
			GLC_StructReference* pCurrentRef= createReferenceRep();
			if (NULL != pCurrentRef)
			{
				m_ExternalReferenceHash.insert(currentRefFileName, pCurrentRef);
			}
			else
			{
				qDebug() << "GLC_3dxmlToWorld::loadExternalRef3D No File Found";
			}
		}
		else if(m_LoadStructureOnly)
		{
			GLC_3DRep* pRep= new GLC_3DRep();
			if (m_IsInArchive)
			{
				pRep->setFileName("Zip::" + m_FileName + "::" + currentRefFileName);
			}
			else
			{
				const QString repFileName= QFileInfo(m_FileName).absolutePath() + QDir::separator() + currentRefFileName;
				pRep->setFileName(repFileName);
				m_ListOfAttachedFileName << repFileName;
			}
			GLC_StructReference* pCurrentRef= new GLC_StructReference(pRep);
			pCurrentRef->setName(QFileInfo(currentRefFileName).baseName());
			m_ExternalReferenceHash.insert(currentRefFileName, pCurrentRef);
		}
		else
		{
			qDebug() << "GLC_3dxmlToWorld::loadExternalRef3D No File Found";
		}

		++currentFileIndex;
		// Progrees bar indicator
		currentQuantumValue = static_cast<int>((static_cast<double>(currentFileIndex) / size) * 100);
		if (currentQuantumValue > previousQuantumValue)
		{
			emit currentQuantum(currentQuantumValue);
		}
		previousQuantumValue= currentQuantumValue;

		++iExtRef;
	}
	m_SetOfExtRef.clear();

}

// Create Instance from 3DXML Rep
GLC_StructReference* GLC_3dxmlToWorld::createReferenceRep(QString repId)
{
	QString refName;

	if (repId.isEmpty())
	{
		goToElement("ProductStructure");
		checkForXmlError("Element ProductStructure not found");

		goToElement("Reference3D");
		checkForXmlError("Element Reference3D not found");
		refName= readAttribute("name", true);

		goToElement("ReferenceRep");
		checkForXmlError("Element ReferenceRep not found");

		const QString format(readAttribute("format", true));
		if (format != "TESSELLATED")
		{
			QString message(QString("GLC_3dxmlToWorld::addExtenalRef 3D rep format ") + format + " Not Supported");
			qDebug() << message;
			GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
			clear();
			throw(fileFormatException);
		}

		repId= readAttribute("associatedFile");

		const QString local= "urn:3DXML:Representation:loc:";
		const QString ext= "urn:3DXML:Representation:ext:";
		if (repId.contains(ext))
		{
			repId.remove(ext);
			repId.resize(repId.size() - 2);
			if (setStreamReaderToFile(repId))
			{
				return createReferenceRep();
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			repId.remove(local);
		}

		checkForXmlError("attribute associatedFile not found");
		goToRepId(repId);
		checkForXmlError("repId not found");
	}

	GLC_ExtendedMesh* pMesh= new GLC_ExtendedMesh();
	pMesh->setName(refName);
	GLC_3DRep currentMesh3DRep(pMesh);

	int numberOfMesh= 1;
	while (endElementNotReached("Representation"))
	{
		gotToPolygonalRepType();
		if (m_pStreamReader->atEnd() or m_pStreamReader->hasError())
		{
			if (m_pStreamReader->hasError())
			{
				QString message(QString("An element have not been found in file ") + m_FileName);
				qDebug() << m_pStreamReader->errorString();
				GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
				clear();
				throw(fileFormatException);
			}
			else
			{
				if (numberOfMesh > 1)
				{
					pMesh->finished();
					return new GLC_StructReference(new GLC_3DRep(currentMesh3DRep));
				}
				else
				{
					return new GLC_StructReference(refName);
				}
			}
		}
		if (numberOfMesh > 1)
		{
			pMesh->finished();
			pMesh = new GLC_ExtendedMesh();
			pMesh->setName(refName);
			currentMesh3DRep.addGeom(pMesh);
		}

		// Get the master lod accuracy
		double masteLodAccuracy= readAttribute("accuracy", false).toDouble();

		loadLOD(pMesh);
		if (m_pStreamReader->atEnd() or m_pStreamReader->hasError())
		{
			//qDebug() << " Master LOD not found";
			return new GLC_StructReference("Empty Rep");
		}

		// Load Faces index data
		while (endElementNotReached("Faces"))
		{
			m_pStreamReader->readNext();
			if ( m_pStreamReader->name() == "Face")
			{
				loadFace(pMesh, 0, masteLodAccuracy);
			}
		}
		checkForXmlError("End of Faces not found");

		goToElement("VertexBuffer");
		checkForXmlError("Element VertexBuffer not found");

		{
			QString verticePosition= getContent("Positions").replace(',', ' ');
			//qDebug() << "Position " << verticePosition;
			checkForXmlError("Error while retrieving Position ContentVertexBuffer");
			// Load Vertice position
			QTextStream verticeStream(&verticePosition);
			QList<GLfloat> verticeValues;
			QString buff;
			while ((!verticeStream.atEnd()))
			{
				verticeStream >> buff;
				verticeValues.append(buff.toFloat());
			}
			pMesh->addVertices(verticeValues.toVector());

		}

		{
			QString normals= getContent("Normals").replace(',', ' ');
			//qDebug() << "Normals " << normals;
			checkForXmlError("Error while retrieving Normals values");
			// Load Vertice Normals
			QTextStream normalsStream(&normals);
			QList<GLfloat> normalValues;
			QString buff;
			while ((!normalsStream.atEnd()))
			{
				normalsStream >> buff;
				normalValues.append(buff.toFloat());
			}
			pMesh->addNormals(normalValues.toVector());
		}

		// Try to find texture coordinate
		while (endElementNotReached("VertexBuffer"))
		{
			if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "TextureCoordinates"))
			{
				QString texels= getContent("TextureCoordinates").replace(',', ' ');
				checkForXmlError("Error while retrieving Texture coordinates");
				QTextStream texelStream(&texels);
				QList<GLfloat> texelValues;
				QString buff;
				while ((!texelStream.atEnd()))
				{
					texelStream >> buff;
					texelValues.append(buff.toFloat());
				}
				pMesh->addTexels(texelValues.toVector());
			}
			m_pStreamReader->readNext();
		}

		++numberOfMesh;
	}

	pMesh->finished();

	currentMesh3DRep.removeEmptyGeometry();
	if (not currentMesh3DRep.isEmpty())
	{
		return new GLC_StructReference(new GLC_3DRep(currentMesh3DRep));
	}
	else
	{
		return new GLC_StructReference("Empty Rep");
	}
}

// Load Matrix
GLC_Matrix4x4 GLC_3dxmlToWorld::loadMatrix(const QString& stringMatrix)
{

	QStringList stringValues(stringMatrix.split(" "));

	if (stringValues.size() != 12) return GLC_Matrix4x4();

	QVector<double> values(16);
	// Rotation
	values[0]= stringValues[0].toDouble();
	values[1]= stringValues[1].toDouble();
	values[2]= stringValues[2].toDouble();
	values[3]= 0.0;
	values[4]= stringValues[3].toDouble();
	values[5]= stringValues[4].toDouble();
	values[6]= stringValues[5].toDouble();
	values[7]= 0.0;
	values[8]= stringValues[6].toDouble();
	values[9]= stringValues[7].toDouble();
	values[10]= stringValues[8].toDouble();
	values[11]= 0.0;
	// Translation
	values[12]= stringValues[9].toDouble();
	values[13]= stringValues[10].toDouble();
	values[14]= stringValues[11].toDouble();
	values[15]= 1.0;

	return GLC_Matrix4x4(values.data());
}

// Create the unfolded  tree
void GLC_3dxmlToWorld::createUnfoldedTree()
{
	//qDebug() << "createUnfoldedTree";
	// Run throw all link in the list of link
	AssyLinkList::iterator iLink= m_AssyLinkList.begin();
	while(iLink != m_AssyLinkList.constEnd())
	{
		GLC_StructInstance* pChildInstance= (*iLink).m_pChildInstance;
		if (pChildInstance->structReference() == NULL)
		{
			pChildInstance->setReference(new GLC_StructReference("Part"));
		}
		Q_ASSERT(m_ReferenceHash.contains((*iLink).m_ParentRefId));
		GLC_StructReference* pRef= m_ReferenceHash.value((*iLink).m_ParentRefId);
		// Attach pInstance at all Occurence of pRef
		QList<GLC_StructInstance*> instanceList= pRef->listOfStructInstances();
		const int instanceNumber= instanceList.size();
		for (int i= 0; i < instanceNumber; ++i)
		{
			if (instanceList.at(i)->hasStructOccurence())
			{
				QList<GLC_StructOccurence*> occurenceList= instanceList.at(i)->listOfStructOccurences();
				const int occurenceNumber= occurenceList.size();
				for (int j= 0; j < occurenceNumber; ++j)
				{
					if (pChildInstance->hasStructOccurence() and pChildInstance->firstOccurenceHandle()->isOrphan())
					{
						Q_ASSERT(pChildInstance->listOfStructOccurences().size() == 1);
						occurenceList.at(j)->addChild(pChildInstance->firstOccurenceHandle());
					}
					else
					{
						occurenceList.at(j)->addChild(pChildInstance);
					}
				}
			}
			else
			{
				GLC_StructOccurence* pOccurence= new GLC_StructOccurence(m_pWorld->worldHandle(), instanceList.at(i));

				if (pChildInstance->hasStructOccurence() and pChildInstance->firstOccurenceHandle()->isOrphan())
				{
					Q_ASSERT(pChildInstance->listOfStructOccurences().size() == 1);
					pOccurence->addChild(pChildInstance->firstOccurenceHandle());
				}
				else
				{
					pOccurence->addChild(pChildInstance);
				}
			}
		}

		++iLink;
	}
	m_AssyLinkList.clear();
	m_ReferenceHash.clear();

	// Update position
	m_pWorld->rootOccurence()->updateChildrenAbsoluteMatrix();

}
// Check for XML error
void GLC_3dxmlToWorld::checkForXmlError(const QString& info)
{
	if (m_pStreamReader->atEnd() or m_pStreamReader->hasError())
	{
		QString message(QString("An element have not been found in file ") + m_FileName);
		qDebug() << info << " " << m_pStreamReader->errorString() << "  " << message;
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}
}
// Go to the master LOD
void GLC_3dxmlToWorld::loadLOD(GLC_ExtendedMesh* pMesh)
{
	int lodIndex= 1;
	while(not m_pStreamReader->atEnd() and not ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "Faces")))
	{
		m_pStreamReader->readNext();
		if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "SurfaceAttributes"))
		{
			while (endElementNotReached("SurfaceAttributes"))
			{
				m_pStreamReader->readNext();
				if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "MaterialId"))
				{
					//qDebug() << m_p3dxmlFile->getActualFileName();
					checkForXmlError("Material ID not found");
					QString materialId= readAttribute("id", true).remove("urn:3DXML:CATMaterialRef.3dxml#");
					m_pCurrentMaterial= m_MaterialHash.value(materialId);
				}
			}
		}
		else if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "PolygonalLOD"))
		{
			double accuracy= readAttribute("accuracy", true).toDouble();
			// Load Faces index data
			while (endElementNotReached("Faces"))
			{
				m_pStreamReader->readNext();
				if ( m_pStreamReader->name() == "Face")
				{
					loadFace(pMesh, lodIndex, accuracy);
				}
			}
			checkForXmlError("End of Faces not found");
			++lodIndex;
		}
	}
}
// Load a face
void GLC_3dxmlToWorld::loadFace(GLC_ExtendedMesh* pMesh, const int lod, double accuracy)
{
	//qDebug() << "GLC_3dxmlToWorld::loadFace" << m_pStreamReader->name();
	// List of index declaration
	QString triangles= readAttribute("triangles", false).trimmed();
	QString strips= readAttribute("strips", false).trimmed();
	QString fans= readAttribute("fans", false).trimmed();

	if (triangles.isEmpty() and strips.isEmpty() and fans.isEmpty())
	{
		QString message(QString("Empty face Found ") + m_FileName);
		qDebug() << message << " " << m_pStreamReader->errorString();
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}

	GLC_Material* pCurrentMaterial= NULL;

	while(endElementNotReached("Face"))
	{
		if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "Color"))
		{
			pCurrentMaterial= getMaterial();
		}
		m_pStreamReader->readNext();
	}
	if (NULL == pCurrentMaterial)
	{
		pCurrentMaterial= m_pCurrentMaterial;
	}

	// Trying to find triangles
	if (not triangles.isEmpty())
	{
		// For 3dvia mesh
		if (triangles.contains(','))
		{
			triangles.remove(',');
		}
		QTextStream trianglesStream(&triangles);
		IndexList trianglesIndex;
		QString buff;
		while ((!trianglesStream.atEnd()))
		{
			trianglesStream >> buff;
			trianglesIndex.append(buff.toUInt());
		}
		pMesh->addTriangles(pCurrentMaterial, trianglesIndex, lod, accuracy);
	}
	// Trying to find trips
	if (not strips.isEmpty())
	{

		QStringList stripsList(strips.split(","));
		const int size= stripsList.size();
		for (int i= 0; i < size; ++i)
		{
			//qDebug() << "Strip " << stripsList.at(i);
			QTextStream stripsStream(&stripsList[i]);
			IndexList stripsIndex;
			QString buff;
			while ((!stripsStream.atEnd()))
			{
				stripsStream >> buff;
				stripsIndex.append(buff.toUInt());
			}
			pMesh->addTrianglesStrip(pCurrentMaterial, stripsIndex, lod, accuracy);
		}
	}
	// Trying to find fans
	if (not fans.isEmpty())
	{
		QStringList fansList(fans.split(","));
		const int size= fansList.size();
		for (int i= 0; i < size; ++i)
		{
			QTextStream fansStream(&fansList[i]);
			IndexList fansIndex;
			QString buff;
			while ((!fansStream.atEnd()))
			{
				fansStream >> buff;
				fansIndex.append(buff.toUInt());
			}
			pMesh->addTrianglesFan(pCurrentMaterial, fansIndex, lod, accuracy);
		}
	}

}

// Clear material hash
void GLC_3dxmlToWorld::clearMaterialHash()
{
	MaterialHash::iterator iMaterial= m_MaterialHash.begin();
	while (m_MaterialHash.constEnd() != iMaterial)
	{
		if (iMaterial.value()->isUnused())
		{
			delete iMaterial.value();
		}
		++iMaterial;
	}

	m_MaterialHash.clear();
}

// get material
GLC_Material* GLC_3dxmlToWorld::getMaterial()
{
	GLC_Material* pMaterial= NULL;
	const QString red(readAttribute("red", true));
	const QString green(readAttribute("green", true));
	const QString blue(readAttribute("blue", true));

	const QString matKey= red + green + blue;
	if (m_MaterialHash.contains(matKey))
	{
		pMaterial= m_MaterialHash.value(matKey);
	}
	else
	{
		qreal redReal= red.toDouble();
		qreal greenReal= green.toDouble();
		qreal blueReal= blue.toDouble();
		QColor diffuse;
		diffuse.setRgbF(redReal, greenReal, blueReal);
		pMaterial= new GLC_Material(diffuse);
		pMaterial->setName("Material_" + QString::number(m_MaterialHash.size()));
		pMaterial->setAmbientColor(Qt::black);
		pMaterial->setSpecularColor(Qt::white);
		pMaterial->setShininess(25.0);
		m_MaterialHash.insert(matKey, pMaterial);
	}

	return pMaterial;
}

// Set the stream reader to the specified file
bool GLC_3dxmlToWorld::setStreamReaderToFile(QString fileName, bool test)
{
	if (m_IsInArchive)
	{
		delete m_p3dxmlFile;
		// Create QuaZip File
		m_p3dxmlFile= new QuaZipFile(m_p3dxmlArchive);

		// Get the file of the 3dxml
		if (not m_p3dxmlArchive->setCurrentFile(fileName, QuaZip::csInsensitive))
		{
			if (not test)
			{
				QString message(QString("GLC_3dxmlToWorld::setStreamReaderToFile File ") + m_FileName + " doesn't contains " + fileName);
				qDebug() << message;
				GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
				clear();
				throw(fileFormatException);
			}
			else return false;
		}

		// Open the file of the 3dxml
		if(not m_p3dxmlFile->open(QIODevice::ReadOnly))
	    {
			QString message(QString("GLC_3dxmlToWorld::setStreamReaderToFile Unable to Open ") + fileName);
			qDebug() << message;
			GLC_FileFormatException fileFormatException(message, fileName, GLC_FileFormatException::FileNotSupported);
			clear();
			throw(fileFormatException);
	    }

		// Set the stream reader
		delete m_pStreamReader;
		m_pStreamReader= new QXmlStreamReader(m_p3dxmlFile);
	}
	else
	{
		delete m_pCurrentFile;
		// Create the file to load
		if (fileName != m_FileName and not m_FileName.isEmpty())
		{
			fileName= QFileInfo(m_FileName).absolutePath() + QDir::separator() + fileName;
		}
		m_pCurrentFile= new QFile(fileName);
		if (not m_pCurrentFile->open(QIODevice::ReadOnly))
		{
			QString message(QString("GLC_3dxmlToWorld::setStreamReaderToFile File ") + fileName + QString(" not found"));
			qDebug() << message;
			//GLC_FileFormatException fileFormatException(message, fileName, GLC_FileFormatException::FileNotFound);
			//clear();
			return false;
			//throw(fileFormatException);
		}
		else if (m_FileName != fileName)
		{
			m_ListOfAttachedFileName << fileName;
		}
		// Set the stream reader
		delete m_pStreamReader;
		m_pStreamReader= new QXmlStreamReader(m_pCurrentFile);
	}
	return true;
}

// Load the local representation
void GLC_3dxmlToWorld::loadLocalRepresentations()
{
	if (m_LocalRepLinkList.isEmpty()) return;
	QHash<const QString, GLC_3DRep> repHash;

	// Load all local ref
	goToElement("GeometricRepresentationSet");
	while (endElementNotReached("GeometricRepresentationSet"))
	{
		if (m_pStreamReader->name() == "Representation")
		{
			QString id= readAttribute("id", true);

			GLC_StructReference* pRef= createReferenceRep("Local");
			GLC_3DRep representation(*(dynamic_cast<GLC_3DRep*>(pRef->representationHandle())));

			delete pRef;
			repHash.insert(id, representation);
		}
		m_pStreamReader->readNext();
	}
	//qDebug() << "Local rep loaded";

	// Attach the ref to the structure reference
	RepLinkList::iterator iLocalRep= m_LocalRepLinkList.begin();
	while (iLocalRep != m_LocalRepLinkList.constEnd())
	{
		unsigned int referenceId= (*iLocalRep).m_ReferenceId;
		unsigned int refId= (*iLocalRep).m_RepId;

		GLC_StructReference* pReference= m_ReferenceHash.value(referenceId);
		const QString representationID= m_ReferenceRepHash.value(refId);
		pReference->setRepresentation(repHash.value(representationID));

		++iLocalRep;
	}
}

// Load the extern representation
void GLC_3dxmlToWorld::loadExternRepresentations()
{
	if (m_ExternRepLinkList.isEmpty()) return;

	QHash<const unsigned int, GLC_3DRep> repHash;

	// Progress bar variables
	const int size= m_ReferenceRepHash.size();
	int previousQuantumValue= 0;
	int currentQuantumValue= 0;
	int currentFileIndex= 0;
	emit currentQuantum(currentQuantumValue);

	// Load all external rep
	ReferenceRepHash::iterator iRefRep= m_ReferenceRepHash.begin();
	while (iRefRep != m_ReferenceRepHash.constEnd())
	{
		const QString currentRefFileName= iRefRep.value();
		const unsigned int id= iRefRep.key();

		if (not m_LoadStructureOnly and setStreamReaderToFile(currentRefFileName))
		{
			GLC_3DRep representation= loadCurrentExtRep();
			representation.removeEmptyGeometry();
			if (not representation.isEmpty())
			{
				repHash.insert(id, representation);
			}
		}
		else if (m_LoadStructureOnly)
		{
			GLC_3DRep representation;
			if (m_IsInArchive)
			{
				representation.setFileName("Zip::" + m_FileName + "::" + currentRefFileName);
			}
			else
			{
				const QString repFileName= QFileInfo(m_FileName).absolutePath() + QDir::separator() + currentRefFileName;
				representation.setFileName(repFileName);
				m_ListOfAttachedFileName << repFileName;
			}

			repHash.insert(id, representation);
		}

		// Progrees bar indicator
		++currentFileIndex;
		currentQuantumValue = static_cast<int>((static_cast<double>(currentFileIndex) / size) * 100);
		if (currentQuantumValue > previousQuantumValue)
		{
			emit currentQuantum(currentQuantumValue);
		}
		previousQuantumValue= currentQuantumValue;

		++iRefRep;
	}

	// Attach the ref to the structure reference
	RepLinkList::iterator iExtRep= m_ExternRepLinkList.begin();
	while (iExtRep != m_ExternRepLinkList.constEnd())
	{
		unsigned int referenceId= (*iExtRep).m_ReferenceId;
		unsigned int refId= (*iExtRep).m_RepId;

		GLC_StructReference* pReference= m_ReferenceHash.value(referenceId);
		const QString representationID= m_ReferenceRepHash.value(refId);
		pReference->setRepresentation(repHash.value(refId));

		++iExtRep;
	}

}

// Return the instance of the current extern representation
GLC_3DRep GLC_3dxmlToWorld::loadCurrentExtRep()
{
	GLC_ExtendedMesh* pMesh= new GLC_ExtendedMesh();
	GLC_3DRep currentMeshRep(pMesh);
	currentMeshRep.setName(QString());
	int numberOfMesh= 1;
	while (not m_pStreamReader->atEnd())
	{
		m_pCurrentMaterial= NULL;
		gotToPolygonalRepType();
		if (m_pStreamReader->atEnd() or m_pStreamReader->hasError())
		{
			if (m_pStreamReader->hasError())
			{
				QString message(QString("An element have not been found in file ") + m_FileName);
				qDebug() << m_pStreamReader->errorString();
				GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
				clear();
				throw(fileFormatException);
			}
			else
			{
				pMesh->finished();
				return currentMeshRep;
			}
		}
		if (numberOfMesh > 1)
		{
			pMesh->finished();
			pMesh = new GLC_ExtendedMesh();
			currentMeshRep.addGeom(pMesh);
		}

		// Get the master lod accuracy
		double masteLodAccuracy= readAttribute("accuracy", false).toDouble();

		loadLOD(pMesh);
		if (m_pStreamReader->atEnd() or m_pStreamReader->hasError())
		{
			qDebug() << " Master LOD not found";
			pMesh->finished();
			return currentMeshRep;
		}

		// Load Faces index data
		while (endElementNotReached("Faces"))
		{
			m_pStreamReader->readNext();
			if ( m_pStreamReader->name() == "Face")
			{
				loadFace(pMesh, 0, masteLodAccuracy);
			}
		}
		checkForXmlError("End of Faces not found");

		goToElement("VertexBuffer");
		checkForXmlError("Element VertexBuffer not found");

		{
			QString verticePosition= getContent("Positions").replace(',', ' ');
			//qDebug() << "Position " << verticePosition;
			checkForXmlError("Error while retrieving Position ContentVertexBuffer");
			// Load Vertice position
			QTextStream verticeStream(&verticePosition);
			QList<GLfloat> verticeValues;
			QString buff;
			while ((!verticeStream.atEnd()))
			{
				verticeStream >> buff;
				verticeValues.append(buff.toFloat());
			}
			pMesh->addVertices(verticeValues.toVector());

		}

		{
			QString normals= getContent("Normals").replace(',', ' ');
			//qDebug() << "Normals " << normals;
			checkForXmlError("Error while retrieving Normals values");
			// Load Vertice Normals
			QTextStream normalsStream(&normals);
			QList<GLfloat> normalValues;
			QString buff;
			while ((!normalsStream.atEnd()))
			{
				normalsStream >> buff;
				normalValues.append(buff.toFloat());
			}
			pMesh->addNormals(normalValues.toVector());
		}
		// Try to find texture coordinate
		while (endElementNotReached("VertexBuffer"))
		{
			if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and (m_pStreamReader->name() == "TextureCoordinates"))
			{
				QString texels= getContent("TextureCoordinates").replace(',', ' ');
				checkForXmlError("Error while retrieving Texture coordinates");
				QTextStream texelStream(&texels);
				QList<GLfloat> texelValues;
				QString buff;
				while ((!texelStream.atEnd()))
				{
					texelStream >> buff;
					texelValues.append(buff.toFloat());
				}
				pMesh->addTexels(texelValues.toVector());
			}
			m_pStreamReader->readNext();
		}
		++numberOfMesh;
	}

	pMesh->finished();

	return currentMeshRep;
}
// Load CatMaterial Ref if present
void GLC_3dxmlToWorld::loadCatMaterialRef()
{

	QList<MaterialRef> materialRefList;

	// Load material Name, Id and associated File
	if (setStreamReaderToFile("CATMaterialRef.3dxml", true))
	{
		// Load the material file
		qDebug() << "CATMaterialRef.3dxml found and current";
		goToElement("CATMaterialRef");
		checkForXmlError("Element CATMaterialRef not found in CATMaterialRef.3dxml");
		while (endElementNotReached("CATMaterialRef"))
		{
			if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
			{
				const QStringRef currentElementName= m_pStreamReader->name();
				if (currentElementName == "CATMatReference")
				{
					MaterialRef currentMaterial;
					currentMaterial.m_Id= readAttribute("id", true);
					currentMaterial.m_Name= readAttribute("name", true);
					goToElement("MaterialDomain");
					checkForXmlError("Element MaterialDomain not found after CATMatReference Element");
					currentMaterial.m_AssociatedFile= readAttribute("associatedFile", true).remove("urn:3DXML:");
					materialRefList.append(currentMaterial);
					//qDebug() << "Material " << currentMaterial.m_Name << " Added";
				}
			}
			m_pStreamReader->readNext();
		}
	}
	// Load material files
	const int size= materialRefList.size();
	for (int i= 0; i < size; ++i)
	{
		if (setStreamReaderToFile(materialRefList.at(i).m_AssociatedFile, true))
		{
			//qDebug() << "Load MaterialDef : " << materialRefList.at(i).m_AssociatedFile;
			loadMaterialDef(materialRefList.at(i));
		}
	}
}

// Create material from material def file
void GLC_3dxmlToWorld::loadMaterialDef(const MaterialRef& materialRef)
{
	GLC_Material* pMaterial= new GLC_Material();
	goToElement("Osm");
	checkForXmlError(QString("Element Osm not found in file : ") + materialRef.m_AssociatedFile);
	while (endElementNotReached("Osm"))
	{
		m_pStreamReader->readNext();
		if ((QXmlStreamReader::StartElement == m_pStreamReader->tokenType()) and m_pStreamReader->name() == "Attr")
		{
			const QString currentName= readAttribute("Name", true);
			if (currentName == "DiffuseColor")
			{
				QString color= m_pStreamReader->attributes().value("Value").toString();
				color.remove('[');
				color.remove(']');
				QStringList colors(color.split(","));
				QColor diffuseColor;
				diffuseColor.setRedF(colors.at(0).toDouble());
				diffuseColor.setGreenF(colors.at(1).toDouble());
				diffuseColor.setBlueF(colors.at(2).toDouble());
				pMaterial->setDiffuseColor(diffuseColor);
			}
			else if (currentName == "Transparency")
			{
				double transparency= readAttribute("Value", true).toDouble();
				transparency= 1.0 - transparency;
				pMaterial->setTransparency(transparency);
			}
			else if (currentName == "SpecularExponent")
			{

			}
			else if (currentName == "TextureImage")
			{
				//qDebug() << "TextureImage";
				QString imageId= readAttribute("Value", true).remove("urn:3DXML:CATRepImage.3dxml#");
				if (m_TextureImagesHash.contains(imageId))
				{
					QString imageName= m_TextureImagesHash.value(imageId);
					QImage textureImage= loadImage(imageName);
					if (not textureImage.isNull())
					{
						pMaterial->setTexture(new GLC_Texture(m_pQGLContext, textureImage));
					}
				}
			}
			else if (currentName == "EmissiveCoef")
			{

			}
			else if (currentName == "SpecularColor")
			{
				QString color= readAttribute("Value", true);
				color.remove('[');
				color.remove(']');
				QStringList colors(color.split(","));
				QColor specularColor;
				specularColor.setRedF(colors.at(0).toDouble());
				specularColor.setGreenF(colors.at(1).toDouble());
				specularColor.setBlueF(colors.at(2).toDouble());
				pMaterial->setSpecularColor(specularColor);
			}
			else if (currentName == "AmbientColor")
			{
				QString color= readAttribute("Value", true);
				color.remove('[');
				color.remove(']');
				QStringList colors(color.split(","));
				QColor ambientColor;
				ambientColor.setRedF(colors.at(0).toDouble());
				ambientColor.setGreenF(colors.at(1).toDouble());
				ambientColor.setBlueF(colors.at(2).toDouble());
				pMaterial->setAmbientColor(ambientColor);
			}

		}
	}
	pMaterial->setName(materialRef.m_Name);
	m_MaterialHash.insert(materialRef.m_Id, pMaterial);
}

// Load CATRepIage if present
void GLC_3dxmlToWorld::loadCatRepImage()
{
	// Load texture image name
	if (setStreamReaderToFile("CATRepImage.3dxml", true))
	{
		qDebug() << "CATRepImage.3dxml Found";
		goToElement("CATRepImage");
		checkForXmlError("Element CATRepImage not found in CATRepImage.3dxml");
		while (endElementNotReached("CATRepImage"))
		{
			if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
			{
				const QStringRef currentElementName= m_pStreamReader->name();
				if (currentElementName == "CATRepresentationImage")
				{
					QString id= readAttribute("id", true);
					QString associatedFile= readAttribute("associatedFile", true).remove("urn:3DXML:");
					m_TextureImagesHash.insert(id,associatedFile);
				}
			}
			m_pStreamReader->readNext();
		}
		qDebug() << "CATRepImage.3dxml Load";
	}
}

// try to load the specified image
QImage GLC_3dxmlToWorld::loadImage(QString fileName)
{
	QString format= QFileInfo(fileName).suffix().toUpper();
	QImage resultImage;
	if (m_IsInArchive)
	{
		// Create QuaZip File
		QuaZipFile* p3dxmlFile= new QuaZipFile(m_p3dxmlArchive);

		// Get the file of the 3dxml
		if (not m_p3dxmlArchive->setCurrentFile(fileName, QuaZip::csInsensitive))
		{
			return QImage();
		}

		// Open the file of the 3dxml
		if(not p3dxmlFile->open(QIODevice::ReadOnly))
	    {
			delete p3dxmlFile;
			QString message(QString("GLC_3dxmlToWorld::setStreamReaderToFile Unable to Open ") + fileName);
			qDebug() << message;
			GLC_FileFormatException fileFormatException(message, fileName, GLC_FileFormatException::FileNotSupported);
			clear();
			throw(fileFormatException);
	    }
		resultImage.load(p3dxmlFile, format.toLocal8Bit());
		p3dxmlFile->close();
		delete p3dxmlFile;
	}
	else
	{
		// Create the file to load
		if (fileName != m_FileName)
		{
			fileName= QFileInfo(m_FileName).absolutePath() + QDir::separator() + fileName;
		}
		QFile* pCurrentFile= new QFile(fileName);
		if (not pCurrentFile->open(QIODevice::ReadOnly))
		{
			delete pCurrentFile;
			QString message(QString("GLC_3dxmlToWorld::setStreamReaderToFile File ") + fileName + QString(" not found"));
			qDebug() << message;
			return QImage();
		}
		else
		{
			m_ListOfAttachedFileName << fileName;
		}
		resultImage.load(pCurrentFile, format.toLocal8Bit());
		pCurrentFile->close();
		delete pCurrentFile;
	}

	return resultImage;
}
