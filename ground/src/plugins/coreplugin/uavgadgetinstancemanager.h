/**
 ******************************************************************************
 *
 * @file       uavgadgetinstancemanager.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   coreplugin
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

#ifndef UAVGADGETINSTANCEMANAGER_H
#define UAVGADGETINSTANCEMANAGER_H

#include <QObject>
#include <QtCore/QMap>
#include <QtCore/QStringList>

namespace Core
{

class IUAVGadget;
class IUAVGadgetConfiguration;
class UAVGadgetOptionsPage;
class IUAVGadgetFactory;

class UAVGadgetInstanceManager : public QObject
{
Q_OBJECT
public:
    explicit UAVGadgetInstanceManager(QObject *parent = 0);
    void readUAVGadgetConfigurations();
    void writeUAVGadgetConfigurations();
    IUAVGadget *createUAVGadget(QString classId, QWidget *parent);
    bool deleteUAVGadgetConfiguration(QString classId, QString configName);
    QStringList uavGadgetClassIds() const { return m_uavGadgetClassIds.keys(); }
    QStringList uavGadgetConfigurationNames(QString classId) const;
    QString uavGadgetName(QString classId) const;

signals:

public slots:

private:
    IUAVGadgetFactory *uavGadgetFactory(QString classId) const;
    void createUAVGadgetOptionPages();
    QList<IUAVGadgetConfiguration*> *uavGadgetConfigurations(QString classId) const;
//    UAVGadgetOptionsPage *createUAVGadgetOptionsPage(QString classId);
    QList<IUAVGadget*> m_uavGadgetInstances;
    QList<IUAVGadgetFactory*> m_uavGadgetFactories;
    QList<IUAVGadgetConfiguration*> m_uavGadgetConfigurations;
    QMap<QString, QString> m_uavGadgetClassIds;

};

} // namespace Core

#endif // UAVGADGETINSTANCEMANAGER_H
