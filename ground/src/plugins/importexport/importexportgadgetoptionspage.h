/**
 ******************************************************************************
 *
 * @file       importexportgadgetoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Linear Dial Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   importexportplugin
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

#ifndef IMPORTEXPORTGADGETOPTIONSPAGE_H
#define IMPORTEXPORTGADGETOPTIONSPAGE_H

#include "importexport_global.h"
#include "coreplugin/dialogs/ioptionspage.h"
#include <QString>
#include <QFont>
#include <QStringList>
#include <QDebug>

namespace Core {
class IUAVGadgetConfiguration;
}

class ImportExportGadgetConfiguration;

namespace Ui {
    class ImportExportGadgetOptionsPage;
}

using namespace Core;

class IMPORTEXPORT_EXPORT ImportExportGadgetOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit ImportExportGadgetOptionsPage(ImportExportGadgetConfiguration *config, QObject *parent = 0);

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

private:
    Ui::ImportExportGadgetOptionsPage *options_page;
    ImportExportGadgetConfiguration *m_config;
    QFont font;

private slots:

};

#endif // IMPORTEXPORTGADGETOPTIONSPAGE_H
