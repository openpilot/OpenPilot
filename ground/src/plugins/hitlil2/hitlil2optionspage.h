/**
 ******************************************************************************
 *
 * @file       hitlil2optionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlil2plugin
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

#ifndef HITLIL2OPTIONSPAGE_H
#define HITLIL2OPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"

namespace Core {
class IUAVGadgetConfiguration;
}

class HITLIL2Configuration;

using namespace Core;

namespace Ui {
    class HITLIL2OptionsPage;
}

class HITLIL2OptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit HITLIL2OptionsPage(HITLIL2Configuration *config, QObject *parent = 0);

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

signals:

private slots:

private:
    HITLIL2Configuration* m_config;
    Ui::HITLIL2OptionsPage* m_optionsPage;

};

#endif // HITLIL2OPTIONSPAGE_H
