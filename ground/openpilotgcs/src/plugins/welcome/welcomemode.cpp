/**
 ******************************************************************************
 *
 * @file       welcomemode.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup WelcomePlugin Welcome Plugin
 * @{
 * @brief The GCS Welcome plugin
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


#include "welcomemode.h"
#include <extensionsystem/pluginmanager.h>

#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/modemanager.h>

#include <utils/styledbar.h>
#include <utils/welcomemodetreewidget.h>
#include <utils/iwelcomepage.h>

#include <QtGui/QDesktopServices>

#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtCore/QDebug>

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/qdeclarativeview.h>
#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativecontext.h>

#include <cstdlib>

using namespace ExtensionSystem;
using namespace Utils;

namespace Welcome {

struct WelcomeModePrivate
{
    WelcomeModePrivate();

    QDeclarativeView *declarativeView;
};

WelcomeModePrivate::WelcomeModePrivate()
{
}

// ---  WelcomeMode
WelcomeMode::WelcomeMode() :
    m_d(new WelcomeModePrivate),
    m_priority(Core::Constants::P_MODE_WELCOME)
{
    m_name = tr("Welcome");
    m_qmlPath = QLatin1String("qrc:/welcome/qml/main.qml");
    m_icon = QIcon(QLatin1String(":/core/images/openpilot_logo_64.png"));
    m_d->declarativeView = new QDeclarativeView;
    m_d->declarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    m_d->declarativeView->engine()->rootContext()->setContextProperty("welcomePlugin", this);
    m_d->declarativeView->setSource(QUrl(m_qmlPath));
}

WelcomeMode::~WelcomeMode()
{
    delete m_d->declarativeView;
    delete m_d;
}

QString WelcomeMode::name() const
{
    return m_name;
}

void WelcomeMode::setName(QString name)
{
    m_name = name;
}

QIcon WelcomeMode::icon() const
{
    return m_icon;
}

void WelcomeMode::setIcon(QIcon icon)
{
    m_icon = icon;
}

QString WelcomeMode::qmlPath() const
{
    return m_qmlPath;
}

void WelcomeMode::setQmlPath(QString path)
{
    m_qmlPath = path;
    m_d->declarativeView->setSource(QUrl(m_qmlPath));
}

int WelcomeMode::priority() const
{
    return m_priority;
}

QWidget* WelcomeMode::widget()
{
    return m_d->declarativeView;
}

const char* WelcomeMode::uniqueModeName() const
{
    return Core::Constants::MODE_WELCOME;
}

QList<int> WelcomeMode::context() const
{
    static QList<int> contexts = QList<int>()
                                 << Core::UniqueIDManager::instance()->uniqueIdentifier(Core::Constants::C_WELCOME_MODE);
    return contexts;
}

void WelcomeMode::openUrl(const QString &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

void WelcomeMode::openPage(const QString &page)
{
    Core::ModeManager::instance()->activateMode(page);
}

} // namespace Welcome
