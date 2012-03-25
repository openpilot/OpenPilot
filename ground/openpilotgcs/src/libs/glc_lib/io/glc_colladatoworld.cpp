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

#include "glc_colladatoworld.h"
#include "../sceneGraph/glc_world.h"
#include "../glc_fileformatexception.h"
#include "../maths/glc_geomtools.h"
#include "../glc_factory.h"
#include "glc_xmlutil.h"

static QString prefixNodeId= "GLC_LIB_COLLADA_ID_";
static int currentNodeId= 0;

using namespace glcXmlUtil;

// Default constructor
GLC_ColladaToWorld::GLC_ColladaToWorld()
: QObject()
, m_pWorld(NULL)
, m_pStreamReader(NULL)
, m_FileName()
, m_pFile(NULL)
, m_ImageFileHash()
, m_MaterialLibHash()
, m_SurfaceImageHash()
, m_MaterialEffectHash()
, m_pCurrentMaterial(NULL)
, m_TextureToMaterialHash()
, m_BulkDataHash()
, m_DataAccessorHash()
, m_VerticesSourceHash()
, m_pMeshInfo(NULL)
, m_GeometryHash()
, m_ColladaNodeHash()
, m_TopLevelColladaNode()
, m_MaterialInstanceMap()
, m_3DRepHash()
, m_StructInstanceHash()
, m_CurrentId()
, m_FileSize(0)
, m_CurrentOffset(0)
, m_ListOfAttachedFileName()
, m_TransparentIsRgbZero(false)
{
	currentNodeId= 0;
}

// Destructor
GLC_ColladaToWorld::~GLC_ColladaToWorld()
{
	// Normal ends, world has not to be deleted
	m_pWorld= NULL;
	clear();
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
// Create an GLC_World from an input Collada File
GLC_World* GLC_ColladaToWorld::CreateWorldFromCollada(QFile &file)
{
	m_pWorld= new GLC_World();
	m_FileName= file.fileName();
	m_pFile= &file;

	//////////////////////////////////////////////////////////////////
	// Test if the file exist and can be opened
	//////////////////////////////////////////////////////////////////
	if (!m_pFile->open(QIODevice::ReadOnly))
	{
		QString message(QString("GLC_ColladaToWorld::CreateWorldFromCollada File ") + m_FileName + QString(" doesn't exist"));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::FileNotFound);
		throw(fileFormatException);
	}
	// Get the file size
	m_FileSize= QFileInfo(m_FileName).size();

	m_pStreamReader= new QXmlStreamReader(m_pFile);

	// Go to the collada root Element
	goToElement("COLLADA");

	// Read the collada version
	QString version= readAttribute("version", true);

	// Go to the asset Element to get the Up vector
	goToElement("asset");
	while (endElementNotReached(m_pStreamReader, "asset"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "up_axis")
			{
				const QString upAxis= getContent("up_axis");
				if (upAxis == "X_UP") m_pWorld->setUpVector(glc::X_AXIS);
				else if (upAxis == "Y_UP") m_pWorld->setUpVector(glc::Y_AXIS);
				else if (upAxis == "Z_UP") m_pWorld->setUpVector(glc::Z_AXIS);
			}
		}
		m_pStreamReader->readNext();
	}

	while (endElementNotReached(m_pStreamReader, "COLLADA"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "library_images") loadLibraryImage();
			else if (currentElementName == "library_materials") loadLibraryMaterials();
			else if (currentElementName == "library_effects") loadLibraryEffects();
			else if (currentElementName == "library_geometries") loadLibraryGeometries();
			else if (currentElementName == "library_nodes") loadLibraryNodes();
			else if (currentElementName == "library_controllers") loadLibraryContollers();
			else if (currentElementName == "library_visual_scenes") loadVisualScenes();
			else if (currentElementName == "scene") loadScene();
		}

		m_pStreamReader->readNext();
	}

	m_pFile->close();
	m_pFile= NULL;

	// Link the textures to materials
	linkTexturesToMaterials();

	// Create the mesh and link them to material
	createMesh();

	// Create the scene graph struct
	createSceneGraph();

	emit currentQuantum(100);

	return m_pWorld;
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
// Go to Element
void GLC_ColladaToWorld::goToElement(const QString& elementName)
{
	while(startElementNotReached(m_pStreamReader, elementName))
	{
		m_pStreamReader->readNext();
	}
	checkForXmlError(QString("Element ") + elementName + QString(" Not Found"));
}

// Go to the end Element of a xml
void GLC_ColladaToWorld::goToEndElement(const QString& elementName)
{
	while(endElementNotReached(m_pStreamReader, elementName))
	{
		m_pStreamReader->readNext();
	}
	checkForXmlError(QString("End Element ") + elementName + QString(" Not Found"));
}

// Return the content of an element
QString GLC_ColladaToWorld::getContent(const QString& element)
{
	QString Content;
	while(endElementNotReached(m_pStreamReader, element))
	{
		m_pStreamReader->readNext();
		if (m_pStreamReader->isCharacters() && !m_pStreamReader->text().isEmpty())
		{
			Content+= m_pStreamReader->text().toString();
		}
	}

	return Content.simplified();
}

// Read the specified attribute
QString GLC_ColladaToWorld::readAttribute(const QString& name, bool required)
{
	QString attributeValue;
	if (required && !m_pStreamReader->attributes().hasAttribute(name))
	{
		QString message(QString("required attribute ") + name + QString(" Not found"));
		GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}
	else
	{
		attributeValue= m_pStreamReader->attributes().value(name).toString();
	}
	return attributeValue;
}

// Check for XML error
void GLC_ColladaToWorld::checkForXmlError(const QString& info)
{
	if (m_pStreamReader->atEnd() || m_pStreamReader->hasError())
	{
		GLC_FileFormatException fileFormatException(info, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}
}
// Throw an exception with the specified text
void GLC_ColladaToWorld::throwException(const QString& message)
{
	GLC_FileFormatException fileFormatException(message, m_FileName, GLC_FileFormatException::WrongFileFormat);
	clear();
	throw(fileFormatException);
}

// Clear memmory
void GLC_ColladaToWorld::clear()
{
	delete m_pWorld;
	m_pWorld= NULL;


	delete m_pStreamReader;
	m_pStreamReader= NULL;

	if (m_pFile != NULL) m_pFile->close();
	m_pFile= NULL;

	m_ImageFileHash.clear();
	m_MaterialLibHash.clear();
	m_SurfaceImageHash.clear();

	// Clear the material effect hash table
	MaterialHash::iterator iMat= m_MaterialEffectHash.begin();
	while (iMat != m_MaterialEffectHash.constEnd())
	{
		if (iMat.value()->isUnused()) delete iMat.value();
		++iMat;
	}
	m_MaterialEffectHash.clear();

	delete m_pCurrentMaterial;
	m_pCurrentMaterial= NULL;

	m_TextureToMaterialHash.clear();

	m_BulkDataHash.clear();
	m_DataAccessorHash.clear();

	m_VerticesSourceHash.clear();

	delete m_pMeshInfo;
	m_pMeshInfo= NULL;

	// Delete all geometry from the geometry hash
	QHash<const QString, MeshInfo*>::iterator iGeomHash= m_GeometryHash.begin();
	while (m_GeometryHash.constEnd() != iGeomHash)
	{
		delete iGeomHash.value();
		++iGeomHash;
	}
	m_GeometryHash.clear();

	// Delete all collada node from the colalda node hash
	QHash<const QString, ColladaNode*>::iterator iColladaNode= m_ColladaNodeHash.begin();
	while (m_ColladaNodeHash.constEnd() != iColladaNode)
	{
		delete iColladaNode.value();
		++iColladaNode;
	}
	m_ColladaNodeHash.clear();

	// Clear the list of top level node (Not must not to be deleted)
	m_TopLevelColladaNode.clear();

	// Clear the material instance map
	m_MaterialInstanceMap.clear();

	//! Delete all 3DRep
	QHash<const QString, GLC_3DRep*>::iterator i3DRep= m_3DRepHash.begin();
	while (m_3DRepHash.constEnd() != i3DRep)
	{
		delete i3DRep.value();
		++i3DRep;
	}
	m_3DRepHash.clear();

	// Clear instance Hash table
	m_StructInstanceHash.clear();

	m_CurrentId.clear();

	m_ListOfAttachedFileName.clear();
}

// Load library_images element
void GLC_ColladaToWorld::loadLibraryImage()
{
	while (endElementNotReached(m_pStreamReader, "library_images"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "image") loadImage();
		}
		m_pStreamReader->readNext();

		updateProgressBar();
	}
	checkForXmlError("Error occur while loading element : library_images");
}

