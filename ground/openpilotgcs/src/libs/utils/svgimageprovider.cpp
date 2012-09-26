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
    QObject(),
    QDeclarativeImageProvider(QDeclarativeImageProvider::Image),
    m_basePath(basePath)
{
}

SvgImageProvider::~SvgImageProvider()
{
    qDeleteAll(m_renderers);
}

QSvgRenderer *SvgImageProvider::loadRenderer(const QString &svgFile)
{
    QSvgRenderer *renderer = m_renderers.value(svgFile);
    if (!renderer) {
        renderer = new QSvgRenderer(svgFile);

        QString fn = QUrl::fromLocalFile(m_basePath).resolved(svgFile).toLocalFile();

        //convert path to be relative to base
        if (!renderer->isValid())
            renderer->load(fn);

        if (!renderer->isValid()) {
            qWarning() << "Failed to load svg file:" << svgFile << fn;
            delete renderer;
            return 0;
        }

        m_renderers.insert(svgFile, renderer);
    }

    return renderer;
}

/**
   Supported id format: fileName[!elementName[?parameters]]
   where parameters may be:
   vslice=1:2;hslice=2:4 - use the 3rd horizontal slice of total 4 slices, slice numbering starts from 0
   borders=1 - 1 pixel wide transparent border

   requestedSize is related to the whole element size, even if slice is requested.

   usage:

   Image {
       source: "image://svg/pfd.svg!world"
   }
*/
QImage SvgImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString svgFile = id;
    QString element;
    QString parameters;

    int separatorPos = id.indexOf('!');
    if (separatorPos != -1) {
        svgFile = id.left(separatorPos);
        element = id.mid(separatorPos+1);
    }

    int parametersPos = element.indexOf('?');
    if (parametersPos != -1) {
        parameters = element.mid(parametersPos+1);
        element = element.left(parametersPos);
    }

    int hSlicesCount = 0;
    int hSlice = 0;
    int vSlicesCount = 0;
    int vSlice = 0;
    int border = 0;
    if (!parameters.isEmpty()) {
        QRegExp hSliceRx("hslice=(\\d+):(\\d+)");
        if (hSliceRx.indexIn(parameters) != -1) {
            hSlice = hSliceRx.cap(1).toInt();
            hSlicesCount = hSliceRx.cap(2).toInt();
        }
        QRegExp vSliceRx("vslice=(\\d+):(\\d+)");
        if (vSliceRx.indexIn(parameters) != -1) {
            vSlice = vSliceRx.cap(1).toInt();
            vSlicesCount = vSliceRx.cap(2).toInt();
        }
        QRegExp borderRx("border=(\\d+)");
        if (borderRx.indexIn(parameters) != -1) {
            border = borderRx.cap(1).toInt();
        }
    }

    if (size)
        *size = QSize();

    QSvgRenderer *renderer = loadRenderer(svgFile);
    if (!renderer)
        return QImage();

    qreal xScale = 1.0;
    qreal yScale = 1.0;

    QSize docSize = renderer->defaultSize();

    if (!requestedSize.isEmpty()) {
        if (!element.isEmpty()) {
            QRectF elementBounds = renderer->boundsOnElement(element);
            xScale = qreal(requestedSize.width())/elementBounds.width();
            yScale = qreal(requestedSize.height())/elementBounds.height();
        } else if (!docSize.isEmpty()) {
            xScale = qreal(requestedSize.width())/docSize.width();
            yScale = qreal(requestedSize.height())/docSize.height();
        }
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
        int elementWidth = qRound(elementBounds.width() * xScale);
        int elementHeigh = qRound(elementBounds.height() * yScale);
        int w = elementWidth;
        int h = elementHeigh;
        int x = 0;
        int y = 0;

        if (hSlicesCount > 1) {
            x = (w*hSlice)/hSlicesCount;
            w = (w*(hSlice+1))/hSlicesCount - x;
        }

        if (vSlicesCount > 1) {
            y = (h*(vSlice))/vSlicesCount;
            h = (h*(vSlice+1))/vSlicesCount - y;
        }

        QImage img(w+border*2, h+border*2, QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter p(&img);
        p.setRenderHints(QPainter::TextAntialiasing |
                         QPainter::Antialiasing |
                         QPainter::SmoothPixmapTransform);

        p.translate(-x+border,-y+border);
        renderer->render(&p, element, QRectF(0, 0, elementWidth, elementHeigh));

        if (size)
            *size = QSize(w, h);

        //img.save(element+parameters+".png");
        return img;
    } else {
        //render the whole svg file
        int w = qRound(docSize.width() * xScale);
        int h = qRound(docSize.height() * yScale);

        QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter p(&img);
        p.setRenderHints(QPainter::TextAntialiasing |
                         QPainter::Antialiasing |
                         QPainter::SmoothPixmapTransform);

        p.scale(xScale, yScale);
        renderer->render(&p, QRectF(QPointF(), QSizeF(docSize)));

        if (size)
            *size = QSize(w, h);
        return img;
    }
}

QPixmap SvgImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return QPixmap::fromImage(requestImage(id, size, requestedSize));
}

/*!
  \fn SvgImageProvider::scaledElementBounds(const QString &svgFile, const QString &element)

  Returns the bound of \a element in logical coordinates,
  scalled to the default size of svg document (so the bounds of whole doc would be (0,0,1,1) ).
*/
QRectF SvgImageProvider::scaledElementBounds(const QString &svgFile, const QString &elementName)
{
    QSvgRenderer *renderer = loadRenderer(svgFile);

    if (!renderer)
        return QRectF();

    if (!renderer->elementExists(elementName)) {
        qWarning() << "invalid element:" << elementName << "of" << svgFile;
        return QRectF();
    }

    QRectF elementBounds = renderer->boundsOnElement(elementName);
    QMatrix matrix = renderer->matrixForElement(elementName);
    elementBounds = matrix.mapRect(elementBounds);

    QSize docSize = renderer->defaultSize();
    return QRectF(elementBounds.x()/docSize.width(),
                  elementBounds.y()/docSize.height(),
                  elementBounds.width()/docSize.width(),
                  elementBounds.height()/docSize.height());
}
