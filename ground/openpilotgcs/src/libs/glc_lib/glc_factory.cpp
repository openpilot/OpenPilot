/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Copyright (C) 2011 JŽr™me Forrissier
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

//! \file glc_factory.cpp implementation of the GLC_Factory class.

#include "glc_factory.h"
#include "io/glc_fileloader.h"
#include "io/glc_3dxmltoworld.h"
#include "io/glc_worldreaderplugin.h"

#include "viewport/glc_panmover.h"
#include "viewport/glc_zoommover.h"
#include "viewport/glc_settargetmover.h"
#include "viewport/glc_trackballmover.h"
#include "viewport/glc_turntablemover.h"
#include "viewport/glc_repcrossmover.h"
#include "viewport/glc_reptrackballmover.h"
#include "viewport/glc_flymover.h"
#include "viewport/glc_repflymover.h"
#include "viewport/glc_tsrmover.h"
#include "maths/glc_line3d.h"
#include "maths/glc_geomtools.h"

#include "glc_fileformatexception.h"

// init static member
GLC_Factory* GLC_Factory::m_pFactory= NULL;
QList<GLC_WorldReaderPlugin*> GLC_Factory::m_WorldReaderPluginList;
QSet<QString> GLC_Factory::m_SupportedExtensionSet;

//////////////////////////////////////////////////////////////////////
// static method
//////////////////////////////////////////////////////////////////////
// Return the unique instance of the factory
GLC_Factory* GLC_Factory::instance()
{
	if(m_pFactory == NULL)
	{
		m_pFactory= new GLC_Factory();
	}
	return m_pFactory;
}

//////////////////////////////////////////////////////////////////////
// Constructor destructor
//////////////////////////////////////////////////////////////////////

// Protected constructor
GLC_Factory::GLC_Factory()
{
	loadPlugins();
}

// Destructor
GLC_Factory::~GLC_Factory()
{

}

//////////////////////////////////////////////////////////////////////
// Create functions
//////////////////////////////////////////////////////////////////////

GLC_3DRep GLC_Factory::createPoint(const GLC_Point3d &coord) const
{
	GLC_3DRep newPoint(new GLC_Point(coord));
	return newPoint;
}

GLC_3DRep GLC_Factory::createPoint(double x, double y, double z) const
{
	GLC_3DRep newPoint(new GLC_Point(x, y, z));
	return newPoint;
}

GLC_3DRep GLC_Factory::createPointCloud(const GLfloatVector& data, const QColor& color)
{
	GLC_PointCloud* pPointCloud= new GLC_PointCloud();
	pPointCloud->addPoint(data);
	pPointCloud->setWireColor(color);
	return GLC_3DRep(pPointCloud);
}

GLC_3DRep GLC_Factory::createPointCloud(const QList<GLC_Point3d>& pointList, const QColor& color)
{
	GLC_PointCloud* pPointCloud= new GLC_PointCloud();
	pPointCloud->addPoint(pointList);
	pPointCloud->setWireColor(color);
	return GLC_3DRep(pPointCloud);
}

GLC_3DRep GLC_Factory::createPointCloud(const QList<GLC_Point3df>& pointList, const QColor& color)
{
	GLC_PointCloud* pPointCloud= new GLC_PointCloud();
	pPointCloud->addPoint(pointList);
	pPointCloud->setWireColor(color);
	return GLC_3DRep(pPointCloud);
}


GLC_3DRep GLC_Factory::createPointSprite(float size, GLC_Material* pMaterial) const
{
	GLC_3DRep newPoint(new GLC_PointSprite(size, pMaterial));
	return newPoint;
}

GLC_3DRep GLC_Factory::createLine(const GLC_Point3d& point1, const GLC_Point3d& point2) const
{
	GLC_3DRep newPoint(new GLC_Line(point1, point2));
	return newPoint;
}

GLC_3DRep GLC_Factory::createCircle(double radius, double angle) const
{
	GLC_3DRep newCircle(new GLC_Circle(radius, angle));
	return newCircle;
}

GLC_3DRep GLC_Factory::createBox(double lx, double ly, double lz) const
{

	GLC_3DRep newBox(new GLC_Box(lx, ly, lz));
	return newBox;
}

GLC_3DViewInstance GLC_Factory::createBox(const GLC_BoundingBox& boundingBox) const
{
	const double lx= boundingBox.upperCorner().x() - boundingBox.lowerCorner().x();
	const double ly= boundingBox.upperCorner().y() - boundingBox.lowerCorner().y();
	const double lz= boundingBox.upperCorner().z() - boundingBox.lowerCorner().z();
	GLC_Box* pBox= new GLC_Box(lx, ly, lz);
	GLC_3DViewInstance newBox(pBox);
	newBox.translate(boundingBox.center().x(), boundingBox.center().y()
					, boundingBox.center().z());
	return newBox;
}

GLC_3DRep GLC_Factory::createCylinder(double radius, double length) const
{

	GLC_3DRep newCylinder(new GLC_Cylinder(radius, length));
	return newCylinder;
}

GLC_3DRep GLC_Factory::createCone(double radius, double length) const
{
	GLC_3DRep newCone(new GLC_Cone(radius, length));
	return newCone;
}

