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

#include "glc_3drep.h"
#include "../glc_factory.h"
#include "glc_mesh.h"
#include "glc_errorlog.h"

// Class chunk id
quint32 GLC_3DRep::m_ChunkId= 0xA702;

GLC_3DRep::GLC_3DRep()
: GLC_Rep()
, m_pGeomList(new QList<GLC_Geometry*>)
, m_pType(new int(GLC_Rep::GLC_VBOGEOM))
{

}

GLC_3DRep::GLC_3DRep(GLC_Geometry* pGeom)
: GLC_Rep()
, m_pGeomList(new QList<GLC_Geometry*>)
, m_pType(new int(GLC_Rep::GLC_VBOGEOM))
{
	m_pGeomList->append(pGeom);
	*m_pIsLoaded= true;
	setName(pGeom->name());
}

GLC_3DRep::GLC_3DRep(const GLC_3DRep& rep)
: GLC_Rep(rep)
, m_pGeomList(rep.m_pGeomList)
, m_pType(rep.m_pType)
{

}

GLC_3DRep& GLC_3DRep::operator=(const GLC_Rep& rep)
{
	const GLC_3DRep* p3DRep= dynamic_cast<const GLC_3DRep*>(&rep);
	Q_ASSERT(NULL != p3DRep);
	if (this != &rep)
	{
		clear();
		GLC_Rep::operator=(rep);
		m_pGeomList= p3DRep->m_pGeomList;
		m_pType= p3DRep->m_pType;
	}

	return *this;
}

GLC_Rep* GLC_3DRep::clone() const
{
	return new GLC_3DRep(*this);
}

GLC_Rep* GLC_3DRep::deepCopy() const
{
	GLC_3DRep* pCloneRep= new GLC_3DRep;
	// Copy fields of the base class
	pCloneRep->setFileName(fileName());
	pCloneRep->setName(name());
	// Copy representation geometries
	const int size= m_pGeomList->size();
	for (int i= 0; i < size; ++i)
	{
		pCloneRep->addGeom(m_pGeomList->at(i)->clone());
	}
	return pCloneRep;
}

GLC_3DRep::~GLC_3DRep()
{
	clear();
}

//////////////////////////////////////////////////////////////////////
// Get functions
//////////////////////////////////////////////////////////////////////
quint32 GLC_3DRep::chunckID()
{
	return m_ChunkId;
}

int GLC_3DRep::type() const
{
	return (*m_pType);
}

//////////////////////////////////////////////////////////////////////
// Get functions
//////////////////////////////////////////////////////////////////////

bool GLC_3DRep::boundingBoxIsValid() const
{
	bool result= !m_pGeomList->isEmpty();
	const int max= m_pGeomList->size();
	int index= 0;
	while (result && (index < max))
	{
		result= result && m_pGeomList->at(index)->boundingBoxIsValid();
		++index;
	}
	return result;
}

GLC_BoundingBox GLC_3DRep::boundingBox() const
{
	GLC_BoundingBox resultBox;
	const int size= m_pGeomList->size();
	for (int i= 0; i < size; ++i)
	{
		resultBox.combine(m_pGeomList->at(i)->boundingBox());
	}
	return resultBox;
}

unsigned int GLC_3DRep::faceCount() const
{
	unsigned int result= 0;
	if (!m_pGeomList->isEmpty())
	{
		const int size= m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			result+= m_pGeomList->at(i)->faceCount();
		}
	}

	return result;
}

unsigned int GLC_3DRep::vertexCount() const
{
	unsigned int result= 0;
	if (!m_pGeomList->isEmpty())
	{
		const int size= m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			result+= m_pGeomList->at(i)->VertexCount();
		}
	}

	return result;
}

unsigned int GLC_3DRep::materialCount() const
{
	unsigned int result= 0;
	if (!m_pGeomList->isEmpty())
	{
		const int size= m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			result+= m_pGeomList->at(i)->materialCount();
		}
	}

	return result;
}

QSet<GLC_Material*> GLC_3DRep::materialSet() const
{
	QSet<GLC_Material*> result;
	if (!m_pGeomList->isEmpty())
	{
		const int size= m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			result.unite(m_pGeomList->at(i)->materialSet());
		}
	}

	return result;
}

