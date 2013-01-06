/**
 ******************************************************************************
 *
 * @file       abstractwizard.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Setup Wizard  Plugin
 * @{
 * @brief An abstract Wizard class to base wizards on.
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

#ifndef ABSTRACTWIZARD_H
#define ABSTRACTWIZARD_H

#include <QWizard>
#include <coreplugin/icore.h>
#include <coreplugin/connectionmanager.h>

class AbstractWizard : public QWizard
{
    Q_OBJECT

public:
    AbstractWizard(QWidget *parent = 0);

    Core::ConnectionManager* getConnectionManager() {
        if (!m_connectionManager) {
            m_connectionManager = Core::ICore::instance()->connectionManager();
            Q_ASSERT(m_connectionManager);
        }
        return m_connectionManager;
    }

private:
    Core::ConnectionManager *m_connectionManager;

};

#endif // ABSTRACTWIZARD_H
