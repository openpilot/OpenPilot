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

//! \file glc_xmlutil.h interface for some QStream XML utilities.

#ifndef XMLUTIL_H_
#define XMLUTIL_H_
#include <QString>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QtDebug>

namespace glcXmlUtil
{
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

};


bool glcXmlUtil::goToElement(QXmlStreamReader* pReader, const QString& element)
{
	while(!pReader->atEnd() && !(pReader->isStartElement() && (pReader->name() == element)))
	{
		pReader->readNext();
	}
	return !pReader->atEnd();
}

QString glcXmlUtil::getContent(QXmlStreamReader* pReader, const QString& element)
{
	QString content;
	while(endElementNotReached(pReader, element))
	{
		pReader->readNext();
		if (pReader->isCharacters() && !pReader->text().isEmpty())
		{
			content+= pReader->text().toString();
		}
	}

	return content.trimmed();
}

QString glcXmlUtil::readAttribute(QXmlStreamReader* pReader, const QString& attribute)
{
	return pReader->attributes().value(attribute).toString();
}

bool glcXmlUtil::endElementNotReached(QXmlStreamReader* pReader, const QString& element)
{
	return !pReader->atEnd() && !(pReader->isEndElement() && (pReader->name() == element));
}

bool glcXmlUtil::startElementNotReached(QXmlStreamReader* pReader, const QString& element)
{
	return !pReader->atEnd() && !(pReader->isStartElement() && (pReader->name() == element));
}

void glcXmlUtil::goToEndElement(QXmlStreamReader* pReader, const QString& element)
{
	while(endElementNotReached(pReader, element))
	{
		pReader->readNext();
	}
}

#endif /* XMLUTIL_H_ */