GLC_3DRep GLC_Factory::createSphere(double radius) const
{
	GLC_3DRep newSphere(new GLC_Sphere(radius));
	return newSphere;
}

GLC_3DRep GLC_Factory::createRectangle(double l1, double l2)
{
	GLC_3DRep newRectangle(new GLC_Rectangle(l1, l2));
	return newRectangle;
}

GLC_3DViewInstance GLC_Factory::createRectangle(const GLC_Point3d& point, const GLC_Vector3d& normal, double l1, double l2)
{
	// Create the rectangle to (0,0) and  z normal
	GLC_3DViewInstance rectangleInstance(createRectangle(l1, l2));

	// Create the plane rotation matrix
	const GLC_Matrix4x4 rotationMatrix(glc::Z_AXIS, normal);
	// Vector from origin to the plane
	rectangleInstance.setMatrix(GLC_Matrix4x4(point) * rotationMatrix);

	return rectangleInstance;
}

GLC_3DViewInstance GLC_Factory::createCuttingPlane(const GLC_Point3d& point, const GLC_Vector3d& normal, double l1, double l2, GLC_Material* pMat)
{
	// Create the rectangle to (0,0) and  z normal
	GLC_Rectangle* pRectangle= new GLC_Rectangle(l1, l2);
	pRectangle->replaceMasterMaterial(pMat);

	GLC_3DViewInstance rectangleInstance(pRectangle);

	// Create the plane rotation matrix
	const GLC_Matrix4x4 rotationMatrix(glc::Z_AXIS, normal);
	// Vector from origin to the plane
	rectangleInstance.setMatrix(GLC_Matrix4x4(point) * rotationMatrix);

	return rectangleInstance;

}

