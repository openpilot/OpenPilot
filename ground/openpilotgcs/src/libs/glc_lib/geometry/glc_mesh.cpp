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

//! \file glc_mesh.cpp Implementation for the GLC_Mesh class.

#include "glc_mesh.h"
#include "../glc_renderstatistics.h"
#include "../glc_context.h"

// Class chunk id
quint32 GLC_Mesh::m_ChunkId= 0xA701;

GLC_Mesh::GLC_Mesh()
:GLC_Geometry("Mesh", false)
, m_NextPrimitiveLocalId(1)
, m_PrimitiveGroups()
, m_DefaultMaterialId(0)
, m_NumberOfVertice(0)
, m_NumberOfNormals(0)
, m_ColorPearVertex(false)
, m_MeshData()
, m_CurrentLod(0)
{

}

GLC_Mesh::GLC_Mesh(const GLC_Mesh& mesh)
:GLC_Geometry(mesh)
, m_NextPrimitiveLocalId(mesh.m_NextPrimitiveLocalId)
, m_PrimitiveGroups(mesh.m_PrimitiveGroups)
, m_DefaultMaterialId(mesh.m_DefaultMaterialId)
, m_NumberOfVertice(mesh.m_NumberOfVertice)
, m_NumberOfNormals(mesh.m_NumberOfNormals)
, m_ColorPearVertex(mesh.m_ColorPearVertex)
, m_MeshData(mesh.m_MeshData)
, m_CurrentLod(0)
{
	// Make a copy of m_PrimitiveGroups with new material id
	PrimitiveGroupsHash::const_iterator iPrimitiveGroups= mesh.m_PrimitiveGroups.constBegin();
	while (mesh.m_PrimitiveGroups.constEnd() != iPrimitiveGroups)
	{
		LodPrimitiveGroups* pPrimitiveGroups= new LodPrimitiveGroups();
		m_PrimitiveGroups.insert(iPrimitiveGroups.key(), pPrimitiveGroups);

		LodPrimitiveGroups::const_iterator iPrimitiveGroup= iPrimitiveGroups.value()->constBegin();
		while (iPrimitiveGroups.value()->constEnd() != iPrimitiveGroup)
		{
			GLC_PrimitiveGroup* pPrimitiveGroup= new GLC_PrimitiveGroup(*(iPrimitiveGroup.value()), iPrimitiveGroup.key());
			pPrimitiveGroups->insert(iPrimitiveGroup.key(), pPrimitiveGroup);

			++iPrimitiveGroup;
		}

		++iPrimitiveGroups;
	}

}

// Overload "=" operator
GLC_Mesh& GLC_Mesh::operator=(const GLC_Mesh& mesh)
{
	if (this != &mesh)
	{
		// Call the operator of the super class
		GLC_Geometry::operator=(mesh);

		// Clear the mesh
		clearMeshWireAndBoundingBox();

		// Copy members
		m_NextPrimitiveLocalId= mesh.m_NextPrimitiveLocalId;
		m_PrimitiveGroups= mesh.m_PrimitiveGroups;
		m_DefaultMaterialId= mesh.m_DefaultMaterialId;
		m_NumberOfVertice= mesh.m_NumberOfVertice;
		m_NumberOfNormals= mesh.m_NumberOfNormals;
		m_ColorPearVertex= mesh.m_ColorPearVertex;
		m_MeshData= mesh.m_MeshData;
		m_CurrentLod= 0;

		// Make a copy of m_PrimitiveGroups with new material id
		PrimitiveGroupsHash::const_iterator iPrimitiveGroups= mesh.m_PrimitiveGroups.constBegin();
		while (mesh.m_PrimitiveGroups.constEnd() != iPrimitiveGroups)
		{
			LodPrimitiveGroups* pPrimitiveGroups= new LodPrimitiveGroups();
			m_PrimitiveGroups.insert(iPrimitiveGroups.key(), pPrimitiveGroups);

			LodPrimitiveGroups::const_iterator iPrimitiveGroup= iPrimitiveGroups.value()->constBegin();
			while (iPrimitiveGroups.value()->constEnd() != iPrimitiveGroup)
			{
				GLC_PrimitiveGroup* pPrimitiveGroup= new GLC_PrimitiveGroup(*(iPrimitiveGroup.value()), iPrimitiveGroup.key());
				pPrimitiveGroups->insert(iPrimitiveGroup.key(), pPrimitiveGroup);

				++iPrimitiveGroup;
			}

			++iPrimitiveGroups;
		}
	}

	return *this;
}

// Destructor
GLC_Mesh::~GLC_Mesh()
{
	PrimitiveGroupsHash::iterator iGroups= m_PrimitiveGroups.begin();
	while (iGroups != m_PrimitiveGroups.constEnd())
	{
		LodPrimitiveGroups::iterator iGroup= iGroups.value()->begin();
		while (iGroup != iGroups.value()->constEnd())
		{
			delete iGroup.value();

			++iGroup;
		}
		delete iGroups.value();
		++iGroups;
	}
}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////
// Return the class Chunk ID
quint32 GLC_Mesh::chunckID()
{
	return m_ChunkId;
}

// Get number of faces
unsigned int GLC_Mesh::faceCount(int lod) const
{
	return m_MeshData.trianglesCount(lod);
}

// Get number of vertex
unsigned int GLC_Mesh::VertexCount() const
{
	return m_NumberOfVertice;
}

// return the mesh bounding box
const GLC_BoundingBox& GLC_Mesh::boundingBox()
{
	if (NULL == m_pBoundingBox)
	{
		//qDebug() << "GLC_Mesh2::boundingBox create boundingBox";
		m_pBoundingBox= new GLC_BoundingBox();

		if (m_MeshData.positionVectorHandle()->isEmpty())
		{
			qDebug() << "GLC_Mesh::boundingBox empty m_Positions";
		}
		else
		{
			GLfloatVector* pVertexVector= m_MeshData.positionVectorHandle();
			const int max= pVertexVector->size();
			for (int i= 0; i < max; i= i + 3)
			{
				GLC_Vector3d vector((*pVertexVector)[i], (*pVertexVector)[i + 1], (*pVertexVector)[i + 2]);
				m_pBoundingBox->combine(vector);
			}
		}
		// Combine with the wiredata bounding box
		m_pBoundingBox->combine(m_WireData.boundingBox());
	}
	return *m_pBoundingBox;

}

// Return a copy of the Mesh as GLC_Geometry pointer
GLC_Geometry* GLC_Mesh::clone() const
{
	return new GLC_Mesh(*this);
}

// Return true if the mesh contains triangles
bool GLC_Mesh::containsTriangles(int lod, GLC_uint materialId) const
{
	// Check if the lod exist and material exists
	Q_ASSERT(m_PrimitiveGroups.contains(lod));
	if (!m_PrimitiveGroups.value(lod)->contains(materialId)) return false;
	else return m_PrimitiveGroups.value(lod)->value(materialId)->containsTriangles();
}

// Return the specified index
QVector<GLuint> GLC_Mesh::getTrianglesIndex(int lod, GLC_uint materialId) const
{
	// Check if the mesh contains triangles
	Q_ASSERT(containsTriangles(lod, materialId));

	GLC_PrimitiveGroup* pPrimitiveGroup= m_PrimitiveGroups.value(lod)->value(materialId);

	int offset= 0;
	if (vboIsUsed())
	{
		offset= static_cast<int>(reinterpret_cast<GLsizeiptr>(pPrimitiveGroup->trianglesIndexOffset()) / sizeof(GLuint));
	}
	else
	{
		offset= pPrimitiveGroup->trianglesIndexOffseti();
	}
	const int size= pPrimitiveGroup->trianglesIndexSize();

	QVector<GLuint> resultIndex(size);

	memcpy((void*)resultIndex.data(), &(m_MeshData.indexVector(lod).data())[offset], size * sizeof(GLuint));

	return resultIndex;
}

