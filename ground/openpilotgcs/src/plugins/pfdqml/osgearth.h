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

#ifndef OSGEARTH_H
#define OSGEARTH_H

#include <osgViewer/Viewer>

#include <QtDeclarative/QDeclarativeItem>
#include <osgQt/GraphicsWindowQt>

class QGLFramebufferObject;
class QGLWidget;
class OsgEarthItemRenderer;

class OsgEarthItem : public QDeclarativeItem
{
    Q_OBJECT
    Q_DISABLE_COPY(OsgEarthItem)

    Q_PROPERTY(QString sceneFile READ sceneFile WRITE setSceneFile NOTIFY sceneFileChanged)
    Q_PROPERTY(qreal fieldOfView READ fieldOfView WRITE setFieldOfView NOTIFY fieldOfViewChanged)

    Q_PROPERTY(qreal roll READ roll WRITE setRoll NOTIFY rollChanged)
    Q_PROPERTY(qreal pitch READ pitch WRITE setPitch NOTIFY pitchChanged)
    Q_PROPERTY(qreal yaw READ yaw WRITE setYaw NOTIFY yawChanged)

    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(double altitude READ altitude WRITE setAltitude NOTIFY altitudeChanged)

public:
    OsgEarthItem(QDeclarativeItem *parent = 0);
    ~OsgEarthItem();

    QString sceneFile() const { return m_sceneFile; }
    QString resolvedSceneFile() const;
    qreal fieldOfView() const { return m_fieldOfView; }

    qreal roll() const { return m_roll; }
    qreal pitch() const { return m_pitch; }
    qreal yaw() const { return m_yaw; }

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double altitude() const { return m_altitude; }

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget);

public slots:
    void updateView();
    void setSceneFile(QString arg);
    void setFieldOfView(qreal arg);

    void setRoll(qreal arg);
    void setPitch(qreal arg);
    void setYaw(qreal arg);

    void setLatitude(double arg);
    void setLongitude(double arg);
    void setAltitude(double arg);

signals:
    void rollChanged(qreal arg);
    void pitchChanged(qreal arg);
    void yawChanged(qreal arg);

    void latitudeChanged(double arg);
    void longitudeChanged(double arg);
    void altitudeChanged(double arg);

    void sceneFileChanged(QString arg);
    void fieldOfViewChanged(qreal arg);

private slots:
    void updateFrame();

private:
    OsgEarthItemRenderer *m_renderer;
    QThread *m_rendererThread;

    QSize m_currentSize;

    qreal m_roll;
    qreal m_pitch;
    qreal m_yaw;

    double m_latitude;
    double m_longitude;
    double m_altitude;

    qreal m_fieldOfView;
    QString m_sceneFile;

};

class OsgEarthItemRenderer : public QObject
{
    Q_OBJECT
public:
    OsgEarthItemRenderer(OsgEarthItem *item, QGLWidget *glWidget);
    ~OsgEarthItemRenderer();

    QGLFramebufferObject *lastFrame();
    void markDirty() { m_cameraDirty = true; }

public slots:
    void initScene();
    void updateFrame();

signals:
    void frameReady();

private:
    enum { FboCount = 3 };
    OsgEarthItem *m_item;

    osg::ref_ptr<osgViewer::Viewer> m_viewer;
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_gw;
    osg::ref_ptr<osg::Node> m_model;
    QWeakPointer<QGLWidget> m_glWidget;

    QGLFramebufferObject* m_fbo[FboCount];
    int m_lastFboNumber;

    QSize m_currentSize;

    bool m_cameraDirty;
};

QML_DECLARE_TYPE(OsgEarthItem)

#endif // OSGEARTH_H