GLC_World GLC_Factory::createWorldFromFile(QFile &file, QStringList* pAttachedFileName) const
{
	GLC_FileLoader* pLoader = createFileLoader();
	connect(pLoader, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
	GLC_World world = pLoader->createWorldFromFile(file, pAttachedFileName);
	delete pLoader;

	return world;
}

GLC_World GLC_Factory::createWorldStructureFrom3dxml(QFile &file, bool GetExtRefName) const
{
	GLC_World* pWorld= NULL;

	if (QFileInfo(file).suffix().toLower() == "3dxml")
	{
		GLC_3dxmlToWorld d3dxmlToWorld;
		connect(&d3dxmlToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		pWorld= d3dxmlToWorld.createWorldFrom3dxml(file, true, GetExtRefName);
	}

	if (NULL == pWorld)
	{
		// File extension not recognize or file not loaded
		QString message(QString("GLC_Factory::createWorldStructureFrom3dxml File ") + file.fileName() + QString(" not loaded"));
		GLC_FileFormatException fileFormatException(message, file.fileName(), GLC_FileFormatException::FileNotSupported);
		throw(fileFormatException);
	}
	GLC_World resulWorld(*pWorld);
	delete pWorld;

	return resulWorld;
}

GLC_3DRep GLC_Factory::create3DRepFromFile(const QString& fileName) const
{
	GLC_3DRep rep;

	if ((QFileInfo(fileName).suffix().toLower() == "3dxml") || (QFileInfo(fileName).suffix().toLower() == "3drep") || (QFileInfo(fileName).suffix().toLower() == "xml"))
	{
		GLC_3dxmlToWorld d3dxmlToWorld;
		connect(&d3dxmlToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		rep= d3dxmlToWorld.create3DrepFrom3dxmlRep(fileName);
	}

	return rep;

}

GLC_FileLoader* GLC_Factory::createFileLoader() const
{
    return new GLC_FileLoader;
}

GLC_Material* GLC_Factory::createMaterial() const
{
	return new GLC_Material();
}

GLC_Material* GLC_Factory::createMaterial(const GLfloat *pAmbiantColor) const
{
	return new GLC_Material("Material", pAmbiantColor);
}

GLC_Material* GLC_Factory::createMaterial(const QColor &color) const
{
	return new GLC_Material(color);
}

GLC_Material* GLC_Factory::createMaterial(GLC_Texture* pTexture) const
{
	return new GLC_Material(pTexture, "TextureMaterial");
}

GLC_Material* GLC_Factory::createMaterial(const QString &textureFullFileName) const
{
	GLC_Texture* pTexture= createTexture(textureFullFileName);
	return createMaterial(pTexture);
}

GLC_Material* GLC_Factory::createMaterial(const QImage &image) const
{
	GLC_Texture* pTexture= createTexture(image);
	return createMaterial(pTexture);
}

GLC_Texture* GLC_Factory::createTexture(const QString &textureFullFileName) const
{
	return new GLC_Texture(textureFullFileName);
}

GLC_Texture* GLC_Factory::createTexture(const QImage & image, const QString& imageFileName) const
{
	return new GLC_Texture(image, imageFileName);
}

GLC_MoverController GLC_Factory::createDefaultMoverController(const QColor& color, GLC_Viewport* pViewport)
{
	GLC_MoverController defaultController;

	//////////////////////////////////////////////////////////////////////
	// Pan Mover
	//////////////////////////////////////////////////////////////////////
	// Create Pan Mover representation
	GLC_RepMover* pRepMover= new GLC_RepCrossMover(pViewport);
	pRepMover->setMainColor(color);
	QList<GLC_RepMover*> listOfRep;
	listOfRep.append(pRepMover);
	// Create the Pan Mover
	GLC_Mover* pMover= new GLC_PanMover(pViewport, listOfRep);
	// Add the Pan Mover to the controller
	defaultController.addMover(pMover, GLC_MoverController::Pan);

	//////////////////////////////////////////////////////////////////////
	// Zoom Mover
	//////////////////////////////////////////////////////////////////////
	// Copy the pan Mover representation
	pRepMover= pRepMover->clone();
	listOfRep.clear();
	listOfRep.append(pRepMover);
	// Create the Zoom Mover
	pMover= new GLC_ZoomMover(pViewport, listOfRep);
	// Add the Zoom Mover to the controller
	defaultController.addMover(pMover, GLC_MoverController::Zoom);

	//////////////////////////////////////////////////////////////////////
	// Set Target Mover
	//////////////////////////////////////////////////////////////////////
	// Create the Zoom Mover
	pMover= new GLC_SetTargetMover(pViewport);
	// Add the Zoom Mover to the controller
	defaultController.addMover(pMover, GLC_MoverController::Target);

	//////////////////////////////////////////////////////////////////////
	// Track Ball Mover
	//////////////////////////////////////////////////////////////////////
	// Copy the pan Mover representation
	pRepMover= pRepMover->clone();
	listOfRep.clear();
	listOfRep.append(pRepMover);
	// Create the track ball representation
	pRepMover= new GLC_RepTrackBallMover(pViewport);
	pRepMover->setMainColor(color);
	listOfRep.append(pRepMover);
	// Create the Track Ball Mover
	pMover= new GLC_TrackBallMover(pViewport, listOfRep);
	// Add the Track ball Mover to the controller
	defaultController.addMover(pMover, GLC_MoverController::TrackBall);

	//////////////////////////////////////////////////////////////////////
	// Turn Table Mover
	//////////////////////////////////////////////////////////////////////
	// Create the Turn Table Mover
	pMover= new GLC_TurnTableMover(pViewport);
	// Add the Turn Table Mover to the controller
	defaultController.addMover(pMover, GLC_MoverController::TurnTable);

	//////////////////////////////////////////////////////////////////////
	// Fly Mover
	//////////////////////////////////////////////////////////////////////
	listOfRep.clear();
	pRepMover= new GLC_RepFlyMover(pViewport);
	pRepMover->setMainColor(color);
	listOfRep.append(pRepMover);
	// Create the fly mover
	pMover= new GLC_FlyMover(pViewport, listOfRep);
	// Add the fly mover to the controller
	defaultController.addMover(pMover, GLC_MoverController::Fly);

	//////////////////////////////////////////////////////////////////////
	// Translation, rotation and scaling Mover
	//////////////////////////////////////////////////////////////////////
	// Create the Turn Table Mover
	pMover= new GLC_TsrMover(pViewport);
	// Add the Turn Table Mover to the controller
	defaultController.addMover(pMover, GLC_MoverController::TSR);

	return defaultController;
}


void GLC_Factory::loadPlugins()
{
	Q_ASSERT(NULL != QCoreApplication::instance());
	const QStringList libraryPath= QCoreApplication::libraryPaths();
	foreach(QString path, libraryPath)
	{
		const QDir pluginsDir= QDir(path);
		const QStringList pluginNames= pluginsDir.entryList(QDir::Files);
		foreach (QString fileName, pluginNames)
		{
	         QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
	         QObject* pPlugin= loader.instance();
	         GLC_WorldReaderPlugin* pWorldReader = qobject_cast<GLC_WorldReaderPlugin *>(pPlugin);
	         if (pWorldReader)
	         {
	        	 m_WorldReaderPluginList.append(pWorldReader);
	        	 m_SupportedExtensionSet.unite(QSet<QString>::fromList(pWorldReader->keys()));
	         }
		}
	}

	//qDebug() << m_WorldReaderPluginList.size() << " Loaded plugins.";
}

QList<GLC_WorldReaderPlugin*> GLC_Factory::worldReaderPlugins()
{
	if (NULL == m_pFactory)
	{
		instance();
	}
	return m_WorldReaderPluginList;
}

bool GLC_Factory::canBeLoaded(const QString& extension)
{
	if (NULL == m_pFactory)
	{
		instance();
	}

	return m_SupportedExtensionSet.contains(extension.toLower());
}

GLC_WorldReaderHandler* GLC_Factory::loadingHandler(const QString& fileName)
{
	if (NULL == m_pFactory)
	{
		instance();
	}

	const QString extension= QFileInfo(fileName).suffix();
	if (canBeLoaded(extension))
	{
		foreach(GLC_WorldReaderPlugin* pPlugin, m_WorldReaderPluginList)
		{
			if (pPlugin->keys().contains(extension.toLower()))
			{
				return pPlugin->readerHandler();
			}
		}
	}
	return NULL;
}


