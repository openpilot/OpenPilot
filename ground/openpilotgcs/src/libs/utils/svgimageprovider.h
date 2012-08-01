/**
 ******************************************************************************
 *
 * @file       svgimageprovider.h
 * @author     Dmytro Poplavskiy Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin QML Viewer Plugin
 * @{
 * @brief Svg declarative image provider
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef SVGIMAGEPROVIDER_H_
#define SVGIMAGEPROVIDER_H_

#include <QObject>
#include <QtDeclarative/qdeclarativeimageprovider.h>
#include <QSvgRenderer>
#include <QMap>

#include "utils_global.h"

class QTCREATOR_UTILS_EXPORT SvgImageProvider : public QObject, public QDeclarativeImageProvider
{
    Q_OBJECT
public:
    SvgImageProvider(const QString &basePath);
   ~SvgImageProvider();

    QSvgRenderer *loadRenderer(const QString &svgFile);

    QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize& requestedSize);

    Q_INVOKABLE QRectF scaledElementBounds(const QString &svgFile, const QString &elementName);

private:
    QMap<QString, QSvgRenderer*> m_renderers;
    QString m_basePath;
};

#endif
