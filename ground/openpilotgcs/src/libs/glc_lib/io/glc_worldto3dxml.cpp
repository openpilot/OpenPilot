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
//! \file glc_worldto3dxml.cpp implementation of the GLC_WorldTo3dxml class.

#include "glc_worldto3dxml.h"
// Quazip library
#include "../3rdparty/quazip/quazip.h"
#include "../3rdparty/quazip/quazipfile.h"
#include "../glc_exception.h"
#include "../geometry/glc_mesh.h"

#include <QFileInfo>

GLC_WorldTo3dxml::GLC_WorldTo3dxml(const GLC_World& world, bool threaded)
: QObject()
, m_World(world)
, m_ExportType(Compressed3dxml)
, m_FileName()
, m_pOutStream(NULL)
, m_Generator("GLC_LIB")
, m_CurrentId(0)
, m_p3dxmlArchive(NULL)
, m_pCurrentZipFile(NULL)
, m_pCurrentFile(NULL)
, m_AbsolutePath()
, m_ReferenceToIdHash()
, m_InstanceToIdHash()
, m_ReferenceRepToIdHash()
, m_ReferenceRepTo3dxmlFileName()
, m_InstanceRep()
, m_MaterialIdToMaterialName()
, m_MaterialIdToMaterialId()
, m_MaterialIdToTexture3dxmlName()
, m_MaterialIdTo3dxmlImageId()
, m_ExportMaterial(true)
, m_3dxmlFileSet()
, m_FileNameIncrement(0)
, m_ListOfOverLoadedOccurence()
, m_pReadWriteLock(NULL)
, m_pIsInterupted(NULL)
, m_IsThreaded(threaded)
{
	m_World.rootOccurence()->updateOccurenceNumber(1);
}

GLC_WorldTo3dxml::~GLC_WorldTo3dxml()
{
	delete m_p3dxmlArchive;
	delete m_pCurrentZipFile;
	delete m_pCurrentFile;
}

bool GLC_WorldTo3dxml::exportTo3dxml(const QString& filename, GLC_WorldTo3dxml::ExportType exportType, bool exportMaterial)
{
	m_3dxmlFileSet.clear();
	m_ListOfOverLoadedOccurence.clear();
	m_FileNameIncrement= 0;
	m_ExportMaterial= exportMaterial;
	m_FileName= filename;
	m_ExportType= exportType;
	bool isExported= false;
	if (m_ExportType == Compressed3dxml)
	{
		m_p3dxmlArchive= new QuaZip(m_FileName);
		isExported= m_p3dxmlArchive->open(QuaZip::mdCreate);
		// Add the manifest
		addManifest();

	}
	else
	{
		m_AbsolutePath= QFileInfo(m_FileName).absolutePath() + QDir::separator();
		QFile exportFile(m_FileName);
		isExported= exportFile.open(QIODevice::WriteOnly);
		exportFile.close();
	}
	if (isExported)
	{
		if (m_ExportMaterial && (m_ExportType != StructureOnly))
		{
			writeAllMaterialRelatedFilesIn3dxml();
		}

		// Export the assembly structure from the list of structure reference
		exportAssemblyStructure();

		if (m_ExportType != StructureOnly)
		{
			int previousQuantumValue= 0;
			int currentQuantumValue= 0;
			emit currentQuantum(currentQuantumValue);

			int currentRepIndex= 0;
			const int size= m_ReferenceRepTo3dxmlFileName.size();
			// Export the representation
			QHash<const GLC_3DRep*, QString>::const_iterator iRep= m_ReferenceRepTo3dxmlFileName.constBegin();
			while ((m_ReferenceRepTo3dxmlFileName.constEnd() != iRep) && continu())
			{
				write3DRep(iRep.key(), iRep.value());
				++iRep;

				// Progrees bar indicator
				++currentRepIndex;
				currentQuantumValue = static_cast<int>((static_cast<double>(currentRepIndex) / size) * 100);
				if (currentQuantumValue > previousQuantumValue)
				{
					emit currentQuantum(currentQuantumValue);
				}
				previousQuantumValue= currentQuantumValue;
				if (!m_IsThreaded)
				{
					QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
				}
			}
		}
	}

	emit currentQuantum(100);
	return isExported;
}

bool GLC_WorldTo3dxml::exportReferenceTo3DRep(const GLC_3DRep* p3DRep, const QString& fullFileName)
{
	m_3dxmlFileSet.clear();
	m_ListOfOverLoadedOccurence.clear();
	m_FileNameIncrement= 0;
	m_ExportMaterial= false;

	m_AbsolutePath= QFileInfo(fullFileName).absolutePath() + QDir::separator();

	write3DRep(p3DRep, QFileInfo(fullFileName).fileName());

	return true;
}

void GLC_WorldTo3dxml::setInterupt(QReadWriteLock* pReadWriteLock, bool* pInterupt)
{
	m_pReadWriteLock= pReadWriteLock;
	m_pIsInterupted= pInterupt;
}

void GLC_WorldTo3dxml::writeHeader()
{
	const QString title(QFileInfo(m_FileName).fileName());

	m_pOutStream->writeStartElement("Header");
		m_pOutStream->writeTextElement("SchemaVersion", "4.0");
		m_pOutStream->writeTextElement("Title", title);
		m_pOutStream->writeTextElement("Generator", m_Generator);
		m_pOutStream->writeTextElement("Created", QDate::currentDate().toString(Qt::ISODate));
	m_pOutStream->writeEndElement();
}

void GLC_WorldTo3dxml::writeReference3D(const GLC_StructReference* pRef)
{
	m_pOutStream->writeStartElement("Reference3D");
		m_pOutStream->writeAttribute("xsi:type", "Reference3DType");
		m_pOutStream->writeAttribute("id", QString::number(++m_CurrentId));
		m_pOutStream->writeAttribute("name", pRef->name());
		if (pRef->containsAttributes())
		{
			m_pOutStream->writeStartElement("Reference3DExtensionType");
			writeExtensionAttributes(pRef->attributesHandle());
			m_pOutStream->writeEndElement(); // Reference3DExtensionType
		}
	m_pOutStream->writeEndElement(); // Reference3D

	m_ReferenceToIdHash.insert(pRef, m_CurrentId);
}

