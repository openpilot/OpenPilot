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
//! \file glc_bsrep.h interface for the GLC_BSRep class.

#ifndef GLC_BSREP_H_
#define GLC_BSREP_H_

#include <QString>
#include <QFileInfo>
#include <QDataStream>
#include <QUuid>
#include <QDateTime>
#include <QMutex>

#include "../glc_config.h"
#include "glc_3drep.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_BSRep
/*! \brief GLC_BSRep : The 3D Binary serialised representation*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_BSRep
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_BSRep(const QString& absoluteFileName= QString(), bool useCompression= true);

	//! Copy constructor
	GLC_BSRep(const GLC_BSRep&);

	//! Destructor
	virtual ~GLC_BSRep();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return the binary representation file name
	inline QString absoluteFileName() const
	{return m_FileInfo.fileName();}

	//! Return true if the binary rep is usable
	bool isUsable(const QDateTime&);

	//! Load the binary rep
	GLC_3DRep loadRep();

	//! Return the bounding box of the binary representation
	GLC_BoundingBox boundingBox();

	//! Return bsrep suffix
	static QString suffix();

	//! Return bsrep version
	static quint32 version();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set the binary representation file name
	void setAbsoluteFileName(const QString&);

	//! Save the GLC_3DRep in serialised binary
	bool save(const GLC_3DRep&);

	//! Set the compression usage for saving a 3DREP in binary format
	inline void setCompressionUsage(bool usage)
	{m_UseCompression= usage;}

	//! Set the compression level if compression is used when saving in binary format
	inline void setCompressionLevel(int level)
	{m_CompressionLevel= level;}

//@}

private:
//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////

	//! Open the file
	bool open(QIODevice::OpenMode);

	//! Close the file
	bool close();

	//! Write the header
	void writeHeader(const QDateTime&);

	//! Check the header
	bool headerIsOk();

	//! Check the time Stamp
	bool timeStampOk(const QDateTime&);

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! The binary rep suffix
	static const QString m_Suffix;

	//! The binary rep magic number
	static const QUuid m_Uuid;

	//! The binary rep version
	static const quint32 m_Version;

	//! the Binary representation file informations
	QFileInfo m_FileInfo;

	//! The brep file
	QFile* m_pFile;

	//! The Data stream
	QDataStream m_DataStream;

	//! Compress Data
	bool m_UseCompression;

	//! The compression level
	int m_CompressionLevel;

	//! Compression Mutex
	static QMutex m_CompressionMutex;

};

#endif /* GLC_BSREP_H_ */
