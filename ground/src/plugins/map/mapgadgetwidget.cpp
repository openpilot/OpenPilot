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

MapGadgetWidget::MapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    int size = 256;

    gcsButton = 0;	// added by cathy
    uavButton = 0;	// added by cathy

    follow_uav = false;	// added by cathy

    m_mc = new MapControl(QSize(size, size));

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_mc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_mc->setMinimumSize(64, 64);
    m_mc->showScale(true);
    m_mc->showLatLon(true); // added by cathy

    m_osmAdapter = new OSMMapAdapter();
    m_googleSatAdapter = new GoogleSatMapAdapter();
    m_googleAdapter = new GoogleMapAdapter();
    m_yahooAdapter = new YahooMapAdapter();

    m_osmLayer = new MapLayer("OpenStreetMap", m_osmAdapter);
    m_googleLayer = new MapLayer("Google", m_googleAdapter);
    m_googleSatLayer = new MapLayer("Google Sat", m_googleSatAdapter);
    m_yahooLayer = new MapLayer("Yahoo", m_yahooAdapter);

    m_osmLayer->setVisible(true);
    m_googleLayer->setVisible(false);
    m_googleSatLayer->setVisible(false);
    m_yahooLayer->setVisible(false);

    m_mc->addLayer(m_osmLayer);
    m_mc->addLayer(m_googleLayer);
    m_mc->addLayer(m_googleSatLayer);
    m_mc->addLayer(m_yahooLayer);

    addUserControls();

    m_mc->setView(QPointF(5.718888888888, 58.963333333333));
    m_mc->setZoom(10);
    m_mc->updateRequestNew();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mc);
    setLayout(layout);

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    m_positionActual = PositionActual::GetInstance(objManager);

    // added by cathy .. slot for receiving mouse click Coordinate events
//    connect(m_mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)), this, SLOT(coordinateClicked(const QMouseEvent*, const QPointF)));	// cmoss

    m_updateTimer = new QTimer();
    m_updateTimer->setInterval(250);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();
}

MapGadgetWidget::~MapGadgetWidget()
{
   // Do nothing
}
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

void MapGadgetWidget::updatePosition()
{
    if (follow_uav)
    {
	PositionActual::DataFields data = m_positionActual->getData();
	setPosition(QPointF(data.Longitude, data.Latitude));
    }
}

void MapGadgetWidget::setMapProvider(QString provider)
{
    foreach(QString layerName, m_mc->layers())
        m_mc->layer(layerName)->setVisible(layerName == provider);
}

void MapGadgetWidget::resizeEvent(QResizeEvent *event)
{
    m_mc->resize(QSize(width(), height()));
    update();
    QWidget::resizeEvent(event);
}

// changed by cathy
void MapGadgetWidget::addUserControls()
{
	// create user controls
	gcsButton = new QPushButton("GCS");	// added by cathy
	uavButton = new QPushButton("UAV");	// added by cathy
	QPushButton* zoomin = new QPushButton("+");
	QPushButton* zoomout = new QPushButton("-");

	// cathy
	gcsButton->setMinimumWidth(50);
	gcsButton->setMaximumWidth(50);
	gcsButton->setToolTip("Jump too ground station control");
	connect(gcsButton, SIGNAL(clicked(bool)), this, SLOT(gcsButtonClick()));

	// cathy
	uavButton->setMinimumWidth(50);
	uavButton->setMaximumWidth(50);
	uavButton->setCheckable(true);
	uavButton->setToolTip("Stay centered on the UAV if checked");
	connect(uavButton, SIGNAL(clicked(bool)), this, SLOT(uavButtonClick()));

	zoomin->setMinimumWidth(50);
	zoomin->setMaximumWidth(50);
	zoomin->setToolTip("Zoom in");
	connect(zoomin, SIGNAL(clicked(bool)), m_mc, SLOT(zoomIn()));

	zoomout->setMinimumWidth(50);
	zoomout->setMaximumWidth(50);
	zoomout->setToolTip("Zoom out");
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

void MapGadgetWidget::gscButtonClick()  // added by cathy
{
    uavButton->setChecked(false);
    follow_uav = uavButton->isChecked();

    // jump straight too the GSC location

}

void MapGadgetWidget::uavButtonClick()  // added by cathy
{
    follow_uav = uavButton->isChecked();

    if (follow_uav)
    {
	PositionActual::DataFields data = m_positionActual->getData();
	setPosition(QPointF(data.Longitude, data.Latitude));
    }
}

/*
// added by cathy .. comes here when the user mouse clicks on the map
void MapGadgetWidget::coordinateClicked(const QMouseEvent * evnt, const QPointF coordinate) // cmoss
{
    if (evnt->type() == QEvent::MouseButtonPress && evnt->buttons() == Qt::LeftButton)
    {
	QString coord_str = "lat " + QString::number(coordinate.y(), 'f', 6) + ", lon " + QString::number(coordinate.x(), 'f', 6);

	qDebug() << coordinate << ": " << evnt->x() << " / " << evnt->y() << " / " << coord_str;

	QMessageBox::information(this, "Coordinate Clicked", coord_str);
    }
}
*/
// added by cathy .. comes here when the user mouse clicks on an added geometry feature (ie, way points etc)
void MapGadgetWidget::geometryClicked(Geometry* geom, QPoint)	// cmoss
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
		QMessageBox::information(this, geom->name(), "just a point");
	}
}

// added by cathy
void MapGadgetWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) // ESC
    {
//	emit(close());
    }
    else
    {
	qDebug() << event->key() << endl;
    }
}