void GLC_WorldTo3dxml::writeReferenceRep(const GLC_3DRep* p3DRep)
{
	const QString id(QString::number(++m_CurrentId));
	m_ReferenceRepToIdHash.insert(p3DRep, m_CurrentId);
	const QString associateFile(representationFileName(p3DRep));
	m_ReferenceRepTo3dxmlFileName.insert(p3DRep, QString(associateFile).remove("urn:3DXML:"));

	m_pOutStream->writeStartElement("ReferenceRep");
		m_pOutStream->writeAttribute("xsi:type", "ReferenceRepType");
		m_pOutStream->writeAttribute("id", id);
		m_pOutStream->writeAttribute("name", p3DRep->name());
		m_pOutStream->writeAttribute("format", "TESSELLATED");
		m_pOutStream->writeAttribute("version", "1.2");
		m_pOutStream->writeAttribute("associatedFile", associateFile);
		m_pOutStream->writeTextElement("PLM_ExternalID", p3DRep->name());
		m_pOutStream->writeTextElement("V_discipline", "Design");
		m_pOutStream->writeTextElement("V_usage", "3DShape");
		m_pOutStream->writeTextElement("V_nature", "1");
	m_pOutStream->writeEndElement();


}
void GLC_WorldTo3dxml::writeInstance3D(const GLC_StructInstance* pInstance, unsigned int parentId)
{
	const GLC_StructReference* pRef= pInstance->structReference();
	const unsigned int referenceId= m_ReferenceToIdHash.value(pRef);
	const QString instanceMatrix(matrixString(pInstance->relativeMatrix()));

	m_pOutStream->writeStartElement("Instance3D");
		m_pOutStream->writeAttribute("xsi:type", "Instance3DType");
		m_pOutStream->writeAttribute("id", QString::number(++m_CurrentId));
		m_pOutStream->writeAttribute("name", pInstance->name());
		m_pOutStream->writeTextElement("IsAggregatedBy", QString::number(parentId));
		m_pOutStream->writeTextElement("IsInstanceOf", QString::number(referenceId));
		m_pOutStream->writeTextElement("RelativeMatrix", instanceMatrix);
		if (pInstance->containsAttributes())
		{
			m_pOutStream->writeStartElement("Instance3DExtensionType");
			writeExtensionAttributes(pInstance->attributesHandle());
			m_pOutStream->writeEndElement(); // Instance3DExtensionType
		}

	m_pOutStream->writeEndElement(); // Instance3D

	m_InstanceToIdHash.insert(pInstance, m_CurrentId);
}

void  GLC_WorldTo3dxml::writeInstanceRep(const GLC_3DRep* p3DRep, unsigned int parentId)
{
	const unsigned int referenceId= m_ReferenceRepToIdHash.value(p3DRep);
	m_pOutStream->writeStartElement("InstanceRep");
		m_pOutStream->writeAttribute("xsi:type", "InstanceRepType");
		m_pOutStream->writeAttribute("id", QString::number(++m_CurrentId));
		m_pOutStream->writeAttribute("name", p3DRep->name());
		m_pOutStream->writeTextElement("IsAggregatedBy", QString::number(parentId));
		m_pOutStream->writeTextElement("IsInstanceOf", QString::number(referenceId));
	m_pOutStream->writeEndElement();

	m_InstanceRep.insert(parentId);

}
void GLC_WorldTo3dxml::setStreamWriterToFile(const QString& fileName)
{
	delete m_pOutStream;
	m_pOutStream= NULL;

	bool success= false;
	if (NULL != m_p3dxmlArchive)
	{
		if (NULL != m_pCurrentZipFile)
		{
			m_pCurrentZipFile->close();
			delete m_pOutStream;
			delete m_pCurrentZipFile;
		}
		QuaZipNewInfo quazipNewInfo(fileName);
		m_pCurrentZipFile= new QuaZipFile(m_p3dxmlArchive);
		success= m_pCurrentZipFile->open(QIODevice::WriteOnly, quazipNewInfo);
		if (success)
		{
			m_pOutStream= new QXmlStreamWriter(m_pCurrentZipFile);
		}
	}
	else
	{
		delete m_pCurrentFile;
		m_pCurrentFile= new QFile(m_AbsolutePath + fileName);
		success= m_pCurrentFile->open(QIODevice::WriteOnly);
		if (success)
		{
			m_pOutStream= new QXmlStreamWriter(m_pCurrentFile);
		}
	}

	if (NULL == m_pOutStream)
	{
		QString message(QString("GLC_WorldTo3dxml::setStreamWriterToFile Unable to create ") + fileName);
		GLC_Exception fileException(message);
		throw(fileException);
	}
	else
	{
		m_pOutStream->setAutoFormatting(true);
	}
}

void GLC_WorldTo3dxml::addManifest()
{
	setStreamWriterToFile("Manifest.xml");
	m_pOutStream->writeStartDocument();

	m_pOutStream->writeStartElement("Manifest");
		m_pOutStream->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
		m_pOutStream->writeAttribute("xsi:noNamespaceSchemaLocation", "Manifest.xsd");
		m_pOutStream->writeTextElement("Root", QFileInfo(m_FileName).fileName());
		m_pOutStream->writeEndElement();
	m_pOutStream->writeEndElement();

	m_pOutStream->writeEndDocument();
}

void GLC_WorldTo3dxml::exportAssemblyStructure()
{
	m_ReferenceToIdHash.clear();
	m_InstanceToIdHash.clear();
	m_ReferenceRepToIdHash.clear();
	m_ReferenceRepTo3dxmlFileName.clear();
	m_InstanceRep.clear();

	// Create the assembly file
	setStreamWriterToFile(QFileInfo(m_FileName).fileName());
	m_pOutStream->writeStartDocument();
	m_pOutStream->writeStartElement("Model_3dxml");
	m_pOutStream->writeAttribute("xmlns", "http://www.3ds.com/xsd/3DXML");
	m_pOutStream->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	m_pOutStream->writeAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");

	writeHeader();

	// Product Structure
	m_pOutStream->writeStartElement("ProductStructure");
	m_pOutStream->writeAttribute("root", "1");
	exportAssemblyFromOccurence(m_World.rootOccurence());
	m_pOutStream->writeEndElement(); // ProductStructure

	if (!m_ListOfOverLoadedOccurence.isEmpty())
	{
		m_pOutStream->writeStartElement("DefaultView");
			const int size= m_ListOfOverLoadedOccurence.size();
			for (int i= 0; i < size; ++i)
			{
				writeOccurenceDefaultViewProperty(m_ListOfOverLoadedOccurence.at(i));
			}
		m_pOutStream->writeEndElement(); // DefaultView
	}

	m_pOutStream->writeEndElement(); // Model_3dxml

	m_pOutStream->writeEndDocument();
}

