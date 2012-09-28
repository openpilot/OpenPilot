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

#include "osgearth.h"

#include <QtCore/qfileinfo.h>
#include <QtCore/qthread.h>
#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/qdeclarativeview.h>
#include <QtDeclarative/qdeclarativeengine.h>
#include <QtGui/qpainter.h>
#include <QtGui/qvector3d.h>
#include <QtOpenGL/qglframebufferobject.h>

#include <osg/MatrixTransform>
#include <osg/AutoTransform>
#include <osg/Camera>
#include <osg/TexMat>
#include <osg/TextureRectangle>
#include <osg/Texture2D>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ObjectPlacer>
#include <osgEarth/Map>

#include <QtCore/qtimer.h>

#include "utils/pathutils.h"

OsgEarthItem::OsgEarthItem(QDeclarativeItem *parent):
    QDeclarativeItem(parent),
    m_renderer(0),
    m_rendererThread(0),
    m_currentSize(640, 480),
    m_roll(0.0),
    m_pitch(0.0),
    m_yaw(0.0),
    m_latitude(-28.5),
    m_longitude(153.0),
    m_altitude(400.0),
    m_fieldOfView(90.0),
    m_sceneFile(QLatin1String("/usr/share/osgearth/maps/srtm.earth"))
{
    setSize(m_currentSize);
    setFlag(ItemHasNoContents, false);
}

OsgEarthItem::~OsgEarthItem()
{
    if (m_renderer) {
        m_rendererThread->exit();
        //wait up to 10 seconds for renderer thread to exit
        m_rendererThread->wait(10*1000);

        delete m_renderer;
        delete m_rendererThread;
    }
}

QString OsgEarthItem::resolvedSceneFile() const
{
    QString sceneFile = m_sceneFile;

    //try to resolve the relative scene file name:
    if (!QFileInfo(sceneFile).exists()) {
        QDeclarativeView *view = qobject_cast<QDeclarativeView*>(scene()->views().first());

        if (view) {
            QUrl baseUrl = view->engine()->baseUrl();
            sceneFile = baseUrl.resolved(sceneFile).toLocalFile();
        }
    }

    return sceneFile;
}

void OsgEarthItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry);
    Q_UNUSED(newGeometry);

    //Dynamic gyometry changes are not supported yet,
    //terrain is rendered to fixed geompetry and scalled for now

    /*
    qDebug() << Q_FUNC_INFO << newGeometry;

    int w = qRound(newGeometry.width());
    int h = qRound(newGeometry.height());

    if (m_currentSize != QSize(w,h) && m_gw.get()) {
        m_currentSize = QSize(w,h);

        m_gw->getEventQueue()->windowResize(0,0,w,h);
        m_gw->resized(0,0,w,h);

        osg::Camera *camera = m_viewer->getCamera();
        camera->setViewport(new osg::Viewport(0,0,w,h));
        camera->setProjectionMatrixAsPerspective(m_fieldOfView, qreal(w)/h, 1.0f, 10000.0f);
    }
    */
}

void OsgEarthItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(style);
    QGLWidget *glWidget = qobject_cast<QGLWidget*>(widget);

    if (!m_renderer) {
        m_renderer = new OsgEarthItemRenderer(this, glWidget);
        connect(m_renderer, SIGNAL(frameReady()),
                this, SLOT(updateView()), Qt::QueuedConnection);

        m_rendererThread = new QThread(this);
        m_renderer->moveToThread(m_rendererThread);
        m_rendererThread->start();

        QMetaObject::invokeMethod(m_renderer, "initScene", Qt::QueuedConnection);
        return;
    }

    QGLFramebufferObject *fbo = m_renderer->lastFrame();

    if (glWidget && fbo)
        glWidget->drawTexture(boundingRect(), fbo->texture());
}

void OsgEarthItem::updateView()
{
    update();
}