// Return the number of triangles
int GLC_Mesh::numberOfTriangles(int lod, GLC_uint materialId) const
{
	// Check if the lod exist and material exists
	if (!m_PrimitiveGroups.contains(lod))return 0;
	else if (!m_PrimitiveGroups.value(lod)->contains(materialId)) return 0;
	else return m_PrimitiveGroups.value(lod)->value(materialId)->trianglesIndexSize();
}

// Return true if the mesh contains trips
bool GLC_Mesh::containsStrips(int lod, GLC_uint materialId) const
{
	// Check if the lod exist and material exists
	if (!m_PrimitiveGroups.contains(lod))return false;
	else if (!m_PrimitiveGroups.value(lod)->contains(materialId)) return false;
	else return m_PrimitiveGroups.value(lod)->value(materialId)->containsStrip();

}

// Return the strips index
QList<QVector<GLuint> > GLC_Mesh::getStripsIndex(int lod, GLC_uint materialId) const
{
	// Check if the mesh contains trips
	Q_ASSERT(containsStrips(lod, materialId));

	GLC_PrimitiveGroup* pPrimitiveGroup= m_PrimitiveGroups.value(lod)->value(materialId);

	QList<int> offsets;
	QList<int> sizes;
	int stripsCount;

	if (vboIsUsed())
	{
		stripsCount= pPrimitiveGroup->stripsOffset().size();
		for (int i= 0; i < stripsCount; ++i)
		{
			offsets.append(static_cast<int>(reinterpret_cast<GLsizeiptr>(pPrimitiveGroup->stripsOffset().at(i)) / sizeof(GLuint)));
			sizes.append(static_cast<int>(pPrimitiveGroup->stripsSizes().at(i)));
		}
	}
	else
	{
		stripsCount= pPrimitiveGroup->stripsOffseti().size();
		for (int i= 0; i < stripsCount; ++i)
		{
			offsets.append(static_cast<int>(pPrimitiveGroup->stripsOffseti().at(i)));
			sizes.append(static_cast<int>(pPrimitiveGroup->stripsSizes().at(i)));
		}

	}
	// The result list of vector
	QList<QVector<GLuint> > result;
	// The copy of the mesh Data LOD index vector
	QVector<GLuint> SourceIndex(m_MeshData.indexVector(lod));
	for (int i= 0; i < stripsCount; ++i)
	{
		QVector<GLuint> currentStrip(sizes.at(i));
		memcpy((void*)currentStrip.data(), &(SourceIndex.data())[offsets.at(i)], sizes.at(i) * sizeof(GLuint));
		result.append(currentStrip);
	}

	return result;
}

// Return the number of strips
int GLC_Mesh::numberOfStrips(int lod, GLC_uint materialId) const
{
	// Check if the lod exist and material exists
	if (!m_PrimitiveGroups.contains(lod))return 0;
	else if (!m_PrimitiveGroups.value(lod)->contains(materialId)) return 0;
	else return m_PrimitiveGroups.value(lod)->value(materialId)->stripsSizes().size();
}

// Return true if the mesh contains fans
bool GLC_Mesh::containsFans(int lod, GLC_uint materialId) const
{
	// Check if the lod exist and material exists
	if (!m_PrimitiveGroups.contains(lod))return false;
	else if (!m_PrimitiveGroups.value(lod)->contains(materialId)) return false;
	else return m_PrimitiveGroups.value(lod)->value(materialId)->containsFan();

}

//! Return the number of fans
int GLC_Mesh::numberOfFans(int lod, GLC_uint materialId) const
{
	// Check if the lod exist and material exists
	if(!m_PrimitiveGroups.contains(lod))return 0;
	else if (!m_PrimitiveGroups.value(lod)->contains(materialId)) return 0;
	else return m_PrimitiveGroups.value(lod)->value(materialId)->fansSizes().size();
}

// Return the strips index
QList<QVector<GLuint> > GLC_Mesh::getFansIndex(int lod, GLC_uint materialId) const
{
	// Check if the mesh contains trips
	Q_ASSERT(containsFans(lod, materialId));

	GLC_PrimitiveGroup* pPrimitiveGroup= m_PrimitiveGroups.value(lod)->value(materialId);

	QList<int> offsets;
	QList<int> sizes;
	int fansCount;

	if (vboIsUsed())
	{
		fansCount= pPrimitiveGroup->fansOffset().size();
		for (int i= 0; i < fansCount; ++i)
		{
			offsets.append(static_cast<int>(reinterpret_cast<GLsizeiptr>(pPrimitiveGroup->fansOffset().at(i)) / sizeof(GLuint)));
			sizes.append(static_cast<int>(pPrimitiveGroup->fansSizes().at(i)));
		}
	}
	else
	{
		fansCount= pPrimitiveGroup->fansOffseti().size();
		for (int i= 0; i < fansCount; ++i)
		{
			offsets.append(static_cast<int>(pPrimitiveGroup->fansOffseti().at(i)));
			sizes.append(static_cast<int>(pPrimitiveGroup->fansSizes().at(i)));
		}

	}
	// The result list of vector
	QList<QVector<GLuint> > result;
	// The copy of the mesh Data LOD index vector
	QVector<GLuint> SourceIndex(m_MeshData.indexVector(lod));
	for (int i= 0; i < fansCount; ++i)
	{
		QVector<GLuint> currentFan(sizes.at(i));
		memcpy((void*)currentFan.data(), &(SourceIndex.data())[offsets.at(i)], sizes.at(i) * sizeof(GLuint));
		result.append(currentFan);
	}

	return result;
}

GLC_Mesh* GLC_Mesh::createMeshOfGivenLod(int lodIndex)
{
	Q_ASSERT(m_MeshData.lodCount() > lodIndex);

	copyVboToClientSide();
	GLC_Mesh* pLodMesh= new GLC_Mesh;
	pLodMesh->setName(this->name() + "-LOD-" + QString::number(lodIndex));
	QHash<GLuint, GLuint> sourceToTargetIndexMap;
	QHash<GLuint, GLuint> tagetToSourceIndexMap;
	int maxIndex= -1;

	int targetLod= 0;
	copyIndex(lodIndex, pLodMesh, sourceToTargetIndexMap, tagetToSourceIndexMap, maxIndex, targetLod);

	copyBulkData(pLodMesh, tagetToSourceIndexMap, maxIndex);

	pLodMesh->finish();

	releaseVboClientSide(false);

	return pLodMesh;
}

