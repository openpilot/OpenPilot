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

//! \file glc_objmtlloader.h interface for the GLC_ObjMtlLoader class.

#ifndef GLC_OBJMTLLOADER_H_
#define GLC_OBJMTLLOADER_H_

#include <QString>
#include <QHash>
#include <QTextStream>
#include "../shading/glc_material.h"

#include "../glc_config.h"

class QGLContext;
//////////////////////////////////////////////////////////////////////
//! \class GLC_ObjMtlLoader
/*! \brief GLC_ObjMtlLoader : Load the mtl file associated to a OBJ File */

/*! An GLC_ObjMtlLoader create GLC_Material from the .mtl file \n
  */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_ObjMtlLoader
{

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	GLC_ObjMtlLoader(const QString&);

	virtual ~GLC_ObjMtlLoader();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true if the material name is found
	inline bool contains(const QString &name) const
	{return m_Materials.contains(name);}

	//! Get a material from is name
	GLC_Material* material(const QString&);

	//! Get the list of attached files
	inline QStringList listOfAttachedFileName() const
	{return m_ListOfAttachedFileName.toList();}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Load the materials
	bool loadMaterials();

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Private services functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Extract the material name
	bool extractMaterialName(QString &);

	//! Extract the texture file name
	void extractTextureFileName(QString &);

	//! Extract RGB value
	bool extractRGBValue(QString &);

	//! Extract One value
	bool extractOneValue(QString &);

	//! Get texture file name without parameters
	QString getTextureName(QTextStream &, const QString &);

	//! Process Maya specific obj
	void processMayaSpecific();


//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! The mtl file name
	QString m_FileName;

	//! Current material
	GLC_Material* m_pCurrentMaterial;

	//! The GLC_Material Hash Table
	QHash<QString, GLC_Material*> m_Materials;

	//! the Load status
	QString m_LoadStatus;

	//! The list of attached file name
	QSet<QString> m_ListOfAttachedFileName;


};

#endif /*GLC_OBJMTLLOADER_H_*/
