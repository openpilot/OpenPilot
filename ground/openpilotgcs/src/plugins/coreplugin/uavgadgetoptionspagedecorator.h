/**
 ******************************************************************************
 *
 * @file       uavgadgetoptionspagedecorator.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#ifndef UAVGADGETOPTIONSPAGEDECORATOR_H
#define UAVGADGETOPTIONSPAGEDECORATOR_H

#include "iuavgadgetconfiguration.h"
#include <coreplugin/core_global.h>
#include <coreplugin/dialogs/ioptionspage.h>

class Ui_TopOptionsPage;

namespace Core {

class IUAVGadgetConfiguration;
class UAVGadgetInstanceManager;

class CORE_EXPORT UAVGadgetOptionsPageDecorator : public Core::IOptionsPage
{
Q_OBJECT
public:
    explicit UAVGadgetOptionsPageDecorator(IOptionsPage *page, IUAVGadgetConfiguration *config, bool isSingleConfigurationGadget = false, QObject *parent = 0);

    QString id() const { return m_id; }
    QString trName() const { return m_id; }
    QString category() const { return m_category; }
    QString trCategory() const { return m_categoryTr; }

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

signals:

public slots:

private slots:
    void cloneConfiguration();
    void deleteConfiguration();
    void textEdited(QString);

private:
    IOptionsPage *m_optionsPage;
    IUAVGadgetConfiguration *m_config;
    bool m_isSingleConfigurationGadget;
    UAVGadgetInstanceManager *m_instanceManager;
    QString m_id;
    QString m_category;
    QString m_categoryTr;

    Ui_TopOptionsPage *m_page;
};

} // namespace Core

#endif // UAVGADGETOPTIONSPAGEDECORATOR_H