void GLC_WorldTo3dxml::exportAssemblyFromOccurence(const GLC_StructOccurence* pOccurence)
{
	if (pOccurence->isOrphan())
	{
		writeReference3D(pOccurence->structReference());
	}
	else
	{
		// Reference 3D
		GLC_StructReference* pCurrentRef= pOccurence->structReference();
		if (!m_ReferenceToIdHash.contains(pCurrentRef))
		{
			writeReference3D(pCurrentRef);
			// Reference Rep
			if (pCurrentRef->hasRepresentation())
			{
				GLC_3DRep* pCurrentRep= dynamic_cast<GLC_3DRep*>(pCurrentRef->representationHandle());
				if (NULL != pCurrentRep && !m_ReferenceRepToIdHash.contains(pCurrentRep))
				{
					writeReferenceRep(pCurrentRep);
				}
			}
		}
		// Instance 3D and instance rep
		GLC_StructInstance* pCurrentInstance= pOccurence->structInstance();
		if (!m_InstanceToIdHash.contains(pCurrentInstance))
		{
			// Instance 3D
			const unsigned int parentId= m_ReferenceToIdHash.value(pOccurence->parent()->structReference());
			writeInstance3D(pCurrentInstance, parentId);

			// Instance Rep
			if (pCurrentRef->hasRepresentation())
			{
				GLC_3DRep* pCurrentRep= dynamic_cast<GLC_3DRep*>(pCurrentRef->representationHandle());
				const unsigned int parentId= m_ReferenceToIdHash.value(pCurrentRef);
				if (NULL != pCurrentRep && !m_InstanceRep.contains(parentId))
				{
					writeInstanceRep(pCurrentRep, parentId);
				}
			}
		}
	}

	// Process children
	const int childCount= pOccurence->childCount();
	for (int i= 0; i < childCount; ++i)
	{
		exportAssemblyFromOccurence(pOccurence->child(i));
	}

	// Add occurence with Overload properties to a list
	if (m_World.collection()->contains(pOccurence->id()))
	{
		GLC_3DViewInstance* pInstance= m_World.collection()->instanceHandle(pOccurence->id());
		Q_ASSERT(NULL != pInstance);
		const bool isVisible= pInstance->isVisible();
		const bool isOverload= !isVisible || !pInstance->renderPropertiesHandle()->isDefault() || pOccurence->isFlexible();
		if (isOverload)
		{
			m_ListOfOverLoadedOccurence.append(pOccurence);
		}
	}

}

QString GLC_WorldTo3dxml::matrixString(const GLC_Matrix4x4& matrix)
{
	QString resultMatrix;
	const QChar spaceChar(' ');
	// Rotation
	resultMatrix+= QString::number(matrix.getData()[0], 'g', 16) + spaceChar;
	resultMatrix+= QString::number(matrix.getData()[1], 'g', 16) + spaceChar;
	resultMatrix+= QString::number(matrix.getData()[2], 'g', 16) + spaceChar;

	resultMatrix+= QString::number(matrix.getData()[4], 'g', 16) + spaceChar;
	resultMatrix+= QString::number(matrix.getData()[5], 'g', 16) + spaceChar;
	resultMatrix+= QString::number(matrix.getData()[6], 'g', 16) + spaceChar;

	resultMatrix+= QString::number(matrix.getData()[8], 'g', 16) + spaceChar;
	resultMatrix+= QString::number(matrix.getData()[9], 'g', 16) + spaceChar;
	resultMatrix+= QString::number(matrix.getData()[10], 'g', 16) + spaceChar;

	// Translation
	resultMatrix+= QString::number(matrix.getData()[12], 'g', 16) + spaceChar;
	resultMatrix+= QString::number(matrix.getData()[13], 'g', 16) + spaceChar;
	resultMatrix+= QString::number(matrix.getData()[14], 'g', 16);

	return resultMatrix;
}

void GLC_WorldTo3dxml::write3DRep(const GLC_3DRep* pRep, const QString& fileName)
{
	setStreamWriterToFile(fileName);

	m_pOutStream->writeStartDocument();
	m_pOutStream->writeStartElement("XMLRepresentation");
	m_pOutStream->writeAttribute("version", "1.2");
	m_pOutStream->writeAttribute("xmlns", "http://www.3ds.com/xsd/3DXML");
	m_pOutStream->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	m_pOutStream->writeAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
	m_pOutStream->writeAttribute("xsi:schemaLocation", "http://www.3ds.com/xsd/3DXML ./3DXMLMesh.xsd");

	m_pOutStream->writeStartElement("Root"); // Root
	m_pOutStream->writeAttribute("xsi:type", "BagRepType");
	m_pOutStream->writeAttribute("id", QString::number(++m_CurrentId));
	const int bodyCount= pRep->numberOfBody();
	for (int i= 0; i < bodyCount; ++i)
	{
		GLC_Mesh* pMesh= dynamic_cast<GLC_Mesh*>(pRep->geomAt(i));
		if (NULL != pMesh)
		{
			writeGeometry(pMesh);
		}
	}
	m_pOutStream->writeEndElement(); // Root

	m_pOutStream->writeEndElement(); // XMLRepresentation

	m_pOutStream->writeEndDocument();
}

QString GLC_WorldTo3dxml::representationFileName(const GLC_3DRep* pRep)
{
	Q_ASSERT(m_ReferenceRepToIdHash.contains(pRep));
	QString repName= pRep->name();
	QString fileName;
	if (m_ExportType == StructureOnly)
	{
		QString newFileName= pRep->fileName();
		// Test if the file name is encoded by GLC_Lib (Structure only loaded)
		if (glc::isFileString(newFileName))
		{
			newFileName= glc::archiveEntryFileName(newFileName);
		}
		if (newFileName.isEmpty() || (glc::isArchiveString(newFileName)))
		{
			fileName= "urn:3DXML:NoFile_0.3DRep";
		}
		else
		{
			// Compute the relative fileName from the structure
			QDir structureDir(m_AbsolutePath);
			QString relativeFilePath= structureDir.relativeFilePath(newFileName);
			fileName= "urn:3DXML:" + relativeFilePath;
		}
	}
	else if (repName.isEmpty())
	{
		fileName= "urn:3DXML:Representation_0.3DRep";
	}
	else
	{
		fileName= "urn:3DXML:" + repName + ".3DRep";
	}
	return xmlFileName(fileName);
}

