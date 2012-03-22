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

//! \file glc_factory.h Interface for the GLC_Factory class.

#ifndef GLC_FACTORY_
#define GLC_FACTORY_

#include <QObject>
#include <QtOpenGL>
#include <QString>
#include <QSet>

//class to built
#include "geometry/glc_point.h"
#include "geometry/glc_pointsprite.h"
#include "geometry/glc_line.h"
#include "geometry/glc_circle.h"
#include "geometry/glc_box.h"
#include "geometry/glc_cylinder.h"
#include "geometry/glc_cone.h"
#include "geometry/glc_sphere.h"
#include "geometry/glc_rectangle.h"
#include "geometry/glc_3drep.h"
#include "geometry/glc_pointcloud.h"
#include "shading/glc_material.h"
#include "shading/glc_texture.h"
#include "sceneGraph/glc_world.h"
#include "sceneGraph/glc_3dviewinstance.h"
#include "glc_boundingbox.h"
#include "viewport/glc_movercontroller.h"
#include "viewport/glc_viewport.h"
#include "io/glc_fileloader.h"

// end of class to built

class GLC_WorldReaderHandler;
class GLC_WorldReaderPlugin;

#include "glc_config.h"
//////////////////////////////////////////////////////////////////////
//! \class GLC_Factory
/*! \brief GLC_Factory : Factory for all geometrical objects
 *  this class is a singleton
 */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Factory : public QObject
{
	Q_OBJECT

public:
	//! Get unique instance of the factory
	static GLC_Factory* instance();

protected:
	//! Constructor
	GLC_Factory();
public:
	//! Destructor
	~GLC_Factory();

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Create a GLC_Point
	GLC_3DRep createPoint(const GLC_Point3d &coord) const;

	GLC_3DRep createPoint(double x, double y, double z) const;

	//! Create a cloud of points
	GLC_3DRep createPointCloud(const GLfloatVector& data, const QColor& color);

	GLC_3DRep createPointCloud(const QList<GLC_Point3d>& pointList, const QColor& color);

	GLC_3DRep createPointCloud(const QList<GLC_Point3df>& pointList, const QColor& color);

	//! Create a GLC_PointSprite
	GLC_3DRep createPointSprite(float, GLC_Material*) const;

	//! Create a GLC_Line
	GLC_3DRep createLine(const GLC_Point3d&, const GLC_Point3d&) const;

	//!  Create a GLC_Circle
	GLC_3DRep createCircle(double radius, double angle= 2 * glc::PI) const;

	//! Create a GLC_Box
	GLC_3DRep createBox(double lx, double ly, double lz) const;

	GLC_3DViewInstance createBox(const GLC_BoundingBox& boundingBox) const;

	//! Create a GLC_Cylinder
	GLC_3DRep createCylinder(double radius, double length) const;

	//! Create a GLC_Cone
	GLC_3DRep createCone(double radius, double length) const;

	//! Create a GLC_Sphere
	GLC_3DRep createSphere(double radius) const;

	//!Create a GLC_Rectangle
	GLC_3DRep createRectangle(double, double);

	//! Create a GLC_Rectangle from the given 3d point, normal and the given lenght
	GLC_3DViewInstance createRectangle(const GLC_Point3d& point, const GLC_Vector3d& normal, double l1, double l2);

	//! Create the representation of a cutting from the given 3d point, normal, lenght and material
	GLC_3DViewInstance createCuttingPlane(const GLC_Point3d& point, const GLC_Vector3d& normal, double l1, double l2, GLC_Material* pMat);

	//! Create a GLC_World from a QFile
	GLC_World createWorldFromFile(QFile &file, QStringList* pAttachedFileName= NULL) const;

	//! Create a GLC_World containing only the 3dxml structure
	GLC_World createWorldStructureFrom3dxml(QFile &file, bool GetExtRefName= false) const;

	//! Create 3DRep from 3dxml or 3DRep file
	GLC_3DRep create3DRepFromFile(const QString&) const;

	//! Create a GLC_FileLoader
	GLC_FileLoader* createFileLoader() const;

	//! Create default material
	GLC_Material* createMaterial() const;

	//! create a material with an ambient color
	GLC_Material* createMaterial(const GLfloat *pAmbiantColor) const;

	//! create a material with an ambient color
	GLC_Material* createMaterial(const QColor &color) const;

	//! create a material textured with a texture
	GLC_Material* createMaterial(GLC_Texture* pTexture) const;

	//! create a material textured with a image file name
	GLC_Material* createMaterial(const QString &textureFullFileName) const;

	//! create a material textured with a QImage
	GLC_Material* createMaterial(const QImage &) const;

	//! Create a GLC_Texture
	GLC_Texture* createTexture(const QString &textureFullFileName) const;

	//! Create a GLC_Texture with a QImage
	GLC_Texture* createTexture(const QImage &, const QString& imageFileName= QString()) const;

	//! Create the default mover controller
	GLC_MoverController createDefaultMoverController(const QColor&, GLC_Viewport*);

	//! Return the list of world reader plugin
	static QList<GLC_WorldReaderPlugin*> worldReaderPlugins();

	//! Return true if the given file extension can be loaded
	static bool canBeLoaded(const QString& extension);

	//! Return an handle to the plugin tu use for the given file
	static GLC_WorldReaderHandler* loadingHandler(const QString& fileName);

//@}

signals:
	//! For progress bar management
	void currentQuantum(int) const;

//////////////////////////////////////////////////////////////////////
// Private services functions
//////////////////////////////////////////////////////////////////////
private:
	//! Load GLC_lib plugins
	void loadPlugins();

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////

private:
	//! The unique instance of the factory
	static GLC_Factory* m_pFactory;

	//! The list off worldReader plugins
	static QList<GLC_WorldReaderPlugin*> m_WorldReaderPluginList;

	//! The supported extension set
	static QSet<QString> m_SupportedExtensionSet;

};

#endif /*GLC_FACTORY_*/
