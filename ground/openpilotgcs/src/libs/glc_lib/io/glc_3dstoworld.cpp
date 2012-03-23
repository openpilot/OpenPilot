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

//! \file glc_3dstoworld.cpp implementation of the GLC_3dsToWorld class.

#include "glc_3dstoworld.h"

#include "../geometry/glc_mesh.h"
#include "../sceneGraph/glc_world.h"
#include "../glc_fileformatexception.h"
#include "../geometry/glc_circle.h"
#include "../shading/glc_material.h"
#include "../maths/glc_vector2df.h"
#include "../maths/glc_vector3df.h"
#include "../sceneGraph/glc_structreference.h"
#include "../sceneGraph/glc_structinstance.h"
#include "../sceneGraph/glc_structoccurence.h"

// Lib3ds Header
#include "3rdparty/lib3ds/file.h"
#include "3rdparty/lib3ds/mesh.h"
#include "3rdparty/lib3ds/node.h"
#include "3rdparty/lib3ds/matrix.h"
#include "3rdparty/lib3ds/material.h"

#include <QFileInfo>
#include <QGLContext>

GLC_3dsToWorld::GLC_3dsToWorld()
: m_pWorld(NULL)
, m_FileName()
, m_pCurrentMesh(NULL)
, m_pLib3dsFile(NULL)
, m_Materials()
, m_NextMaterialIndex(0)
, m_LoadedMeshes()
, m_InitQuantumValue(50)
, m_CurrentQuantumValue(0)
, m_PreviousQuantumValue(0)
, m_NumberOfMeshes(0)
, m_CurrentMeshNumber(0)
, m_ListOfAttachedFileName()
{
}

GLC_3dsToWorld::~GLC_3dsToWorld()
{
	clear();
}

// Create an GLC_World from an input 3DS File
GLC_World* GLC_3dsToWorld::CreateWorldFrom3ds(QFile &file)
{
	clear();
	m_FileName= file.fileName();

	//////////////////////////////////////////////////////////////////
	// Test if the file exist and can be opened
	//////////////////////////////////////////////////////////////////
	if (!file.open(QIODevice::ReadOnly))
	{
		QString message(QString("GLC_3dsToWorld::CreateWorldFrom3ds File ") + m_FileName + QString(" doesn't exist"));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotFound);
		throw(fileFormatException);
	}
	// Close the file before open it with lib3ds
	file.close();

	//////////////////////////////////////////////////////////////////
	// Init member
	//////////////////////////////////////////////////////////////////
	m_pWorld= new GLC_World;

	//Load 3ds File
	m_pLib3dsFile=lib3ds_file_load(m_FileName.toLocal8Bit().data());
	if (!m_pLib3dsFile)
	{
		QString message= "GLC_3dsToWorld::CreateWorldFrom3ds : Loading Failed";
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotSupported);
		clear();
		throw(fileFormatException);
	}
	// Evaluate Nodes Matrix for the first frame (Needed by instances)
	lib3ds_file_eval(m_pLib3dsFile, 0.0);
	m_CurrentQuantumValue= m_InitQuantumValue;
	m_PreviousQuantumValue= m_CurrentQuantumValue;

	emit currentQuantum(m_CurrentQuantumValue);
	// Count the number of meshes
	for(Lib3dsMesh *pMesh= m_pLib3dsFile->meshes; pMesh != NULL; pMesh = pMesh->next)
	{
		++m_NumberOfMeshes;
	}
	// Check if there is some meshes in the 3ds file
	if (0 == m_NumberOfMeshes)
	{
		QString message= "GLC_3dsToWorld::CreateWorldFrom3ds : No mesh found !";
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::NoMeshFound);
		clear();
		throw(fileFormatException);
	}

	// Create GLC_3DViewInstance with Node
	for (Lib3dsNode *pNode=m_pLib3dsFile->nodes; pNode!=0; pNode=pNode->next)
	{
		createMeshes(m_pWorld->rootOccurence(), pNode);
	}

	// Load unloaded mesh name
	for(Lib3dsMesh *pMesh= m_pLib3dsFile->meshes; pMesh != NULL; pMesh = pMesh->next)
	{
		if (!m_LoadedMeshes.contains(QString(pMesh->name)))
		{
			//qDebug() << "Mesh without parent found" << QString(pMesh->name);
			Lib3dsNode *pNode= lib3ds_node_new_object();
			strcpy(pNode->name, pMesh->name);
			pNode->parent_id= LIB3DS_NO_PARENT;
			lib3ds_file_insert_node(m_pLib3dsFile, pNode);
			createMeshes(m_pWorld->rootOccurence(), pNode);
		}
	}

	// Free Lib3dsFile and all its ressources
	lib3ds_file_free(m_pLib3dsFile);
	m_pLib3dsFile= NULL;
	emit currentQuantum(100);
	// Create the world bounding box
	m_pWorld->collection()->boundingBox();
	return m_pWorld;
}

