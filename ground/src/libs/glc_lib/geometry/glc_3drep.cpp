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

#include "glc_3drep.h"
#include "../glc_factory.h"

// Default constructor
GLC_3DRep::GLC_3DRep()
: GLC_Rep()
, m_pGeomList(new QList<GLC_VboGeom*>)
, m_pType(new int(GLC_Rep::GLC_VBOGEOM))
{

}

// Construct a 3DRep with a geometry
GLC_3DRep::GLC_3DRep(GLC_VboGeom* pGeom)
: GLC_Rep()
, m_pGeomList(new QList<GLC_VboGeom*>)
, m_pType(new int(GLC_Rep::GLC_VBOGEOM))
{
	m_pGeomList->append(pGeom);
	setName(pGeom->name());
}

// Copy Constructor
GLC_3DRep::GLC_3DRep(const GLC_3DRep& rep)
: GLC_Rep(rep)
, m_pGeomList(rep.m_pGeomList)
, m_pType(rep.m_pType)
{

}

// Assignement operator
GLC_3DRep& GLC_3DRep::operator=(const GLC_3DRep& rep)
{
	clear();
	GLC_Rep::operator=(rep);
	m_pGeomList= rep.m_pGeomList;
	m_pType= rep.m_pType;

	return *this;
}

// Clone the representation
GLC_Rep* GLC_3DRep::clone() const
{
	return new GLC_3DRep(*this);
}

// Make a deep copy of the 3DRep
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

// Destructor
GLC_3DRep::~GLC_3DRep()
{
	clear();
}

//////////////////////////////////////////////////////////////////////
// Get functions
//////////////////////////////////////////////////////////////////////
// Return the type of representation
int GLC_3DRep::type() const
{
	return (*m_pType);
}

//////////////////////////////////////////////////////////////////////
// Get functions
//////////////////////////////////////////////////////////////////////

// Return true if the rep bounding box is valid
bool GLC_3DRep::boundingBoxIsValid() const
{
	bool result= not m_pGeomList->isEmpty();
	const int max= m_pGeomList->size();
	int index= 0;
	while (result and (index < max))
	{
		result= result and m_pGeomList->at(index)->boundingBoxIsValid();
		++index;
	}
	return result;
}

// Get number of faces
unsigned int GLC_3DRep::numberOfFaces() const
{
	unsigned int result= 0;
	if (not m_pGeomList->isEmpty())
	{
		const int size= m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			result+= m_pGeomList->at(i)->numberOfFaces();
		}
	}

	return result;
}

// Get number of vertex
unsigned int GLC_3DRep::numberOfVertex() const
{
	unsigned int result= 0;
	if (not m_pGeomList->isEmpty())
	{
		const int size= m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			result+= m_pGeomList->at(i)->numberOfVertex();
		}
	}

	return result;
}

// Get number of materials
unsigned int GLC_3DRep::numberOfMaterials() const
{
	unsigned int result= 0;
	if (not m_pGeomList->isEmpty())
	{
		const int size= m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			result+= m_pGeomList->at(i)->numberOfMaterials();
		}
	}

	return result;
}

// Get materials List
QSet<GLC_Material*> GLC_3DRep::materialSet() const
{
	QSet<GLC_Material*> result;
	if (not m_pGeomList->isEmpty())
	{
		const int size= m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			result.unite(m_pGeomList->at(i)->materialSet());
		}
	}

	return result;
}

// Remove empty geometries
void GLC_3DRep::removeEmptyGeometry()
{
	QList<GLC_VboGeom*>::iterator iGeomList= m_pGeomList->begin();
	while(iGeomList != m_pGeomList->constEnd())
	{
		if ((*iGeomList)->numberOfVertex() == 0)
		{
			delete (*iGeomList);
			iGeomList= m_pGeomList->erase(iGeomList);
		}
		else
		{
			++iGeomList;
		}
	}
}

// Reverse geometries normals
void GLC_3DRep::reverseNormals()
{
	const int size= m_pGeomList->size();
	for (int i= 0; i < size; ++i)
	{
		(*m_pGeomList)[i]->reverseNormals();
	}
}

// Load the representation
bool GLC_3DRep::load()
{
	Q_ASSERT((not (*m_pIsLoaded)) == m_pGeomList->isEmpty());
	if ((*m_pIsLoaded) or fileName().isEmpty())
	{
		qDebug() << "GLC_3DRep::load() Allready loaded or empty fileName";
		return false;
	}
	GLC_3DRep newRep= GLC_Factory::instance()->create3DrepFromFile(fileName());
	if (not newRep.isEmpty())
	{
		const int size= newRep.m_pGeomList->size();
		for (int i= 0; i < size; ++i)
		{
			m_pGeomList->append(newRep.m_pGeomList->at(i));
		}
		newRep.m_pGeomList->clear();
		(*m_pIsLoaded)= true;
		return true;
	}
	else
	{
		return false;
	}
}
// Replace the representation
void GLC_3DRep::replace(GLC_Rep* pRep)
{
	GLC_3DRep* p3DRep= dynamic_cast<GLC_3DRep*>(pRep);
	Q_ASSERT(NULL != p3DRep);

	setFileName(p3DRep->fileName());
	setName(p3DRep->name());

	if (not p3DRep->isEmpty())
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

// UnLoad the representation
bool GLC_3DRep::unload()
{
	Q_ASSERT((not (*m_pIsLoaded)) == m_pGeomList->isEmpty());
	if (not (*m_pIsLoaded) or fileName().isEmpty())
	{
		qDebug() << "GLC_3DRep::unload() Not loaded or empty fileName";
		return false;
	}

	const int size= m_pGeomList->size();
	for (int i= 0; i < size; ++i)
	{
		delete (*m_pGeomList)[i];
	}

	(*m_pIsLoaded)= false;
	return true;
}

//////////////////////////////////////////////////////////////////////
// private services functions
//////////////////////////////////////////////////////////////////////

// Clear the 3D representation
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