void GLC_WorldTo3dxml::writeGeometry(const GLC_Mesh* pMesh)
{
	// Get the list of material id
	QList<GLC_uint> materialList= pMesh->materialIds();
	const int materialCount= materialList.size();

	m_pOutStream->writeStartElement("Rep");
	m_pOutStream->writeAttribute("xsi:type", "PolygonalRepType");
	m_pOutStream->writeAttribute("id", QString::number(++m_CurrentId));
	const double masterAccuracy= pMesh->getLodAccuracy(0);
	m_pOutStream->writeAttribute("accuracy", QString::number(masterAccuracy));
	m_pOutStream->writeAttribute("solid", "1");
	const int lodCount= pMesh->lodCount();
	if (lodCount > 1)
	{
		// The mesh contains LOD
		for (int i= 1; i < lodCount; ++i)
		{
			const double lodAccuracy= pMesh->getLodAccuracy(i);
			m_pOutStream->writeStartElement("PolygonalLOD");
			m_pOutStream->writeAttribute("accuracy", QString::number(lodAccuracy));
			m_pOutStream->writeStartElement("Faces");
			for (int matIndex= 0; matIndex < materialCount; ++matIndex)
			{
				const GLC_uint materialId= materialList.at(matIndex);
				if (pMesh->lodContainsMaterial(i, materialId))
				{
					writeGeometryFace(pMesh, i, materialId);
				}
			}
			m_pOutStream->writeEndElement(); // Faces
			m_pOutStream->writeEndElement(); // PolygonalLOD
		}
	}

	// Master LOD
	m_pOutStream->writeStartElement("Faces");
	for (int matIndex= 0; matIndex < materialCount; ++matIndex)
	{
		const GLC_uint materialId= materialList.at(matIndex);
		if (pMesh->lodContainsMaterial(0, materialId))
		{
			writeGeometryFace(pMesh, 0, materialId);
		}
	}
	m_pOutStream->writeEndElement(); // Faces
	if (!pMesh->wireDataIsEmpty())
	{
		writeEdges(pMesh);
	}

	// Save Bulk data
	m_pOutStream->writeStartElement("VertexBuffer");
	{
		// Get positions
		GLfloatVector positionVector= pMesh->positionVector();
		QString positions;
		const int size= positionVector.size();
		for (int i= 0; i < size; i+=3)
		{
			positions.append(QString::number(positionVector.at(i)));
			positions.append(' ');
			positions.append(QString::number(positionVector.at(i + 1)));
			positions.append(' ');
			positions.append(QString::number(positionVector.at(i + 2)));
			positions.append(", ");
		}
		positions.remove(positions.size() - 2, 2);
		m_pOutStream->writeTextElement("Positions", positions);
	}
	{
		// Get normals
		GLfloatVector normalVector= pMesh->normalVector();
		QString normals;
		const int size= normalVector.size();
		for (int i= 0; i < size; i+=3)
		{
			normals.append(QString::number(normalVector.at(i)));
			normals.append(' ');
			normals.append(QString::number(normalVector.at(i + 1)));
			normals.append(' ');
			normals.append(QString::number(normalVector.at(i + 2)));
			normals.append(", ");
		}
		normals.remove(normals.size() - 2, 2);
		m_pOutStream->writeTextElement("Normals", normals);
	}
	{
		// Get texture coordinates
		GLfloatVector texelVector= pMesh->texelVector();
		if (!texelVector.isEmpty())
		{
			QString texels;
			const int size= texelVector.size();
			for (int i= 0; i < size; i+=2)
			{
				texels.append(QString::number(texelVector.at(i)));
				texels.append(' ');
				texels.append(QString::number(texelVector.at(i + 1)));
				texels.append(", ");
			}
			texels.remove(texels.size() - 2, 2);

			m_pOutStream->writeStartElement("TextureCoordinates");
			m_pOutStream->writeAttribute("dimension", "2D");
			m_pOutStream->writeAttribute("channel", "0");
			m_pOutStream->writeCharacters(texels);
			m_pOutStream->writeEndElement(); // TexturesCoordinates
		}
	}


	m_pOutStream->writeEndElement(); // VertexBuffer
	m_pOutStream->writeEndElement(); // Rep

}
void GLC_WorldTo3dxml::writeGeometryFace(const GLC_Mesh* pMesh, int lod, GLC_uint materialId)
{
	m_pOutStream->writeStartElement("Face");
	if (pMesh->containsTriangles(lod, materialId))
	{
		QVector<GLuint> triangleIndex= pMesh->getTrianglesIndex(lod, materialId);
		QString indexString;
		const int indexSize= triangleIndex.size();
		for (int index= 0; index < indexSize; ++index)
		{
			indexString.append(QString::number(triangleIndex.at(index)));
			indexString.append(' ');
		}
		indexString.remove(indexString.size() - 1, 1);
		m_pOutStream->writeAttribute("triangles", indexString);
	}
	if (pMesh->containsStrips(lod, materialId))
	{
		QList<QVector<GLuint> > stripsIndex= pMesh->getStripsIndex(lod, materialId);
		QString indexStrips;
		const int stripCount= stripsIndex.size();
		for (int stripIndex= 0; stripIndex < stripCount; ++stripIndex)
		{
			QVector<GLuint> currentStripIndex= stripsIndex.at(stripIndex);
			QString currentIndex;
			const int indexSize= currentStripIndex.size();
			for (int index= 0; index < indexSize; ++index)
			{
				currentIndex.append(QString::number(currentStripIndex.at(index)));
				currentIndex.append(' ');
			}
			currentIndex.remove(currentIndex.size() - 1, 1);
			indexStrips.append(currentIndex);
			indexStrips.append(',');
		}
		indexStrips.remove(indexStrips.size() - 1, 1);
		m_pOutStream->writeAttribute("strips", indexStrips);
	}
	if (pMesh->containsFans(lod, materialId))
	{
		QList<QVector<GLuint> > fansIndex= pMesh->getFansIndex(lod, materialId);
		QString indexFans;
		const int fanCount= fansIndex.size();
		for (int fanIndex= 0; fanIndex < fanCount; ++fanIndex)
		{
			QVector<GLuint> currentFanIndex= fansIndex.at(fanIndex);
			QString currentIndex;
			const int indexSize= currentFanIndex.size();
			for (int index= 0; index < indexSize; ++index)
			{
				currentIndex.append(QString::number(currentFanIndex.at(index)));
				currentIndex.append(' ');
			}
			currentIndex.remove(currentIndex.size() - 1, 1);
			indexFans.append(currentIndex);
			indexFans.append(',');
		}
		indexFans.remove(indexFans.size() - 1, 1);
		m_pOutStream->writeAttribute("fans", indexFans);
	}

	writeSurfaceAttributes(pMesh->material(materialId));

	m_pOutStream->writeEndElement(); // Face

}