GLC_Mesh* GLC_Mesh::createMeshFromGivenLod(int lodIndex)
{
	const int lodCount= m_MeshData.lodCount();
	Q_ASSERT(lodCount > lodIndex);

	copyVboToClientSide();
	GLC_Mesh* pLodMesh= new GLC_Mesh;
	pLodMesh->setName(this->name() + "-LOD-" + QString::number(lodIndex));
	QHash<GLuint, GLuint> sourceToTargetIndexMap;
	QHash<GLuint, GLuint> tagetToSourceIndexMap;
	int maxIndex= -1;

	if ((lodCount - lodIndex) > 1)
	{
		int targetLod= 1;
		for (int i= lodIndex + 1; i < lodCount; ++i)
		{
			copyIndex(i, pLodMesh, sourceToTargetIndexMap, tagetToSourceIndexMap, maxIndex, targetLod);
			++targetLod;
		}
		copyIndex(lodIndex, pLodMesh, sourceToTargetIndexMap, tagetToSourceIndexMap, maxIndex, 0);
	}
	else
	{
		copyIndex(lodIndex, pLodMesh, sourceToTargetIndexMap, tagetToSourceIndexMap, maxIndex, 0);
	}


	copyBulkData(pLodMesh, tagetToSourceIndexMap, maxIndex);

	pLodMesh->finish();

	releaseVboClientSide(false);

	return pLodMesh;
}
GLC_Mesh& GLC_Mesh::transformVertice(const GLC_Matrix4x4& matrix)
{
	if (matrix.type() != GLC_Matrix4x4::Identity)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
		copyVboToClientSide();
		const int stride= 3;
		GLfloatVector* pVectPos= m_MeshData.positionVectorHandle();
		const GLC_Matrix4x4 rotationMatrix= matrix.rotationMatrix();
		GLfloatVector* pVectNormal= m_MeshData.normalVectorHandle();
		const int verticeCount= pVectPos->size() / stride;
		for (int i= 0; i < verticeCount; ++i)
		{
			GLC_Vector3d newPos(pVectPos->at(stride * i), pVectPos->at(stride * i + 1), pVectPos->at(stride * i + 2));
			newPos= matrix * newPos;
			pVectPos->operator[](stride * i)= static_cast<GLfloat>(newPos.x());
			pVectPos->operator[](stride * i + 1)= static_cast<GLfloat>(newPos.y());
			pVectPos->operator[](stride * i + 2)= static_cast<GLfloat>(newPos.z());

			GLC_Vector3d newNormal(pVectNormal->at(stride * i), pVectNormal->at(stride * i + 1), pVectNormal->at(stride * i + 2));
			newNormal= rotationMatrix * newNormal;
			pVectNormal->operator[](stride * i)= static_cast<GLfloat>(newNormal.x());
			pVectNormal->operator[](stride * i + 1)= static_cast<GLfloat>(newNormal.y());
			pVectNormal->operator[](stride * i + 2)= static_cast<GLfloat>(newNormal.z());
		}
		releaseVboClientSide(true);
	}

	return *this;
}

