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
//! \file glc_colladatoworld.h interface for the GLC_ColladaToWorld class.

#ifndef GLC_COLLADATOWORLD_H_
#define GLC_COLLADATOWORLD_H_

#include <QObject>
#include <QString>
#include <QFile>
#include <QXmlStreamReader>
#include <QHash>
#include <QColor>

#include "../shading/glc_material.h"
#include "../geometry/glc_mesh.h"
#include "../sceneGraph/glc_structoccurence.h"

#include "../glc_config.h"

class GLC_World;
class QGLContext;

//////////////////////////////////////////////////////////////////////
//! \class GLC_ColladaToWorld
/*! \brief GLC_ColladaToWorld : Create an GLC_World from dae file */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_ColladaToWorld : public QObject
{
private:
	Q_OBJECT

	// The 3 supported semantic
	enum Semantic
	{ // Values are very important !
		VERTEX= 0,
		NORMAL= 1,
		TEXCOORD= 2
	};

	// input data info
	struct InputData
	{
		int m_Offset;
		QString m_Source;
		Semantic m_Semantic;
	};
public:
	// Collada Vertice (Position index, Normal index and TexCoord index)
	struct ColladaVertice
	{
		ColladaVertice()
		: m_Values(3)
		{
			m_Values[0]= 0;
			m_Values[1]= 0;
			m_Values[2]= 0;
		}

		QVector<int> m_Values;
	};
private:

	// Material assignement
	struct MatOffsetSize
	{
		int m_Offset;
		int m_size;
	};

	// Accessor a source data (bulk)
	struct Accessor
	{
		Accessor()
		: m_Count(0)
		, m_Offset(0)
		, m_Stride(1)
		{}
		unsigned int m_Count;
		unsigned int m_Offset;
		unsigned int m_Stride;
	};

	// The loading mesh info
	struct MeshInfo
	{
		MeshInfo()
		: m_pMesh(NULL)
		, m_Datas(3)
		, m_Mapping()
		, m_Index()
		, m_FreeIndex(0)
		{}

		~MeshInfo() {delete m_pMesh;}
		// Mesh of the mesh info
		GLC_Mesh* m_pMesh;
		// Bulk data vector (Position, normal, texel)
		QVector<QList<float> > m_Datas;
		// Mapping between collada vertice and index
		QHash<ColladaVertice, GLuint> m_Mapping;
		// Triangle index
		IndexList m_Index;
		// Next index Position
		GLuint m_FreeIndex;
		// QHash containing material id and associated offset and size
		QHash<QString, MatOffsetSize> m_Materials;
	};

	// The collada Node
	struct ColladaNode
	{
		ColladaNode(const QString id, ColladaNode* pParent)
		: m_Id(id)
		, m_Matrix()
		, m_InstanceGeometryIDs()
		, m_InstanceOffNodeIds()
		, m_ChildNodes()
		, m_pParent(pParent)
		{}
		// Destrucot not needed
		// The node id
		QString m_Id;
		// Position matrix
		GLC_Matrix4x4 m_Matrix;
		// Instance geometry id
		QList<QString> m_InstanceGeometryIDs;
		// Instance off another node
		QList<QString> m_InstanceOffNodeIds;
		// Child Node
		QList<ColladaNode*> m_ChildNodes;
		// Parent Node
		ColladaNode* m_pParent;
	};

	typedef QHash<const QString, GLC_Material*> MaterialHash;
	typedef QHash<const QString, QList<float> > BulkDataHash;
	typedef QHash<const QString, Accessor> DataAccessorHash;
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_ColladaToWorld();

	//! Destructor
	virtual ~GLC_ColladaToWorld();
//@}

//////////////////////////////////////////////////////////////////////
/*! @name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Create an GLC_World from an input Collada File
	GLC_World* CreateWorldFromCollada(QFile &);

	//! Get the list of attached files
	inline QStringList listOfAttachedFileName() const
	{return m_ListOfAttachedFileName.toList();}

//@}

//////////////////////////////////////////////////////////////////////
// Qt Signals
//////////////////////////////////////////////////////////////////////
	signals:
	void currentQuantum(int);

//////////////////////////////////////////////////////////////////////
/*! @name Private services functions */
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Go to an Element of a xml
	void goToElement(const QString&);

	//! Go to the end Element of a xml
	void goToEndElement(const QString&);

	// Return the content of an element
	QString getContent(const QString&);

	//! Read the specified attribute
	QString readAttribute(const QString&, bool required= false);

	//! Check for XML error
	//! Throw ecxeption if error occur
	void checkForXmlError(const QString&);

	//! Throw an exception with the specified text
	void throwException(const QString&);

	//! Clear memmory
	void clear();

	//! Load library_images element
	void loadLibraryImage();

	//! Load image element
	void loadImage();

	//! Load library_materials element
	void loadLibraryMaterials();

	//! Load a material
	void loadMaterial();

	//! Load library_effects element
	void loadLibraryEffects();

	//! Load an effect
	void loadEffect();

	//! Load profile_COMMON
	void loadProfileCommon();

	//! Load a new param
	void loadNewParam();

	//! Load a surface
	void loadSurface(const QString&);

	//! Load Sampler 2D
	void loadSampler2D(const QString&);

	//! Load technique
	void loadTechnique();

	//! load material technique
	void loadMaterialTechnique(const QString&);

	//! load common color or texture
	void loadCommonColorOrTexture(const QString&);

	//! Load transparent
	void loadTransparent();

	//! Load transparency
	void loadTransparency(const QString&);

	//! Load shininess
	void loadShininess(const QString&);

	//! Read a xml Color
	QColor readXmlColor();

	//! Load library_geometries element
	void loadLibraryGeometries();

	//! Load an geometry element
	void loadGeometry();

	//! Load a mesh
	void loadMesh();

	//! Load Vertex bulk data
	void loadVertexBulkData();

	//! Load Technique Common
	void loadTechniqueCommon();

	//! Load Accessor
	void loadAccessor();

	//! Load attributes and identity of mesh vertices
	void loadVertices();

	//! Load polylist
	void loadPolylist();

	//! Load Polygons
	void loadPolygons();

	//! Add the polylist to the current mesh
	void addPolylistToCurrentMesh(const QList<InputData>&, const QList<int>&, const QList<int>&, const QString&);

	//! Compute Normals for the current primitive element of the current mesh from the specified offset
	void computeNormalOfCurrentPrimitiveOfCurrentMesh(int offset);

	//! Load triangles
	void loadTriangles();

	//! Add the triangles to current mesh
	void addTrianglesToCurrentMesh(const QList<InputData>&, const QList<int>&, const QString&);

	//! Load the library nodes
	void loadLibraryNodes();

	//! Load the library controllers
	void loadLibraryContollers();

	//! Load library_visual_scenes element
	void loadVisualScenes();

	//! Load an instance geometry
	void loadInstanceGeometry(ColladaNode*);

	//! Load an instance geometry
	void loadInstanceNode(ColladaNode*);

	//! Load an instance Controller
	void loadInstanceController(ColladaNode*);

	//! Load a collada controller node
	void loadController();

	//! Load a Collada Node element and return it
	ColladaNode* loadNode(ColladaNode*);

	//! Translate the node
	void translateNode(ColladaNode*);

	//! Scale the node
	void scaleNode(ColladaNode*);

	//! Rotate the node
	void rotateNode(ColladaNode*);

	//! Compose Node matrix
	void composeMatrixNode(ColladaNode*);

	//! Load scene element
	void loadScene();

	//! Link texture to material
	void linkTexturesToMaterials();

	//! Create mesh and link them to material
	void createMesh();

	//! Create the scene graph struct
	void createSceneGraph();

	//! Create Occurence tree from node tree
	GLC_StructOccurence* createOccurenceFromNode(ColladaNode*);

	//! Update progress bar
	void updateProgressBar();



//@}
//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! The world to built
	GLC_World* m_pWorld;

	//! Xml Reader
	QXmlStreamReader* m_pStreamReader;

	//! The collada fileName
	QString m_FileName;

	//! The collada file
	QFile* m_pFile;

	//! Map image id to image file name
	QHash<QString, QString> m_ImageFileHash;

	//! Map materialInstance to material
	QHash<QString, QString> m_MaterialLibHash;

	//! Map surface sid to image id
	QHash<QString, QString> m_SurfaceImageHash;

	//! Map sampler2D sid to surface sid
	QHash<QString, QString> m_Sampler2DSurfaceHash;

	//! Material Effect hash table
	MaterialHash m_MaterialEffectHash;

	//! The current material
	GLC_Material* m_pCurrentMaterial;

	//! Texture to material link
	MaterialHash m_TextureToMaterialHash;

	//! Bulk data hash table
	BulkDataHash m_BulkDataHash;

	//! Data accessor hash
	DataAccessorHash m_DataAccessorHash;

	//! Map vertices id to source data id
	QHash<QString, QString> m_VerticesSourceHash;

	//! The current loadeed mesh
	MeshInfo* m_pMeshInfo;

	//! Hash table off geometry (MeshInfo*)
	QHash<const QString, MeshInfo*> m_GeometryHash;

	//! Hash table off collada node
	QHash<const QString, ColladaNode*> m_ColladaNodeHash;

	//! The list of top level node
	QList<ColladaNode*> m_TopLevelColladaNode;

	//! Mapping between material instance and material
	QHash<const QString, QString> m_MaterialInstanceMap;

	//! 3DRep hash table
	QHash<const QString, GLC_3DRep*> m_3DRepHash;

	//! GLC instance Hash table
	QHash<const QString, GLC_StructInstance*> m_StructInstanceHash;

	//! The current Collada Element id
	QString m_CurrentId;

	//! The Collada file size
	qint64 m_FileSize;

	//! The current offset in the collada file
	int m_CurrentOffset;

	//! The list of attached file name
	QSet<QString> m_ListOfAttachedFileName;

	//! The transparent mode is RGB_ZERO
	bool m_TransparentIsRgbZero;

};

// To use ColladaVertice as a QHash key
inline bool operator==(const GLC_ColladaToWorld::ColladaVertice& vertice1, const GLC_ColladaToWorld::ColladaVertice& vertice2)
{ return (vertice1.m_Values == vertice2.m_Values);}

inline uint qHash(const GLC_ColladaToWorld::ColladaVertice& vertice)
{ return qHash(QString::number(vertice.m_Values.at(0)) + QString::number(vertice.m_Values.at(1)) + QString::number(vertice.m_Values.at(2)));}

#endif /* GLC_COLLADATOWORLD_H_ */