void GLC_WorldTo3dxml::writeSurfaceAttributes(const GLC_Material* pMaterial)
{
	QColor diffuseColor= pMaterial->diffuseColor();
	m_pOutStream->writeStartElement("SurfaceAttributes");
	if (m_ExportMaterial)
	{
		const QString material3dxmlId=(QString::number(m_MaterialIdToMaterialId.value(pMaterial->id())));
		m_pOutStream->writeStartElement("MaterialApplication");
			m_pOutStream->writeAttribute("xsi:type", "MaterialApplicationType");
			m_pOutStream->writeAttribute("mappingChannel", "0");
			m_pOutStream->writeStartElement("MaterialId");
				m_pOutStream->writeAttribute("id", "urn:3DXML:CATMaterialRef.3dxml#" + material3dxmlId);
			m_pOutStream->writeEndElement(); // MaterialId
		m_pOutStream->writeEndElement(); // MaterialApplication
	}
	else
	{
		m_pOutStream->writeStartElement("Color");
			m_pOutStream->writeAttribute("xsi:type", "RGBAColorType");
			m_pOutStream->writeAttribute("red", QString::number(diffuseColor.redF()));
			m_pOutStream->writeAttribute("green", QString::number(diffuseColor.greenF()));
			m_pOutStream->writeAttribute("blue", QString::number(diffuseColor.blueF()));
			m_pOutStream->writeAttribute("alpha", QString::number(diffuseColor.alphaF()));
		m_pOutStream->writeEndElement(); // Color
	}
	m_pOutStream->writeEndElement(); // SurfaceAttributes
}

void GLC_WorldTo3dxml::writeEdges(const GLC_Mesh* pMesh)
{
	m_pOutStream->writeStartElement("Edges");
	writeLineAttributes(pMesh->wireColor());

	GLfloatVector positionVector= pMesh->wirePositionVector();
	const int polylineCount= pMesh->wirePolylineCount();
	for (int i= 0; i < polylineCount; ++i)
	{
		m_pOutStream->writeStartElement("Polyline");
		QString polylinePosition;
		const GLuint offset= pMesh->wirePolylineOffset(i);
		const GLsizei size= pMesh->wirePolylineSize(i);
		for (GLsizei index= 0; index < size; ++index)
		{
			const int startIndex= 3 * (offset + index);
			polylinePosition.append(QString::number(positionVector.at(startIndex)));
			polylinePosition.append(' ');
			polylinePosition.append(QString::number(positionVector.at(startIndex + 1)));
			polylinePosition.append(' ');
			polylinePosition.append(QString::number(positionVector.at(startIndex + 2)));
			polylinePosition.append(',');
		}
		polylinePosition.remove(polylinePosition.size() - 1, 1);
		m_pOutStream->writeAttribute("vertices", polylinePosition);
		m_pOutStream->writeEndElement(); // Polyline
	}
	m_pOutStream->writeEndElement(); // Edges
}

void GLC_WorldTo3dxml::writeLineAttributes(const QColor& color)
{
	m_pOutStream->writeStartElement("LineAttributes");
	m_pOutStream->writeAttribute("lineType", "SOLID");
	m_pOutStream->writeAttribute("thickness", "1");
		m_pOutStream->writeStartElement("Color");
			m_pOutStream->writeAttribute("xsi:type", "RGBAColorType");
			m_pOutStream->writeAttribute("red", QString::number(color.redF()));
			m_pOutStream->writeAttribute("green", QString::number(color.greenF()));
			m_pOutStream->writeAttribute("blue", QString::number(color.blueF()));
			m_pOutStream->writeAttribute("alpha", QString::number(color.alphaF()));
		m_pOutStream->writeEndElement(); // Color
	m_pOutStream->writeEndElement(); // LineAttributes
}

void GLC_WorldTo3dxml::writeMaterial(const GLC_Material* pMaterial)
{
	const GLC_uint materialId= pMaterial->id();
	QString materialName;
	if (pMaterial->name().isEmpty())
	{
		materialName= "Material_0" + QString::number(materialId);
	}
	else
	{
		materialName= symplifyName(pMaterial->name());
	}


	m_MaterialIdToMaterialName.insert(materialId, materialName);

	const QString fileName= xmlFileName(materialName + "_Rendering.3DRep");
	setStreamWriterToFile(fileName);

	// Begin to write the material file
	m_pOutStream->writeStartDocument();
	m_pOutStream->writeStartElement("Osm");
		m_pOutStream->writeAttribute("xmlns", "http://www.3ds.com/xsd/osm.xsd");
		m_pOutStream->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
		m_pOutStream->writeAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
		m_pOutStream->writeAttribute("xsi:schemaLocation", "http://www.3ds.com/xsd/3DXML ./osm.xsd");
		m_pOutStream->writeStartElement("Feature");
			m_pOutStream->writeAttribute("Alias", "RenderingRootFeature");
			m_pOutStream->writeAttribute("Id", "1");
			m_pOutStream->writeAttribute("StartUp", "RenderingRootFeature");
		m_pOutStream->writeEndElement(); // Feature
		m_pOutStream->writeStartElement("Feature");
			m_pOutStream->writeAttribute("Alias", "RenderingFeature");
			m_pOutStream->writeAttribute("Id", "2");
			m_pOutStream->writeAttribute("StartUp", "RenderingFeature");
			m_pOutStream->writeAttribute("Aggregating", "1");

			writeMaterialAttributtes("Filtering", "int", "1");
			writeMaterialAttributtes("Rotation", "double", "0");
			writeMaterialAttributtes("PreviewType", "int", "1");
			writeMaterialAttributtes("AmbientCoef", "double", "1");
			writeMaterialAttributtes("DiffuseCoef", "double", "1");
			writeMaterialAttributtes("SpecularCoef", "double", "1");
			writeMaterialAttributtes("EmissiveCoef", "double", "0");
			writeMaterialAttributtes("AlphaTest", "boolean", "false");
			writeMaterialAttributtes("TextureFunction", "int", "0");
			writeMaterialAttributtes("MappingType", "int", "2");
			writeMaterialAttributtes("Refraction", "double", "1");
			writeMaterialAttributtes("TextureDimension", "int", "2");
			writeMaterialAttributtes("TranslationU", "double", "0");
			writeMaterialAttributtes("TranslationV", "double", "0");
			writeMaterialAttributtes("FlipU", "boolean", "false");
			writeMaterialAttributtes("FlipV", "boolean", "false");
			writeMaterialAttributtes("WrappingModeU", "int", "1");
			writeMaterialAttributtes("WrappingModeV", "int", "1");
			writeMaterialAttributtes("ScaleU", "double", "1");
			writeMaterialAttributtes("ScaleV", "double", "1");
			writeMaterialAttributtes("Reflectivity", "double", "0.1");
			writeMaterialAttributtes("BlendColor", "double", "[1,1,1]");

			writeMaterialAttributtes("Transparency", "double", QString::number(1.0 - pMaterial->opacity()));
			double specularExponent= pMaterial->shininess() / 128.0;
			writeMaterialAttributtes("SpecularExponent", "double", QString::number(specularExponent));
			writeMaterialAttributtes("DiffuseColor", "double", colorToString(pMaterial->diffuseColor()));
			writeMaterialAttributtes("SpecularColor", "double", colorToString(pMaterial->specularColor()));
			writeMaterialAttributtes("AmbientColor", "double", colorToString(pMaterial->ambientColor()));
			writeMaterialAttributtes("EmissiveColor", "double", colorToString(pMaterial->emissiveColor()));
			if (pMaterial->hasTexture())
			{
				Q_ASSERT(m_MaterialIdTo3dxmlImageId.contains(pMaterial->id()));
				const QString imageId(QString::number(m_MaterialIdTo3dxmlImageId.value(pMaterial->id())));

				writeMaterialAttributtes("TextureImage", "external", "urn:3DXML:CATRepImage.3dxml#" + imageId);
			}
		m_pOutStream->writeEndElement(); // Feature
	m_pOutStream->writeEndElement(); // Osm
	m_pOutStream->writeEndDocument();
}

