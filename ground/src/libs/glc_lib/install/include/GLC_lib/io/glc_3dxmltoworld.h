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

//! \file glc_3dxmltoworld.h interface for the GLC_3dxmlToWorld class.

#ifndef GLC_3DXMLTOWORLD_H_
#define GLC_3DXMLTOWORLD_H_

#include <QFile>
#include <QObject>
#include <QXmlStreamReader>
#include <QHash>
#include <QSet>
#include "../maths/glc_matrix4x4.h"
#include "../sceneGraph/glc_3dviewinstance.h"

class GLC_World;
class QGLContext;
class QuaZip;
class QuaZipFile;
class GLC_StructReference;
class GLC_StructInstance;
class GLC_ExtendedMesh;

//////////////////////////////////////////////////////////////////////
//! \class GLC_3dxmlToWorld
/*! \brief GLC_3dxmlToWorld : Create an GLC_World from 3dxml file */
//////////////////////////////////////////////////////////////////////
class GLC_3dxmlToWorld : public QObject
{
	Q_OBJECT

	//! \struct AssyLink
	/*! \brief AssyLink : Assemblage link between parent id and GLC_StructInstance* */
	struct AssyLink
	{
		unsigned int m_ParentRefId;
		GLC_StructInstance* m_pChildInstance;
	};
	//! \class RepLink
	/*! \brief RepLink : Representation link between reference id and representation id */
	struct RepLink
	{
		unsigned int m_ReferenceId;
		unsigned int  m_RepId;
	};
	//! \class MaterialRef
	/*! \brief MaterialRef : Material reference containing id, name and associated file */
	struct MaterialRef
	{
		QString m_Id;
		QString m_Name;
		QString m_AssociatedFile;
	};

	typedef QHash<unsigned int, GLC_StructReference*> ReferenceHash;
	typedef QHash<GLC_StructInstance*, unsigned int> InstanceOfHash;
	typedef QHash<GLC_StructInstance*, QString> InstanceOfExtRefHash;
	typedef QSet<const QString> SetOfExtRef;
	typedef QList<AssyLink> AssyLinkList;
	typedef QList<RepLink> RepLinkList;
	typedef QHash<const QString, GLC_StructReference*> ExternalReferenceHash;
	typedef QHash<const QString, GLC_Material*> MaterialHash;
	typedef QHash<const unsigned int, QString> ReferenceRepHash;

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_3dxmlToWorld(const QGLContext*);

	virtual ~GLC_3dxmlToWorld();
//@}
//////////////////////////////////////////////////////////////////////
/*! @name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Create an GLC_World from an input 3DXML File
	GLC_World* CreateWorldFrom3dxml(QFile &, bool StructureOnly);

	//! Create 3DRep from an 3DXML rep
	GLC_3DRep Create3DrepFrom3dxmlRep(const QString&);

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
	//! Load the 3dxml's manifest
	void loadManifest();

	//! Close all files and clear memmory
	void clear();

	//! Go to an Element of a xml
	void goToElement(const QString&);

	//! Go to a Rep of a xml
	void goToRepId(const QString&);

	//! Go to Polygonal Rep Type
	void gotToPolygonalRepType();

	// Return the content of an element
	QString getContent(const QString&);

	//! Read the specified attribute
	QString readAttribute(const QString&, bool required= false);

	//! Load the product structure
	void loadProductStructure();

	//! Load a Reference3D
	void loadReference3D();

	//! Load a Instance3D
	void loadInstance3D();

	//! Load a Reference representation
	void loadReferenceRep();

	//! Load a Instance representation
	void loadInstanceRep();

	//! Load External Ref
	void loadExternalRef3D();

	//! Add a reference from 3dxml to m_ExternalReferenceHash
	GLC_StructReference* createReferenceRep(QString id= QString());

	//! Load Matrix
	GLC_Matrix4x4 loadMatrix(const QString&);

	//! Create the unfolded  tree
	void createUnfoldedTree();

	//! Check for XML error
	//! Throw ecxeption if error occur
	void checkForXmlError(const QString&);

	//! Load Level of detail
	void loadLOD(GLC_ExtendedMesh*);

	//! Return true if the end of specified element is not reached
	inline bool endElementNotReached(const QString& element)
	{return not m_pStreamReader->atEnd() and not (m_pStreamReader->isEndElement() and (m_pStreamReader->name() == element));}

	//! Return true if the start of specified element is not reached
	inline bool startElementNotReached(const QString& element)
	{return not m_pStreamReader->atEnd() and not (m_pStreamReader->isStartElement() and (m_pStreamReader->name() == element));}

	//! Load a face
	void loadFace(GLC_ExtendedMesh*, const int lod, double accuracy);

	//! Clear material hash
	void clearMaterialHash();

	//! get material
	GLC_Material* getMaterial();

	//! Set the stream reader to the specified file
	bool setStreamReaderToFile(QString, bool test= false);

	//! Load the local representation
	void loadLocalRepresentations();

	//! Load the extern representation
	void loadExternRepresentations();

	//! Return the instance of the current extern representation
	GLC_3DRep loadCurrentExtRep();

	//! Load CatMaterial Ref if present
	void loadCatMaterialRef();

	//! Create material from material def file
	void loadMaterialDef(const MaterialRef&);

	//! Load CATRepIage if present
	void loadCatRepImage();

	//! try to load the specified image
	QImage loadImage(QString);

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! OpenGL Context
	const QGLContext* m_pQGLContext;

	//! Xml Reader
	QXmlStreamReader* m_pStreamReader;

	//! The 3dxml fileName
	QString m_FileName;

	//! The Quazip archive
	QuaZip* m_p3dxmlArchive;

	//! The Quazip file (Entry or archive)
	QuaZipFile* m_p3dxmlFile;

	//! The current file (if there is no archive)
	QFile* m_pCurrentFile;

	//! The root Name of the 3dxml file
	QString m_RootName;

	//! The World to return
	GLC_World* m_pWorld;

	//! Reference Hash Table
	ReferenceHash m_ReferenceHash;

	//! The Structure Link Hash Table
	AssyLinkList m_AssyLinkList;

	//! Instance of Hash table
	InstanceOfHash m_InstanceOf;

	//! The set of ext ref to load
	SetOfExtRef m_SetOfExtRef;

	//! Instance of ext ref hash table
	InstanceOfExtRefHash m_InstanceOfExtRefHash;

	//! Externam reference hash table
	ExternalReferenceHash m_ExternalReferenceHash;

	//! Hash table of material
	MaterialHash m_MaterialHash;

	//! Flag to know if the 3dxml is in an archive
	bool m_IsInArchive;

	//! The Reference representation hash table
	ReferenceRepHash m_ReferenceRepHash;

	//! The list of local representation link
	RepLinkList m_LocalRepLinkList;

	//! The list of extern representation link
	RepLinkList m_ExternRepLinkList;

	//! The set of ext rep to load
	SetOfExtRef m_SetOfExtRep;

	//! The 3DREP current material
	GLC_Material* m_pCurrentMaterial;

	//! The image file hash table
	QHash<QString, QString> m_TextureImagesHash;

	//! Flag indicate the loading method
	bool m_LoadStructureOnly;

	//! The list of attached file name
	QSet<QString> m_ListOfAttachedFileName;



};

#endif /* GLC_3DXMLTOWORLD_H_ */
