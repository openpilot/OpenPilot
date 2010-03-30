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

//! \file glc_factory.cpp implementation of the GLC_Factory class.

#include "glc_factory.h"
#include "io/glc_objtoworld.h"
#include "io/glc_stltoworld.h"
#include "io/glc_offtoworld.h"
#include "io/glc_3dstoworld.h"
#include "io/glc_3dxmltoworld.h"
#include "io/glc_colladatoworld.h"

#include "viewport/glc_panmover.h"
#include "viewport/glc_zoommover.h"
#include "viewport/glc_settargetmover.h"
#include "viewport/glc_trackballmover.h"
#include "viewport/glc_turntablemover.h"
#include "viewport/glc_repcrossmover.h"
#include "viewport/glc_reptrackballmover.h"

// init static member
GLC_Factory* GLC_Factory::m_pFactory= NULL;

//////////////////////////////////////////////////////////////////////
// static method
//////////////////////////////////////////////////////////////////////
// Return the unique instance of the factory
GLC_Factory* GLC_Factory::instance(const QGLContext *pContext)
{
	if(m_pFactory == NULL)
	{
		m_pFactory= new GLC_Factory(pContext);
	}
	return m_pFactory;
}

//////////////////////////////////////////////////////////////////////
// Constructor destructor
//////////////////////////////////////////////////////////////////////

// Protected constructor
GLC_Factory::GLC_Factory(const QGLContext *pContext)
: m_pQGLContext(pContext)
{

}

// Destructor
GLC_Factory::~GLC_Factory()
{
	m_pFactory= NULL;
}

//////////////////////////////////////////////////////////////////////
// Create functions
//////////////////////////////////////////////////////////////////////

// Create an GLC_Point
GLC_3DRep GLC_Factory::createPoint(const GLC_Vector4d &coord) const
{
	GLC_3DRep newPoint(new GLC_Point(coord));
	return newPoint;
}
// Create an GLC_Point
GLC_3DRep GLC_Factory::createPoint(double x, double y, double z) const
{
	GLC_3DRep newPoint(new GLC_Point(x, y, z));
	return newPoint;
}

// Create an GLC_PointSprite
GLC_3DRep GLC_Factory::createPointSprite(float size, GLC_Material* pMaterial) const
{
	GLC_3DRep newPoint(new GLC_PointSprite(size, pMaterial));
	return newPoint;
}

// Create an GLC_Line
GLC_3DRep GLC_Factory::createLine(const GLC_Point4d& point1, const GLC_Point4d& point2) const
{
	GLC_3DRep newPoint(new GLC_Line(point1, point2));
	return newPoint;
}

//  Create an GLC_Circle
GLC_3DRep GLC_Factory::createCircle(double radius, double angle) const
{
	GLC_3DRep newCircle(new GLC_Circle(radius, angle));
	return newCircle;
}
// Create an GLC_Box
GLC_3DRep GLC_Factory::createBox(double lx, double ly, double lz) const
{

	GLC_3DRep newBox(new GLC_Box(lx, ly, lz));
	return newBox;
}

// Create an GLC_Box
GLC_3DViewInstance GLC_Factory::createBox(const GLC_BoundingBox& boundingBox) const
{
	const double lx= boundingBox.upperCorner().X() - boundingBox.lowerCorner().X();
	const double ly= boundingBox.upperCorner().Y() - boundingBox.lowerCorner().Y();
	const double lz= boundingBox.upperCorner().Z() - boundingBox.lowerCorner().Z();
	GLC_Box* pBox= new GLC_Box(lx, ly, lz);
	GLC_3DViewInstance newBox(pBox);
	newBox.translate(boundingBox.center().X(), boundingBox.center().Y()
					, boundingBox.center().Z());
	return newBox;
}

// Create an GLC_Cylinder
GLC_3DRep GLC_Factory::createCylinder(double radius, double length) const
{

	GLC_3DRep newCylinder(new GLC_Cylinder(radius, length));
	return newCylinder;
}

// Create ang GLC_Rectangle
GLC_3DRep GLC_Factory::createRectangle(const GLC_Vector4d& normal, double l1, double l2)
{
	GLC_3DRep newRectangle(new GLC_Rectangle(normal, l1, l2));
	return newRectangle;
}