void GLC_WorldTo3dxml::writeMaterialAttributtes(const QString& name, const QString& type, const QString& value)
{
	m_pOutStream->writeStartElement("Attr");
		m_pOutStream->writeAttribute("Name", name);
		m_pOutStream->writeAttribute("Type", type);
		m_pOutStream->writeAttribute("Value", value);
	m_pOutStream->writeEndElement(); // Attr
}

QString GLC_WorldTo3dxml::colorToString(const QColor& color)
{
	return QString('[' + QString::number(color.redF()) + ',' + QString::number(color.greenF()) + ',' + QString::number(color.blueF()) + ']');
}

void GLC_WorldTo3dxml::writeCatRepImageFile(const QList<GLC_Material*>& materialList)
{
	unsigned int currentId= 0;
	const QString fileName("CATRepImage.3dxml");
	setStreamWriterToFile(fileName);

	// Begin to write the rep image file
	m_pOutStream->writeStartDocument();
	m_pOutStream->writeStartElement("Model_3dxml");
		m_pOutStream->writeAttribute("xmlns", "http://www.3ds.com/xsd/3DXML");
		m_pOutStream->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
		m_pOutStream->writeAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
		m_pOutStream->writeAttribute("xsi:schemaLocation", "http://www.3ds.com/xsd/3DXML ./3DXML.xsd");
		writeHeader();
		m_pOutStream->writeStartElement("CATRepImage");
			m_pOutStream->writeAttribute("root", QString::number(currentId));
			const int size= materialList.size();
			for (int i= 0; i < size; ++i)
			{
				writeCATRepresentationImage(materialList.at(i), ++currentId);
			}
		m_pOutStream->writeEndElement(); // CATRepImage
	m_pOutStream->writeEndElement(); // Model_3dxml
	m_pOutStream->writeEndDocument();
}

void GLC_WorldTo3dxml::writeCATRepresentationImage(const GLC_Material* pMat, unsigned int id)
{
	Q_ASSERT(pMat->hasTexture());
	m_MaterialIdTo3dxmlImageId.insert(pMat->id(), id);

	const QString imageName= pMat->name();
	QString imageFileName;
	QString imageFormat;
	const QString textureFileName= pMat->textureHandle()->fileName();
	if (textureFileName.isEmpty())
	{
		imageFormat= "jpg";
		if (imageName.isEmpty())
		{
			imageFileName= xmlFileName("textureImage." + imageFormat);
		}
		else
		{
			imageFileName= xmlFileName(imageName + '.' + imageFormat);
		}
	}
	else
	{
		imageFormat= QFileInfo(textureFileName).suffix();
		imageFileName= xmlFileName(QFileInfo(textureFileName).fileName());
	}
	m_MaterialIdToTexture3dxmlName.insert(pMat->id(), imageFileName);

	m_pOutStream->writeStartElement("CATRepresentationImage");
		m_pOutStream->writeAttribute("xsi:type", "CATRepresentationImageType");
		m_pOutStream->writeAttribute("id", QString::number(id));
		m_pOutStream->writeAttribute("name", imageName);
		m_pOutStream->writeAttribute("format", imageFormat);
		m_pOutStream->writeAttribute("associatedFile", QString("urn:3DXML:" + imageFileName));

		m_pOutStream->writeTextElement("PLM_ExternalID", imageName);
		m_pOutStream->writeTextElement("V_PrimaryMimeType", imageFormat);
		m_pOutStream->writeTextElement("V_PrimaryFileName", imageFileName);
	m_pOutStream->writeEndElement(); // CATRepresentationImage
}

void GLC_WorldTo3dxml::writeAllMaterialRelatedFilesIn3dxml()
{
	m_MaterialIdToMaterialName.clear();
	m_MaterialIdToMaterialId.clear();
	m_MaterialIdToTexture3dxmlName.clear();
	m_MaterialIdTo3dxmlImageId.clear();

	// Get the list of material
	QList<GLC_Material*> materialList= m_World.listOfMaterials();

	// Create the list of textured material and modified material list
	QList<GLC_Material*> texturedMaterial;
	const int size= materialList.size();
	for (int i= 0; i < size; ++i)
	{
		if (materialList.at(i)->hasTexture())
		{
			texturedMaterial.append(materialList.at(i));
		}
	}

	if (!texturedMaterial.isEmpty())
	{
		writeCatRepImageFile(texturedMaterial);
		writeImageFileIn3dxml(texturedMaterial);
	}


	// Write material 3DRep in the 3DXML
	for (int i= 0; i < size; ++i)
	{
		writeMaterial(materialList.at(i));
	}

	writeCatMaterialRef(materialList);



}
void GLC_WorldTo3dxml::writeImageFileIn3dxml(const QList<GLC_Material*>& materialList)
{
	const int size= materialList.size();
	Q_ASSERT(size == m_MaterialIdToTexture3dxmlName.size());
	for (int i= 0; i < size; ++i)
	{
		const GLC_Material* pCurrentMaterial= materialList.at(i);
		const GLC_Texture* pTexture= pCurrentMaterial->textureHandle();
		const QString imageName(m_MaterialIdToTexture3dxmlName.value(pCurrentMaterial->id()));
		// Get the texture image
		if (!pTexture->fileName().isEmpty())
		{
			// Try to load the texture
			QImage textureImage(pTexture->fileName());
			if (! textureImage.isNull())
			{
				addImageTextureTo3dxml(textureImage, imageName);
			}
			else
			{
				addImageTextureTo3dxml(pTexture->imageOfTexture(), imageName);
			}
		}
		else
		{
			addImageTextureTo3dxml(pTexture->imageOfTexture(), imageName);
		}
	}

}

