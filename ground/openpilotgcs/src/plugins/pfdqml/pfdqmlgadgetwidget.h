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

#ifndef PFDQMLGADGETWIDGET_H_
#define PFDQMLGADGETWIDGET_H_

#include "pfdqmlgadgetconfiguration.h"
#include <QtDeclarative/qdeclarativeview.h>

class PfdQmlGadgetWidget : public QDeclarativeView
{
    Q_OBJECT
    Q_PROPERTY(QString earthFile READ earthFile WRITE setEarthFile NOTIFY earthFileChanged)
    Q_PROPERTY(bool terrainEnabled READ terrainEnabled WRITE setTerrainEnabled NOTIFY terrainEnabledChanged)

    Q_PROPERTY(bool actualPositionUsed READ actualPositionUsed WRITE setActualPositionUsed NOTIFY actualPositionUsedChanged)

    //pre-defined fallback position
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(double altitude READ altitude WRITE setAltitude NOTIFY altitudeChanged)

public:
    PfdQmlGadgetWidget(QWidget *parent = 0);
   ~PfdQmlGadgetWidget();
    void setQmlFile(QString fn);

    QString earthFile() const { return m_earthFile; }
    bool terrainEnabled() const { return m_terrainEnabled && m_openGLEnabled; }

    bool actualPositionUsed() const { return m_actualPositionUsed; }
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double altitude() const { return m_altitude; }

public slots:
    void setEarthFile(QString arg);
    void setTerrainEnabled(bool arg);
    void setOpenGLEnabled(bool arg);

    void setLatitude(double arg);
    void setLongitude(double arg);
    void setAltitude(double arg);

    void setActualPositionUsed(bool arg);

signals:
    void earthFileChanged(QString arg);
    void terrainEnabledChanged(bool arg);

    void actualPositionUsedChanged(bool arg);
    void latitudeChanged(double arg);
    void longitudeChanged(double arg);
    void altitudeChanged(double arg);

private:
    QString m_qmlFileName;
    QString m_earthFile;
    bool m_openGLEnabled;
    bool m_terrainEnabled;

    bool m_actualPositionUsed;
    double m_latitude;
    double m_longitude;
    double m_altitude;
};

#endif /* PFDQMLGADGETWIDGET_H_ */