double GLC_3DRep::volume() const
{
	double resultVolume= 0.0;
	const int geomCount= m_pGeomList->count();
	for (int i= 0; i < geomCount; ++i)
	{
		resultVolume+= m_pGeomList->at(i)->volume();
	}

	return resultVolume;
}

void GLC_3DRep::clean()
{
	QList<GLC_Geometry*>::iterator iGeomList= m_pGeomList->begin();
	while(iGeomList != m_pGeomList->constEnd())
	{
		if ((*iGeomList)->VertexCount() == 0)
		{
			qDebug() << "Delete empty geom--------------------";
			delete (*iGeomList);
			iGeomList= m_pGeomList->erase(iGeomList);
		}
		else
		{
			++iGeomList;
		}
	}
}

void GLC_3DRep::reverseNormals()
{
	const int size= m_pGeomList->size();
	for (int i= 0; i < size; ++i)
	{
		(*m_pGeomList)[i]->reverseNormals();
	}
}

bool GLC_3DRep::load()
{
	bool loadSucces= false;

	if(!(*m_pIsLoaded))
	{
		Q_ASSERT(m_pGeomList->isEmpty());
		if (fileName().isEmpty())
		{
			QStringList stringList("GLC_3DRep::load");
			stringList.append("Representation : " + GLC_Rep::name());
			stringList.append("Empty File Name");
			GLC_ErrorLog::addError(stringList);
		}
		else
		{
			GLC_3DRep newRep= GLC_Factory::instance()->create3DRepFromFile(fileName());
			if (!newRep.isEmpty())
			{
				const int size= newRep.m_pGeomList->size();
				for (int i= 0; i < size; ++i)
				{
					m_pGeomList->append(newRep.m_pGeomList->at(i));
				}
				newRep.m_pGeomList->clear();
				(*m_pIsLoaded)= true;
				loadSucces= true;
			}
		}
	}

	return loadSucces;

}

void GLC_3DRep::replace(GLC_Rep* pRep)
{
	GLC_3DRep* p3DRep= dynamic_cast<GLC_3DRep*>(pRep);
	Q_ASSERT(NULL != p3DRep);

	setFileName(p3DRep->fileName());
	setName(p3DRep->name());

	if (!p3DRep->isEmpty())
	{
		const int size= p3DRep->m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			m_pGeomList->append(p3DRep->m_pGeomList->at(i));
		}
		p3DRep->m_pGeomList->clear();
		(*m_pIsLoaded)= true;
	}
}

void GLC_3DRep::replaceMaterial(GLC_uint oldId, GLC_Material* pNewMaterial)
{
	//qDebug() << "GLC_3DRep::replaceMaterial";
	const int size= m_pGeomList->size();
	for (int i= 0; i < size; ++i)
	{
		GLC_Geometry* pGeom= m_pGeomList->at(i);
		if (pGeom->containsMaterial(oldId))
		{
			Q_ASSERT(!pGeom->containsMaterial(pNewMaterial->id()));
			GLC_Mesh* pMesh= dynamic_cast<GLC_Mesh*>(m_pGeomList->at(i));
			if (NULL != pMesh)
			{
				pMesh->replaceMaterial(oldId, pNewMaterial);
			}
		}
	}
}

void GLC_3DRep::merge(const GLC_3DRep* pRep)
{
	// Get the number of geometry of pRep
	const int pRepSize= pRep->m_pGeomList->size();
	for (int i= 0; i < pRepSize; ++i)
	{
		addGeom(pRep->geomAt(i)->clone());
	}
}

void GLC_3DRep::take(GLC_3DRep* pSource)
{
	// Get the number of geometry of pRep
	const int pRepSize= pSource->m_pGeomList->size();
	for (int i= 0; i < pRepSize; ++i)
	{
		addGeom(pSource->geomAt(i));
	}
	pSource->m_pGeomList->clear();
}

void GLC_3DRep::copyVboToClientSide()
{
	// Get the number of geometry of pRep
	const int pRepSize= m_pGeomList->size();
	for (int i= 0; i < pRepSize; ++i)
	{
		geomAt(i)->copyVboToClientSide();
	}
}

void GLC_3DRep::releaseVboClientSide(bool update)
{
	// Get the number of geometry of pRep
	const int pRepSize= m_pGeomList->size();
	for (int i= 0; i < pRepSize; ++i)
	{
		geomAt(i)->releaseVboClientSide(update);
	}
}