// Load image element
void GLC_ColladaToWorld::loadImage()
{
	//qDebug() << "GLC_ColladaToWorld::loadImage()";
	// load image id
	m_CurrentId= readAttribute("id", true);
	QString fileName;
	// Trying to find external image fileName
	while (endElementNotReached(m_pStreamReader, "image"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "init_from")
			{
				fileName= getContent("init_from");
			}
		}
		m_pStreamReader->readNext();
	}

	checkForXmlError("Error occur while loading element : image");

	// Add the image in the image fileName Hash table
	if (!fileName.isEmpty())
	{
		m_ImageFileHash.insert(m_CurrentId, fileName);
	}
}

// Load library_materials element
void GLC_ColladaToWorld::loadLibraryMaterials()
{
	while (endElementNotReached(m_pStreamReader, "library_materials"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "material") loadMaterial();
		}
		m_pStreamReader->readNext();

		updateProgressBar();
	}
	checkForXmlError("Error occur while loading element : library_materials");

}

// Load a material
void GLC_ColladaToWorld::loadMaterial()
{
	// load material id
	m_CurrentId= readAttribute("id", true);

	goToElement("instance_effect");

	// Load instance effect url
	const QString url= readAttribute("url", true).remove('#');
	//qDebug() << "instance effect URL : " << url;

	// Read instance effect parameters
	while (endElementNotReached(m_pStreamReader, "instance_effect"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "setparam")
			{
				qDebug() << "GLC_ColladaToWorld::loadMaterial : setparam found";
			}
		}
		m_pStreamReader->readNext();
	}

	checkForXmlError("Error occur while loading element : material");

	// Add the image in the image fileName Hash table
	if (!url.isEmpty())
	{
		//qDebug() << "insert material : " << m_CurrentId << " url: " << url;
		m_MaterialLibHash.insert(m_CurrentId, url);
	}

}

// Load library_effects element
void GLC_ColladaToWorld::loadLibraryEffects()
{
	while (endElementNotReached(m_pStreamReader, "library_effects"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "effect") loadEffect();
		}
		m_pStreamReader->readNext();

		updateProgressBar();
	}
	checkForXmlError("Error occur while loading element : library_effects");

}

// Load an effect
void GLC_ColladaToWorld::loadEffect()
{
	// load effect id
	const QString id= readAttribute("id", true);
	m_CurrentId= id;
	m_pCurrentMaterial= new GLC_Material();
	m_pCurrentMaterial->setName(id);

	while (endElementNotReached(m_pStreamReader, "effect"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "profile_COMMON") loadProfileCommon();
		}
		m_pStreamReader->readNext();
	}

	checkForXmlError("Error occur while loading element : effect");

	m_MaterialEffectHash.insert(id, m_pCurrentMaterial);
	m_pCurrentMaterial= NULL;

}

// Load profile_COMMON
void GLC_ColladaToWorld::loadProfileCommon()
{
	//qDebug() << "GLC_ColladaToWorld::loadProfileCommon";
	while (endElementNotReached(m_pStreamReader, "profile_COMMON"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "image") loadImage();
			else if (currentElementName == "newparam") loadNewParam();
			else if (currentElementName == "technique") loadTechnique();
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : profile_COMMON");
}

// Load a new param of the common profile
void GLC_ColladaToWorld::loadNewParam()
{
	//qDebug() << "GLC_ColladaToWorld::loadNewParam";
	// load param sid
	const QString sid= m_CurrentId + "::" + readAttribute("sid", true);
	while (endElementNotReached(m_pStreamReader, "newparam"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "surface") loadSurface(sid);
			else if (currentElementName == "sampler2D") loadSampler2D(sid);
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : profile_COMMON");
}

// Load a surface
void GLC_ColladaToWorld::loadSurface(const QString& sid)
{
	//qDebug() << "GLC_ColladaToWorld::loadSurface sid=" << sid ;
	while (endElementNotReached(m_pStreamReader, "surface"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "init_from")
			{
				const QString imageId= getContent("init_from");
				m_SurfaceImageHash.insert(sid, imageId);
			}
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : surface");
}

// Load Sampler 2D
void GLC_ColladaToWorld::loadSampler2D(const QString& sid)
{
	//qDebug() << "GLC_ColladaToWorld::loadSampler2D sid= " << sid;
	while (endElementNotReached(m_pStreamReader, "sampler2D"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "source")
			{
				const QString source= m_CurrentId + "::" + getContent("source");
				m_Sampler2DSurfaceHash.insert(sid, source);
			}
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : sampler2D");
}

// Load technique
void GLC_ColladaToWorld::loadTechnique()
{
	//qDebug() << "GLC_ColladaToWorld::loadTechnique";
	while (endElementNotReached(m_pStreamReader, "technique"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "phong") loadMaterialTechnique(currentElementName.toString());
			if (currentElementName == "lambert") loadMaterialTechnique(currentElementName.toString());
			if (currentElementName == "blinn") loadMaterialTechnique(currentElementName.toString());
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : technique");
}

// load phong material
void GLC_ColladaToWorld::loadMaterialTechnique(const QString& elementName)
{
	//qDebug() << "GLC_ColladaToWorld::loadMaterialTechnique";
	while (endElementNotReached(m_pStreamReader, elementName))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "emission")
					|| (currentElementName == "ambient")
					|| (currentElementName == "diffuse")
					||(currentElementName == "specular"))
				loadCommonColorOrTexture(currentElementName.toString());
			else if (currentElementName == "transparent") loadTransparent();
			else if (currentElementName == "transparency") loadTransparency(currentElementName.toString());
			else if (currentElementName == "shininess") loadShininess(currentElementName.toString());
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : " + elementName);
}

// load common color or texture
void GLC_ColladaToWorld::loadCommonColorOrTexture(const QString& name)
{
	//qDebug() << "GLC_ColladaToWorld::loadCommonColorOrTexture " << name;
	Q_ASSERT(NULL != m_pCurrentMaterial);

	while (endElementNotReached(m_pStreamReader, name))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "color")
			{
				if (name == "emission") m_pCurrentMaterial->setEmissiveColor(readXmlColor());
				else if (name == "ambient") m_pCurrentMaterial->setAmbientColor(readXmlColor());
				else if (name == "diffuse") m_pCurrentMaterial->setDiffuseColor(readXmlColor());
				else if (name == "specular") m_pCurrentMaterial->setSpecularColor(readXmlColor());
			}
			else if (currentElementName == "texture")
			{
				//qDebug() << "Load texture " << name;
				const QString sid = m_CurrentId + "::" + readAttribute("texture", true);
				m_TextureToMaterialHash.insert(sid, m_pCurrentMaterial);
			}
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : " + name);
}

// Load transparent
void GLC_ColladaToWorld::loadTransparent()
{
	const QString opaque= readAttribute("opaque", false);
	if (opaque == "RGB_ZERO") m_TransparentIsRgbZero= true;
	else m_TransparentIsRgbZero= false;
}

// Load transparency
void GLC_ColladaToWorld::loadTransparency(const QString& name)
{
	//qDebug() << "GLC_ColladaToWorld::loadTransparency";
	Q_ASSERT(NULL != m_pCurrentMaterial);
	while (endElementNotReached(m_pStreamReader, name))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "float")
			{
				bool stringToFloatOk= false;
				const QString alphaString= getContent("float");
				float alpha;
				if (m_TransparentIsRgbZero)
				{
					alpha= alphaString.toFloat(&stringToFloatOk);
				}
				else
				{
					alpha= 1.0f - alphaString.toFloat(&stringToFloatOk);
				}
				// A material mustn't be invisible (no sense)
				if (qFuzzyCompare(alpha, 0.0f)) alpha= 1.0f;

				m_pCurrentMaterial->setOpacity(alpha);
				if (!stringToFloatOk) throwException("Error while trying to convert :" + alphaString + " to float");
			}
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : " + name);
}

// Load shininess
void GLC_ColladaToWorld::loadShininess(const QString& name)
{
	//qDebug() << "GLC_ColladaToWorld::loadShininess";
	Q_ASSERT(NULL != m_pCurrentMaterial);
	while (endElementNotReached(m_pStreamReader, name))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "float")
			{
				bool stringToFloatOk= false;
				const QString shininessString= getContent("float");
				const float shininess= shininessString.toFloat(&stringToFloatOk);
				if (!stringToFloatOk)
				{
					QStringList stringList(m_FileName);
					stringList.append("Error while trying to convert :" + shininessString + " to float");
					GLC_ErrorLog::addError(stringList);
				}
				else m_pCurrentMaterial->setShininess(shininess);
			}
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : " + name);
}

