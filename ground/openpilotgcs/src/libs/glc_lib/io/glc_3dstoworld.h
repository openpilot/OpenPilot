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

//! \file glc_3dstoworld.h interface for the GLC_3dsToWorld class.

#ifndef GLC_3DSTOWORLD_H_
#define GLC_3DSTOWORLD_H_

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QString>
#include <QHash>
#include <QSet>
#include <QStringList>

#include "../sceneGraph/glc_3dviewinstance.h"

#include "../glc_config.h"

class GLC_World;
class QGLContext;
class GLC_Mesh;
class GLC_StructOccurence;
class GLC_Material;

struct Lib3dsFile;
struct Lib3dsNode;
struct Lib3dsMesh;
struct Lib3dsMaterial;

//////////////////////////////////////////////////////////////////////
//! \class GLC_3dsToWorld
/*! \brief GLC_3dsToWorld : Create an GLC_World from 3ds file */

/*! An GLC_3dsToWorld extract meshs from an .3ds file \n
 * 	List of elements extracted from the 3ds
 * 		- Vertex
 * 		- Face
 * 		- Normal coordinate
 * 		- Material
 * 		- Meshes
  */
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_3dsToWorld : public QObject
{
	Q_OBJECT

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	GLC_3dsToWorld();
	virtual ~GLC_3dsToWorld();
//@}

//////////////////////////////////////////////////////////////////////
/*! @name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Create an GLC_World from an input 3DS File
	GLC_World* CreateWorldFrom3ds(QFile &file);

	//! Get the list of attached files
	inline QStringList listOfAttachedFileName() const
	{return m_ListOfAttachedFileName.toList();}

//@}

//////////////////////////////////////////////////////////////////////
/*! @name Private services functions */
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! clear 3dsToWorld allocate memmory
	void clear();

	//! Create meshes from the 3ds File
	void createMeshes(GLC_StructOccurence*, Lib3dsNode*);

	//! Create 3DRep from a Lib3dsMesh
	GLC_3DRep create3DRep(Lib3dsMesh*);

	//! Load Material
	void loadMaterial(Lib3dsMaterial*);

//@}

//////////////////////////////////////////////////////////////////////
// Qt Signals
//////////////////////////////////////////////////////////////////////
	signals:
	void currentQuantum(int);

//////////////////////////////////////////////////////////////////////
	/* Private members */
//////////////////////////////////////////////////////////////////////
private:
	//! pointer to a GLC_World
	GLC_World* m_pWorld;

	//! The 3DS File name
	QString m_FileName;

	//! The current mesh
	GLC_Mesh* m_pCurrentMesh;

	//! The Lib3dsFile Structure
	Lib3dsFile* m_pLib3dsFile;

	//! The GLC_Material Hash Table
	QHash<QString, GLC_Material*> m_Materials;

	//! The next material index
	int m_NextMaterialIndex;

	// The Hash of loaded meshes
	QSet<QString> m_LoadedMeshes;

	// Initial quantum value
	const int m_InitQuantumValue;

	// The current quantum value
	int m_CurrentQuantumValue;

	// The previous quantum value
	int m_PreviousQuantumValue;

	// The number of meshes
	int m_NumberOfMeshes;

	// The Current mesh index
	int m_CurrentMeshNumber;

	//! The list of attached file name
	QSet<QString> m_ListOfAttachedFileName;





};

#endif /*GLC_3DSTOWORLD_H_*/
