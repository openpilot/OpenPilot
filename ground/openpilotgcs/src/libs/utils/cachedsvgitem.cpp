/**
 ******************************************************************************
 *
 * @file       cachedsvgitem.h
 * @author     Dmytro Poplavskiy Copyright (C) 2011.
 * @{
 * @brief OpenGL texture cached SVG item
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

#include "cachedsvgitem.h"
#include <QGLContext>
#include <QDebug>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

CachedSvgItem::CachedSvgItem(QGraphicsItem * parent) :
    QGraphicsSvgItem(parent),
    m_context(0),
    m_texture(0),
    m_scale(1.0)
{
    setCacheMode(NoCache);
}

CachedSvgItem::CachedSvgItem(const QString & fileName, QGraphicsItem * parent):
    QGraphicsSvgItem(fileName, parent),
    m_context(0),
    m_texture(0),
    m_scale(1.0)
{
    setCacheMode(NoCache);
}

CachedSvgItem::~CachedSvgItem()
{
    if (m_context && m_texture) {
        m_context->makeCurrent();
        glDeleteTextures(1, &m_texture);
    }
}

void CachedSvgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    if (painter->paintEngine()->type() != QPaintEngine::OpenGL &&
            painter->paintEngine()->type() != QPaintEngine::OpenGL2) {
        //Fallback to direct painting
        QGraphicsSvgItem::paint(painter, option, widget);
        return;
    }

    QRectF br = boundingRect();
    QTransform transform = painter->worldTransform();
    qreal sceneScale = transform.map(QLineF(0,0,1,0)).length();

    bool stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
    bool scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);

    painter->beginNativePainting();

    if (stencilTestEnabled)
        glEnable(GL_STENCIL_TEST);
    if (scissorTestEnabled)
        glEnable(GL_SCISSOR_TEST);

    bool dirty = false;
    if (!m_texture) {
        glGenTextures(1, &m_texture);
        m_context = const_cast<QGLContext*>(QGLContext::currentContext());

        dirty = true;
    }

    if (!qFuzzyCompare(sceneScale, m_scale)) {
        m_scale = sceneScale;
        dirty = true;
    }

    int textureWidth = (int(br.width()*m_scale) + 3) & ~3;
    int textureHeight = (int(br.height()*m_scale) + 3) & ~3;

    if (dirty) {
        //qDebug() << "re-render image";

        QImage img(textureWidth, textureHeight, QImage::Format_ARGB32);
        {
            img.fill(Qt::transparent);
            QPainter p;
            p.begin(&img);
            p.setRenderHints(painter->renderHints());
            p.translate(br.topLeft());
            p.scale(m_scale, m_scale);
            QGraphicsSvgItem::paint(&p, option, 0);
            p.end();

            img = img.rgbSwapped();
        }

        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                textureWidth,
                textureHeight,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                img.bits());

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glDisable(GL_TEXTURE_2D);

        dirty = false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, m_texture);

    //texture may be slightly large than svn image, ensure only used area is rendered
    qreal tw = br.width()*m_scale/textureWidth;
    qreal th = br.height()*m_scale/textureHeight;

    glBegin(GL_QUADS);
    glTexCoord2d(0,  0 ); glVertex3d(br.left(), br.top(), -1);
    glTexCoord2d(tw, 0 ); glVertex3d(br.right(), br.top(), -1);
    glTexCoord2d(tw, th); glVertex3d(br.right(), br.bottom(), -1);
    glTexCoord2d(0,  th); glVertex3d(br.left(), br.bottom(), -1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    painter->endNativePainting();
}
