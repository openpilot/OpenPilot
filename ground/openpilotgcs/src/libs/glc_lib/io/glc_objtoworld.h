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

//! \file glc_objToworld.h interface for the GLC_ObjToWorld class.

#ifndef GLC_OBJTOWORLD_H_
#define GLC_OBJTOWORLD_H_

#include <QFile>
#include <QString>
#include <QObject>
#include <QHash>
#include <QVector>
#include <QStringList>

#include "../maths/glc_vector3d.h"
#include "../maths/glc_vector2df.h"
#include "../maths/glc_vector3df.h"
#include "../geometry/glc_mesh.h"

#include "../glc_config.h"

enum FaceType
{
	notSet,
	coordinate,
	coordinateAndTexture,
	coordinateAndNormal,
	coordinateAndTextureAndNormal
};

class GLC_World;
class GLC_ObjMtlLoader;
class QGLContext;

//////////////////////////////////////////////////////////////////////
//! \class GLC_ObjToWorld
/*! \brief GLC_ObjToWorld : Create an GLC_World from obj file */

/*! An GLC_ObjToWorld extract the meshs from an .obj file \n
 * 	List of elements extracted from the OBJ
 * 		- Vertex
 * 		- Face
 * 		- Texture coordinate
 * 		- Normal coordinate
  */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_ObjToWorld : public QObject
{
	Q_OBJECT

public:
	// OBJ Vertice (Position index, Normal index and TexCoord index)
	struct ObjVertice
	{
		ObjVertice()
		: m_Values(3)
		{
			m_Values[0]= 0;
			m_Values[1]= 0;
			m_Values[2]= 0;
		}
		ObjVertice(int v1, int v2, int v3)
		: m_Values(3)
		{
			m_Values[0]= v1;
			m_Values[1]= v2;
			m_Values[2]= v3;
		}

		QVector<int> m_Values;
	};

	// Material assignement
	struct MatOffsetSize
	{
		MatOffsetSize()
		: m_Offset(0)
		, m_size(0)
		{}
		int m_Offset;
		int m_size;
	};

	// Current OBJ Mesh
	struct CurrentObjMesh
	{
		CurrentObjMesh(const QString materialName)
		: m_pMesh(new GLC_Mesh())
		, m_Positions()
		, m_Normals()
		, m_Texels()
		, m_Index()
		, m_pLastOffsetSize(new MatOffsetSize())
		, m_Materials()
		, m_NextFreeIndex(0)
		, m_ObjVerticeIndexMap()
		{
			m_Materials.insert(materialName, m_pLastOffsetSize);
		}
		~CurrentObjMesh()
		{
			QHash<QString, MatOffsetSize*>::iterator i= m_Materials.begin();
			while (m_Materials.constEnd() != i)
			{
				delete i.value();
				++i;
			}
		}
		GLC_Mesh* m_pMesh;
		QList<float> m_Positions;
		QList<float> m_Normals;
		QList<float> m_Texels;
		//! The index of the current Mesh
		IndexList m_Index;
		// Pointer to the last matOffsetSize
		MatOffsetSize* m_pLastOffsetSize;
		// QHash containing material id and associated offset and size
		QHash<QString, MatOffsetSize*> m_Materials;
		//! The next free index
		int m_NextFreeIndex;
		//! The Hash table of obj vertice mapping to index
		QHash<ObjVertice, GLuint> m_ObjVerticeIndexMap;
	};

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	GLC_ObjToWorld();
	virtual ~GLC_ObjToWorld();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Create an GLC_World from an input OBJ File
	GLC_World* CreateWorldFromObj(QFile &file);

	//! Get the list of attached files
	inline QStringList listOfAttachedFileName() const{return m_ListOfAttachedFileName;}
//@}

//////////////////////////////////////////////////////////////////////
// Private services functions
//////////////////////////////////////////////////////////////////////
private:
	//! Return the name of the mtl file
	QString getMtlLibFileName(QString);

	//! Scan a line previously extracted from OBJ file
	void scanLigne(QString &);

	//! Change current group
	void changeGroup(QString);

	//! Extract a 3D Vector from a string
	QList<float> extract3dVect(QString &);

	//! Extract a 2D Vector from a string
	QList<float> extract2dVect(QString &);

	//! Extract a face from a string
	void extractFaceIndex(QString &);

	//! Set Current material index
	void setCurrentMaterial(QString &line);

	//! Extract a vertex from a string
	void extractVertexIndex(QString ligne, int &Coordinate, int &Normal, int &TextureCoordinate);

	//! set the OBJ File type
	void setObjType(QString &);

	//! compute face normal
	GLC_Vector3df computeNormal(GLuint, GLuint, GLuint);

	//! clear objToWorld allocate memmory
	void clear();

	//! Merge Mutli line in one
	void mergeLines(QString*, QTextStream*);

	//! Add the current Obj mesh to the world
	void addCurrentObjMeshToWorld();



//////////////////////////////////////////////////////////////////////
// Qt Signals
//////////////////////////////////////////////////////////////////////
	signals:
	void currentQuantum(int);

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! pointer to a GLC_World
	GLC_World* m_pWorld;

	//! The Obj File name
	QString m_FileName;

	//! the Obj Mtl loader
	GLC_ObjMtlLoader* m_pMtlLoader;

	//! The current line number
	int m_CurrentLineNumber;

	//! The current mesh
	CurrentObjMesh* m_pCurrentObjMesh;

	//! Face type
	FaceType m_FaceType;

	//! List of material already used by the current mesh
	QHash<QString, int> m_CurrentMeshMaterials;

	//! Current material name
	QString m_CurrentMaterialName;

	//! The list of attached file name
	QStringList m_ListOfAttachedFileName;

	//! The position bulk data
	QList<float> m_Positions;

	//! The normal bulk data
	QList<float> m_Normals;

	//! The texture coordinate bulk data
	QList<float> m_Texels;
};

// To use ObjVertice as a QHash key
inline bool operator==(const GLC_ObjToWorld::ObjVertice& vertice1, const GLC_ObjToWorld::ObjVertice& vertice2)
{ return (vertice1.m_Values == vertice2.m_Values);}

inline uint qHash(const GLC_ObjToWorld::ObjVertice& vertice)
{ return qHash(QString::number(vertice.m_Values.at(0)) + QString::number(vertice.m_Values.at(1)) + QString::number(vertice.m_Values.at(2)));}


#endif /*GLC_OBJTOWORLD_H_*/