/////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

// clear 3dsToWorld allocate memory and reset member
void GLC_3dsToWorld::clear()
{
	if (NULL != m_pCurrentMesh)
	{
		delete m_pCurrentMesh;
		m_pCurrentMesh= NULL;
	}
	m_pWorld= NULL;
	m_FileName.clear();
	if (NULL != m_pLib3dsFile)
	{
		lib3ds_file_free(m_pLib3dsFile);
		m_pLib3dsFile= NULL;
	}

	// Remove unused material
	QHash<QString, GLC_Material*>::iterator i;
	for (i= m_Materials.begin(); i != m_Materials.end(); ++i)
	{
		if (i.value()->isUnused()) delete i.value();
	}
	m_Materials.clear();
	m_NextMaterialIndex= 0;
	// Clear the loaded meshes Set
	m_LoadedMeshes.clear();
	// Progress indicator
	m_CurrentQuantumValue= 0;
	m_PreviousQuantumValue= 0;
	m_NumberOfMeshes= 0;
	m_CurrentMeshNumber= 0;
	m_ListOfAttachedFileName.clear();
}

// Create meshes from the 3ds File
void GLC_3dsToWorld::createMeshes(GLC_StructOccurence* pProduct, Lib3dsNode* pFatherNode)
{
	GLC_StructOccurence* pChildProduct= NULL;
	Lib3dsMesh *pMesh= NULL;

	if (pFatherNode->type == LIB3DS_OBJECT_NODE)
	{
		//qDebug() << "Node type LIB3DS_OBJECT_NODE is named : " << QString(pFatherNode->name);
		//qDebug() << "Node Matrix :";
		//qDebug() << GLC_Matrix4x4(&(pFatherNode->matrix[0][0])).toString();

		// Check if the node is a mesh or dummy
		if (!(strcmp(pFatherNode->name,"$$$DUMMY")==0))
		{
	    	pMesh = lib3ds_file_mesh_by_name(m_pLib3dsFile, pFatherNode->name);
		    if( pMesh != NULL )
		    {
		    	GLC_3DRep representation(create3DRep(pMesh));
		    	// Test if there is vertex in the mesh
		    	if (0 != representation.vertexCount())
		    	{
		    		m_LoadedMeshes.insert(representation.name());
			    	// Load node matrix
			    	GLC_Matrix4x4 nodeMat(&(pFatherNode->matrix[0][0]));
					// The mesh matrix to inverse
			    	GLC_Matrix4x4 matInv(&(pMesh->matrix[0][0]));
					matInv.invert();
					// Get the node pivot
					Lib3dsObjectData *pObjectData;
					pObjectData= &pFatherNode->data.object;
					GLC_Matrix4x4 trans(-pObjectData->pivot[0], -pObjectData->pivot[1], -pObjectData->pivot[2]);
					// Compute the part matrix
					nodeMat= nodeMat * trans * matInv; // I don't know why...
					nodeMat.optimise();
					// move the part by the matrix
					pProduct->addChild((new GLC_StructInstance(new GLC_3DRep(representation)))->move(nodeMat));
		    	}
		    	else
		    	{
		    		// the instance will be deleted, check material usage
		    		QSet<GLC_Material*> meshMaterials= representation.materialSet();
		    		QSet<GLC_Material*>::const_iterator iMat= meshMaterials.constBegin();
		    		while (iMat != meshMaterials.constEnd())
		    		{
		    			if ((*iMat)->numberOfUsage() == 1)
		    			{
		    				m_Materials.remove((*iMat)->name());
		    			}
		    			++iMat;
		    		}
		    	}
		    }
		} // End If DUMMY
	}
	else return;
	// If there is a child, create a child product
	if (NULL != pFatherNode->childs)
	{
		pChildProduct= new GLC_StructOccurence();
		pProduct->addChild(pChildProduct);

		pChildProduct->setName(QString("Product") + QString::number(pFatherNode->node_id));

		//pChildProduct->move(GLC_Matrix4x4(&(pFatherNode->matrix[0][0])));

		// Create Childs meshes if exists
		for (Lib3dsNode* pNode= pFatherNode->childs; pNode!=0; pNode= pNode->next)
		{
			createMeshes(pChildProduct, pNode);
		}
	}


}
//! Create 3DRep from a Lib3dsNode
GLC_3DRep GLC_3dsToWorld::create3DRep(Lib3dsMesh* p3dsMesh)
{
	QString meshName(p3dsMesh->name);
	if (m_LoadedMeshes.contains(meshName))
	{
		// This mesh as been already loaded
		QList<GLC_3DViewInstance*> instancesList(m_pWorld->collection()->instancesHandle());
		GLC_3DViewInstance* pCurrentInstance= NULL;
		int currentIndex= -1;
		do
		{
			pCurrentInstance= instancesList[++currentIndex];
		} while (pCurrentInstance->name() != meshName);
		// return an instance.
		//qDebug() << "instance";
		return pCurrentInstance->representation();
	}
	GLC_Mesh * pMesh= new GLC_Mesh();
	pMesh->setName(p3dsMesh->name);
	// The mesh normals
	const int normalsNumber= p3dsMesh->faces * 3;

	Lib3dsVector *normalL= static_cast<Lib3dsVector*>(malloc(normalsNumber * sizeof(Lib3dsVector)));
	lib3ds_mesh_calculate_normals(p3dsMesh, normalL);

	// Position vector
	QVector<float> position(normalsNumber * 3);

	// Normal Vector
	QVector<float> normal(normalsNumber * 3);
	memcpy((void*)normal.data(), normalL, normalsNumber * 3 * sizeof(float));

	// Texel Vector
	QVector<float> texel;
	if (p3dsMesh->texels > 0)
	{
		texel.resize(normalsNumber * 2);
	}

	int normalIndex= 0;
	for (unsigned int i= 0; i < p3dsMesh->faces; ++i)
	{
		IndexList triangleIndex;
		Lib3dsFace *p3dsFace=&p3dsMesh->faceL[i];
		for (int i=0; i < 3; ++i)
		{
			triangleIndex.append(normalIndex);
			// Add vertex coordinate
			memcpy((void*)&(position.data()[normalIndex * 3]), &p3dsMesh->pointL[p3dsFace->points[i]], 3 * sizeof(float));

			// Add texel
			if (p3dsMesh->texels > 0)
			{
				memcpy((void*)&(texel.data()[normalIndex * 2]), &p3dsMesh->texelL[p3dsFace->points[i]], 2 * sizeof(float));
			}
			++normalIndex;
		}

		// Load the material
		// The material current face index
		GLC_Material* pCurMaterial= NULL;
		if (p3dsFace->material[0])
		{
			Lib3dsMaterial* p3dsMat=lib3ds_file_material_by_name(m_pLib3dsFile, p3dsFace->material);
			if (NULL != p3dsMat)
			{
				// Check it this material as already been loaded
				const QString materialName(p3dsFace->material);

				if (!m_Materials.contains(materialName))
				{ // Material not already loaded, load it
					loadMaterial(p3dsMat);
				}
				pCurMaterial= m_Materials.value(materialName);
			}
		}
		pMesh->addTriangles(pCurMaterial, triangleIndex);
	}
	pMesh->addVertice(position);
	pMesh->addNormals(normal);
	if (p3dsMesh->texels > 0)
	{
		pMesh->addTexels(texel);
	}

	// free normal memmory
	delete[] normalL;
	// Compute loading progress
	++m_CurrentMeshNumber;
	m_CurrentQuantumValue = static_cast<int>((static_cast<double>(m_CurrentMeshNumber) / m_NumberOfMeshes) * (100 - m_InitQuantumValue)) + m_InitQuantumValue;
	if (m_CurrentQuantumValue > m_PreviousQuantumValue)
	{
		emit currentQuantum(m_CurrentQuantumValue);
	}
	m_PreviousQuantumValue= m_CurrentQuantumValue;

	pMesh->finish();
	return GLC_3DRep(pMesh);
}