double GLC_Mesh::volume()
{
	double resultVolume= 0.0;

	if (!m_MeshData.isEmpty())
	{
		IndexList triangleIndex;
		QList<GLC_Material*> materials= materialSet().toList();
		const int materialsCount= materials.count();
		for (int i= 0; i < materialsCount; ++i)
		{
			GLC_uint materialId= materials.at(i)->id();
			if (containsTriangles(0, materialId))
			{
				triangleIndex.append(getTrianglesIndex(0, materialId).toList());
			}
			if (containsStrips(0, materialId))
			{
				triangleIndex.append(equivalentTrianglesIndexOfstripsIndex(0, materialId));
			}
			if (containsFans(0, materialId))
			{
				triangleIndex.append(equivalentTrianglesIndexOfFansIndex(0, materialId));
			}
		}

		GLfloatVector vertices= m_MeshData.positionVector();
		Q_ASSERT((triangleIndex.count() % 3) == 0);
		const int triangleCount= triangleIndex.count() / 3;
		for (int i= 0; i < triangleCount; ++i)
		{
			const int index= i * 3;
			const double v1x= vertices.at(triangleIndex.at(index) * 3);
			const double v1y= vertices.at(triangleIndex.at(index) * 3 + 1);
			const double v1z= vertices.at(triangleIndex.at(index) * 3 + 2);

			const double v2x= vertices.at(triangleIndex.at(index + 1) * 3);
			const double v2y= vertices.at(triangleIndex.at(index + 1) * 3 + 1);
			const double v2z= vertices.at(triangleIndex.at(index + 1) * 3 + 2);

			const double v3x= vertices.at(triangleIndex.at(index + 2) * 3);
			const double v3y= vertices.at(triangleIndex.at(index + 2) * 3 + 1);
			const double v3z= vertices.at(triangleIndex.at(index + 2) * 3 + 2);

			resultVolume+= ((v2y - v1y) * (v3z - v1z) - (v2z - v1z) * (v3y - v1y)) * (v1x + v2x + v3x);
		}

		resultVolume= resultVolume / 6.0;
	}

	return resultVolume;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Clear the content of the mesh and super class GLC_Geometry
void GLC_Mesh::clear()
{
	// Clear the mesh content
	clearMeshWireAndBoundingBox();

	// Clear the super class GLC_Geometry
	GLC_Geometry::clear();
}


// Clear the content off the mesh and makes it empty
void GLC_Mesh::clearMeshWireAndBoundingBox()
{
	// Reset primitive local id
	m_NextPrimitiveLocalId= 1;

	// Remove all primitive groups
	PrimitiveGroupsHash::iterator iGroups= m_PrimitiveGroups.begin();
	while (iGroups != m_PrimitiveGroups.constEnd())
	{
		LodPrimitiveGroups::iterator iGroup= iGroups.value()->begin();
		while (iGroup != iGroups.value()->constEnd())
		{
			delete iGroup.value();

			++iGroup;
		}
		delete iGroups.value();
		++iGroups;
	}
	m_PrimitiveGroups.clear();

	m_DefaultMaterialId= 0;
	m_NumberOfVertice= 0;
	m_NumberOfNormals= 0;
	m_IsSelected= false;
	m_ColorPearVertex= false;
	// Clear data of the mesh
	m_MeshData.clear();
	m_CurrentLod= 0;

	GLC_Geometry::clearWireAndBoundingBox();
}

// Add triangles
GLC_uint GLC_Mesh::addTriangles(GLC_Material* pMaterial, const IndexList& indexList, const int lod, double accuracy)
{
	GLC_uint groupId= setCurrentMaterial(pMaterial, lod, accuracy);
	Q_ASSERT(m_PrimitiveGroups.value(lod)->contains(groupId));
	Q_ASSERT(!indexList.isEmpty());

	GLC_uint id= 0;
	if (0 == lod)
	{
		id= m_NextPrimitiveLocalId++;
	}
	m_MeshData.trianglesAdded(lod, indexList.size() / 3);

	m_PrimitiveGroups.value(lod)->value(groupId)->addTriangles(indexList, id);

	// Invalid the geometry
	m_GeometryIsValid = false;

	return id;
}

// Add triangles Strip ans return his id
GLC_uint GLC_Mesh::addTrianglesStrip(GLC_Material* pMaterial, const IndexList& indexList, const int lod, double accuracy)
{
	GLC_uint groupId= setCurrentMaterial(pMaterial, lod, accuracy);
	Q_ASSERT(m_PrimitiveGroups.value(lod)->contains(groupId));
	Q_ASSERT(!indexList.isEmpty());

	GLC_uint id= 0;
	if (0 == lod)
	{
		id= m_NextPrimitiveLocalId++;
	}
	m_MeshData.trianglesAdded(lod, indexList.size() - 2);

	m_PrimitiveGroups.value(lod)->value(groupId)->addTrianglesStrip(indexList, id);

	// Invalid the geometry
	m_GeometryIsValid = false;

	return id;
}
// Add triangles Fan
GLC_uint GLC_Mesh::addTrianglesFan(GLC_Material* pMaterial, const IndexList& indexList, const int lod, double accuracy)
{
	GLC_uint groupId= setCurrentMaterial(pMaterial, lod, accuracy);
	Q_ASSERT(m_PrimitiveGroups.value(lod)->contains(groupId));
	Q_ASSERT(!indexList.isEmpty());

	GLC_uint id= 0;
	if (0 == lod)
	{
		id= m_NextPrimitiveLocalId++;
	}
	m_MeshData.trianglesAdded(lod, indexList.size() - 2);
	m_PrimitiveGroups.value(lod)->value(groupId)->addTrianglesFan(indexList, id);

	// Invalid the geometry
	m_GeometryIsValid = false;

	return id;
}

// Reverse mesh normal
void GLC_Mesh::reverseNormals()
{
	GLfloatVector* pNormalVector= m_MeshData.normalVectorHandle();
	if (pNormalVector->isEmpty())
	{
		(*m_MeshData.normalVectorHandle())= m_MeshData.normalVector();
	}
	const int size= pNormalVector->size();
	for (int i= 0; i < size; ++i)
	{
		(*pNormalVector)[i]= - pNormalVector->at(i);
	}
	if (vboIsUsed())
	{
		m_MeshData.fillVbo(GLC_MeshData::GLC_Normal);
		m_MeshData.useVBO(false, GLC_MeshData::GLC_Normal);
	}
}

// Copy index list in a vector for Vertex Array Use
void GLC_Mesh::finish()
{
	boundingBox();

	m_MeshData.finishLod();

	moveIndexToMeshDataLod();

	//qDebug() << "Mesh mem size= " << memmorySize();
}


// Set the lod Index
void GLC_Mesh::setCurrentLod(const int value)
{
	if (value)
	{
		const int numberOfLod= m_MeshData.lodCount() - 1;
		// Clamp value to number of load
		m_CurrentLod= qRound(static_cast<int>((static_cast<double>(value) / 100.0) * numberOfLod));
	}
	else
	{
		m_CurrentLod= 0;
	}
}
// Replace the Master material
void GLC_Mesh::replaceMasterMaterial(GLC_Material* pMat)
{
	if (hasMaterial())
	{
		GLC_uint oldId= firstMaterial()->id();
		replaceMaterial(oldId, pMat);
	}
	else
	{
		addMaterial(pMat);
	}
}

// Replace the material specified by id with another one
void GLC_Mesh::replaceMaterial(const GLC_uint oldId, GLC_Material* pMat)
{
	Q_ASSERT(containsMaterial(oldId));
	Q_ASSERT(!containsMaterial(pMat->id()) || (pMat->id() == oldId));

	if (pMat->id() != oldId)
	{
		// Iterate over Level of detail
		PrimitiveGroupsHash::const_iterator iGroups= m_PrimitiveGroups.constBegin();
		while (m_PrimitiveGroups.constEnd() != iGroups)
		{
			LodPrimitiveGroups* pPrimitiveGroups= iGroups.value();
			// Iterate over material group
			LodPrimitiveGroups::iterator iGroup= pPrimitiveGroups->begin();
			while (pPrimitiveGroups->constEnd() != iGroup)
			{
				if (iGroup.key() == oldId)
				{
					GLC_PrimitiveGroup* pGroup= iGroup.value();
					// Erase old group pointer
					pPrimitiveGroups->erase(iGroup);
					// Change the group ID
					pGroup->setId(pMat->id());
					// Add the group with  new ID
					pPrimitiveGroups->insert(pMat->id(), pGroup);
					iGroup= pPrimitiveGroups->end();
				}
				else
				{
					++iGroup;
				}
			}
			++iGroups;
		}
	}

	if (pMat != m_MaterialHash.value(oldId))
	{
		// Remove old material
		removeMaterial(oldId);

		addMaterial(pMat);
	}

}

void GLC_Mesh::copyVboToClientSide()
{
	m_MeshData.copyVboToClientSide();
	GLC_Geometry::copyVboToClientSide();
}

void GLC_Mesh::releaseVboClientSide(bool update)
{
	m_MeshData.releaseVboClientSide(update);
	GLC_Geometry::releaseVboClientSide(update);
}

void GLC_Mesh::setVboUsage(bool usage)
{
	if (!isEmpty())
	{
		GLC_Geometry::setVboUsage(usage);
		m_MeshData.setVboUsage(usage);
	}
}

// Load the mesh from binary data stream
void GLC_Mesh::loadFromDataStream(QDataStream& stream, const MaterialHash& materialHash, const QHash<GLC_uint, GLC_uint>& materialIdMap)
{
	quint32 chunckId;
	stream >> chunckId;
	Q_ASSERT(chunckId == m_ChunkId);

	// The mesh name
	QString meshName;
	stream >> meshName;
	setName(meshName);

	// The wire data
	stream >> GLC_Geometry::m_WireData;

	// The mesh next primitive local id
	GLC_uint localId;
	stream >> localId;
	setNextPrimitiveLocalId(localId);

	// Retrieve geom mesh data
	stream >> m_MeshData;

	// Retrieve primitiveGroupLodList
	QList<int> primitiveGroupLodList;
	stream >> primitiveGroupLodList;

	// Retrieve primitiveGroup list
	QList<QList<GLC_PrimitiveGroup> > primitiveListOfGroupList;
	stream >> primitiveListOfGroupList;

	// Construct mesh primitiveGroupHash
	const int lodCount= primitiveGroupLodList.size();
	for (int i= 0; i < lodCount; ++i)
	{
		GLC_Mesh::LodPrimitiveGroups* pCurrentPrimitiveGroup= new GLC_Mesh::LodPrimitiveGroups();
		m_PrimitiveGroups.insert(primitiveGroupLodList.at(i), pCurrentPrimitiveGroup);
		const int groupCount= primitiveListOfGroupList.at(i).size();
		for (int iGroup= 0; iGroup < groupCount; ++iGroup)
		{
			Q_ASSERT(materialIdMap.contains(primitiveListOfGroupList.at(i).at(iGroup).id()));
			const GLC_uint newId= materialIdMap.value(primitiveListOfGroupList.at(i).at(iGroup).id());
			// Test if the mesh contains the material
			if (!containsMaterial(newId))
			{
				addMaterial(materialHash.value(newId));
			}
			GLC_PrimitiveGroup* pGroup= new GLC_PrimitiveGroup(primitiveListOfGroupList.at(i).at(iGroup), newId);

			Q_ASSERT(! m_PrimitiveGroups.value(primitiveGroupLodList.at(i))->contains(newId));
			m_PrimitiveGroups.value(primitiveGroupLodList.at(i))->insert(newId, pGroup);
		}
	}
	stream >> m_NumberOfVertice;
	stream >> m_NumberOfNormals;

	finishSerialized();
}

// Save the mesh to binary data stream
void GLC_Mesh::saveToDataStream(QDataStream& stream) const
{
	quint32 chunckId= m_ChunkId;
	stream << chunckId;

	// The mesh name
	stream << name();

	// The wire data
	stream << m_WireData;

	// The mesh next primitive local id
	stream << nextPrimitiveLocalId();

	// Mesh data serialisation
	stream << m_MeshData;

	// Primitive groups serialisation
	QList<int> primitiveGroupLodList;
	QList<QList<GLC_PrimitiveGroup> > primitiveListOfGroupList;

	GLC_Mesh::PrimitiveGroupsHash::const_iterator iGroupsHash= m_PrimitiveGroups.constBegin();
	while (m_PrimitiveGroups.constEnd() != iGroupsHash)
	{
		primitiveGroupLodList.append(iGroupsHash.key());
		QList<GLC_PrimitiveGroup> primitiveGroupList;
		GLC_Mesh::LodPrimitiveGroups::const_iterator iGroups= iGroupsHash.value()->constBegin();
		while (iGroupsHash.value()->constEnd() != iGroups)
		{
			primitiveGroupList.append(*(iGroups.value()));
			++iGroups;
		}
		primitiveListOfGroupList.append(primitiveGroupList);
		++iGroupsHash;
	}
	stream << primitiveGroupLodList;
	stream << primitiveListOfGroupList;

	stream << m_NumberOfVertice;
	stream << m_NumberOfNormals;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Virtual interface for OpenGL Geometry set up.
void GLC_Mesh::glDraw(const GLC_RenderProperties& renderProperties)
{

	Q_ASSERT(m_GeometryIsValid || !m_MeshData.positionSizeIsSet());

	const bool vboIsUsed= GLC_Geometry::vboIsUsed()  && GLC_State::vboSupported();

	if (m_IsSelected && (renderProperties.renderingMode() == glc::PrimitiveSelected) && !GLC_State::isInSelectionMode()
	&& !renderProperties.setOfSelectedPrimitiveIdIsEmpty())
	{
		m_CurrentLod= 0;
	}

	if (vboIsUsed)
	{
		m_MeshData.createVBOs();

		// Create VBO and IBO
		if (!m_GeometryIsValid && !m_MeshData.positionSizeIsSet())
		{
			fillVbosAndIbos();
		}

		// Activate mesh VBOs and IBO of the current LOD
		activateVboAndIbo();
	}
	else
	{
		if (!m_GeometryIsValid)
		{
			m_MeshData.initPositionSize();
		}
		activateVertexArray();
	}

	if (GLC_State::isInSelectionMode())
	{
		if (renderProperties.renderingMode() == glc::PrimitiveSelection)
		{
			primitiveSelectionRenderLoop(vboIsUsed);
		}
		else if (renderProperties.renderingMode() == glc::BodySelection)
		{
			bodySelectionRenderLoop(vboIsUsed);
		}
		else
		{
			normalRenderLoop(renderProperties, vboIsUsed);
		}
	}
	else if (m_IsSelected)
	{
		if (renderProperties.renderingMode() == glc::PrimitiveSelected)
		{
			if (!renderProperties.setOfSelectedPrimitiveIdIsEmpty())
			{
				primitiveSelectedRenderLoop(renderProperties, vboIsUsed);
			}
			else
			{
				m_IsSelected= false;
				if ((m_CurrentLod == 0) && (renderProperties.savedRenderingMode() == glc::OverwritePrimitiveMaterial) && !renderProperties.hashOfOverwritePrimitiveMaterialsIsEmpty())
					primitiveRenderLoop(renderProperties, vboIsUsed);
				else
					normalRenderLoop(renderProperties, vboIsUsed);
				m_IsSelected= true;
			}
		}
		else
		{
			normalRenderLoop(renderProperties, vboIsUsed);
		}
	}
	else
	{
		// Choose the accurate render loop
		switch (renderProperties.renderingMode())
		{
		case glc::NormalRenderMode:
			normalRenderLoop(renderProperties, vboIsUsed);
			break;
		case glc::OverwriteMaterial:
			OverwriteMaterialRenderLoop(renderProperties, vboIsUsed);
			break;
		case glc::OverwriteTransparency:
			OverwriteTransparencyRenderLoop(renderProperties, vboIsUsed);
			break;
		case glc::OverwriteTransparencyAndMaterial:
			OverwriteTransparencyAndMaterialRenderLoop(renderProperties, vboIsUsed);
			break;
		case glc::OverwritePrimitiveMaterial:
			if ((m_CurrentLod == 0) && !renderProperties.hashOfOverwritePrimitiveMaterialsIsEmpty())
				primitiveRenderLoop(renderProperties, vboIsUsed);
			else
				normalRenderLoop(renderProperties, vboIsUsed);
			break;
		default:
			Q_ASSERT(false);
			break;
		}
	}


	// Restore client state

	if (m_ColorPearVertex && !m_IsSelected && !GLC_State::isInSelectionMode())
	{
		glDisableClientState(GL_COLOR_ARRAY);
		glDisable(GL_COLOR_MATERIAL);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (vboIsUsed)
	{
		QGLBuffer::release(QGLBuffer::IndexBuffer);
		QGLBuffer::release(QGLBuffer::VertexBuffer);
	}

	// Draw mesh's wire if necessary
	if ((renderProperties.renderingFlag() == glc::WireRenderFlag) && !m_WireData.isEmpty() && !GLC_Geometry::typeIsWire())
	{
		if (!GLC_State::isInSelectionMode())
		{
			GLC_Context::current()->glcEnableLighting(false);
			// Set polyline colors
			GLfloat color[4]= {static_cast<float>(m_WireColor.redF()),
									static_cast<float>(m_WireColor.greenF()),
									static_cast<float>(m_WireColor.blueF()),
									static_cast<float>(m_WireColor.alphaF())};

			glColor4fv(color);
			m_WireData.glDraw(renderProperties, GL_LINE_STRIP);
			GLC_Context::current()->glcEnableLighting(true);
		}
		else
		{
			m_WireData.glDraw(renderProperties, GL_LINE_STRIP);
		}
	}

	// Update statistics
	GLC_RenderStatistics::addBodies(1);
	GLC_RenderStatistics::addTriangles(m_MeshData.trianglesCount(m_CurrentLod));
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

// Set the current material
GLC_uint GLC_Mesh::setCurrentMaterial(GLC_Material* pMaterial, int lod, double accuracy)
{

	// Test if a primitive group hash exists for the specified lod
	if (!m_PrimitiveGroups.contains(lod))
	{
		m_PrimitiveGroups.insert(lod, new LodPrimitiveGroups());

		m_MeshData.appendLod(accuracy);
	}

	GLC_uint returnId;
	if (NULL == pMaterial)
	{
		returnId= m_DefaultMaterialId; // Default material id

		// Test if the material has been already load
		if (m_DefaultMaterialId == 0)
		{
			pMaterial= new GLC_Material();
			// Add the material to the mesh
			addMaterial(pMaterial);
			m_DefaultMaterialId= pMaterial->id();
			returnId= m_DefaultMaterialId;

		}
		// Test if a primitive group for this material exist
		if (!m_PrimitiveGroups.value(lod)->contains(returnId))
		{
			m_PrimitiveGroups.value(lod)->insert(returnId, new GLC_PrimitiveGroup(returnId));
		}
	}
	else
	{
		returnId= pMaterial->id();
		// Test if the material has been already load
		if (!containsMaterial(returnId))
		{
			// Add the material to the mesh
			addMaterial(pMaterial);
			m_PrimitiveGroups.value(lod)->insert(returnId, new GLC_PrimitiveGroup(returnId));

		}
		else if (!m_PrimitiveGroups.value(lod)->contains(returnId))
		{
			// Add the material to the group
			m_PrimitiveGroups.value(lod)->insert(returnId, new GLC_PrimitiveGroup(returnId));
		}
	}

	return returnId;
}

// Fill VBOs and IBOs
void GLC_Mesh::fillVbosAndIbos()
{
	// Fill VBO of vertices
	m_MeshData.fillVbo(GLC_MeshData::GLC_Vertex);

	// Fill VBO of normals
	m_MeshData.fillVbo(GLC_MeshData::GLC_Normal);

	// Fill VBO of texel if needed
	m_MeshData.fillVbo(GLC_MeshData::GLC_Texel);

	// Fill VBO of color if needed
	m_MeshData.fillVbo(GLC_MeshData::GLC_Color);

	// Fill a lod IBO
	m_MeshData.fillLodIbo();

}
// set primitive group offset
void GLC_Mesh::finishSerialized()
{
	PrimitiveGroupsHash::iterator iGroups= m_PrimitiveGroups.begin();
	while (iGroups != m_PrimitiveGroups.constEnd())
	{
		LodPrimitiveGroups::iterator iGroup= iGroups.value()->begin();
		while (iGroup != iGroups.value()->constEnd())
		{
			iGroup.value()->computeVboOffset();
			++iGroup;
		}
		++iGroups;
	}
}
/*
// Move Indexs from the primitive groups to the mesh Data LOD and Set IBOs offsets
void GLC_Mesh::finishVbo()
{
	PrimitiveGroupsHash::iterator iGroups= m_PrimitiveGroups.begin();
	while (iGroups != m_PrimitiveGroups.constEnd())
	{
		int currentLod= iGroups.key();
		LodPrimitiveGroups::iterator iGroup= iGroups.value()->begin();
		while (iGroup != iGroups.value()->constEnd())
		{
			// Add group triangles index to mesh Data LOD triangles index vector
			if (iGroup.value()->containsTriangles())
			{
				iGroup.value()->setTrianglesOffset(BUFFER_OFFSET(m_MeshData.indexVectorSize(currentLod) * sizeof(GLuint)));
				(*m_MeshData.indexVectorHandle(currentLod))+= iGroup.value()->trianglesIndex().toVector();
			}

			// Add group strip index to mesh Data LOD strip index vector
			if (iGroup.value()->containsStrip())
			{
				iGroup.value()->setBaseTrianglesStripOffset(BUFFER_OFFSET(m_MeshData.indexVectorSize(currentLod) * sizeof(GLuint)));
				(*m_MeshData.indexVectorHandle(currentLod))+= iGroup.value()->stripsIndex().toVector();
			}

			// Add group fan index to mesh Data LOD fan index vector
			if (iGroup.value()->containsFan())
			{
				iGroup.value()->setBaseTrianglesFanOffset(BUFFER_OFFSET(m_MeshData.indexVectorSize(currentLod) * sizeof(GLuint)));
				(*m_MeshData.indexVectorHandle(currentLod))+= iGroup.value()->fansIndex().toVector();
			}

			iGroup.value()->finish();
			++iGroup;
		}
		++iGroups;

	}
}
*/

// Move Indexs from the primitive groups to the mesh Data LOD and Set Index offsets
void GLC_Mesh::moveIndexToMeshDataLod()
{
	//qDebug() << "GLC_Mesh::moveIndexToMeshDataLod()";
	PrimitiveGroupsHash::iterator iGroups= m_PrimitiveGroups.begin();
	while (iGroups != m_PrimitiveGroups.constEnd())
	{
		int currentLod= iGroups.key();
		LodPrimitiveGroups::iterator iGroup= iGroups.value()->begin();
		while (iGroup != iGroups.value()->constEnd())
		{
			// Add group triangles index to mesh Data LOD triangles index vector
			if (iGroup.value()->containsTriangles())
			{
				iGroup.value()->setTrianglesOffseti(m_MeshData.indexVectorSize(currentLod));
				(*m_MeshData.indexVectorHandle(currentLod))+= iGroup.value()->trianglesIndex().toVector();
			}

			// Add group strip index to mesh Data LOD strip index vector
			if (iGroup.value()->containsStrip())
			{
				iGroup.value()->setBaseTrianglesStripOffseti(m_MeshData.indexVectorSize(currentLod));
				(*m_MeshData.indexVectorHandle(currentLod))+= iGroup.value()->stripsIndex().toVector();
			}

			// Add group fan index to mesh Data LOD fan index vector
			if (iGroup.value()->containsFan())
			{
				iGroup.value()->setBaseTrianglesFanOffseti(m_MeshData.indexVectorSize(currentLod));
				(*m_MeshData.indexVectorHandle(currentLod))+= iGroup.value()->fansIndex().toVector();
			}

			iGroup.value()->computeVboOffset();
			iGroup.value()->finish();
			++iGroup;
		}
		++iGroups;
	}
}

// The normal display loop
void GLC_Mesh::normalRenderLoop(const GLC_RenderProperties& renderProperties, bool vboIsUsed)
{
	const bool isTransparent= (renderProperties.renderingFlag() == glc::TransparentRenderFlag);
	if ((!m_IsSelected || !isTransparent) || GLC_State::isInSelectionMode())
	{
		LodPrimitiveGroups::iterator iGroup= m_PrimitiveGroups.value(m_CurrentLod)->begin();
		while (iGroup != m_PrimitiveGroups.value(m_CurrentLod)->constEnd())
		{
			GLC_PrimitiveGroup* pCurrentGroup= iGroup.value();
			GLC_Material* pCurrentMaterial= m_MaterialHash.value(pCurrentGroup->id());

			// Test if the current material is renderable
			bool materialIsrenderable = (pCurrentMaterial->isTransparent() == isTransparent);

			// Choose the material to render
	   		if ((materialIsrenderable || m_IsSelected) && !GLC_State::isInSelectionMode())
	    	{
				// Execute current material
				pCurrentMaterial->glExecute();

				if (m_IsSelected) GLC_SelectionMaterial::glExecute();
			}

	   		// Choose the primitives to render
			if (m_IsSelected || GLC_State::isInSelectionMode() || materialIsrenderable)
			{

				if (vboIsUsed)
				{
					vboDrawPrimitivesOf(pCurrentGroup);
				}
				else
				{
					vertexArrayDrawPrimitivesOf(pCurrentGroup);
				}
			}

			++iGroup;
		}
	}
}

//  The overwrite material render loop
void GLC_Mesh::OverwriteMaterialRenderLoop(const GLC_RenderProperties& renderProperties, bool vboIsUsed)
{
	// Get the overwrite material
	GLC_Material* pOverwriteMaterial= renderProperties.overwriteMaterial();
	Q_ASSERT(NULL != pOverwriteMaterial);
	pOverwriteMaterial->glExecute();
	if (m_IsSelected) GLC_SelectionMaterial::glExecute();

	LodPrimitiveGroups::iterator iGroup= m_PrimitiveGroups.value(m_CurrentLod)->begin();
	while (iGroup != m_PrimitiveGroups.value(m_CurrentLod)->constEnd())
	{
		GLC_PrimitiveGroup* pCurrentGroup= iGroup.value();

		// Test if the current material is renderable
		bool materialIsrenderable = (pOverwriteMaterial->isTransparent() == (renderProperties.renderingFlag() == glc::TransparentRenderFlag));

   		// Choose the primitives to render
		if (m_IsSelected || materialIsrenderable)
		{

			if (vboIsUsed)
				vboDrawPrimitivesOf(pCurrentGroup);
			else
				vertexArrayDrawPrimitivesOf(pCurrentGroup);
		}

		++iGroup;
	}
}
// The overwrite transparency render loop
void GLC_Mesh::OverwriteTransparencyRenderLoop(const GLC_RenderProperties& renderProperties, bool vboIsUsed)
{
	// Get transparency value
	const float alpha= renderProperties.overwriteTransparency();
	Q_ASSERT(-1.0f != alpha);

	// Test if the current material is renderable
	bool materialIsrenderable = (renderProperties.renderingFlag() == glc::TransparentRenderFlag);

	if (materialIsrenderable || m_IsSelected)
	{
		LodPrimitiveGroups::iterator iGroup= m_PrimitiveGroups.value(m_CurrentLod)->begin();
		while (iGroup != m_PrimitiveGroups.value(m_CurrentLod)->constEnd())
		{
			GLC_PrimitiveGroup* pCurrentGroup= iGroup.value();
			GLC_Material* pCurrentMaterial= m_MaterialHash.value(pCurrentGroup->id());

			// Execute current material
			pCurrentMaterial->glExecute(alpha);

			if (m_IsSelected) GLC_SelectionMaterial::glExecute();

	   		// Choose the primitives to render
			if (m_IsSelected || materialIsrenderable)
			{

				if (vboIsUsed)
					vboDrawPrimitivesOf(pCurrentGroup);
				else
					vertexArrayDrawPrimitivesOf(pCurrentGroup);
			}
			++iGroup;
		}
	}
}

void GLC_Mesh::OverwriteTransparencyAndMaterialRenderLoop(const GLC_RenderProperties& renderProperties, bool vboIsUsed)
{
	// Get transparency value
	const float alpha= renderProperties.overwriteTransparency();
	Q_ASSERT(-1.0f != alpha);

	// Get the overwrite material
	GLC_Material* pOverwriteMaterial= renderProperties.overwriteMaterial();
	Q_ASSERT(NULL != pOverwriteMaterial);
	pOverwriteMaterial->glExecute(alpha);
	if (m_IsSelected) GLC_SelectionMaterial::glExecute();

	LodPrimitiveGroups::iterator iGroup= m_PrimitiveGroups.value(m_CurrentLod)->begin();
	while (iGroup != m_PrimitiveGroups.value(m_CurrentLod)->constEnd())
	{
		GLC_PrimitiveGroup* pCurrentGroup= iGroup.value();

		// Test if the current material is renderable
		bool materialIsrenderable = (renderProperties.renderingFlag() == glc::TransparentRenderFlag);

   		// Choose the primitives to render
		if (m_IsSelected || materialIsrenderable)
		{

			if (vboIsUsed)
				vboDrawPrimitivesOf(pCurrentGroup);
			else
				vertexArrayDrawPrimitivesOf(pCurrentGroup);
		}

		++iGroup;
	}
}

// The body selection render loop
void GLC_Mesh::bodySelectionRenderLoop(bool vboIsUsed)
{
	Q_ASSERT(GLC_State::isInSelectionMode());

	LodPrimitiveGroups::iterator iGroup= m_PrimitiveGroups.value(m_CurrentLod)->begin();
	while (iGroup != m_PrimitiveGroups.value(m_CurrentLod)->constEnd())
	{
		GLC_PrimitiveGroup* pCurrentGroup= iGroup.value();

		if (vboIsUsed)
			vboDrawPrimitivesOf(pCurrentGroup);
		else
			vertexArrayDrawPrimitivesOf(pCurrentGroup);

		++iGroup;
	}
}

// The primitive selection render loop
void GLC_Mesh::primitiveSelectionRenderLoop(bool vboIsUsed)
{
	Q_ASSERT(GLC_State::isInSelectionMode());

	LodPrimitiveGroups::iterator iGroup= m_PrimitiveGroups.value(m_CurrentLod)->begin();

	while (iGroup != m_PrimitiveGroups.value(m_CurrentLod)->constEnd())
	{
		GLC_PrimitiveGroup* pCurrentGroup= iGroup.value();

		if (vboIsUsed)
			vboDrawInSelectionModePrimitivesOf(pCurrentGroup);
		else
			vertexArrayDrawInSelectionModePrimitivesOf(pCurrentGroup);

		++iGroup;
	}
}

// The primitive rendeder loop
void GLC_Mesh::primitiveRenderLoop(const GLC_RenderProperties& renderProperties, bool vboIsUsed)
{
	const bool isTransparent= (renderProperties.renderingFlag() == glc::TransparentRenderFlag);
	LodPrimitiveGroups::iterator iGroup= m_PrimitiveGroups.value(m_CurrentLod)->begin();
	while (iGroup != m_PrimitiveGroups.value(m_CurrentLod)->constEnd())
	{
		GLC_PrimitiveGroup* pCurrentGroup= iGroup.value();
		GLC_Material* pCurrentMaterial= m_MaterialHash.value(pCurrentGroup->id());

		// Test if the current material is renderable
		const bool materialIsrenderable = (pCurrentMaterial->isTransparent() == isTransparent);

		if (materialIsrenderable)
		{
			pCurrentMaterial->glExecute();
		}
		if (vboIsUsed)
			vboDrawPrimitivesGroupOf(pCurrentGroup, pCurrentMaterial, materialIsrenderable, isTransparent, renderProperties.hashOfOverwritePrimitiveMaterials());
		else
			vertexArrayDrawPrimitivesGroupOf(pCurrentGroup, pCurrentMaterial, materialIsrenderable, isTransparent, renderProperties.hashOfOverwritePrimitiveMaterials());

		++iGroup;
	}
}

// The primitive Selected render loop
void GLC_Mesh::primitiveSelectedRenderLoop(const GLC_RenderProperties& renderProperties, bool vboIsUsed)
{
	const bool isTransparent= (renderProperties.renderingFlag() == glc::TransparentRenderFlag);
	LodPrimitiveGroups::iterator iGroup= m_PrimitiveGroups.value(m_CurrentLod)->begin();
	while (iGroup != m_PrimitiveGroups.value(m_CurrentLod)->constEnd())
	{
		GLC_PrimitiveGroup* pCurrentGroup= iGroup.value();
		GLC_Material* pCurrentMaterial= m_MaterialHash.value(pCurrentGroup->id());

		// Test if the current material is renderable
		const bool materialIsrenderable = (pCurrentMaterial->isTransparent() == isTransparent);

		if (materialIsrenderable)
		{
			pCurrentMaterial->glExecute();
		}

		if (vboIsUsed)
			vboDrawSelectedPrimitivesGroupOf(pCurrentGroup, pCurrentMaterial, materialIsrenderable, isTransparent, renderProperties);
		else
			vertexArrayDrawSelectedPrimitivesGroupOf(pCurrentGroup, pCurrentMaterial, materialIsrenderable, isTransparent, renderProperties);

		++iGroup;
	}
}

void GLC_Mesh::copyIndex(int lodIndex, GLC_Mesh* pLodMesh, QHash<GLuint, GLuint>& sourceToTargetIndexMap, QHash<GLuint, GLuint>& tagetToSourceIndexMap, int& maxIndex, int targetLod)
{
	//! The list of LOD material ID
	QList<GLuint> materialId= m_PrimitiveGroups.value(lodIndex)->keys();

	const int materialCount= materialId.size();
	for (int i= 0; i < materialCount; ++i)
	{
		GLuint currentMaterialId= materialId.at(i);
		GLC_Material* pCurrentMaterial= GLC_Geometry::material(currentMaterialId);

		// Triangles
		if (containsTriangles(lodIndex, currentMaterialId))
		{
			QVector<GLuint> sourceTriangleIndex= getTrianglesIndex(lodIndex, currentMaterialId);
			QList<GLuint> targetTriangleIndex;
			const int triangleIndexCount= sourceTriangleIndex.size();
			for (int i= 0; i < triangleIndexCount; ++i)
			{
				const GLuint currentIndex= sourceTriangleIndex.at(i);
				if (!sourceToTargetIndexMap.contains(currentIndex))
				{
					sourceToTargetIndexMap.insert(currentIndex, ++maxIndex);
					tagetToSourceIndexMap.insert(maxIndex, currentIndex);
					targetTriangleIndex.append(maxIndex);
				}
				else
				{
					targetTriangleIndex.append(sourceToTargetIndexMap.value(currentIndex));
				}
			}
			pLodMesh->addTriangles(pCurrentMaterial, targetTriangleIndex, targetLod, m_MeshData.getLod(lodIndex)->accuracy());
		}

		//Triangles strips
		if (containsStrips(lodIndex, currentMaterialId))
		{
			QList<QVector<GLuint> > sourceStripIndex= getStripsIndex(lodIndex, currentMaterialId);
			const int stripCount= sourceStripIndex.size();
			for (int stripIndex= 0; stripIndex < stripCount; ++stripIndex)
			{

				QVector<GLuint> sourceTriangleStripIndex= sourceStripIndex.at(stripIndex);
				QList<GLuint> targetTriangleStripIndex;
				const int triangleStripIndexCount= sourceTriangleStripIndex.size();
				for (int i= 0; i < triangleStripIndexCount; ++i)
				{
					const GLuint currentIndex= sourceTriangleStripIndex.at(i);
					if (!sourceToTargetIndexMap.contains(currentIndex))
					{
						sourceToTargetIndexMap.insert(currentIndex, ++maxIndex);
						tagetToSourceIndexMap.insert(maxIndex, currentIndex);
						targetTriangleStripIndex.append(maxIndex);
					}
					else
					{
						targetTriangleStripIndex.append(sourceToTargetIndexMap.value(currentIndex));
					}
				}
				pLodMesh->addTrianglesStrip(pCurrentMaterial, targetTriangleStripIndex, targetLod, m_MeshData.getLod(lodIndex)->accuracy());
			}
		}
		//Triangles fans
		if (containsFans(lodIndex, currentMaterialId))
		{
			QList<QVector<GLuint> > sourceFanIndex= getFansIndex(lodIndex, currentMaterialId);
			const int fanCount= sourceFanIndex.size();
			for (int fanIndex= 0; fanIndex < fanCount; ++fanIndex)
			{

				QVector<GLuint> sourceTriangleFanIndex= sourceFanIndex.at(fanIndex);
				QList<GLuint> targetTriangleFanIndex;
				const int triangleFanIndexCount= sourceTriangleFanIndex.size();
				for (int i= 0; i < triangleFanIndexCount; ++i)
				{
					const GLuint currentIndex= sourceTriangleFanIndex.at(i);
					if (!sourceToTargetIndexMap.contains(currentIndex))
					{
						sourceToTargetIndexMap.insert(currentIndex, ++maxIndex);
						tagetToSourceIndexMap.insert(maxIndex, currentIndex);
						targetTriangleFanIndex.append(maxIndex);
					}
					else
					{
						targetTriangleFanIndex.append(sourceToTargetIndexMap.value(currentIndex));
					}
				}
				pLodMesh->addTrianglesFan(pCurrentMaterial, targetTriangleFanIndex, targetLod, m_MeshData.getLod(lodIndex)->accuracy());
			}
		}
	}
}

void GLC_Mesh::copyBulkData(GLC_Mesh* pLodMesh, const QHash<GLuint, GLuint>& tagetToSourceIndexMap, int maxIndex)
{
	GLfloatVector tempFloatVector;
	int stride= 3;
	// Extract position bulk data
	Q_ASSERT(!m_MeshData.positionVectorHandle()->isEmpty());
	tempFloatVector.resize(stride * (maxIndex + 1));
	for (int i= 0; i < maxIndex + 1; ++i)
	{
		GLfloat* pTarget= &(tempFloatVector.data()[i * stride]);
		GLfloat* pSource= &(m_MeshData.positionVectorHandle()->data()[tagetToSourceIndexMap.value(i) * stride]);

		memcpy(pTarget, pSource, sizeof(GLfloat) * stride);
	}
	pLodMesh->addVertice(tempFloatVector);
	tempFloatVector.clear();

	// Extract normal bulk data
	Q_ASSERT(!m_MeshData.normalVectorHandle()->isEmpty());
	tempFloatVector.resize(stride * (maxIndex + 1));
	for (int i= 0; i < maxIndex + 1; ++i)
	{
		GLfloat* pTarget= &(tempFloatVector.data()[i * stride]);
		GLfloat* pSource= &(m_MeshData.normalVectorHandle()->data()[tagetToSourceIndexMap.value(i) * stride]);

		memcpy(pTarget, pSource, sizeof(GLfloat) * stride);
	}
	pLodMesh->addNormals(tempFloatVector);
	tempFloatVector.clear();

	if (!m_MeshData.texelVectorHandle()->isEmpty())
	{
		// Extract texel bulk data
		stride= 2;
		tempFloatVector.resize(stride * (maxIndex + 1));

		for (int i= 0; i < maxIndex + 1; ++i)
		{
			GLfloat* pTarget= &(tempFloatVector.data()[i * stride]);
			GLfloat* pSource= &(m_MeshData.texelVectorHandle()->data()[tagetToSourceIndexMap.value(i) * stride]);

			memcpy(pTarget, pSource, sizeof(GLfloat) * stride);
		}
		pLodMesh->addTexels(tempFloatVector);
		tempFloatVector.clear();
	}
}

IndexList GLC_Mesh::equivalentTrianglesIndexOfstripsIndex(int lodIndex, GLC_uint materialId)
{
	IndexList trianglesIndex;
	if (containsStrips(lodIndex, materialId))
	{
		const QList<QVector<GLuint> > stripsIndex= getStripsIndex(lodIndex, materialId);
		const int stripCount= stripsIndex.count();
		for (int i= 0; i < stripCount; ++i)
		{
			const QVector<GLuint> currentStripIndex= stripsIndex.at(i);

			trianglesIndex.append(currentStripIndex.at(0));
			trianglesIndex.append(currentStripIndex.at(1));
			trianglesIndex.append(currentStripIndex.at(2));
			const int stripSize= currentStripIndex.size();
			for (int j= 3; j < stripSize; ++j)
			{
				if ((j % 2) != 0)
				{
					trianglesIndex.append(currentStripIndex.at(j));
					trianglesIndex.append(currentStripIndex.at(j - 1));
					trianglesIndex.append(currentStripIndex.at(j - 2));
				}
				else
				{
					trianglesIndex.append(currentStripIndex.at(j));
					trianglesIndex.append(currentStripIndex.at(j - 2));
					trianglesIndex.append(currentStripIndex.at(j - 1));
				}
			}
		}
	}
	return trianglesIndex;
}

IndexList GLC_Mesh::equivalentTrianglesIndexOfFansIndex(int lodIndex, GLC_uint materialId)
{
	IndexList trianglesIndex;
	if (containsFans(lodIndex, materialId))
	{
		const QList<QVector<GLuint> > fanIndex= getFansIndex(lodIndex, materialId);
		const int fanCount= fanIndex.count();
		for (int i= 0; i < fanCount; ++i)
		{
			const QVector<GLuint> currentFanIndex= fanIndex.at(i);
			const int size= currentFanIndex.size();
			for (int j= 1; j < size - 1; ++j)
			{
				trianglesIndex.append(currentFanIndex.first());
				trianglesIndex.append(currentFanIndex.at(j));
				trianglesIndex.append(currentFanIndex.at(j + 1));
			}
		}
	}
	return trianglesIndex;
}

