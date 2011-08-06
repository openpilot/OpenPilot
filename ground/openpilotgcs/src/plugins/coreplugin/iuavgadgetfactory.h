/**
 ******************************************************************************
 *
 * @file       iuavgadgetfactory.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
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

#ifndef IUAVGADGETFACTORY_H
#define IUAVGADGETFACTORY_H
#include "core_global.h"

#include <QtCore/QObject>
#include <QtGui/QIcon>
#include <QSettings>
#include "uavconfiginfo.h"

QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE

namespace Core {

class IUAVGadget;
class IUAVGadgetConfiguration;
class IOptionsPage;

class CORE_EXPORT IUAVGadgetFactory : public QObject
{
    Q_OBJECT
public:
    IUAVGadgetFactory(QString classId, QString name, QObject *parent = 0) :
            QObject(parent),
            m_classId(classId),
            m_name(name),
            m_icon(QIcon()),
            m_singleConfigurationGadget(false) {}
    virtual ~IUAVGadgetFactory() {}

    virtual IUAVGadget *createGadget(QWidget *parent) = 0;
    virtual IUAVGadgetConfiguration *createConfiguration(QSettings* /*qSettings*/) { return 0; }
    virtual IUAVGadgetConfiguration *createConfiguration(QSettings* qs, UAVConfigInfo */*configInfo*/) { return createConfiguration(qs); }
    virtual IOptionsPage *createOptionsPage(IUAVGadgetConfiguration */*config*/) { return 0; }
    QString classId() const { return m_classId; }
    QString name() const { return m_name; }
    QIcon icon() const { return m_icon; }
    bool isSingleConfigurationGadget() { return m_singleConfigurationGadget; }
protected:
    void setIcon(QIcon icon) { m_icon = icon; }
    void setSingleConfigurationGadgetTrue() { m_singleConfigurationGadget = true; }
private:
    QString m_classId; // unique class id
    QString m_name; // display name, should also be unique
    QIcon m_icon;
    bool m_singleConfigurationGadget; // true if there is exactly one configuration for this gadget
};

} // namespace Core

#endif // IUAVGADGETFACTORY_H
