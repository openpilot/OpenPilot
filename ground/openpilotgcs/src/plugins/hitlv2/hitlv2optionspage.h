/**
 ******************************************************************************
 *
 * @file       hitlv2optionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup HITLPlugin HITLv2 Plugin
 * @{
 * @brief The Hardware In The Loop plugin version 2
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

#ifndef HITLV2OPTIONSPAGE_H
#define HITLV2OPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

namespace Core {
class IUAVGadgetConfiguration;
}

class HITLConfiguration;

using namespace Core;

namespace Ui {
class HITLOptionsPage;
}

class HITLOptionsPage : public IOptionsPage
{
    Q_OBJECT
public:
    explicit HITLOptionsPage(HITLConfiguration *conf, QObject *parent = 0);

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();
    bool isDecorated() const { return true; }

private:
    HITLConfiguration* config;
    Ui::HITLOptionsPage* m_optionsPage;
};

#endif // HITLV2OPTIONSPAGE_H
