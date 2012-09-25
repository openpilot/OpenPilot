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

#ifndef PFDQMLGADGETCONFIGURATION_H
#define PFDQMLGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

class PfdQmlGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit PfdQmlGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    void setQmlFile(const QString &fileName) { m_qmlFile=fileName; }
    void setEarthFile(const QString &fileName) { m_earthFile=fileName; }
    void setOpenGLEnabled(bool flag) { m_openGLEnabled = flag; }
    void setTerrainEnabled(bool flag) { m_terrainEnabled = flag; }
    void setActualPositionUsed(bool flag) { m_actualPositionUsed = flag; }
    void setLatitude(double value) { m_latitude = value; }
    void setLongitude(double value) { m_longitude = value; }
    void setAltitude(double value) { m_altitude = value; }
    void setCacheOnly(bool flag) { m_cacheOnly = flag; }

    QString qmlFile() const { return m_qmlFile; }
    QString earthFile() const { return m_earthFile; }
    bool openGLEnabled() const { return m_openGLEnabled; }
    bool terrainEnabled() const { return m_terrainEnabled; }
    bool actualPositionUsed() const { return m_actualPositionUsed; }
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double altitude() const { return m_altitude; }
    bool cacheOnly() const { return m_cacheOnly; }

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

private:
    QString m_qmlFile; // The name of the dial's SVG source file
    QString m_earthFile; // The name of osgearth terrain file
    bool m_openGLEnabled;
    bool m_terrainEnabled;
    bool m_actualPositionUsed;
    double m_latitude;
    double m_longitude;
    double m_altitude;
    bool m_cacheOnly;
};

#endif // PfdQmlGADGETCONFIGURATION_H
