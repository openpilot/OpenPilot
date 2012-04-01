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
//! \file glc_cachemanager.cpp implementation of the GLC_CacheManager class.

#include "glc_cachemanager.h"
#include <QtDebug>


GLC_CacheManager::GLC_CacheManager(const QString& path)
: m_Dir()
, m_UseCompression(true)
, m_CompressionLevel(-1)
{
	if (! path.isEmpty())
	{
		QFileInfo pathInfo(path);
		if (pathInfo.isDir() && pathInfo.isReadable())
		{
			m_Dir.setPath(path);
		}
	}
}

// Copy constructor
GLC_CacheManager::GLC_CacheManager(const GLC_CacheManager& cacheManager)
:m_Dir(cacheManager.m_Dir)
, m_UseCompression(cacheManager.m_UseCompression)
, m_CompressionLevel(cacheManager.m_CompressionLevel)
{

}

// Assignement operator
GLC_CacheManager& GLC_CacheManager::operator=(const GLC_CacheManager& cacheManager)
{
	m_Dir= cacheManager.m_Dir;
	m_UseCompression= cacheManager.m_UseCompression;
	m_CompressionLevel= cacheManager.m_CompressionLevel;

	return *this;
}

GLC_CacheManager::~GLC_CacheManager()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return true if the cache is is readable
bool GLC_CacheManager::isReadable() const
{
	bool isReadable= true;
	isReadable= isReadable && m_Dir.exists();
	QFileInfo dirInfo(m_Dir.absolutePath());
	isReadable= isReadable && dirInfo.isReadable();

	return isReadable;

}

// Return true if the cache is writable
bool GLC_CacheManager::isWritable() const
{
	bool isWritable= true;
	isWritable= isWritable && m_Dir.exists();
	QFileInfo dirInfo(m_Dir.absolutePath());
	isWritable= isWritable && dirInfo.isWritable();

	return isWritable;
}

// Return True if the specified file is cashed in the specified context
bool GLC_CacheManager::isCashed(const QString& context, const QString& fileName) const
{
	if (! isReadable()) return false;

	QFileInfo fileInfo(m_Dir.absolutePath() + QDir::separator() + context + QDir::separator() + fileName + '.' + GLC_BSRep::suffix());
	return fileInfo.exists();
}

// Return True if the cached file is usable
bool GLC_CacheManager::isUsable(const QDateTime& timeStamp, const QString& context, const QString& fileName) const
{
	bool result= isCashed(context, fileName);

	if (result)
	{
		QFileInfo cacheFileInfo(m_Dir.absolutePath() + QDir::separator() + context + QDir::separator() + fileName+ '.' + GLC_BSRep::suffix());
		//result= result && (timeStamp == cacheFileInfo.lastModified());
		result= result && cacheFileInfo.isReadable();
		if (result)
		{
			GLC_BSRep binaryRep;
			binaryRep.setAbsoluteFileName(cacheFileInfo.absoluteFilePath());
			result= result && binaryRep.isUsable(timeStamp);
		}
	}

	return result;
}

// Return the binary serialized representation of the specified file
GLC_BSRep GLC_CacheManager::binary3DRep(const QString& context, const QString& fileName) const
{
	const QString absoluteFileName(m_Dir.absolutePath() + QDir::separator() + context + QDir::separator() + fileName + '.' + GLC_BSRep::suffix());
	GLC_BSRep binaryRep(absoluteFileName);

	return binaryRep;
}

// Add the specified file in the cache
bool GLC_CacheManager::addToCache(const QString& context, const GLC_3DRep& rep)
{
	Q_ASSERT(!rep.fileName().isEmpty());
	bool addedToCache= isWritable();
	if (addedToCache)
	{
		QFileInfo contextCacheInfo(m_Dir.absolutePath() + QDir::separator() + context);
		if (! contextCacheInfo.exists())
		{
			addedToCache= m_Dir.mkdir(context);
		}
		if (addedToCache)
		{
			QString repFileName= rep.fileName();
			if (glc::isArchiveString(repFileName))
			{
				repFileName= glc::archiveEntryFileName(repFileName);
			}
			else
			{
				repFileName= QFileInfo(repFileName).fileName();
			}
			const QString binaryFileName= contextCacheInfo.filePath() + QDir::separator() + repFileName;
			GLC_BSRep binariRep(binaryFileName, m_UseCompression);
			binariRep.setCompressionLevel(m_CompressionLevel);
			addedToCache= binariRep.save(rep);
		}
	}

	return addedToCache;
}

//////////////////////////////////////////////////////////////////////
//Set Functions
//////////////////////////////////////////////////////////////////////

// Set the cache file path
bool GLC_CacheManager::setCachePath(const QString& path)
{
	QFileInfo pathInfo(path);
	bool result= pathInfo.isDir();
	result= result && pathInfo.isReadable();

	if (result)
	{
		m_Dir.setPath(path);
	}
	return result;
}