// Read a xml Color
QColor GLC_ColladaToWorld::readXmlColor()
{
	//qDebug() << "GLC_ColladaToWorld::readXmlColor()";
	QColor resultColor;

	QString colorString= getContent("color");
	QStringList colors= colorString.split(' ');
	if(colors.size() == 4)
	{
		bool okRed, okGreen, okBlue, okAlpha;
		const float red= colors.at(0).toFloat(&okRed);
		const float green= colors.at(1).toFloat(&okGreen);
		const float blue= colors.at(2).toFloat(&okBlue);
		const float alpha= colors.at(3).toFloat(&okAlpha);
		if (okRed && okGreen && okBlue && okAlpha)
		{
			resultColor.setRedF(red);
			resultColor.setGreenF(green);
			resultColor.setBlueF(blue);
			resultColor.setAlphaF(alpha);
		}
		else
		{
			QString info= "Error occur while reading xml color : " + colorString;
			qDebug() << info << " " << m_FileName;
			GLC_FileFormatException fileFormatException(info, m_FileName, GLC_FileFormatException::WrongFileFormat);
			clear();
			throw(fileFormatException);
		}
	}
	else
	{
		QString info= "Error occur while reading xml color : " + colorString;
		qDebug() << info << " " << m_FileName;
		GLC_FileFormatException fileFormatException(info, m_FileName, GLC_FileFormatException::WrongFileFormat);
		clear();
		throw(fileFormatException);
	}

	return resultColor;
}

// Load library_geometries element
void GLC_ColladaToWorld::loadLibraryGeometries()
{
	while (endElementNotReached(m_pStreamReader, "library_geometries"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "geometry") loadGeometry();
		}

		m_pStreamReader->readNext();

		updateProgressBar();
	}
	checkForXmlError("Error occur while loading element : library_geometries");
}

// Load an geometry element
void GLC_ColladaToWorld::loadGeometry()
{
	delete m_pMeshInfo;
	m_pMeshInfo= new MeshInfo();
	m_pMeshInfo->m_pMesh= new GLC_Mesh;

	const QString id= readAttribute("id", false);
	m_CurrentId= id;
	if (!id.isEmpty())
	{
		m_pMeshInfo->m_pMesh->setName(id);
		//qDebug() << "Loading geometry : " << id;
	}
	else
	{
		qDebug() << "Geometry without id found !!";
	}

	while (endElementNotReached(m_pStreamReader, "geometry"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "mesh") loadMesh();
		}

		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : geometry");

	// Add the current mesh info to the geometry hash
	if (!id.isEmpty())
	{
		m_GeometryHash.insert(id, m_pMeshInfo);
		m_pMeshInfo= NULL;
	}
}

// Load a mesh
void GLC_ColladaToWorld::loadMesh()
{
	while (endElementNotReached(m_pStreamReader, "mesh"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "source") loadVertexBulkData();
			else if (currentElementName == "vertices") loadVertices();
			else if (currentElementName == "polylist") loadPolylist();
			else if (currentElementName == "polygons") loadPolygons();
			else if (currentElementName == "triangles") loadTriangles();
			//else if (currentElementName == "trifans") loadTriFans();
			//else if (currentElementName == "tristrips") loadTriStrip();
		}

		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : mesh");
}

// Load Vertex bulk data
void GLC_ColladaToWorld::loadVertexBulkData()
{
	//qDebug() << "GLC_ColladaToWorld::loadVertexBulkData()";
	// load Vertex Bulk data id
	m_CurrentId= readAttribute("id", true);
	//qDebug() << "id=" << m_CurrentId;
	QList<float> vertices;

	while (endElementNotReached(m_pStreamReader, "source"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "float_array"))
			{
				int count= readAttribute("count", true).toInt();
				QString array= getContent("float_array");
				QStringList list= array.split(' ');
				// Check the array size
				if (count != list.size()) throwException("float_array size not match");

				for (int i= 0; i < count; ++i)
				{
					vertices.append(list.at(i).toFloat());
				}
			}
			else if (currentElementName == "technique_common") loadTechniqueCommon();
		}

		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : source");
	m_BulkDataHash.insert(m_CurrentId, vertices);

	updateProgressBar();
}

void GLC_ColladaToWorld::loadTechniqueCommon()
{
	//qDebug() << "GLC_ColladaToWorld::loadTechniqueCommon()";

	while (endElementNotReached(m_pStreamReader, "technique_common"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "accessor")) loadAccessor();
		}

		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : technique_common");

}

void GLC_ColladaToWorld::loadAccessor()
{
	//qDebug() << "GLC_ColladaToWorld::loadAccessor()";
	Accessor accessor;
	const QString count= readAttribute("count", true);
	const QString offset= readAttribute("offset", false);
	const QString stride= readAttribute("stride", false);
	bool conversionOk;
	accessor.m_Count= count.toUInt(&conversionOk);
	if (conversionOk)
	{
		if (!offset.isEmpty())
		{
			accessor.m_Offset= offset.toUInt(&conversionOk);
		}
		if (!stride.isEmpty())
		{
			accessor.m_Stride= stride.toUInt(&conversionOk);
		}
	}

	while (endElementNotReached(m_pStreamReader, "accessor"))
	{
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : technique_common");

	m_DataAccessorHash.insert(m_CurrentId, accessor);
}

// Load attributes and identity of mesh vertices
void GLC_ColladaToWorld::loadVertices()
{
	//qDebug() << "GLC_ColladaToWorld::loadVertices()";
	// load Vertices id
	m_CurrentId= readAttribute("id", true);

	goToElement("input");
	const QString source= readAttribute("source", true).remove('#');
	m_VerticesSourceHash.insert(m_CurrentId, source);
	checkForXmlError("Error occur while loading element : vertices");
}

// Load polygons or polylist
void GLC_ColladaToWorld::loadPolylist()
{
	//qDebug() << "GLC_ColladaToWorld::loadPolylist()";
	// The number of polygon
	const int polygonCount= readAttribute("count", true).toInt();

	// The material id
	const QString materialId= readAttribute("material", false);

	// Offsets and data source list
	QList<InputData> inputDataList;

	// Polygon number of vertice list
	QList<int> vcountList;

	// Polygon index list
	QList<int> polyIndexList;

	while (endElementNotReached(m_pStreamReader, "polylist"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "input") && vcountList.isEmpty())
			{
				InputData currentInput;
				// Get input data offset
				currentInput.m_Offset= readAttribute("offset", true).toInt();
				// Get input data semantic
				const QString semantic= readAttribute("semantic", true);
				if (semantic == "VERTEX") currentInput.m_Semantic= VERTEX;
				else if (semantic == "NORMAL") currentInput.m_Semantic= NORMAL;
				else if (semantic == "TEXCOORD") currentInput.m_Semantic= TEXCOORD;
				else throwException("Source semantic :" + semantic + "Not supported");
				// Get input data source id
				currentInput.m_Source= readAttribute("source", true).remove('#');

				// Bypasss vertices indirection
				if (m_VerticesSourceHash.contains(currentInput.m_Source))
				{
					currentInput.m_Source= m_VerticesSourceHash.value(currentInput.m_Source);
				}
				inputDataList.append(currentInput);
			}
			else if ((currentElementName == "vcount") && (inputDataList.size() > 0))
			{
				QString vcountString= getContent("vcount");
				QStringList vcountStringList= vcountString.split(' ');
				if (vcountStringList.size() != polygonCount) throwException("vcount size not match");
				bool toIntOK;
				for (int i= 0; i < polygonCount; ++i)
				{
					vcountList.append(vcountStringList.at(i).toInt(&toIntOK));
					if (!toIntOK) throwException("Unable to convert string :" + vcountStringList.at(i) + " To int");
				}
			}
			else if ((currentElementName == "p") && !vcountList.isEmpty() && polyIndexList.isEmpty())
			{
				{ // Fill index List
					QString pString= getContent("p");
					QStringList pStringList= pString.split(' ');
					bool toIntOK;
					const int size= pStringList.size();
					for (int i= 0; i < size; ++i)
					{
						polyIndexList.append(pStringList.at(i).toInt(&toIntOK));
						if (!toIntOK) throwException("Unable to convert string :" + pStringList.at(i) + " To int");
					}
				}

			}
		}
		m_pStreamReader->readNext();
	}
	// Add the polylist to the current mesh
	addPolylistToCurrentMesh(inputDataList, vcountList, polyIndexList, materialId);

	updateProgressBar();
}

