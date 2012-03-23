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
//! \file glc_worldto3dxml.h interface for the GLC_WorldTo3dxml class.

#ifndef GLC_WORLDTO3DXML_H_
#define GLC_WORLDTO3DXML_H_
#include <QObject>
#include <QXmlStreamWriter>

#include "../sceneGraph/glc_world.h"
#include "../glc_config.h"
#include <QReadWriteLock>

class QuaZip;
class QuaZipFile;
class QFile;
class GLC_Mesh;

//////////////////////////////////////////////////////////////////////
//! \class GLC_WorldTo3dxml
/*! \brief GLC_WorldTo3dxml : Export a GLC_World to a 3dxml file */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_WorldTo3dxml : public QObject
{
	Q_OBJECT

public:
	enum ExportType
	{
		Compressed3dxml,
		Exploded3dxml,
		StructureOnly
	};
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	GLC_WorldTo3dxml(const GLC_World& world, bool threaded= true);
	virtual ~GLC_WorldTo3dxml();
//@}

//////////////////////////////////////////////////////////////////////
/*! @name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Save the world to the specified file name
	bool exportTo3dxml(const QString& filename, GLC_WorldTo3dxml::ExportType exportType, bool exportMaterial= true);

	//! Save the given 3DRep into the given path name
	bool exportReferenceTo3DRep(const GLC_3DRep* p3DRep, const QString& fullFileName);

	//! Set the name of the 3dxml generator default is GLC_LIB
	inline void setGeneratorName(const QString& generator)
	{m_Generator= generator;}

	//! set interrupt flag adress
	void setInterupt(QReadWriteLock* pReadWriteLock, bool* pInterupt);
//@}

//////////////////////////////////////////////////////////////////////
/*! @name Private services functions */
//@{
//////////////////////////////////////////////////////////////////////
private:

	//! Write 3DXML Header
	void writeHeader();

	//! Write 3DXML reference 3D element
	void writeReference3D(const GLC_StructReference* pRef);

	//! Write 3DXML reference representation
	void writeReferenceRep(const GLC_3DRep* p3DRep);

	//! Write 3DXML instance 3D element
	void writeInstance3D(const GLC_StructInstance* pInstance, unsigned int parentId);

	//! Write 3DXML instance 3D element
	void writeInstanceRep(const GLC_3DRep* p3DRep, unsigned int parentId);

	//! Set the streamwriter to the specified file and return true if success
	void setStreamWriterToFile(const QString& fileName);

	//! Add the manifest to 3DXML compressed file
	void addManifest();

	//! Export the assembly structure from the list of reference
	void exportAssemblyStructure();

	//! Export assembly from the given occurence
	void exportAssemblyFromOccurence(const GLC_StructOccurence* pOccurence);

	//! Return the 3DXML string of the given matrix
	QString matrixString(const GLC_Matrix4x4& matrix);

	//! Write the given 3DRep to 3DXML 3DRep
	void write3DRep(const GLC_3DRep* pRep, const QString& fileName);

	//! Return the file name of the given 3DRep
	QString representationFileName(const GLC_3DRep* pRep);

	//! Write the given mesh to 3DXML 3DRep
	void writeGeometry(const GLC_Mesh* pMesh);

	//! Write the geometry face from the given lod and material
	void writeGeometryFace(const GLC_Mesh* pMesh, int lod, GLC_uint materialId);

	//! Write surface attributes
	void writeSurfaceAttributes(const GLC_Material* pMaterial);

	//! Write edges
	void writeEdges(const GLC_Mesh* pMesh);

	//! Write lines attributes
	void writeLineAttributes(const QColor& color);

	//! Write Material
	void writeMaterial(const GLC_Material* pMaterial);

	//! Write material attributes
	void writeMaterialAttributtes(const QString& name, const QString& type, const QString& value);

	//! Return a QString of a color
	QString colorToString(const QColor& color);

	//! Write the CATRepImage.3dxml file
	void writeCatRepImageFile(const QList<GLC_Material*>& materialList);

	//! Write CATRepresentationImage of the given material and id
	void writeCATRepresentationImage(const GLC_Material* pMat, unsigned int id);

	//! Write all material related files in the 3dxml
	void writeAllMaterialRelatedFilesIn3dxml();

	//! Write image file in 3DXML archive or folder
	void writeImageFileIn3dxml(const QList<GLC_Material*>& materialList);

	//! Write de CATMaterialRef
	void writeCatMaterialRef(const QList<GLC_Material*>& materialList);

	//! Write a material in the CATMaterialRef
	void writeMaterialToCatMaterialRef(const GLC_Material* pMat, unsigned int* id);

	//! Add the given texture to 3DXML with the given name
	void addImageTextureTo3dxml(const QImage& image, const QString& fileName);

	//! Transform the given name to the 3DXML name (no double)
	QString xmlFileName(QString fileName);

	//! Write extension attributes to 3DXML
	void writeExtensionAttributes(GLC_Attributes* pAttributes);

	//! Write the default view property of the given occurence
	void writeOccurenceDefaultViewProperty(const GLC_StructOccurence* pOccurence);

	//! return true if export must continu
	bool continu();

	//! Return the simplified name of the given name
	QString symplifyName(QString name);

	//! Return the path of the given occurence
	QList<unsigned int> instancePath(const GLC_StructOccurence* pOccurence);

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
	//! The world to export
	GLC_World m_World;

	//! The export type
	ExportType m_ExportType;

	//! The file name in which the world is exported
	QString m_FileName;

	//! The Stream writer
	QXmlStreamWriter* m_pOutStream;

	//! QString the 3DXML Generator
	QString m_Generator;

	//! The current 3DXML id
	unsigned int m_CurrentId;

	//! The 3DXML Archive
	QuaZip* m_p3dxmlArchive;

	//! The current quazp file
	QuaZipFile* m_pCurrentZipFile;

	//! The current file
	QFile* m_pCurrentFile;

	//! the 3dxml absolute path
	QString m_AbsolutePath;

	//! Map reference to 3dxml id
	QHash<const GLC_StructReference*, unsigned int> m_ReferenceToIdHash;

	//! Map instance to 3dxml id
	QHash<const GLC_StructInstance*, unsigned int> m_InstanceToIdHash;

	//! Map reference rep to 3dxml id
	QHash<const GLC_3DRep*, unsigned int> m_ReferenceRepToIdHash;

	//! Map Reference rep to 3dxml fileName
	QHash<const GLC_3DRep*, QString> m_ReferenceRepTo3dxmlFileName;

	//! InstanceRep SET
	QSet<unsigned int> m_InstanceRep;

	//! Map between material id and 3DRep name
	QHash<GLC_uint, QString> m_MaterialIdToMaterialName;

	//! Map between material id and 3dxml image id
	QHash<GLC_uint, unsigned int> m_MaterialIdToMaterialId;

	//! Map between material id and 3DXML texture name
	QHash<GLC_uint, QString> m_MaterialIdToTexture3dxmlName;

	//! Map between material id and 3dxml image id
	QHash<GLC_uint, unsigned int> m_MaterialIdTo3dxmlImageId;

	//! Flag to know if material must be exported
	bool m_ExportMaterial;

	//! Set of files in the 3dxml
	QSet<QString> m_3dxmlFileSet;

	//! file name increment
	unsigned int m_FileNameIncrement;

	//! List of structOccurence with overload properties
	QList<const GLC_StructOccurence*> m_ListOfOverLoadedOccurence;

	//! Mutex
	QReadWriteLock* m_pReadWriteLock;

	//! Flag to know if export must be interupted
	bool* m_pIsInterupted;

	//! Flag to know if export is threaded (the default)
	bool m_IsThreaded;

};

#endif /* GLC_WORLDTO3DXML_H_ */