// Load Material
void GLC_3dsToWorld::loadMaterial(Lib3dsMaterial* p3dsMaterial)
{
	GLC_Material* pMaterial= new GLC_Material;
	// Set the material name
	const QString materialName(p3dsMaterial->name);
	pMaterial->setName(materialName);
	// Check if there is a texture
	if (p3dsMaterial->texture1_map.name[0])
	{
		const QString textureName(p3dsMaterial->texture1_map.name);
		// Retrieve the .3ds file path
		QFileInfo fileInfo(m_FileName);
		QString textureFileName(fileInfo.absolutePath() + QDir::separator());
		textureFileName.append(textureName);

		// TGA file type are not supported
		if (!textureName.right(3).contains("TGA", Qt::CaseInsensitive))
		{
			QFile textureFile(textureFileName);

			if (textureFile.open(QIODevice::ReadOnly))
			{
				// Create the texture and assign it to the material
				GLC_Texture *pTexture = new GLC_Texture(textureFile);
				pMaterial->setTexture(pTexture);
				m_ListOfAttachedFileName << textureFileName;
				textureFile.close();
			}
			else
			{
				QStringList stringList(m_FileName);
				stringList.append("Open File : " + textureFileName + " failed");
				GLC_ErrorLog::addError(stringList);
			}

		}
		else
		{
			QStringList stringList(m_FileName);
			stringList.append("Image : " + textureFileName + " not suported");
			GLC_ErrorLog::addError(stringList);
		}
	}

	// Ambient Color
	QColor ambient;
	ambient.setRgbF(p3dsMaterial->ambient[0], p3dsMaterial->ambient[1], p3dsMaterial->ambient[2]);
	ambient.setAlphaF(p3dsMaterial->ambient[3]);
	pMaterial->setAmbientColor(ambient);
	// Diffuse Color
	QColor diffuse;
	diffuse.setRgbF(p3dsMaterial->diffuse[0], p3dsMaterial->diffuse[1], p3dsMaterial->diffuse[2]);
	diffuse.setAlphaF(p3dsMaterial->diffuse[3]);
	pMaterial->setDiffuseColor(diffuse);
	// Specular Color
	QColor specular;
	specular.setRgbF(p3dsMaterial->specular[0], p3dsMaterial->specular[1], p3dsMaterial->specular[2]);
	specular.setAlphaF(p3dsMaterial->specular[3]);
	pMaterial->setSpecularColor(specular);
	// Shininess

	if (0 != p3dsMaterial->shininess)
	{
		float matShininess= p3dsMaterial->shininess * 128.0f;
		if (matShininess > 128.0f) matShininess= 128.0f;
		if (matShininess < 5.0f) matShininess= 20.0f;
		pMaterial->setShininess(matShininess);
	}
	// Transparency

	pMaterial->setOpacity(1.0 - p3dsMaterial->transparency);

	// Add the material to the hash table
	m_Materials.insert(materialName, pMaterial);
}