// Load Polygons
void GLC_ColladaToWorld::loadPolygons()
{
	// The material id
	const QString materialId= readAttribute("material", false);

	// Offsets and data source list
	QList<InputData> inputDataList;

	// Polygon number of vertice list
	QList<int> vcountList;

	// The input number
	int inputCount= 0;
	// Polygon index list
	QList<int> polyIndexList;
	while (endElementNotReached(m_pStreamReader, "polygons"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "input") && vcountList.isEmpty())
			{
				++inputCount;
				InputData currentInput;
				// Get input data offset
				currentInput.m_Offset= readAttribute("offset", true).toInt();
				// Get input data semantic
				const QString semantic= readAttribute("semantic", true);
				if (semantic == "VERTEX") currentInput.m_Semantic= VERTEX;
				else if (semantic == "NORMAL") currentInput.m_Semantic= NORMAL;
				else if (semantic == "TEXCOORD") currentInput.m_Semantic= TEXCOORD;
				else throwException("Source semantic :" + semantic + "Not supported");
				// Get input data source id
				currentInput.m_Source= readAttribute("source", true).remove('#');

				// Bypasss vertices indirection
				if (m_VerticesSourceHash.contains(currentInput.m_Source))
				{
					currentInput.m_Source= m_VerticesSourceHash.value(currentInput.m_Source);
				}
				inputDataList.append(currentInput);
			}
			else if (currentElementName == "p")
			{
				{ // Fill index List
					QString pString= getContent("p");
					QStringList pStringList= pString.split(' ');
					bool toIntOK;
					const int size= pStringList.size();
					for (int i= 0; i < size; ++i)
					{
						polyIndexList.append(pStringList.at(i).toInt(&toIntOK));
						if (!toIntOK) throwException("Unable to convert string :" + pStringList.at(i) + " To int");
					}
					// Add the polygon size in vcountList
					vcountList.append(size / inputCount);
				}
			}
		}
		m_pStreamReader->readNext();
	}
	// Add the polylist to the current mesh
	addPolylistToCurrentMesh(inputDataList, vcountList, polyIndexList, materialId);

	updateProgressBar();
}

// Add the polylist to the current mesh
void GLC_ColladaToWorld::addPolylistToCurrentMesh(const QList<InputData>& inputDataList, const QList<int>& vcountList, const QList<int>& polyIndexList, const QString& materialId)
{
	//qDebug() << "GLC_ColladaToWorld::addPolylistToCurrentMesh";

	const int polygonCount= vcountList.size();
	const int inputDataCount= inputDataList.size();
	const int polyIndexCount= polyIndexList.size();

	// Flag to know if the polylist has normal
	bool hasNormals= false;
	bool hasTexture= false;
	// Check the existance of data source
	for (int dataIndex= 0; dataIndex < inputDataCount; ++dataIndex)
	{
		const QString source= inputDataList.at(dataIndex).m_Source;
		if ( !m_BulkDataHash.contains(source))
		{
			throwException(" Source : " + source + " Not found");
		}
		if (inputDataList.at(dataIndex).m_Semantic == NORMAL) hasNormals= true;
		if (inputDataList.at(dataIndex).m_Semantic == TEXCOORD) hasTexture= true;
	}

	int maxOffset= 0;
	for (int i= 0; i < inputDataCount; ++i)
	{
		if (inputDataList.at(i).m_Offset > maxOffset)
		{
			maxOffset= inputDataList.at(i).m_Offset;
		}
	}
	//qDebug() << " Max Offset :" << maxOffset;

	// the polygonIndex of the polylist
	QList<int> polygonIndex;

	// Fill the mapping, bulk data and index list of the current mesh info
	for (int i= 0; i < polyIndexCount; i+= maxOffset + 1)
	{
		// Create and set the current vertice index
		ColladaVertice currentVertice;
		for (int dataIndex= 0; dataIndex < inputDataCount; ++dataIndex)
		{
			currentVertice.m_Values[inputDataList.at(dataIndex).m_Semantic]= polyIndexList.at(i + inputDataList.at(dataIndex).m_Offset);
		}

		if (m_pMeshInfo->m_Mapping.contains(currentVertice))
		{
			// Add the the index to the polygon index
			polygonIndex.append(m_pMeshInfo->m_Mapping.value(currentVertice));
		}
		else
		{
			// Add the current vertice to the current mesh info mapping hash table and increment the freeIndex
			m_pMeshInfo->m_Mapping.insert(currentVertice, (m_pMeshInfo->m_FreeIndex)++);
			// Add the the index to the polygon index
			polygonIndex.append(m_pMeshInfo->m_Mapping.value(currentVertice));

			// Add the bulk data associated to the current vertice to the current mesh info
			for (int dataIndex= 0; dataIndex < inputDataCount; ++dataIndex)
			{
				// The current input data
				InputData currentInputData= inputDataList.at(dataIndex);
				// QHash iterator on the right QList<float>
				BulkDataHash::const_iterator iBulkHash= m_BulkDataHash.find(currentInputData.m_Source);
				int stride;
				if (m_DataAccessorHash.contains(currentInputData.m_Source))
				{
					stride= m_DataAccessorHash.value(currentInputData.m_Source).m_Stride;
				}
				else if (currentInputData.m_Semantic != TEXCOORD) stride= 3; else stride= 2;
				// Firts value
				m_pMeshInfo->m_Datas[currentInputData.m_Semantic].append(iBulkHash.value().at(polyIndexList.at(i + currentInputData.m_Offset) * stride));
				// Second value
				m_pMeshInfo->m_Datas[currentInputData.m_Semantic].append(iBulkHash.value().at(polyIndexList.at(i + currentInputData.m_Offset) * stride + 1));
				// Fird value
				if (currentInputData.m_Semantic != TEXCOORD)
				{
					m_pMeshInfo->m_Datas[currentInputData.m_Semantic].append(iBulkHash.value().at(polyIndexList.at(i + currentInputData.m_Offset) * stride + 2));
				}
			}
			// Avoid problem wich occur with mesh containing materials with and without texture
			if (!hasTexture)
			{
				m_pMeshInfo->m_Datas[TEXCOORD].append(0.0);
			}

		}
	}

	// Save mesh info index offset
	const int indexOffset= m_pMeshInfo->m_Index.size();
	// Triangulate the polygons of the polylist
	// Input polygon index must start from 0 and succesive : (0 1 2 3 4)
	QList<GLuint> onePolygonIndex;
	for (int i= 0; i < polygonCount; ++i)
	{
		const int polygonSize= vcountList.at(i);
		Q_ASSERT(polygonSize > 2);
		for (int i= 0; i < polygonSize; ++i)
		{
			onePolygonIndex.append(polygonIndex.takeFirst());
		}
		// Triangulate the current polygon if the polygon as more than 3 vertice
		if (polygonSize > 3)
		{
			glc::triangulatePolygon(&onePolygonIndex, m_pMeshInfo->m_Datas.at(VERTEX));
		}
		// Add index to the mesh info
		//Q_ASSERT(not onePolygonIndex.isEmpty());
		if (!onePolygonIndex.isEmpty())
		{
			m_pMeshInfo->m_Index.append(onePolygonIndex);
		}
		else
		{
			QStringList stringList(m_FileName);
			stringList.append("Unable to triangulate a polygon of " + m_pMeshInfo->m_pMesh->name());
			GLC_ErrorLog::addError(stringList);
		}
		onePolygonIndex.clear();
	}

	// Check if normal computation is needed
	if (!hasNormals)
	{
		qDebug() << "Compute Normals with offset " << indexOffset;
		computeNormalOfCurrentPrimitiveOfCurrentMesh(indexOffset);
	}

	// Add material the current mesh info
	MatOffsetSize matInfo;
	matInfo.m_Offset= indexOffset;
	matInfo.m_size= m_pMeshInfo->m_Index.size() - indexOffset;
	m_pMeshInfo->m_Materials.insert(materialId, matInfo);

}
// Compute Normals for the current primitive element of the current mesh
void GLC_ColladaToWorld::computeNormalOfCurrentPrimitiveOfCurrentMesh(int indexOffset)
{
	const QList<float>* pData= &(m_pMeshInfo->m_Datas.at(VERTEX));
	// Fill the list of normal
	QList<float>* pNormal= &(m_pMeshInfo->m_Datas[NORMAL]);
	const int normalOffset= pNormal->size();
	const int normalCount= pData->size() - normalOffset;
	for (int i= 0; i < normalCount; ++i)
	{
		pNormal->append(0.0f);
	}
	// Compute the normals and add them to the current mesh info
	const int size= m_pMeshInfo->m_Index.size() - indexOffset;
	double xn, yn, zn;


	for (int i= indexOffset; i < size; i+=3)
	{
		// Vertex 1
		xn= pData->at(m_pMeshInfo->m_Index.at(i) * 3);
		yn= pData->at(m_pMeshInfo->m_Index.at(i) * 3 + 1);
		zn= pData->at(m_pMeshInfo->m_Index.at(i) * 3 + 2);
		const GLC_Vector3d vect1(xn, yn, zn);

		// Vertex 2
		xn= pData->at(m_pMeshInfo->m_Index.at(i + 1) * 3);
		yn= pData->at(m_pMeshInfo->m_Index.at(i + 1) * 3  + 1);
		zn= pData->at(m_pMeshInfo->m_Index.at(i + 1) * 3 + 2);
		const GLC_Vector3d vect2(xn, yn, zn);

		// Vertex 3
		xn= pData->at(m_pMeshInfo->m_Index.at(i + 2) * 3);
		yn= pData->at(m_pMeshInfo->m_Index.at(i + 2) * 3 + 1);
		zn= pData->at(m_pMeshInfo->m_Index.at(i + 2) * 3 + 2);
		const GLC_Vector3d vect3(xn, yn, zn);

		const GLC_Vector3d edge1(vect3 - vect2);
		const GLC_Vector3d edge2(vect1 - vect2);

		GLC_Vector3d normal(edge1 ^ edge2);
		normal.normalize();

		GLC_Vector3df curNormal= normal.toVector3df();
		for (int curVertex= 0; curVertex < 3; ++curVertex)
		{
			(*pNormal)[m_pMeshInfo->m_Index.at(i + curVertex) * 3]= curNormal.x();
			(*pNormal)[m_pMeshInfo->m_Index.at(i + curVertex) * 3 + 1]= curNormal.y();
			(*pNormal)[m_pMeshInfo->m_Index.at(i + curVertex) * 3 + 2]= curNormal.z();
		}
	}
}

