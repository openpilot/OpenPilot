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

#include <QtGui/QMouseEvent>
#include <QtGui/QScrollArea>
#include <QtGui/QButtonGroup>
#include <QtGui/QDesktopServices>
#include <QtGui/QToolButton>

#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtCore/QDebug>

#include <cstdlib>

#include "ui_welcomemode.h"

using namespace ExtensionSystem;
using namespace Utils;

namespace Welcome {

struct WelcomeModePrivate
{
    WelcomeModePrivate();

    QScrollArea *m_scrollArea;
    QWidget *m_widget;
    QWidget *m_welcomePage;
    QMap<QAbstractButton*, QWidget*> buttonMap;
    QHBoxLayout * buttonLayout;
    Ui::WelcomeMode ui;
    int currentTip;
};

WelcomeModePrivate::WelcomeModePrivate()
{
}

// ---  WelcomeMode
WelcomeMode::WelcomeMode() :
    m_d(new WelcomeModePrivate),
    m_priority(Core::Constants::P_MODE_WELCOME)
{
    m_d->m_widget = new QWidget;
    QVBoxLayout *l = new QVBoxLayout(m_d->m_widget);
    l->setMargin(0);
    l->setSpacing(0);
//    l->addWidget(new Utils::StyledBar(m_d->m_widget));
    m_d->m_welcomePage = new QWidget(m_d->m_widget);
    m_d->ui.setupUi(m_d->m_welcomePage);
    m_d->ui.helpUsLabel->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_d->ui.feedbackButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    l->addWidget(m_d->m_welcomePage);

    m_d->m_scrollArea = new QScrollArea;
    m_d->m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_d->m_scrollArea->setWidget(m_d->m_widget);
    m_d->m_scrollArea->setWidgetResizable(true);

    PluginManager *pluginManager = PluginManager::instance();
    connect(pluginManager, SIGNAL(objectAdded(QObject*)), SLOT(welcomePluginAdded(QObject*)));

    connect(m_d->ui.feedbackButton, SIGNAL(clicked()), SLOT(slotFeedback()));

}

WelcomeMode::~WelcomeMode()
{
    // QSettings *settings = Core::ICore::instance()->settings();
    // settings->setValue("General/WelcomeTab", m_d->ui.stackedWidget->currentIndex());
    delete m_d->m_widget;
    delete m_d;
}

QString WelcomeMode::name() const
{
    return tr("Welcome");
}

QIcon WelcomeMode::icon() const
{
    return QIcon(QLatin1String(":/core/images/openpilot_logo_64.png"));
}

int WelcomeMode::priority() const
{
    return m_priority;
}

QWidget* WelcomeMode::widget()
{
    return m_d->m_scrollArea;
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

bool sortFunction(IWelcomePage * a, IWelcomePage *b)
{
    return a->priority() < b->priority();
}

void WelcomeMode::initPlugins()
{
    m_d->buttonLayout = new QHBoxLayout(m_d->ui.navFrame);
    m_d->buttonLayout->setMargin(0);
    m_d->buttonLayout->setSpacing(0);
    delete m_d->ui.stackedWidget->currentWidget();
    QList<IWelcomePage*> plugins = PluginManager::instance()->getObjects<IWelcomePage>();
    qSort(plugins.begin(), plugins.end(), &sortFunction);
    foreach (IWelcomePage* plugin, plugins) {
//        QToolButton * btn = new QToolButton;
//        btn->setCheckable(true);
//        btn->setText(plugin->title());
//        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
//        btn->setAutoExclusive(true);
//        connect (btn, SIGNAL(clicked()), SLOT(showClickedPage()));
        m_d->ui.stackedWidget->addWidget(plugin->page());
//        m_d->buttonLayout->addWidget(btn);
//        m_d->buttonMap.insert(btn, plugin->page());
    }
    m_d->buttonLayout->addSpacing(5);

    // TODO, This probably can beremoved. See OP-310
    QSettings *settings = Core::ICore::instance()->settings();
    int tabId = settings->value("General/WelcomeTab", 0).toInt();

    int pluginCount = m_d->ui.stackedWidget->count();
    if (tabId < pluginCount) {
        m_d->ui.stackedWidget->setCurrentIndex(tabId);
        QMapIterator<QAbstractButton*, QWidget*> it(m_d->buttonMap);
        while (it.hasNext())
            if (it.next().value() == m_d->ui.stackedWidget->currentWidget()) {
                it.key()->setChecked(true);
                break;
            }
    }
    // TODO Delete until here

}

void WelcomeMode::welcomePluginAdded(QObject *obj)
{
    if (IWelcomePage *plugin = qobject_cast<IWelcomePage*>(obj))
    {
        QToolButton * btn = new QToolButton;
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setText(plugin->title());
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        connect (btn, SIGNAL(clicked()), SLOT(showClickedPage()));
        int insertPos = 0;
        QList<IWelcomePage*> plugins = PluginManager::instance()->getObjects<IWelcomePage>();
        foreach (IWelcomePage* p, plugins) {
            if (plugin->priority() < p->priority())
                insertPos++;
            else
                break;
        }
        m_d->ui.stackedWidget->insertWidget(insertPos, plugin->page());
        m_d->buttonMap.insert(btn, plugin->page());
        m_d->buttonLayout->insertWidget(insertPos, btn);
    }
}

void WelcomeMode::showClickedPage()
{
    QAbstractButton *btn = qobject_cast<QAbstractButton*>(sender());
    QMap<QAbstractButton*, QWidget*>::iterator it = m_d->buttonMap.find(btn);
    if (it.value())
        m_d->ui.stackedWidget->setCurrentWidget(it.value());
}

void WelcomeMode::slotFeedback()
{
    QDesktopServices::openUrl(QUrl(QLatin1String(
        "http://forums.openpilot.org")));
}


} // namespace Welcome
