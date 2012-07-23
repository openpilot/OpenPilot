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
    void setTerrainEnabled(bool flag) { m_terrainEnabled = flag; }

    //get dial configuration functions
    QString qmlFile() const { return m_qmlFile; }
    QString earthFile() const { return m_earthFile; }
    bool terrainEnabled() const { return m_terrainEnabled; }

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

private:
    QString m_qmlFile; // The name of the dial's SVG source file
    QString m_earthFile; // The name of osgearth terrain file
    bool m_terrainEnabled;
};

#endif // PfdQmlGADGETCONFIGURATION_H