// Create an GLC_World* with a QFile
GLC_World* GLC_Factory::createWorld(QFile &file, QStringList* pAttachedFileName) const
{
	GLC_World* pWorld= NULL;
	if (QFileInfo(file).suffix().toLower() == "obj")
	{
		GLC_ObjToWorld objToWorld(m_pQGLContext);
		connect(&objToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		pWorld= objToWorld.CreateWorldFromObj(file);
		if (NULL != pAttachedFileName)
		{
			(*pAttachedFileName)= objToWorld.listOfAttachedFileName();
		}
	}
	else if (QFileInfo(file).suffix().toLower() == "stl")
	{
		GLC_StlToWorld stlToWorld;
		connect(&stlToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		pWorld= stlToWorld.CreateWorldFromStl(file);
	}
	else if (QFileInfo(file).suffix().toLower() == "off")
	{
		GLC_OffToWorld offToWorld;
		connect(&offToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		pWorld= offToWorld.CreateWorldFromOff(file);
	}
	else if (QFileInfo(file).suffix().toLower() == "3ds")
	{
		GLC_3dsToWorld studioToWorld(m_pQGLContext);
		connect(&studioToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		pWorld= studioToWorld.CreateWorldFrom3ds(file);
		if (NULL != pAttachedFileName)
		{
			(*pAttachedFileName)= studioToWorld.listOfAttachedFileName();
		}
	}
	else if (QFileInfo(file).suffix().toLower() == "3dxml")
	{
		GLC_3dxmlToWorld d3dxmlToWorld(m_pQGLContext);
		connect(&d3dxmlToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		pWorld= d3dxmlToWorld.CreateWorldFrom3dxml(file, false);
		if (NULL != pAttachedFileName)
		{
			(*pAttachedFileName)= d3dxmlToWorld.listOfAttachedFileName();
		}
	}
	else if (QFileInfo(file).suffix().toLower() == "dae")
	{
		GLC_ColladaToWorld colladaToWorld(m_pQGLContext);
		connect(&colladaToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		pWorld= colladaToWorld.CreateWorldFromCollada(file);
		if (NULL != pAttachedFileName)
		{
			(*pAttachedFileName)= colladaToWorld.listOfAttachedFileName();
		}
	}


	return pWorld;
}

// Create an GLC_World containing only the 3dxml structure
GLC_World* GLC_Factory::createWorldStructureFrom3dxml(QFile &file) const
{
	GLC_World* pWorld= NULL;

	if (QFileInfo(file).suffix().toLower() == "3dxml")
	{
		GLC_3dxmlToWorld d3dxmlToWorld(m_pQGLContext);
		connect(&d3dxmlToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		pWorld= d3dxmlToWorld.CreateWorldFrom3dxml(file, true);
	}

	return pWorld;
}

// Create 3DRep from 3dxml or 3DRep file
GLC_3DRep GLC_Factory::create3DrepFromFile(const QString& fileName) const
{
	GLC_3DRep rep;

	if ((QFileInfo(fileName).suffix().toLower() == "3dxml") or (QFileInfo(fileName).suffix().toLower() == "3drep"))
	{
		GLC_3dxmlToWorld d3dxmlToWorld(m_pQGLContext);
		connect(&d3dxmlToWorld, SIGNAL(currentQuantum(int)), this, SIGNAL(currentQuantum(int)));
		rep= d3dxmlToWorld.Create3DrepFrom3dxmlRep(fileName);
	}

	return rep;

}

// Create an GLC_Material
GLC_Material* GLC_Factory::createMaterial() const
{
	return new GLC_Material();
}

// Create an GLC_Material
GLC_Material* GLC_Factory::createMaterial(const GLfloat *pAmbiantColor) const
{
	return new GLC_Material("Material", pAmbiantColor);
}
// Create an GLC_Material
GLC_Material* GLC_Factory::createMaterial(const QColor &color) const
{
	return new GLC_Material(color);
}

GLC_Material* GLC_Factory::createMaterial(GLC_Texture* pTexture) const
{
	return new GLC_Material(pTexture, "TextureMaterial");
}
// create an material textured with a image file name
GLC_Material* GLC_Factory::createMaterial(const QString &textureFullFileName) const
{
	GLC_Texture* pTexture= createTexture(textureFullFileName);
	return createMaterial(pTexture);
}

// create an material textured with a QImage
GLC_Material* GLC_Factory::createMaterial(const QImage &image) const
{
	GLC_Texture* pTexture= createTexture(image);
	return createMaterial(pTexture);
}

// Create an GLC_Texture

GLC_Texture* GLC_Factory::createTexture(const QString &textureFullFileName) const
{
	return new GLC_Texture(m_pQGLContext, textureFullFileName);
}

// Create an GLC_Texture with a QImage
GLC_Texture* GLC_Factory::createTexture(const QImage & image) const
{
	return new GLC_Texture(m_pQGLContext, image);
}

// Create the default mover controller
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

	return defaultController;
}