void OsgEarthItem::updateFrame()
{
    if (m_renderer) {
        m_renderer->markDirty();
        QMetaObject::invokeMethod(m_renderer, "updateFrame", Qt::QueuedConnection);
    }
}

void OsgEarthItem::setRoll(qreal arg)
{
    if (!qFuzzyCompare(m_roll, arg)) {
        m_roll = arg;
        updateFrame();
        emit rollChanged(arg);
    }
}

void OsgEarthItem::setPitch(qreal arg)
{
    if (!qFuzzyCompare(m_pitch, arg)) {
        m_pitch = arg;
        updateFrame();
        emit pitchChanged(arg);
    }
}

void OsgEarthItem::setYaw(qreal arg)
{
    if (!qFuzzyCompare(m_yaw, arg)) {
        m_yaw = arg;
        updateFrame();
        emit yawChanged(arg);
    }
}

void OsgEarthItem::setLatitude(double arg)
{
    //not sure qFuzzyCompare is accurate enough for geo coordinates
    if (m_latitude != arg) {
        m_latitude = arg;
        emit latitudeChanged(arg);
    }
}

void OsgEarthItem::setLongitude(double arg)
{
    if (m_longitude != arg) {
        m_longitude = arg;
        emit longitudeChanged(arg);
    }
}

void OsgEarthItem::setAltitude(double arg)
{
    if (!qFuzzyCompare(m_altitude,arg)) {
        m_altitude = arg;
        emit altitudeChanged(arg);
    }
}

//! Camera vertical field of view in degrees
void OsgEarthItem::setFieldOfView(qreal arg)
{
    if (!qFuzzyCompare(m_fieldOfView,arg)) {
        m_fieldOfView = arg;
        emit fieldOfViewChanged(arg);

        //it should be a queued call to OsgEarthItemRenderer instead
        /*if (m_viewer.get()) {
            m_viewer->getCamera()->setProjectionMatrixAsPerspective(
                        m_fieldOfView,
                        qreal(m_currentSize.width())/m_currentSize.height(),
                        1.0f, 10000.0f);
        }*/

        updateFrame();
    }
}

void OsgEarthItem::setSceneFile(QString arg)
{
    if (m_sceneFile != arg) {
        m_sceneFile = arg;
        emit sceneFileChanged(arg);
    }
}

OsgEarthItemRenderer::OsgEarthItemRenderer(OsgEarthItem *item, QGLWidget *glWidget) :
    QObject(0),
    m_item(item),
    m_lastFboNumber(0),
    m_currentSize(640, 480),
    m_cameraDirty(false)
{
    //make a shared gl widget to avoid
    //osg rendering to mess with qpainter state
    //this runs in the main thread
    m_glWidget = new QGLWidget(0, glWidget);
    m_glWidget.data()->setAttribute(Qt::WA_PaintOutsidePaintEvent);

    for (int i=0; i<FboCount; i++) {
        m_fbo[i] = new QGLFramebufferObject(m_currentSize, QGLFramebufferObject::CombinedDepthStencil);
        QPainter p(m_fbo[i]);
        p.fillRect(0,0,m_currentSize.width(), m_currentSize.height(), Qt::gray);
    }
}

OsgEarthItemRenderer::~OsgEarthItemRenderer()
{
    m_glWidget.data()->makeCurrent();
    for (int i=0; i<FboCount; i++) {
        delete m_fbo[i];
        m_fbo[i] = 0;
    }
    m_glWidget.data()->doneCurrent();

    delete m_glWidget.data();
}

QGLFramebufferObject *OsgEarthItemRenderer::lastFrame()
{
    return m_fbo[m_lastFboNumber];
}

