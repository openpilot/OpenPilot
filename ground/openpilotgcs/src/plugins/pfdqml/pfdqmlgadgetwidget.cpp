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

#include "pfdqmlgadgetwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "utils/svgimageprovider.h"
#ifdef USE_OSG
#include "osgearth.h"
#endif
#include <QDebug>
#include <QSvgRenderer>
#include <QtOpenGL/QGLWidget>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>

#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativecontext.h>
#include <QtDeclarative/qdeclarativeengine.h>

PfdQmlGadgetWidget::PfdQmlGadgetWidget(QWidget *parent) :
    QDeclarativeView(parent),
    m_openGLEnabled(false),
    m_terrainEnabled(false),
    m_actualPositionUsed(false),
    m_latitude(46.671478),
    m_longitude(10.158932),
    m_altitude(2000)
{
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setResizeMode(SizeRootObjectToView);

    //setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));

    QStringList objectsToExport;
    objectsToExport << "VelocityActual" <<
                       "PositionActual" <<
                       "AttitudeActual" <<
                       "Accels" <<
                       "VelocityDesired" <<
                       "PositionDesired" <<
                       "AttitudeHoldDesired" <<
                       "GPSPosition" <<
                       "GCSTelemetryStats" <<
                       "FlightBatteryState";

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    foreach (const QString &objectName, objectsToExport) {
        UAVObject* object = objManager->getObject(objectName);
        if (object)
            engine()->rootContext()->setContextProperty(objectName, object);
        else
            qWarning() << "Failed to load object" << objectName;
    }

    //to expose settings values
    engine()->rootContext()->setContextProperty("qmlWidget", this);
#ifdef USE_OSG
    qmlRegisterType<OsgEarthItem>("org.OpenPilot", 1, 0, "OsgEarth");
#endif
}

PfdQmlGadgetWidget::~PfdQmlGadgetWidget()
{
}

void PfdQmlGadgetWidget::setQmlFile(QString fn)
{
    m_qmlFileName = fn;

    engine()->removeImageProvider("svg");
    SvgImageProvider *svgProvider = new SvgImageProvider(fn);
    engine()->addImageProvider("svg", svgProvider);

    //it's necessary to allow qml side to query svg element position
    engine()->rootContext()->setContextProperty("svgRenderer", svgProvider);
    engine()->setBaseUrl(QUrl::fromLocalFile(fn));

    qDebug() << Q_FUNC_INFO << fn;
    setSource(QUrl::fromLocalFile(fn));

    foreach(const QDeclarativeError &error, errors()) {
        qDebug() << error.description();
    }
}

void PfdQmlGadgetWidget::setEarthFile(QString arg)
{
    if (m_earthFile != arg) {
        m_earthFile = arg;
        emit earthFileChanged(arg);
    }
}

void PfdQmlGadgetWidget::setTerrainEnabled(bool arg)
{
    bool wasEnabled = terrainEnabled();
    m_terrainEnabled = arg;

    if (wasEnabled != terrainEnabled())
        emit terrainEnabledChanged(terrainEnabled());
}

void PfdQmlGadgetWidget::setOpenGLEnabled(bool arg)
{
    if (m_openGLEnabled != arg) {
        m_openGLEnabled = arg;

        qDebug() << Q_FUNC_INFO << "Set OPENGL" << m_openGLEnabled;
        if (m_openGLEnabled)
            setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
        else
            setViewport(new QWidget);

        //update terrainEnabled status with opengl status chaged
        setTerrainEnabled(m_terrainEnabled);
    }
}

//Switch between PositionActual UAVObject position
//and pre-defined latitude/longitude/altitude properties
void PfdQmlGadgetWidget::setActualPositionUsed(bool arg)
{
    if (m_actualPositionUsed != arg) {
        m_actualPositionUsed = arg;
        emit actualPositionUsedChanged(arg);
    }
}

void PfdQmlGadgetWidget::setLatitude(double arg)
{
    //not sure qFuzzyCompare is accurate enough for geo coordinates
    if (m_latitude != arg) {
        m_latitude = arg;
        emit latitudeChanged(arg);
    }
}

void PfdQmlGadgetWidget::setLongitude(double arg)
{
    if (m_longitude != arg) {
        m_longitude = arg;
        emit longitudeChanged(arg);
    }
}

void PfdQmlGadgetWidget::setAltitude(double arg)
{
    if (!qFuzzyCompare(m_altitude,arg)) {
        m_altitude = arg;
        emit altitudeChanged(arg);
    }
}
