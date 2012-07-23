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
#include "osgearth.h"

#include <QDebug>
#include <QSvgRenderer>
#include <QtOpenGL/QGLWidget>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>

#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativecontext.h>
#include <QtDeclarative/qdeclarativeengine.h>

PfdQmlGadgetWidget::PfdQmlGadgetWidget(QWidget *parent) :
    QDeclarativeView(parent)
{
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setResizeMode(SizeRootObjectToView);

    setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));

    QStringList objectsToExport;
    objectsToExport << "VelocityActual" <<
                       "PositionActual" <<
                       "AttitudeActual" <<
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

    qmlRegisterType<OsgEarthItem>("org.OpenPilot", 1, 0, "OsgEarth");
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
    if (m_terrainEnabled != arg) {
        m_terrainEnabled = arg;
        emit terrainEnabledChanged(arg);
    }
}
