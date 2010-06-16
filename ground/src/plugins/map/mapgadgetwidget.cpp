/**
 ******************************************************************************
 *
 * @file       mapgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   map
 * @{
 *
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
#include "mapgadgetwidget.h"
#include <QStringList>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include "extensionsystem/pluginmanager.h"

// *************************************************************************************
// constructor

MapGadgetWidget::MapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    int size = 256;

    gcsButton = NULL;
    uavButton = NULL;

    follow_uav = false;

    gcs_pixmap.load(QString::fromUtf8(":/map/images/gcs.png"));
    uav_pixmap.load(QString::fromUtf8(":/map/images/uav.png"));
    waypoint_pixmap.load(QString::fromUtf8(":/map/images/waypoint.png"));
//    waypoint_pixmap.load(QCoreApplication::applicationDirPath() + "/images/waypoint.png");

    // test
    if (gcs_pixmap.isNull()) QMessageBox::warning(this, tr("Image Error"), tr("Missing ") + QString::fromUtf8(":/map/images/gcs.png"));
    if (uav_pixmap.isNull()) QMessageBox::warning(this, tr("Image Error"), tr("Missing ") + QString::fromUtf8(":/map/images/uav.png"));
    if (waypoint_pixmap.isNull()) QMessageBox::warning(this, tr("Image Error"), tr("Missing ") + QString::fromUtf8(":/map/images/waypoint.png"));

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    m_positionActual = PositionActual::GetInstance(objManager);

    m_mc = new qmapcontrol::MapControl(QSize(size, size));

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

//    setMouseTracking(true);

    m_mc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_mc->setMinimumSize(64, 64);
    m_mc->showScale(true);
    m_mc->showLatLon(true);

    m_osmAdapter = new OSMMapAdapter();
    m_googleAdapter = new GoogleMapAdapter();
    m_googleSatAdapter = new GoogleSatMapAdapter();
    m_yahooAdapter = new YahooMapAdapter();

    m_osmLayer = new MapLayer("OpenStreetMap", m_osmAdapter);
    m_googleLayer = new MapLayer("Google", m_googleAdapter);
    m_googleSatLayer = new MapLayer("Google Sat", m_googleSatAdapter);
    m_yahooLayer = new MapLayer("Yahoo", m_yahooAdapter);

    // gcs and uav position layer
    m_positionLayer = new GeometryLayer("PositionsLayer", m_osmAdapter);

    // Waypoint layer
    m_wayPointLayer = new GeometryLayer("WayPointsLayer", m_osmAdapter);

    m_osmLayer->setVisible(true);
    m_googleLayer->setVisible(false);
    m_googleSatLayer->setVisible(false);
    m_yahooLayer->setVisible(false);
    m_positionLayer->setVisible(true);
    m_wayPointLayer->setVisible(true);

    m_mc->addLayer(m_osmLayer);
    m_mc->addLayer(m_googleLayer);
    m_mc->addLayer(m_googleSatLayer);
    m_mc->addLayer(m_yahooLayer);
    m_mc->addLayer(m_wayPointLayer);
    m_mc->addLayer(m_positionLayer);

    PositionActual::DataFields data = m_positionActual->getData();  // get current position data

    heading = data.Heading;

//    addCompass(QPointF(100, 100), 200);

    // create and add the GCS icon
    gcsPoint = new ImagePoint(data.Longitude, data.Latitude, &gcs_pixmap, "GSC", qmapcontrol::Point::Middle);
    m_positionLayer->addGeometry(gcsPoint);
    connect(gcsPoint, SIGNAL(geometryClicked(Geometry*, QPoint)), this, SLOT(gcs_uav_ClickEvent(Geometry*, QPoint)));

    // create and add the UAV icon
    uavPoint = new ImagePoint(data.Longitude, data.Latitude, &uav_pixmap, "UAV", qmapcontrol::Point::Middle);
    m_positionLayer->addGeometry(uavPoint);
    connect(uavPoint, SIGNAL(geometryClicked(Geometry*, QPoint)), this, SLOT(gcs_uav_ClickEvent(Geometry*, QPoint)));

    addUserControls();

    m_mc->setView(gcsPoint->coordinate());
    m_mc->setZoom(2);
    m_mc->updateRequestNew();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mc);
    setLayout(layout);

    connect(m_mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)), this, SLOT(coordinateEvent(const QMouseEvent*, const QPointF)));

//    connect(m_wayPointLayer, SIGNAL(geometryClicked(Geometry*, QPoint)), this, SLOT(wayPointClickEvent(Geometry*, QPoint)));

    m_updateTimer = new QTimer();
    m_updateTimer->setInterval(250);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();
}

// *************************************************************************************
// destructor

MapGadgetWidget::~MapGadgetWidget()
{
   // Do nothing
}

// *************************************************************************************

void MapGadgetWidget::setZoom(int value)
{
    m_mc->setZoom(value);
    m_mc->updateRequestNew();
}

void MapGadgetWidget::setPosition(QPointF pos)
{
    m_mc->setView(pos);
    m_mc->updateRequestNew();
}

void MapGadgetWidget::setMapProvider(QString provider)
{
    foreach (QString layerName, m_mc->layers())
    {
	Layer *layer = m_mc->layer(layerName);
	MapAdapter *ma = (MapAdapter*)layer->mapadapter();

	bool visible = layerName == provider;

	if (layerName == "WayPointsLayer" || layerName == "PositionsLayer")
	{   // leave the way points & positions layers visible
	    continue;
	}

	layer->setVisible(visible);

	if (visible)
	{   // move the geomtery layers over to the selected map
	    m_positionLayer->setMapAdapter(ma);
	    m_wayPointLayer->setMapAdapter(ma);
	}
    }

    m_positionLayer->setVisible(true);
    m_wayPointLayer->setVisible(true);

    m_mc->updateRequestNew();
}

// *************************************************************************************

void MapGadgetWidget::updatePosition()
{
    PositionActual::DataFields data = m_positionActual->getData();	// get current position data

    heading = data.Heading;						// save the current UAV heading

    uavPoint->setCoordinate(QPointF(data.Longitude, data.Latitude));	// move the UAV icon

    if (follow_uav)
	setPosition(uavPoint->coordinate());				// center the map onto the UAV
}

void MapGadgetWidget::resizeEvent(QResizeEvent *event)
{
    m_mc->resize(QSize(width(), height()));
    update();
    QWidget::resizeEvent(event);
}

// *************************************************************************************

void MapGadgetWidget::addUserControls()
{   // create the user controls

	gcsButton = new QPushButton(tr("GCS"));
	gcsButton->setMinimumWidth(50);
	gcsButton->setMaximumWidth(50);
	gcsButton->setToolTip(tr("Center onto ground control station"));
	connect(gcsButton, SIGNAL(clicked(bool)), this, SLOT(gcsButtonClick()));

	uavButton = new QPushButton(tr("UAV"));
	uavButton->setMinimumWidth(50);
	uavButton->setMaximumWidth(50);
	uavButton->setCheckable(true);
	uavButton->setToolTip(tr("Stay centered on the UAV"));
	connect(uavButton, SIGNAL(clicked(bool)), this, SLOT(uavButtonClick(bool)));

	QPushButton* zoomin = new QPushButton("+");
	zoomin->setMinimumWidth(50);
	zoomin->setMaximumWidth(50);
	zoomin->setToolTip(tr("Zoom in"));
	connect(zoomin, SIGNAL(clicked(bool)), m_mc, SLOT(zoomIn()));

	QPushButton* zoomout = new QPushButton("-");
	zoomout->setMinimumWidth(50);
	zoomout->setMaximumWidth(50);
	zoomout->setToolTip(tr("Zoom out"));
	connect(zoomout, SIGNAL(clicked(bool)), m_mc, SLOT(zoomOut()));

	// add zoom buttons to the layout of the MapControl
	QVBoxLayout* innerlayout = new QVBoxLayout;
    	innerlayout->setSpacing(3);
	innerlayout->setMargin(2);
	innerlayout->addSpacing(10);
	innerlayout->addWidget(gcsButton);
	innerlayout->addWidget(uavButton);
	innerlayout->addSpacing(10);
	innerlayout->addWidget(zoomin);
	innerlayout->addWidget(zoomout);
	innerlayout->addStretch(0);
	m_mc->setLayout(innerlayout);
}

// *************************************************************************************

void MapGadgetWidget::gcsButtonClick()
{
    follow_uav = false;
    uavButton->setChecked(follow_uav);

    setPosition(gcsPoint->coordinate());    // center the map onto the ground station
}

// *************************************************************************************

void MapGadgetWidget::uavButtonClick(bool checked)
{
    follow_uav = checked;

    if (follow_uav)
    {	// immediately center the map onto the UAV .. rather than waiting for the timer to it
//	PositionActual::DataFields data = m_positionActual->getData();		// get the current position data
//	heading = data.Heading;							// save the current UAV heading
//	uavPoint->setCoordinate(QPointF(data.Longitude, data.Latitude));	// move the UAV icon
//	setPosition(uavPoint->coordinate());					// center the map on the UAV
    }
}

// *************************************************************************************

void MapGadgetWidget::coordinateEvent(const QMouseEvent * evnt, const QPointF coordinate) // cmoss
{
    QString coord_str = "lat " + QString::number(coordinate.y(), 'f', 6) + ", lon " + QString::number(coordinate.x(), 'f', 6);

    if (evnt->type() == QEvent::MouseButtonPress)
    {	// mouse click event
	if (evnt->buttons() == Qt::RightButton)
	{
	    qDebug() << coordinate << ": " << evnt->x() << " / " << evnt->y() << " / " << coord_str;

	    QMessageBox::information(this, "Coordinate Clicked", coord_str);
	}
    }
    else
    {	// mouse move event
	qDebug() << coordinate << ": " << evnt->x() << " / " << evnt->y() << " / " << coord_str;
    }
}

// *************************************************************************************
// comes here when the user mouse clicks on the GCS or the UAV

void MapGadgetWidget::gcs_uav_ClickEvent(Geometry* geom, QPoint)
{
    qDebug() << "parent: " << geom->parentGeometry();
    qDebug() << "Element clicked: " << geom->name();

    if (geom->hasClickedPoints())
    {
	QList<Geometry*> pp = geom->clickedPoints();

    	qDebug() << "number of child elements: " << pp.size();

	for (int i=0; i<pp.size(); i++)
	{
	    QMessageBox::information(this, geom->name(), pp.at(i)->name());
	}
    }
    else
    if (geom->GeometryType == "Point")
    {
	if (geom->name() == "UAV")
	{
	    QMessageBox::information(this, geom->name(), tr("The UAV location"));
	}
	else
	if (geom->name() == "GCS")
	{
	    QMessageBox::information(this, geom->name(), tr("The GCS location"));
	}
	else
	    QMessageBox::information(this, geom->name(), tr("just a point"));
    }
}

// *************************************************************************************
// comes here when the user mouse clicks on a waypoint

void MapGadgetWidget::wayPointClickEvent(Geometry* geom, QPoint)
{
    qDebug() << "parent: " << geom->parentGeometry();
    qDebug() << "Element clicked: " << geom->name();

    if (geom->hasClickedPoints())
    {
	QList<Geometry*> pp = geom->clickedPoints();

	qDebug() << "number of child elements: " << pp.size();

    	for (int i=0; i<pp.size(); i++)
	{
	    QMessageBox::information(this, geom->name(), pp.at(i)->name());
	}
    }
    else
    if (geom->GeometryType == "Point")
    {
	QMessageBox::information(this, geom->name(), tr("just a point"));
    }
}

// *************************************************************************************
// add a way point

void MapGadgetWidget::addWayPoint(QPointF pos, QString name)
{
    if (waypoint_pixmap.isNull()) return;

    ImagePoint *waypoint = new ImagePoint(pos.x(), pos.y(), &waypoint_pixmap, name, qmapcontrol::Point::BottomRight);
//    waypoint->setBaselevel(0);
    connect(waypoint, SIGNAL(geometryClicked(Geometry *, QPoint)), this, SLOT(wayPointClickEvent(Geometry *, QPoint)));

    m_wayPointLayer->addGeometry(waypoint);
}

// *************************************************************************************
// add the compass

void MapGadgetWidget::addCompass(QPointF pos, int size, QString name)
{

    return;

//    if (compass_background_pixmap.isNull())
    {	// create the compass background image
	QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
	if (image.isNull()) return;

	image.fill(QColor(0, 0, 0, 0).rgba());

	QPainter painter(&image);
	painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing, true);
	painter.setFont(font());

	QPen pen = QPen(Qt::NoPen);
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(1);
	pen.setBrush(Qt::black);
	painter.setPen(pen);

	QRadialGradient gradient(size / 2, size / 2, size / 2);
	gradient.setColorAt(0.0, QColor(0, 0, 0, 0));
	gradient.setColorAt(0.05, QColor(0, 0, 0, 160));
	gradient.setColorAt(1.0, QColor(0, 0, 0, 160));

//	QGradient gradient = QLinearGradient(0, 0, 0, 1);
	gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
	gradient.setSpread(QGradient::PadSpread);
	painter.setBrush(gradient);

//	painter.drawRect(image.rect());
	painter.drawEllipse(image.rect());

	compass_background_pixmap = QPixmap::fromImage(image);
    }

//    ImagePoint *compass = new ImagePoint(pos.x(), pos.y(), &compass_background_pixmap, name, qmapcontrol::Point::Middle);
//    connect(compass, SIGNAL(geometryClicked(Geometry *, QPoint)), this, SLOT(compassClickEvent(Geometry *, QPoint)));

    m_compassImageOverlay = new FixedImageOverlay(0, 0, size, size, &compass_background_pixmap, "compass_background");
//    m_compassImageOverlay->setBaselevel(0);
//    m_compassImageOverlay->setPen();

    m_positionLayer->addGeometry(m_compassImageOverlay);
}

// *************************************************************************************

void MapGadgetWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) // ESC
    {
//	emit(close());
    }
    else
    if (event->key() == Qt::Key_F1) // F1
    {
	gcsButtonClick();
    }
    else
    if (event->key() == Qt::Key_F2) // F2
    {
	follow_uav = !follow_uav;
	uavButton->setChecked(follow_uav);
	uavButtonClick(follow_uav);
    }
    else
    if (event->key() == Qt::Key_Up)
    {
    }
    else
    if (event->key() == Qt::Key_Down)
    {
    }
    else
    if (event->key() == Qt::Key_Left)
    {
    }
    else
    if (event->key() == Qt::Key_Right)
    {
    }
    else
    if (event->key() == Qt::Key_PageUp)
    {
	m_mc->zoomIn();
    }
    else
    if (event->key() == Qt::Key_PageDown)
    {
	m_mc->zoomOut();
    }
    else
    {
	qDebug() << event->key() << endl;
    }
}

// *************************************************************************************