void GLC_WorldTo3dxml::writeCatMaterialRef(const QList<GLC_Material*>& materialList)
{
	const QString fileName("CATMaterialRef.3dxml");

	setStreamWriterToFile(fileName);

	unsigned int currentId= 0;

	// Begin to write the rep image file
	m_pOutStream->writeStartDocument();
	m_pOutStream->writeStartElement("Model_3dxml");
		m_pOutStream->writeAttribute("xmlns", "http://www.3ds.com/xsd/3DXML");
		m_pOutStream->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
		m_pOutStream->writeAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
		m_pOutStream->writeAttribute("xsi:schemaLocation", "http://www.3ds.com/xsd/3DXML ./3DXML.xsd");
		writeHeader();
		m_pOutStream->writeStartElement("CATMaterialRef");
			m_pOutStream->writeAttribute("root", QString::number(currentId));
			const int size= materialList.size();
			for (int i= 0; i < size; ++i)
			{
				writeMaterialToCatMaterialRef(materialList.at(i), &(++currentId));
			}
		m_pOutStream->writeEndElement(); // CATMaterialRef
	m_pOutStream->writeEndElement(); // Model_3dxml
	m_pOutStream->writeEndDocument();
}

void GLC_WorldTo3dxml::writeMaterialToCatMaterialRef(const GLC_Material* pMat, unsigned int* id)
{
	const QString materialName= m_MaterialIdToMaterialName.value(pMat->id());
	m_MaterialIdToMaterialId.insert(pMat->id(), *id);

	m_pOutStream->writeStartElement("CATMatReference");
		m_pOutStream->writeAttribute("xsi:type", "CATMatReferenceType");
		m_pOutStream->writeAttribute("id", QString::number(*id));
		m_pOutStream->writeAttribute("name", materialName);
		m_pOutStream->writeTextElement("PLM_ExternalID", materialName);
	m_pOutStream->writeEndElement(); // CATMatReference

	const QString domainName= materialName + "_Rendering";
	m_pOutStream->writeStartElement("MaterialDomain");
		m_pOutStream->writeAttribute("xsi:type", "MaterialDomainType");
		m_pOutStream->writeAttribute("id", QString::number(++(*id)));
		m_pOutStream->writeAttribute("name", domainName);
		m_pOutStream->writeAttribute("format", "TECHREP");
		const QString associatedFileName("urn:3DXML:" + domainName + ".3DRep");
		m_pOutStream->writeAttribute("associatedFile", associatedFileName);
		m_pOutStream->writeAttribute("version", "1.0");
		m_pOutStream->writeTextElement("PLM_ExternalID", domainName);
		if (pMat->hasTexture())
		{
			m_pOutStream->writeStartElement("PLMRelation");
				m_pOutStream->writeTextElement("C_Semantics", "Reference");
				m_pOutStream->writeTextElement("C_Role", "CATMaterialReferenceToTextureLink");
				m_pOutStream->writeStartElement("Ids");
					const QString imageId(QString::number(m_MaterialIdTo3dxmlImageId.value(pMat->id())));
					m_pOutStream->writeTextElement("id", "urn:3DXML:CATRepImage.3dxml#" + imageId);
				m_pOutStream->writeEndElement(); // Ids
			m_pOutStream->writeEndElement(); // PLMRelation
		}
		m_pOutStream->writeTextElement("V_MatDomain", "Rendering");
	m_pOutStream->writeEndElement(); // MaterialDomain

	m_pOutStream->writeStartElement("MaterialDomainInstance");
		m_pOutStream->writeAttribute("xsi:type", "MaterialDomainInstanceType");
		m_pOutStream->writeAttribute("id", QString::number(++(*id)));
		m_pOutStream->writeAttribute("name", materialName + ".1");
		m_pOutStream->writeTextElement("PLM_ExternalID", materialName + ".1");
		m_pOutStream->writeTextElement("IsAggregatedBy", QString::number((*id) - 2));
		m_pOutStream->writeTextElement("IsInstanceOf", QString::number((*id) - 1));
	m_pOutStream->writeEndElement(); // MaterialDomainInstance
}

void GLC_WorldTo3dxml::addImageTextureTo3dxml(const QImage& image, const QString& fileName)
{
	delete m_pOutStream;
	m_pOutStream= NULL;

	bool success= false;
	if (NULL != m_p3dxmlArchive)
	{
		if (NULL != m_pCurrentZipFile)
		{
			m_pCurrentZipFile->close();
			delete m_pOutStream;
			delete m_pCurrentZipFile;
		}
		QuaZipNewInfo quazipNewInfo(fileName);
		m_pCurrentZipFile= new QuaZipFile(m_p3dxmlArchive);
		success= m_pCurrentZipFile->open(QIODevice::WriteOnly, quazipNewInfo);
		if (success)
		{
			image.save(m_pCurrentZipFile, QFileInfo(fileName).suffix().toAscii().constData());
			m_pCurrentZipFile->close();
			delete m_pCurrentZipFile;
			m_pCurrentZipFile= NULL;
		}
	}
	else
	{
		delete m_pCurrentFile;
		m_pCurrentFile= new QFile(m_AbsolutePath + fileName);
		success= m_pCurrentFile->open(QIODevice::WriteOnly);
		if (success)
		{
			image.save(m_pCurrentFile, QFileInfo(fileName).suffix().toAscii().constData());
			delete m_pCurrentFile;
			m_pCurrentFile= NULL;
		}
	}
}

QString GLC_WorldTo3dxml::xmlFileName(QString fileName)
{
	QString prefix;
	if (fileName.contains("urn:3DXML:"))
	{
		prefix= "urn:3DXML:";
		fileName.remove(prefix);
	}

	fileName= symplifyName(fileName);



	QString newName;
	if (!m_3dxmlFileSet.contains(prefix + fileName))
	{
		fileName.prepend(prefix);
		m_3dxmlFileSet << fileName;
		newName= fileName;
	}
	else
	{
		newName= QFileInfo(fileName).completeBaseName() + QString::number(++m_FileNameIncrement) + '.' + QFileInfo(fileName).suffix();
		newName.prepend(prefix);
	}
	return newName;
}

