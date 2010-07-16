/**
 ******************************************************************************
 *
 * @file       modelviewgadgetoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ModelViewPlugin ModelView Plugin
 * @{
 * @brief A gadget that displays a 3D representation of the UAV 
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

#ifndef MODELVIEWGADGETOPTIONSPAGE_H
#define MODELVIEWGADGETOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"
#include <QtGui/QLabel>
#include <QtGui/QFileDialog>
#include <QtGui/QPushButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

class ModelViewGadgetConfiguration;
class QFileDialog;
namespace Core {
    class IUAVGadgetConfiguration;
}
namespace Ui {
    class ModelViewOptionsPage;
}

using namespace Core;

class ModelViewGadgetOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit ModelViewGadgetOptionsPage(ModelViewGadgetConfiguration *config, QObject *parent = 0);
    QString id() const { return ""; }
    QString trName() const { return ""; }
    QString category() const { return ""; }
    QString trCategory() const { return ""; }

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();
private:

signals:

public slots:
private slots:

private:
    ModelViewGadgetConfiguration *m_config;
    Ui::ModelViewOptionsPage *m_page;
};

#endif // MODELVIEWGADGETOPTIONSPAGE_H