// Load triangles
void  GLC_ColladaToWorld::loadTriangles()
{
	//qDebug() << "GLC_ColladaToWorld::loadTriangles()";
	// The material id
	const QString materialId= readAttribute("material", false);

	// Offsets and data source list
	QList<InputData> inputDataList;

	// triangle index list
	QList<int> trianglesIndexList;

	while (endElementNotReached(m_pStreamReader, "triangles"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "input") && trianglesIndexList.isEmpty())
			{
				InputData currentInput;
				// Get input data offset
				currentInput.m_Offset= readAttribute("offset", true).toInt();
				// Get input data semantic
				const QString semantic= readAttribute("semantic", true);
				if (semantic == "VERTEX") currentInput.m_Semantic= VERTEX;
				else if (semantic == "NORMAL") currentInput.m_Semantic= NORMAL;
				else if (semantic == "TEXCOORD") currentInput.m_Semantic= TEXCOORD;
				else throwException("Source semantic :" + semantic + "Not supported");
				// Get input data source id
				currentInput.m_Source= readAttribute("source", true).remove('#');

				// Bypasss vertices indirection
				if (m_VerticesSourceHash.contains(currentInput.m_Source))
				{
					currentInput.m_Source= m_VerticesSourceHash.value(currentInput.m_Source);
				}
				inputDataList.append(currentInput);
			}
			else if ((currentElementName == "p") && trianglesIndexList.isEmpty())
			{
				{ // Fill index List
					QString pString= getContent("p");
					QStringList pStringList= pString.split(' ');
					bool toIntOK;
					const int size= pStringList.size();
					for (int i= 0; i < size; ++i)
					{
						trianglesIndexList.append(pStringList.at(i).toInt(&toIntOK));
						if (!toIntOK) throwException("Unable to convert string :" + pStringList.at(i) + " To int");
					}
				}

			}
		}
		m_pStreamReader->readNext();
	}

	// Add the polylist to the current mesh
	addTrianglesToCurrentMesh(inputDataList, trianglesIndexList, materialId);

	updateProgressBar();

}

// Add the triangles to current mesh
void GLC_ColladaToWorld::addTrianglesToCurrentMesh(const QList<InputData>& inputDataList, const QList<int>& trianglesIndexList, const QString& materialId)
{
	//qDebug() << "GLC_ColladaToWorld::addTrianglesToCurrentMesh";

	const int inputDataCount= inputDataList.size();
	const int trianglesIndexCount= trianglesIndexList.size();

	// Flag to know if the polylist has normal
	bool hasNormals= false;
	bool hasTexture= false;

	// Check the existance of data source
	for (int dataIndex= 0; dataIndex < inputDataCount; ++dataIndex)
	{
		const QString source= inputDataList.at(dataIndex).m_Source;
		if ( !m_BulkDataHash.contains(source))
		{
			throwException(" Source : " + source + " Not found");
		}
		if (inputDataList.at(dataIndex).m_Semantic == NORMAL) hasNormals= true;
		if (inputDataList.at(dataIndex).m_Semantic == TEXCOORD) hasTexture= true;
	}

	int maxOffset= 0;
	for (int i= 0; i < inputDataCount; ++i)
	{
		if (inputDataList.at(i).m_Offset > maxOffset)
		{
			maxOffset= inputDataList.at(i).m_Offset;
		}
	}
	//qDebug() << " Triangles Max Offset :" << maxOffset;

	// the polygonIndex of the polylist
	QList<GLuint> trianglesIndex;

	// Fill the mapping, bulk data and index list of the current mesh info
	for (int i= 0; i < trianglesIndexCount; i+= maxOffset + 1)
	{
		// Create and set the current vertice index
		ColladaVertice currentVertice;
		for (int dataIndex= 0; dataIndex < inputDataCount; ++dataIndex)
		{
			currentVertice.m_Values[inputDataList.at(dataIndex).m_Semantic]= trianglesIndexList.at(i + inputDataList.at(dataIndex).m_Offset);
		}

		if (m_pMeshInfo->m_Mapping.contains(currentVertice))
		{
			// Add the the index to the triangles index
			trianglesIndex.append(m_pMeshInfo->m_Mapping.value(currentVertice));
		}
		else
		{
			// Add the current vertice to the current mesh info mapping hash table and increment the freeIndex
			m_pMeshInfo->m_Mapping.insert(currentVertice, (m_pMeshInfo->m_FreeIndex)++);
			// Add the the index to the triangles index
			trianglesIndex.append(m_pMeshInfo->m_Mapping.value(currentVertice));

			// Add the bulk data associated to the current vertice to the current mesh info
			for (int dataIndex= 0; dataIndex < inputDataCount; ++dataIndex)
			{
				// The current input data
				InputData currentInputData= inputDataList.at(dataIndex);
				// QHash iterator on the right QList<float>
				BulkDataHash::const_iterator iBulkHash= m_BulkDataHash.find(currentInputData.m_Source);
				int stride;
				if (m_DataAccessorHash.contains(currentInputData.m_Source))
				{
					stride= m_DataAccessorHash.value(currentInputData.m_Source).m_Stride;
				}
				else if (currentInputData.m_Semantic != TEXCOORD) stride= 3; else stride= 2;
				// Firts value
				m_pMeshInfo->m_Datas[currentInputData.m_Semantic].append(iBulkHash.value().at(trianglesIndexList.at(i + currentInputData.m_Offset) * stride));
				// Second value
				m_pMeshInfo->m_Datas[currentInputData.m_Semantic].append(iBulkHash.value().at(trianglesIndexList.at(i + currentInputData.m_Offset) * stride + 1));
				// Fird value
				if (currentInputData.m_Semantic != TEXCOORD)
				{
					m_pMeshInfo->m_Datas[currentInputData.m_Semantic].append(iBulkHash.value().at(trianglesIndexList.at(i + currentInputData.m_Offset) * stride + 2));
				}
				// Avoid problem wich occur with mesh containing materials with and without texture
				if (!hasTexture)
				{
					m_pMeshInfo->m_Datas[TEXCOORD].append(0.0);
				}
			}
		}
	}

	// Save mesh info index offset
	const int indexOffset= m_pMeshInfo->m_Index.size();

	// Add index to the mesh info
	m_pMeshInfo->m_Index.append(trianglesIndex);

	// Check if normal computation is needed
	if (!hasNormals)
	{
		computeNormalOfCurrentPrimitiveOfCurrentMesh(indexOffset);
	}

	// Add material the current mesh info
	MatOffsetSize matInfo;
	matInfo.m_Offset= indexOffset;
	matInfo.m_size= m_pMeshInfo->m_Index.size() - indexOffset;
	m_pMeshInfo->m_Materials.insertMulti(materialId, matInfo);

}