void GLC_3DRep::transformSubGeometries(const GLC_Matrix4x4& matrix)
{
	// Get the number of geometry of pRep
	const int repCount= m_pGeomList->size();
	for (int i= 0; i < repCount; ++i)
	{
		GLC_Mesh* pCurrentMesh= dynamic_cast<GLC_Mesh*>(geomAt(i));
		if (NULL != pCurrentMesh)
		{
			pCurrentMesh->transformVertice(matrix);
		}
	}
}

void GLC_3DRep::setVboUsage(bool usage)
{
	// Get the number of geometry of pRep
	const int repCount= m_pGeomList->size();
	for (int i= 0; i < repCount; ++i)
	{
		m_pGeomList->at(i)->setVboUsage(usage);
	}
}

bool GLC_3DRep::unload()
{
	bool unloadSucess= false;
	if ((NULL != m_pGeomList) && !m_pGeomList->isEmpty())
	{
		if (fileName().isEmpty())
		{
			QStringList stringList("GLC_3DRep::unload()");
			stringList.append("Cannot unload rep without filename");
			GLC_ErrorLog::addError(stringList);
		}
		else
		{
			const int size= m_pGeomList->size();
			for (int i= 0; i < size; ++i)
			{
				delete (*m_pGeomList)[i];
			}
			m_pGeomList->clear();

			(*m_pIsLoaded)= false;
			unloadSucess= true;
		}
	}
	return unloadSucess;
}

//////////////////////////////////////////////////////////////////////
// private services functions
//////////////////////////////////////////////////////////////////////

void GLC_3DRep::clear()
{
	if (isTheLast())
	{
		const int size= m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			delete (*m_pGeomList)[i];
		}
		delete m_pGeomList;
		m_pGeomList= NULL;

		delete m_pType;
		m_pType= NULL;
	}
}
// Non Member methods
QDataStream &operator<<(QDataStream & stream, const GLC_3DRep & rep)
{
	quint32 chunckId= GLC_3DRep::m_ChunkId;
	stream << chunckId;

	// The representation name
	stream << rep.name();

	// Save the list of 3DRep materials
	QList<GLC_Material> materialsList;
	QList<GLC_Material*> sourceMaterialsList= rep.materialSet().toList();
	const int materialNumber= sourceMaterialsList.size();
	for (int i= 0; i < materialNumber; ++i)
	{
		materialsList.append(*(sourceMaterialsList.at(i)));
		materialsList[i].setId(sourceMaterialsList.at(i)->id());
	}
	// Save the list of materials
	stream << materialsList;

	// Save the list of mesh
	const int meshNumber= rep.m_pGeomList->size();
	stream << meshNumber;
	for (int i= 0; i < meshNumber; ++i)
	{
		GLC_Mesh* pMesh= dynamic_cast<GLC_Mesh*>(rep.m_pGeomList->at(i));
		if (NULL != pMesh)
		{
			pMesh->saveToDataStream(stream);
		}
	}

	return stream;
}

QDataStream &operator>>(QDataStream & stream, GLC_3DRep & rep)
{
	Q_ASSERT(rep.isEmpty());

	quint32 chunckId;
	stream >> chunckId;
	Q_ASSERT(chunckId == GLC_3DRep::m_ChunkId);

	// The rep name
	QString name;
	stream >> name;
	rep.setName(name);

	// Retrieve the list of rep materials
	QList<GLC_Material> materialsList;
	stream >> materialsList;
	MaterialHash materialHash;
	// Update mesh materials hash table
	QHash<GLC_uint, GLC_uint> materialIdMap;
	const int materialsCount= materialsList.size();
	for (int i= 0; i < materialsCount; ++i)
	{
		GLC_Material* pMaterial= new GLC_Material(materialsList.at(i));
		pMaterial->setId(glc::GLC_GenID());
		materialIdMap.insert(materialsList.at(i).id(), pMaterial->id());
		materialHash.insert(pMaterial->id(), pMaterial);
	}

	int meshNumber;
	stream >> meshNumber;
	for (int i= 0; i < meshNumber; ++i)
	{
		GLC_Mesh* pMesh= new GLC_Mesh();
		pMesh->loadFromDataStream(stream, materialHash, materialIdMap);

		rep.addGeom(pMesh);
	}

	return stream;
}