void OsgEarthItemRenderer::initScene()
{
    Q_ASSERT(!m_viewer.get());

    int w = m_currentSize.width();
    int h = m_currentSize.height();

    QString sceneFile = m_item->resolvedSceneFile();
    m_model = osgDB::readNodeFile(sceneFile.toStdString());

    //setup caching
    osgEarth::MapNode *mapNode = osgEarth::MapNode::findMapNode(m_model.get());
    if (!mapNode) {
        qWarning() << Q_FUNC_INFO << sceneFile << " doesn't look like an osgEarth file";
    }

    m_gw = new osgViewer::GraphicsWindowEmbedded(0,0,w,h);

    m_viewer = new osgViewer::Viewer();
    m_viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    m_viewer->setSceneData(m_model);
    m_viewer->getDatabasePager()->setDoPreCompile(true);

    osg::Camera *camera = m_viewer->getCamera();
    camera->setViewport(new osg::Viewport(0,0,w,h));
    camera->setGraphicsContext(m_gw);
    camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // configure the near/far so we don't clip things that are up close
    camera->setNearFarRatio(0.00002);
    camera->setProjectionMatrixAsPerspective(m_item->fieldOfView(), qreal(w)/h, 1.0f, 10000.0f);

    updateFrame();
}

void OsgEarthItemRenderer::updateFrame()
{
    if (!m_cameraDirty || !m_viewer.get() || m_glWidget.isNull())
        return;

    m_glWidget.data()->makeCurrent();

    //To find a camera view matrix, find placer matrixes for two points
    //onr at requested coords and another latitude shifted by 0.01 deg
    osgEarth::Util::ObjectPlacer placer(m_viewer->getSceneData());

    m_cameraDirty = false;

    osg::Matrixd positionMatrix;
    placer.createPlacerMatrix(m_item->latitude(), m_item->longitude(), m_item->altitude(), positionMatrix);
    osg::Matrixd positionMatrix2;
    placer.createPlacerMatrix(m_item->latitude()+0.01, m_item->longitude(), m_item->altitude(), positionMatrix2);

    osg::Vec3d eye(0.0f, 0.0f, 0.0f);
    osg::Vec3d viewVector(0.0f, 0.0f, 0.0f);
    osg::Vec3d upVector(0.0f, 0.0f, 1.0f);

    eye = positionMatrix.preMult(eye);
    upVector = positionMatrix.preMult(upVector);
    upVector.normalize();
    viewVector = positionMatrix2.preMult(viewVector) - eye;
    viewVector.normalize();
    viewVector *= 10.0;

    //TODO: clarify the correct rotation order,
    //currently assuming yaw, pitch, roll
    osg::Quat q;
    q.makeRotate(-m_item->yaw()*M_PI/180.0, upVector);
    upVector = q * upVector;
    viewVector = q * viewVector;

    osg::Vec3d side = viewVector ^ upVector;
    q.makeRotate(m_item->pitch()*M_PI/180.0, side);
    upVector = q * upVector;
    viewVector = q * viewVector;

    q.makeRotate(m_item->roll()*M_PI/180.0, viewVector);
    upVector = q * upVector;
    viewVector = q * viewVector;

    osg::Vec3d center = eye + viewVector;

//    qDebug() << "e " << eye.x() << eye.y() << eye.z();
//    qDebug() << "c " << center.x() << center.y() << center.z();
//    qDebug() << "up" << upVector.x() << upVector.y() << upVector.z();

    m_viewer->getCamera()->setViewMatrixAsLookAt(osg::Vec3d(eye.x(), eye.y(), eye.z()),
                                                 osg::Vec3d(center.x(), center.y(), center.z()),
                                                 osg::Vec3d(upVector.x(), upVector.y(), upVector.z()));

    {
        QGLFramebufferObject *fbo = m_fbo[(m_lastFboNumber + 1) % FboCount];
        QPainter fboPainter(fbo);
        fboPainter.beginNativePainting();
        m_viewer->frame();
        fboPainter.endNativePainting();
    }
    m_glWidget.data()->doneCurrent();

    m_lastFboNumber = (m_lastFboNumber + 1) % FboCount;

    emit frameReady();
}
