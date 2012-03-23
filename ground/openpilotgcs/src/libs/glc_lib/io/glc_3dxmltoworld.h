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

//! \file glc_3dxmltoworld.h interface for the GLC_3dxmlToWorld class.

#ifndef GLC_3DXMLTOWORLD_H_
#define GLC_3DXMLTOWORLD_H_

#include <QFile>
#include <QObject>
#include <QXmlStreamReader>
#include <QHash>
#include <QSet>
#include <QDateTime>
#include "../maths/glc_matrix4x4.h"
#include "../sceneGraph/glc_3dviewinstance.h"

#include "../glc_config.h"

class GLC_World;
class QGLContext;
class QuaZip;
class QuaZipFile;
class GLC_StructReference;
class GLC_StructInstance;
class GLC_StructOccurence;
class GLC_Mesh;

//////////////////////////////////////////////////////////////////////
//! \class GLC_3dxmlToWorld
/*! \brief GLC_3dxmlToWorld : Create an GLC_World from 3dxml file */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_3dxmlToWorld : public QObject
{
	Q_OBJECT

	//! \struct AssyLink
	/*! \brief AssyLink : Assemblage link between parent id and GLC_StructInstance* */
	struct AssyLink
	{
		unsigned int m_ParentRefId;
		GLC_StructInstance* m_pChildInstance;
		unsigned int m_InstanceId;
		inline bool operator < (const AssyLink& l) const
		{return m_InstanceId < l.m_InstanceId;}
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

	//! \class V3OccurenceAttrib
	/*! \brief V3OccurenceAttrib : Specifique occurence attribute */
	struct V3OccurenceAttrib
	{
		inline V3OccurenceAttrib()
		: m_IsVisible(true)
		, m_pRenderProperties(NULL)
		{}
		inline ~V3OccurenceAttrib()
		{delete m_pRenderProperties;}

		//! Visibility attribute
		bool m_IsVisible;
		//! Render properties attribute
		GLC_RenderProperties* m_pRenderProperties;
	};

	//! \class V3OccurenceAttrib
	/*! \brief V3OccurenceAttrib : Specifique occurence attribute */
	struct V4OccurenceAttrib
	{
		inline V4OccurenceAttrib()
		: m_IsVisible(true)
		, m_pRenderProperties(NULL)
		, m_pMatrix(NULL)
		, m_Path()
		{}
		inline ~V4OccurenceAttrib()
		{
			delete m_pRenderProperties;
			delete m_pMatrix;
		}

		//! Visibility attribute
		bool m_IsVisible;
		//! Render properties attribute
		GLC_RenderProperties* m_pRenderProperties;
		//! Relative matrix
		GLC_Matrix4x4* m_pMatrix;
		//! The path of this attrib
		QList<unsigned int> m_Path;
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
	GLC_3dxmlToWorld();

	virtual ~GLC_3dxmlToWorld();
//@}
//////////////////////////////////////////////////////////////////////
/*! @name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Create an GLC_World from an input 3DXML File
	GLC_World* createWorldFrom3dxml(QFile &, bool StructureOnly, bool getExternalRef= false);

	//! Create 3DRep from an 3DXML rep
	GLC_3DRep create3DrepFrom3dxmlRep(const QString&);

	//! Get the list of attached files
	inline QStringList listOfAttachedFileName() const
	{return m_SetOfAttachedFileName.toList();}


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

	//! Go to a Rep of a xml
	void goToRepId(const QString&);

	//! Go to Polygonal Rep Type
	void gotToPolygonalRepType();

	//! Read the specified attribute
	QString readAttribute(const QString&, bool required= false);

	//! Read the Header
	void readHeader();

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
	GLC_StructReference* createReferenceRep(QString id, GLC_3DRep* pRep);

	//! Load Matrix
	GLC_Matrix4x4 loadMatrix(const QString&);

	//! Create the unfolded  tree
	void createUnfoldedTree();

	//! Check for XML error
	//! Throw ecxeption if error occur
	void checkForXmlError(const QString&);

	//! Load a face
	void loadFace(GLC_Mesh*, const int lod, double accuracy);

	//! Load polyline
	void loadPolyline(GLC_Mesh*);

	//! Clear material hash
	void clearMaterialHash();

	//! Load surface attributes
	GLC_Material* loadSurfaceAttributes();

	//! get material
	GLC_Material* getMaterial();

	//! Set the stream reader to the specified file
	bool setStreamReaderToFile(QString, bool test= false);

	//! Load default view element
	void loadDefaultView();

	//! Load 3DXML V3 default view property
	void loadV3DefaultViewProperty();

	//! Load 3DXML V4 default view property
	void loadV4DefaultViewProperty();

	//! Return the occurence path of the current DefaultViewProperty
	QList<unsigned int> loadOccurencePath();

	//! Load Graphics properties element
	void loadGraphicProperties(V4OccurenceAttrib* pAttrib);

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

	//! Try to construct a texture with the specified fileName
	GLC_Texture* loadTexture(QString);

	//! Set fileName of the given 3DRep
	void setRepresentationFileName(GLC_3DRep* pRep);

	//! Read next element from the stream
	inline QXmlStreamReader::TokenType readNext();

	//! Go to the given xml Element, return true on succes
	inline bool goToElement(QXmlStreamReader* pReader, const QString& element);

	// Return the content of an element
	inline QString getContent(QXmlStreamReader* pReader, const QString& element);

	//! Read the specified attribute
	inline QString readAttribute(QXmlStreamReader* pReader, const QString& attribute);

	//! Return true if the end of specified element is not reached
	inline bool endElementNotReached(QXmlStreamReader* pReader, const QString& element);

	//! Return true if the start of specified element is not reached
	inline bool startElementNotReached(QXmlStreamReader* pReader, const QString& element);

	//! Go to the end Element of a xml
	inline void goToEndElement(QXmlStreamReader* pReader, const QString& element);

	//! Check if the given file is binary
	void checkFileValidity(QIODevice* pIODevice);

	//! Apply the given attribute to the right occurence from the given occurence
	void applyV4Attribute(GLC_StructOccurence* pOccurence, V4OccurenceAttrib* pV4OccurenceAttrib, QHash<GLC_StructInstance*, unsigned int>& InstanceToIdHash);

	//! Load representation from 3DRep file
	void loadRep(GLC_Mesh* pMesh);

	//! Load The 3DXML vertex buffer
	void loadVertexBuffer(GLC_Mesh* pMesh);

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Xml Reader
	QXmlStreamReader* m_pStreamReader;

	//! The 3dxml fileName
	QString m_FileName;

	//! The Quazip archive
	QuaZip* m_p3dxmlArchive;

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

	//! The Set of attached file name
	QSet<QString> m_SetOfAttachedFileName;

	//! The current file name
	QString m_CurrentFileName;

	//! The current file time and date
	QDateTime m_CurrentDateTime;

	//! Hash table of occurence specific attributes for 3DXML V3
	QHash<unsigned int, V3OccurenceAttrib*> m_V3OccurenceAttribHash;

	//! List of occurence specific attributes for 3DXML V4
	QList<V4OccurenceAttrib*> m_V4OccurenceAttribList;

	//! bool get external ref 3D name
	bool m_GetExternalRef3DName;

	static QMutex m_ZipMutex;

	QList<QByteArray> m_ByteArrayList;

	//! Flag to know if the 3DXML is in version 3.x
	bool m_IsVersion3;

};

QXmlStreamReader::TokenType GLC_3dxmlToWorld::readNext()
{
	QXmlStreamReader::TokenType token= m_pStreamReader->readNext();
	if (QXmlStreamReader::PrematureEndOfDocumentError == m_pStreamReader->error())
	{
		//qDebug() << "QXmlStreamReader::PrematureEndOfDocumentError == m_pStreamReader->error()";
		if (!m_ByteArrayList.isEmpty())
		{
			m_pStreamReader->addData(m_ByteArrayList.takeFirst());
			return readNext();
		}
	}
	return token;
}

bool GLC_3dxmlToWorld::goToElement(QXmlStreamReader* pReader, const QString& element)
{
	while(!pReader->atEnd() && !pReader->hasError() && !(pReader->isStartElement() && (pReader->name() == element)))
	{
		readNext();
	}
	return !pReader->atEnd() && !pReader->hasError();
}

QString GLC_3dxmlToWorld::getContent(QXmlStreamReader* pReader, const QString& element)
{
	QString content;
	while(endElementNotReached(pReader, element))
	{
		readNext();
		if (pReader->isCharacters() && !pReader->text().isEmpty())
		{
			content+= pReader->text().toString();
		}
	}

	return content.trimmed();
}

QString GLC_3dxmlToWorld::readAttribute(QXmlStreamReader* pReader, const QString& attribute)
{
	return pReader->attributes().value(attribute).toString();
}

bool GLC_3dxmlToWorld::endElementNotReached(QXmlStreamReader* pReader, const QString& element)
{
	return !pReader->atEnd() && !pReader->hasError() && !(pReader->isEndElement() && (pReader->name() == element));
}

bool GLC_3dxmlToWorld::startElementNotReached(QXmlStreamReader* pReader, const QString& element)
{
	return !pReader->atEnd() && !pReader->hasError() && !(pReader->isStartElement() && (pReader->name() == element));
}

void GLC_3dxmlToWorld::goToEndElement(QXmlStreamReader* pReader, const QString& element)
{
	while(endElementNotReached(pReader, element))
	{
		readNext();
	}
}


#endif /* GLC_3DXMLTOWORLD_H_ */
