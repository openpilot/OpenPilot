/**
 ******************************************************************************
 *
 * @file       iuavgadget.h
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

#ifndef IUAVGADGET_H
#define IUAVGADGET_H

#include <coreplugin/icontext.h>
#include <coreplugin/core_global.h>
#include <QtGui/QComboBox>
#include <QtCore/QSettings>

QT_BEGIN_NAMESPACE
class QWidget;
class QComboBox;
QT_END_NAMESPACE

namespace Core {

class IUAVGadgetConfiguration;

class CORE_EXPORT IUAVGadget : public IContext
{
    Q_OBJECT
public:
    IUAVGadget(QString classId, QObject *parent = 0) :
            IContext(parent),
            m_classId(classId) { }

    virtual ~IUAVGadget() {}

    QList<int> context() const { return m_context; }
    void setContext(QList<int> context) { m_context = context; }
    virtual QWidget *widget() = 0;
    virtual QComboBox *toolBar() { return 0; }
    virtual QString contextHelpId() const { return QString(); }
    QString classId() const { return m_classId; }

    virtual IUAVGadgetConfiguration *activeConfiguration() { return 0; }
    virtual void loadConfiguration(IUAVGadgetConfiguration*) { }
    virtual void saveState(QSettings* /*qSettings*/) { }
    virtual void restoreState(QByteArray) { }
    virtual void restoreState(QSettings* /*qSettings*/) { }
public slots:
    virtual void configurationChanged(IUAVGadgetConfiguration* ) { }
    virtual void configurationAdded(IUAVGadgetConfiguration*) { }
    virtual void configurationToBeDeleted(IUAVGadgetConfiguration*) { }
    virtual void configurationNameChanged(QString, QString) { }
private slots:
private:
    QString m_classId;
    QList<int> m_context;
};

} // namespace Core

#endif // IUAVGADGET_H
