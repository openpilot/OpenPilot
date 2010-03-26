/**
 ******************************************************************************
 *
 * @file       iuavgadget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#ifndef IUAVGADGET_H
#define IUAVGADGET_H

#include <coreplugin/icontext.h>
#include <coreplugin/core_global.h>
#include <QtGui/QComboBox>

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
    IUAVGadget(QString classId, QList<IUAVGadgetConfiguration*> *configurations, QObject *parent = 0);
    virtual ~IUAVGadget() {}

    virtual QList<int> context() const = 0;
    virtual QWidget *widget() = 0;
    virtual QString contextHelpId() const { return QString(); }
    QString classId() const { return m_classId; }

    virtual void loadConfiguration(IUAVGadgetConfiguration* /*config*/) { }
    IUAVGadgetConfiguration *activeConfiguration() { return m_activeConfiguration; }
    void setActiveConfiguration(IUAVGadgetConfiguration *config);
    QComboBox *toolBar() { return m_toolbar; }
    virtual QByteArray saveState();
    virtual void restoreState(QByteArray state);
public slots:
    void configurationChanged(IUAVGadgetConfiguration* config);
    void configurationAdded(IUAVGadgetConfiguration* config);
    void configurationToBeDeleted(IUAVGadgetConfiguration* config);
    void configurationNameChanged(QString oldName, QString newName);
private slots:
    void loadConfiguration(int index);
protected:
    QComboBox *m_toolbar;
private:
    QString m_classId;
    IUAVGadgetConfiguration *m_activeConfiguration;
    QList<IUAVGadgetConfiguration*> *m_configurations;
};

} // namespace Core

#endif // IUAVGADGET_H
