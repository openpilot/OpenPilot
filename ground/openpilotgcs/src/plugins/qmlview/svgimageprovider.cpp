/**
 ******************************************************************************
 *
 * @file       svgimageprovider.cpp
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

#include "svgimageprovider.h"
#include <QDebug>
#include <QPainter>
#include <QUrl>

SvgImageProvider::SvgImageProvider(const QString &basePath):
    QDeclarativeImageProvider(QDeclarativeImageProvider::Image),
    m_basePath(basePath)
{
}

SvgImageProvider::~SvgImageProvider()
{
    qDeleteAll(m_renderers);
}

/**
  requestedSize is realted to the whole svg file, not to specific element
  */
QImage SvgImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString svgFile = id;
    QString element;

    int separatorPos = id.indexOf('!');
    if (separatorPos != -1) {
        svgFile = id.left(separatorPos);
        element = id.mid(separatorPos+1);
    }

    if (size)
        *size = QSize();

    QSvgRenderer *renderer = m_renderers.value(svgFile);
    if (!renderer) {
        renderer = new QSvgRenderer(svgFile);

        QString fn = QUrl::fromLocalFile(m_basePath).resolved(svgFile).toLocalFile();

        //convert path to be relative to base
        if (!renderer->isValid())
            renderer->load(fn);

        if (!renderer->isValid()) {
            qWarning() << "Failed to load svg file:" << svgFile << fn;
            return QImage();
        }

        m_renderers.insert(svgFile, renderer);
    }

    qreal xScale = 1.0;
    qreal yScale = 1.0;

    QSize docSize = renderer->defaultSize();

    if (!requestedSize.isEmpty() && !docSize.isEmpty()) {
        xScale = qreal(requestedSize.width())/docSize.width();
        yScale = qreal(requestedSize.height())/docSize.height();
    }

    //keep the aspect ratio
    //TODO: how to configure it? as a part of image path?
    xScale = yScale = qMin(xScale, yScale);

    if (!element.isEmpty()) {
        if (!renderer->elementExists(element)) {
            qWarning() << "invalid element:" << element << "of" << svgFile;
            return QImage();
        }

        QRectF elementBounds = renderer->boundsOnElement(element);
        int w = qRound(elementBounds.width() * xScale);
        int h = qRound(elementBounds.height() * yScale);

        QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter p(&img);
        renderer->render(&p, element);

        if (size)
            *size = QSize(w, h);
        return img;
    } else {
        //render the whole svg file
        int w = qRound(docSize.width() * xScale);
        int h = qRound(docSize.height() * yScale);

        QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter p(&img);
        renderer->render(&p);

        if (size)
            *size = QSize(w, h);
        return img;
    }
}

QPixmap SvgImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return QPixmap::fromImage(requestImage(id, size, requestedSize));
}