// Load the library nodes
void GLC_ColladaToWorld::loadLibraryNodes()
{
	//qDebug() << "GLC_ColladaToWorld::loadLibraryNodes";

	while (endElementNotReached(m_pStreamReader, "library_nodes"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "node"))
			{
				GLC_ColladaToWorld::ColladaNode* pNode= loadNode(NULL);
				if (NULL != pNode)
				{

				}
			}
		}

		updateProgressBar();

		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : library_nodes");
}

// Load the library controllers
void GLC_ColladaToWorld::loadLibraryContollers()
{
	//qDebug() << "GLC_ColladaToWorld::loadLibraryContollers";

	while (endElementNotReached(m_pStreamReader, "library_controllers"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "controller")) loadController();
		}

		updateProgressBar();

		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : library_controllers");
}

// Load library_visual_scenes element
void GLC_ColladaToWorld::loadVisualScenes()
{
	//qDebug() << "GLC_ColladaToWorld::loadVisualScenes";
	// The element library visual scene must contains a visual scene element
	goToElement("visual_scene");

	while (endElementNotReached(m_pStreamReader, "visual_scene"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if (currentElementName == "node")
			{
				GLC_ColladaToWorld::ColladaNode* pNode= loadNode(NULL);
				if (NULL != pNode)
				{
					m_TopLevelColladaNode.append(pNode);
				}
			}
		}

		updateProgressBar();

		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : visual_scene");
}

// Load an instance geometry
void GLC_ColladaToWorld::loadInstanceGeometry(ColladaNode* pNode)
{
	//qDebug() << "GLC_ColladaToWorld::loadInstanceGeometry";

	const QString url= readAttribute("url", true).remove('#');
	pNode->m_InstanceGeometryIDs.append(url);

	while (endElementNotReached(m_pStreamReader, "instance_geometry"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "instance_material"))
			{
				const QString target= readAttribute("target", true).remove('#');
				const QString symbol= readAttribute("symbol", true);
				m_MaterialInstanceMap.insert(symbol, target);
			}

		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : instance_geometry");
}

// Load an instance of node
void GLC_ColladaToWorld::loadInstanceNode(ColladaNode* pNode)
{
	//qDebug() << "GLC_ColladaToWorld::loadInstanceNode";
	const QString url= readAttribute("url", true).remove('#');
	pNode->m_InstanceOffNodeIds.append(url);
}

// Load an instance Controller
void GLC_ColladaToWorld::loadInstanceController(ColladaNode* pNode)
{
	const QString url= readAttribute("url", true).remove('#');
	pNode->m_InstanceOffNodeIds.append(url);

	while (endElementNotReached(m_pStreamReader, "instance_controller"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "instance_material"))
			{
				const QString target= readAttribute("target", true).remove('#');
				const QString symbol= readAttribute("symbol", true);
				m_MaterialInstanceMap.insert(symbol, target);
			}
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : instance_controller");
}

// Load a collada controller node
void GLC_ColladaToWorld::loadController()
{

	m_CurrentId= readAttribute("id", true);
	while (endElementNotReached(m_pStreamReader, "controller"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();
			if ((currentElementName == "skin"))
			{
				const QString source= readAttribute("source", true).remove('#');
				ColladaNode* pNode= new ColladaNode(m_CurrentId, NULL);
				pNode->m_InstanceGeometryIDs.append(source);
				m_ColladaNodeHash.insert(m_CurrentId, pNode);
			}
		}
		m_pStreamReader->readNext();
	}
	checkForXmlError("Error occur while loading element : controller");

}

// Load a Collada Node element
GLC_ColladaToWorld::ColladaNode* GLC_ColladaToWorld::loadNode(ColladaNode* pParent)
{
	//qDebug() << "GLC_ColladaToWorld::loadNode";


	QString id= readAttribute("id", false);
	if (id.isEmpty())
	{
		id= readAttribute("name", false);
	}
	if (id.isEmpty())
	{
		id= prefixNodeId + QString::number(++currentNodeId);
	}

	qint64 currentOffset= m_pStreamReader->characterOffset();
	//qDebug() << "Load Node " << id;
	m_CurrentId= id;
	// The node
	ColladaNode* pNode= new ColladaNode(id, pParent);
	// To avoid infinite call
	//m_pStreamReader->readNext();

	while (endElementNotReached(m_pStreamReader, "node"))
	{
		if (QXmlStreamReader::StartElement == m_pStreamReader->tokenType())
		{
			const QStringRef currentElementName= m_pStreamReader->name();

			if ((currentElementName == "translate")) translateNode(pNode);
			else if ((currentElementName == "scale")) scaleNode(pNode);
			else if ((currentElementName == "rotate")) rotateNode(pNode);
			else if ((currentElementName == "matrix")) composeMatrixNode(pNode);
			else if ((currentElementName == "instance_geometry")) loadInstanceGeometry(pNode);
			else if ((currentElementName == "instance_node")) loadInstanceNode(pNode);
			else if ((currentElementName == "instance_controller")) loadInstanceController(pNode);
			else if ((currentElementName == "node"))
			{
				if (currentOffset != m_pStreamReader->characterOffset())
				{
					QString newId= readAttribute("id", false);
					if (newId.isEmpty())
					{
						//qDebug() << "Child ReadAttribute name";
						newId= readAttribute("name", false);
					}
					//qDebug() << "New ID = " << newId;
					GLC_ColladaToWorld::ColladaNode* pChildNode= loadNode(pNode);
					if (NULL != pNode)
					{
						//qDebug() << "Add child";
						pNode->m_ChildNodes.append(pChildNode);
					}
				}
			}
			else if ((currentElementName == "instance_camera")
					|| (currentElementName == "instance_light"))
			{
				// Node type not supported
				delete pNode;
				pNode= NULL;
			}
		}
		m_pStreamReader->readNext();

	}

	if (NULL != pNode)
	{
		// Add the collada node to the collada node hash table
		m_ColladaNodeHash.insert(id, pNode);
	}
	return pNode;
}


// Translate the node
void GLC_ColladaToWorld::translateNode(ColladaNode* pNode)
{
	//qDebug() << "Translate Node";
	Q_ASSERT(NULL != pNode);
	// Load translation values
	QStringList translateStringList= getContent("translate").simplified().split(' ');
	// A translation must contains 3 string
	const int size= translateStringList.size();
	if (translateStringList.size() != 3) throwException("Translate element must contains 3 floats and it's contains :" + QString::number(translateStringList.size()));
	// Convert the string to double
	double translate[3];
	bool toFloatOk= false;
	for (int i= 0; i < size; ++i)
	{
		translate[i]= static_cast<double>(translateStringList.at(i).toFloat(&toFloatOk));
		if (!toFloatOk) throwException("The number :" + translateStringList.at(i) + "Is not a float");
	}
	// Built the translation matrix
	GLC_Matrix4x4 translationMatrix(translate[0], translate[1], translate[2]);
	// Update the node matrix
	pNode->m_Matrix= pNode->m_Matrix * translationMatrix;
}

// Scale the node
void GLC_ColladaToWorld::scaleNode(ColladaNode* pNode)
{
	//qDebug() << "Scale Node";
	Q_ASSERT(NULL != pNode);
	// Load scale values
	QStringList scaleStringList= getContent("scale").simplified().split(' ');
	// A scale must contains 3 string
	const int size= scaleStringList.size();
	if (scaleStringList.size() != 3) throwException("Scale element must contains 3 floats and it's contains :" + QString::number(scaleStringList.size()));
	// Convert the string to double
	double scale[3];
	bool toFloatOk= false;
	for (int i= 0; i < size; ++i)
	{
		scale[i]= static_cast<double>(scaleStringList.at(i).toFloat(&toFloatOk));
		if (!toFloatOk) throwException("The number :" + scaleStringList.at(i) + "Is not a float");
	}
	// Built the translation matrix
	GLC_Matrix4x4 scaleMatrix;
	scaleMatrix.setMatScaling(scale[0], scale[1], scale[2]);
	scaleMatrix.optimise();
	// Update the node matrix
	pNode->m_Matrix= pNode->m_Matrix * scaleMatrix;
}

// Rotate the node
void GLC_ColladaToWorld::rotateNode(ColladaNode* pNode)
{
	//qDebug() << "Rotate Node";
	Q_ASSERT(NULL != pNode);
	// Load rotate values
	QStringList rotateStringList= getContent("rotate").simplified().split(' ');
	// A rotate must contains 4 string (Axis Vector 3 + Angle)
	const int size= rotateStringList.size();
	if (rotateStringList.size() != 4) throwException("Rotate element must contains 4 floats and it's contains :" + QString::number(rotateStringList.size()));
	// Convert the string to double
	double rotate[4];
	bool toFloatOk= false;
	for (int i= 0; i < size; ++i)
	{
		rotate[i]= static_cast<double>(rotateStringList.at(i).toFloat(&toFloatOk));
		if (!toFloatOk) throwException("The number :" + rotateStringList.at(i) + "Is not a float");
	}
	// Rotation vector
	GLC_Vector3d rotationAxis(rotate[0], rotate[1], rotate[2]);
	// Built the rotation matrix
	GLC_Matrix4x4 rotationMatrix(rotationAxis, rotate[3]);
	// Update the node matrix
	pNode->m_Matrix= pNode->m_Matrix * rotationMatrix;
}

// Compose Node matrix
void GLC_ColladaToWorld::composeMatrixNode(ColladaNode* pNode)
{
	Q_ASSERT(NULL != pNode);

	// Load matrix values
	QStringList matrixStringList= getContent("matrix").simplified().split(' ');
	// A rotate must contains 16 string 4 x 4 Matrix
	const int size= matrixStringList.size();
	if (size != 16) throwException("Matrix element must contains 16 floats and it's contains :" + QString::number(size));
	// Convert the string to double
	double matrix[16];
	bool toFloatOk= false;
	for (int i= 0; i < 4; ++i)
	{
		matrix[i]= static_cast<double>(matrixStringList.at(i * 4).toFloat(&toFloatOk));
		if (!toFloatOk) throwException("The number :" + matrixStringList.at(i) + "Is not a float");

		matrix[i + 4]= static_cast<double>(matrixStringList.at(i * 4 + 1).toFloat(&toFloatOk));
		if (!toFloatOk) throwException("The number :" + matrixStringList.at(i * 4 + 1) + "Is not a float");

		matrix[i + 8]= static_cast<double>(matrixStringList.at(i * 4 + 2).toFloat(&toFloatOk));
		if (!toFloatOk) throwException("The number :" + matrixStringList.at(i * 4 + 2) + "Is not a float");

		matrix[i + 12]= static_cast<double>(matrixStringList.at(i * 4 + 3).toFloat(&toFloatOk));
		if (!toFloatOk) throwException("The number :" + matrixStringList.at(i * 4 + 3) + "Is not a float");

	}
	// Built the matrix
	GLC_Matrix4x4 currentMatrix(matrix);
	currentMatrix.optimise();

	// Update the node matrix
	pNode->m_Matrix= pNode->m_Matrix * currentMatrix;
}

// Load scene element
void GLC_ColladaToWorld::loadScene()
{
	//qDebug() << "GLC_ColladaToWorld::loadScene";
	while (endElementNotReached(m_pStreamReader, "scene"))
	{
		// Nothing to do
		m_pStreamReader->readNext();
	}
}

// Link texture to material
void GLC_ColladaToWorld::linkTexturesToMaterials()
{
	// Iterate throuth the the texture id to material hash
	MaterialHash::iterator iMat= m_TextureToMaterialHash.begin();
	while (iMat != m_TextureToMaterialHash.constEnd())
	{
		GLC_Material* pCurrentMaterial= iMat.value();
		const QString textureId= iMat.key();

		// Check that the texture is present
		if (m_Sampler2DSurfaceHash.contains(textureId))
		{
			const QString surfaceId= m_Sampler2DSurfaceHash.value(textureId);
			if (m_SurfaceImageHash.contains(surfaceId))
			{
				const QString imageFileId=  m_SurfaceImageHash.value(surfaceId);
			}
		}
		if (m_Sampler2DSurfaceHash.contains(textureId) && m_SurfaceImageHash.contains(m_Sampler2DSurfaceHash.value(textureId))
				&& m_ImageFileHash.contains(m_SurfaceImageHash.value(m_Sampler2DSurfaceHash.value(textureId))))
		{
			const QString imageFileName= m_ImageFileHash.value(m_SurfaceImageHash.value(m_Sampler2DSurfaceHash.value(textureId)));
			QString fullImageFileName= QFileInfo(m_FileName).absolutePath() + QDir::separator() + imageFileName;
			if (QFileInfo(fullImageFileName).exists())
			{
				m_ListOfAttachedFileName << fullImageFileName;
				GLC_Texture* pTexture= new GLC_Texture(fullImageFileName);
				pCurrentMaterial->setTexture(pTexture);
			}
			else if (QFileInfo(m_FileName).absolutePath() != QFileInfo(fullImageFileName).absolutePath())
			{
				// Trying to find image in collada file path
				QString fullImageFileName= QFileInfo(m_FileName).absolutePath() + QDir::separator() + QFileInfo(imageFileName).fileName();
				if (QFileInfo(fullImageFileName).exists())
				{
					m_ListOfAttachedFileName << fullImageFileName;
					GLC_Texture* pTexture= new GLC_Texture(fullImageFileName);
					pCurrentMaterial->setTexture(pTexture);
				}
				else
				{
					QStringList stringList(m_FileName);
					stringList.append(imageFileName + " Not found");
					GLC_ErrorLog::addError(stringList);
				}
			}
			else
			{
				QStringList stringList(m_FileName);
				stringList.append(imageFileName + " Not found");
				GLC_ErrorLog::addError(stringList);
			}

		}
		else
		{
			QStringList stringList(m_FileName);
			stringList.append("Texture : " + textureId + " Not found");
			GLC_ErrorLog::addError(stringList);
		}
		++iMat;
	}
}
// Create mesh and link them to material
void GLC_ColladaToWorld::createMesh()
{
	//qDebug() << "GLC_ColladaToWorld::createMesh()";
	QHash<const QString, MeshInfo*>::iterator iMeshInfo= m_GeometryHash.begin();
	while (m_GeometryHash.constEnd() != iMeshInfo)
	{
		MeshInfo* pCurrentMeshInfo= iMeshInfo.value();
		// Add Bulk Data to the mesh
		// Add vertice
		pCurrentMeshInfo->m_pMesh->addVertice(pCurrentMeshInfo->m_Datas.at(VERTEX).toVector());
		//qDebug() << "Add " << pCurrentMeshInfo->m_Datas[VERTEX].size() << " Vertice";
		pCurrentMeshInfo->m_Datas[VERTEX].clear();

		// Add Normal
		pCurrentMeshInfo->m_pMesh->addNormals(pCurrentMeshInfo->m_Datas.at(NORMAL).toVector());
		//qDebug() << "Add " << pCurrentMeshInfo->m_Datas[NORMAL].size() << " Normal";
		pCurrentMeshInfo->m_Datas[NORMAL].clear();

		// Add texel if necessary
		//qDebug() << "Add " << pCurrentMeshInfo->m_Datas[TEXCOORD].size() << " texel";
		if (!pCurrentMeshInfo->m_Datas.at(TEXCOORD).isEmpty())
		{
			pCurrentMeshInfo->m_pMesh->addTexels(pCurrentMeshInfo->m_Datas.at(TEXCOORD).toVector());
			pCurrentMeshInfo->m_Datas[TEXCOORD].clear();
		}

		// Add face index and material to the mesh
		QHash<QString, MatOffsetSize>::iterator iMatInfo= pCurrentMeshInfo->m_Materials.begin();
		while (pCurrentMeshInfo->m_Materials.constEnd() != iMatInfo)
		{
			// Trying to find the material
			QString materialId= iMatInfo.key();
			GLC_Material* pCurrentMaterial= NULL;
			if (m_MaterialInstanceMap.contains(materialId))
			{
				//qDebug() << "Map " << materialId << " to " << m_MaterialInstanceMap.value(materialId);
				materialId= m_MaterialInstanceMap.value(materialId);
			}
			//qDebug() << "MaterialLibHash size : " << m_MaterialLibHash.size();
			if (m_MaterialLibHash.contains(materialId))
			{
				materialId= m_MaterialLibHash.value(materialId);
				//qDebug() << "Material id " << materialId;
			}
			if (m_MaterialEffectHash.contains(materialId))
			{
				//qDebug() << "Material " << materialId << " find";
				pCurrentMaterial= m_MaterialEffectHash.value(materialId);
				Q_ASSERT(NULL != pCurrentMaterial);
			}
			else
			{
				QStringList stringList(m_FileName);
				stringList.append("Material " + materialId + " Not found");
				GLC_ErrorLog::addError(stringList);
			}

			// Create the list of triangles to add to the mesh
			const int offset= iMatInfo.value().m_Offset;
			const int size= iMatInfo.value().m_size;
			//qDebug() << "Offset : " << offset << " size : " << size;
			QList<GLuint> triangles;
			for (int i= offset; i < (offset + size); ++i)
			{
				triangles.append(pCurrentMeshInfo->m_Index.at(i));
			}
			//qDebug() << "Add " << triangles.size() << " elment to the triangle index";
			// Add the list of triangle to the mesh
			if (!triangles.isEmpty())
			{
				pCurrentMeshInfo->m_pMesh->addTriangles(pCurrentMaterial, triangles);
			}

			++iMatInfo;
		}
		pCurrentMeshInfo->m_pMesh->finish();
		GLC_3DRep* pRep= new GLC_3DRep(pCurrentMeshInfo->m_pMesh);
		pCurrentMeshInfo->m_pMesh= NULL;
		pRep->clean();
		//qDebug() << "Insert Rep : " << iMeshInfo.key();
		m_3DRepHash.insert(iMeshInfo.key(), pRep);
		++iMeshInfo;
	}
}

// Create the scene graph struct
void GLC_ColladaToWorld::createSceneGraph()
{
	const int topLevelNodeCount= m_TopLevelColladaNode.size();
	for (int i= 0; i < topLevelNodeCount; ++i)
	{
		ColladaNode* pCurrentColladaNode= m_TopLevelColladaNode.at(i);
		//qDebug() << "Top level node is : " << pCurrentColladaNode->m_Id;
		if (NULL != pCurrentColladaNode)
		{
			GLC_StructOccurence* pOccurence= createOccurenceFromNode(pCurrentColladaNode);
			m_pWorld->rootOccurence()->addChild(pOccurence);
		}
	}

	// Update position
	m_pWorld->rootOccurence()->removeEmptyChildren();
	m_pWorld->rootOccurence()->updateChildrenAbsoluteMatrix();

}

// Create Occurence tree from node tree
GLC_StructOccurence* GLC_ColladaToWorld::createOccurenceFromNode(ColladaNode* pNode)
{
	//qDebug() << "GLC_ColladaToWorld::createOccurenceFromNode";
	Q_ASSERT(NULL != pNode);
	GLC_StructInstance* pInstance= NULL;
	GLC_StructOccurence* pOccurence= NULL;
	if (!pNode->m_InstanceGeometryIDs.isEmpty())
	{
		if (m_StructInstanceHash.contains(pNode->m_Id))
		{
			pInstance= new GLC_StructInstance(m_StructInstanceHash.value(pNode->m_Id));
			pInstance->move(pNode->m_Matrix);
			//qDebug() << "Instance move with this matrix :" << pNode->m_Matrix.toString();
			pOccurence= new GLC_StructOccurence(pInstance);
		}
		else
		{
			const int size= pNode->m_InstanceGeometryIDs.size();
			GLC_3DRep* pRep= NULL;
			for (int i= 0; i < size; ++i)
			{
				const QString geometryId= pNode->m_InstanceGeometryIDs.at(i);
				if (NULL == pRep)
				{
					pRep= new GLC_3DRep(*(m_3DRepHash.value(geometryId)));
				}
				else
				{
					pRep->merge(m_3DRepHash.value(geometryId));
				}
			}
			if (NULL != pRep)
			{
				GLC_StructReference* pStructRef= NULL;
				if (pRep->isEmpty())
				{
					QStringList stringList(m_FileName);
					stringList.append("Empty rep : " + pRep->name());
					GLC_ErrorLog::addError(stringList);
					delete pRep;
					pRep= NULL;
				}
				else
				{
					pStructRef= new GLC_StructReference(pRep);
					pInstance= new GLC_StructInstance(pStructRef);

					// Save instance in instance hash Table
					m_StructInstanceHash.insert(pNode->m_Id, pInstance);

					pInstance->move(pNode->m_Matrix);
					//qDebug() << "Instance move with this matrix :" << pNode->m_Matrix.toString();
					pOccurence= new GLC_StructOccurence(pInstance);
				}

			}
			else
			{
				QStringList stringList(m_FileName);
				stringList.append("Geometry Id Not found");
				GLC_ErrorLog::addError(stringList);
			}
		}
	}
	if (!pNode->m_ChildNodes.isEmpty())
	{
		if (NULL == pOccurence) //  The node hasn't geometry -> Create an occurence
		{
			if (m_StructInstanceHash.contains(pNode->m_Id))
			{
				pInstance= new GLC_StructInstance(m_StructInstanceHash.value(pNode->m_Id));
			}
			else
			{
				GLC_StructReference* pStructRef= new GLC_StructReference(pNode->m_Id);
				pInstance= new GLC_StructInstance(pStructRef);
			}

			pInstance->move(pNode->m_Matrix);
			pOccurence= new GLC_StructOccurence(pInstance);
		}

		const int size= pNode->m_ChildNodes.size();
		for (int i= 0; i < size; ++i)
		{
			if (NULL != pNode->m_ChildNodes.at(i))
			{
				pOccurence->addChild(createOccurenceFromNode(pNode->m_ChildNodes.at(i)));
			}
		}
	}
	if (!pNode->m_InstanceOffNodeIds.isEmpty())
	{
		if (NULL == pOccurence) //  The node hasn't geometry and childs -> Create an occurence
		{
			if (m_StructInstanceHash.contains(pNode->m_Id))
			{
				pInstance= new GLC_StructInstance(m_StructInstanceHash.value(pNode->m_Id));
			}
			else
			{
				GLC_StructReference* pStructRef= new GLC_StructReference(pNode->m_Id);
				pInstance= new GLC_StructInstance(pStructRef);
			}

			pInstance->move(pNode->m_Matrix);
			pOccurence= new GLC_StructOccurence(pInstance);
		}

		const int size= pNode->m_InstanceOffNodeIds.size();
		for (int i= 0; i < size; ++i)
		{
			if (m_ColladaNodeHash.contains(pNode->m_InstanceOffNodeIds.at(i)))
			{
				pOccurence->addChild(createOccurenceFromNode(m_ColladaNodeHash.value(pNode->m_InstanceOffNodeIds.at(i))));
			}
			else
			{
				const QString errorMsg= "Instance Node : " + pNode->m_InstanceOffNodeIds.at(i) + "Not Found";
				throwException(errorMsg);
			}
		}
	}
	if (NULL == pOccurence)
	{
		if (m_StructInstanceHash.contains(pNode->m_Id))
		{
			pInstance= new GLC_StructInstance(m_StructInstanceHash.value(pNode->m_Id));
		}
		else
		{
			GLC_StructReference* pStructRef= new GLC_StructReference(pNode->m_Id);
			pInstance= new GLC_StructInstance(pStructRef);
		}

		pInstance->move(pNode->m_Matrix);
		pOccurence= new GLC_StructOccurence(pInstance);
	}

	return pOccurence;
}

// Update progress bar
void GLC_ColladaToWorld::updateProgressBar()
{
	qint64 currentOffset= m_pStreamReader->characterOffset();

	int currentQuantumValue;
	// Progrees bar indicator
	currentQuantumValue = static_cast<int>((static_cast<double>(currentOffset) / m_FileSize) * 100);
	if (currentQuantumValue > m_CurrentOffset)
	{
		emit currentQuantum(currentQuantumValue);
		m_CurrentOffset= currentQuantumValue;
	}

}