void GLC_WorldTo3dxml::writeExtensionAttributes(GLC_Attributes* pAttributes)
{
	QList<QString> attributesNames= pAttributes->names();
	const int size= attributesNames.size();
	for (int i= 0; i < size; ++i)
	{
		const QString name= attributesNames.at(i);
		QString value= pAttributes->value(name);
		m_pOutStream->writeStartElement("Attribute");
			m_pOutStream->writeAttribute("name", name);
			if (name == "FILEPATH")
			{
				QDir thisPath(m_AbsolutePath);
				value= thisPath.relativeFilePath(value);
			}
			m_pOutStream->writeAttribute("value", value);
		m_pOutStream->writeEndElement(); // Attribute
	}
}

void GLC_WorldTo3dxml::writeOccurenceDefaultViewProperty(const GLC_StructOccurence* pOccurence)
{
	QList<unsigned int> path= instancePath(pOccurence);

	GLC_3DViewInstance* pInstance= m_World.collection()->instanceHandle(pOccurence->id());
	Q_ASSERT(NULL != pInstance);
	const bool isVisible= pOccurence->isVisible();
	m_pOutStream->writeStartElement("DefaultViewProperty");
	m_pOutStream->writeStartElement("OccurenceId");
	const QString prefix= "urn:3DXML:" + QFileInfo(m_FileName).fileName() + "#";
	const int pathSize= path.size();
	for (int i= 0; i < pathSize; ++i)
	{
		m_pOutStream->writeTextElement("id", prefix + QString::number(path.at(i)));
	}
	m_pOutStream->writeEndElement(); // OccurenceId

	if (pOccurence->isFlexible())
	{
		m_pOutStream->writeTextElement("RelativePosition", matrixString(pOccurence->occurrenceRelativeMatrix()));
	}

	if (!isVisible || !pInstance->renderPropertiesHandle()->isDefault())
	{
		qDebug() << "(!isVisible || !pInstance->renderPropertiesHandle()->isDefault())";
		m_pOutStream->writeStartElement("GraphicProperties");
		m_pOutStream->writeAttribute("xsi:type", "GraphicPropertiesType");
		if (! isVisible)
		{
			m_pOutStream->writeStartElement("GeneralAttributes");
				m_pOutStream->writeAttribute("xsi:type", "GeneralAttributesType");
				m_pOutStream->writeAttribute("visible", "false");
				m_pOutStream->writeAttribute("selectable", "true");
			m_pOutStream->writeEndElement(); // GeneralAttributes
		}
		if (!pInstance->renderPropertiesHandle()->isDefault())
		{
			const GLC_RenderProperties* pProperties= pInstance->renderPropertiesHandle();
			if ((pProperties->renderingMode() == glc::OverwriteTransparency))
			{
				m_pOutStream->writeStartElement("SurfaceAttributes");
				m_pOutStream->writeAttribute("xsi:type", "SurfaceAttributesType");
					m_pOutStream->writeStartElement("Color");
						m_pOutStream->writeAttribute("xsi:type", "RGBAColorType");
						m_pOutStream->writeAttribute("red", "-1");
						m_pOutStream->writeAttribute("green", "-1");
						m_pOutStream->writeAttribute("blue", "-1");
						m_pOutStream->writeAttribute("alpha", QString::number(pProperties->overwriteTransparency()));
					m_pOutStream->writeEndElement(); // Color
				m_pOutStream->writeEndElement(); // SurfaceAttributes
			}
			else if ((pProperties->renderingMode() == glc::OverwriteTransparencyAndMaterial))
			{
				GLC_Material* pMaterial= pProperties->overwriteMaterial();
				m_pOutStream->writeStartElement("SurfaceAttributes");
				m_pOutStream->writeAttribute("xsi:type", "SurfaceAttributesType");
					m_pOutStream->writeStartElement("Color");
						m_pOutStream->writeAttribute("xsi:type", "RGBAColorType");
						m_pOutStream->writeAttribute("red", QString::number(pMaterial->diffuseColor().redF()));
						m_pOutStream->writeAttribute("green", QString::number(pMaterial->diffuseColor().greenF()));
						m_pOutStream->writeAttribute("blue", QString::number(pMaterial->diffuseColor().blueF()));
						m_pOutStream->writeAttribute("alpha", QString::number(pProperties->overwriteTransparency()));
					m_pOutStream->writeEndElement(); // Color
				m_pOutStream->writeEndElement(); // SurfaceAttributes
			}
			else if ((pProperties->renderingMode() == glc::OverwriteMaterial))
			{
				GLC_Material* pMaterial= pProperties->overwriteMaterial();
				m_pOutStream->writeStartElement("SurfaceAttributes");
				m_pOutStream->writeAttribute("xsi:type", "SurfaceAttributesType");
					m_pOutStream->writeStartElement("Color");
						m_pOutStream->writeAttribute("xsi:type", "RGBAColorType");
						m_pOutStream->writeAttribute("red", QString::number(pMaterial->diffuseColor().redF()));
						m_pOutStream->writeAttribute("green", QString::number(pMaterial->diffuseColor().greenF()));
						m_pOutStream->writeAttribute("blue", QString::number(pMaterial->diffuseColor().blueF()));
						m_pOutStream->writeAttribute("alpha", QString::number(pMaterial->opacity()));
					m_pOutStream->writeEndElement(); // Color
				m_pOutStream->writeEndElement(); // SurfaceAttributes
			}

		}
		m_pOutStream->writeEndElement(); // GraphicProperties
	}
	m_pOutStream->writeEndElement(); // DefaultViewProperty
}

bool GLC_WorldTo3dxml::continu()
{
	bool continuValue= true;
	if (NULL != m_pReadWriteLock)
	{
		Q_ASSERT(NULL != m_pIsInterupted);
		m_pReadWriteLock->lockForRead();
		continuValue= !(*m_pIsInterupted);
		m_pReadWriteLock->unlock();
	}
	return continuValue;
}

QString GLC_WorldTo3dxml::symplifyName(QString name)
{
	const int nameSize= name.size();
	for (int i= 0; i < nameSize; ++i)
	{
		if (!name.at(i).isLetterOrNumber() && (name.at(i) != '.'))
		{
			name.replace(i, 1, '_');
		}
	}

	return name;
}

QList<unsigned int> GLC_WorldTo3dxml::instancePath(const GLC_StructOccurence* pOccurence)
{
	QList<unsigned int> path;
	if (!pOccurence->isOrphan())
	{
		GLC_StructInstance* pInstance= pOccurence->structInstance();
		Q_ASSERT(m_InstanceToIdHash.contains(pInstance));
		path.prepend(m_InstanceToIdHash.value(pInstance));
		QList<unsigned int> subPath(instancePath(pOccurence->parent()));
		subPath.append(path);
		path= subPath;
	}
	return path;
}
