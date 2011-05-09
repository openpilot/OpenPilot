/**
 ******************************************************************************
 *
 * @file       communitywelcomepagewidget.cpp
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

#include "communitywelcomepagewidget.h"
#include "ui_communitywelcomepagewidget.h"

#include "rssfetcher.h"

#include <QtCore/QMap>
#include <QtGui/QDesktopServices>
#include <QtGui/QTreeWidgetItem>

namespace Welcome {
namespace Internal {
CommunityWelcomePageWidget::CommunityWelcomePageWidget(QWidget *parent) :
    QWidget(parent),
    m_rssFetcher(new RSSFetcher(7)),
    ui(new Ui::CommunityWelcomePageWidget)
{
    ui->setupUi(this);
    ui->labsTitleLabel->setStyledText(tr("News From the OpenPilot Project"));
    ui->sitesTitleLabel->setStyledText(tr("OpenPilot Websites"));

    connect(ui->newsTreeWidget, SIGNAL(activated(QString)), SLOT(slotUrlClicked(QString)));
    connect(ui->sitesTreeWidget, SIGNAL(activated(QString)), SLOT(slotUrlClicked(QString)));

    connect(m_rssFetcher, SIGNAL(newsItemReady(QString, QString, QString)),
        ui->newsTreeWidget, SLOT(slotAddNewsItem(QString, QString, QString)));
    //: Add localized feed here only if one exists
    m_rssFetcher->fetch(QUrl(tr("http://www.openpilot.org/feed/")));

    QList<QPair<QString, QString> > sites;
    sites << qMakePair(tr("OpenPilot Home"), QString(QLatin1String("http://www.openpilot.org")));
    sites << qMakePair(tr("OpenPilot Wiki"), QString(QLatin1String("http://wiki.openpilot.org")));
    sites << qMakePair(tr("OpenPilot Store"), QString(QLatin1String("http://www.openpilot.org/hardware/get-hardware/")));
    sites << qMakePair(tr("OpenPilot Forums"), QString(QLatin1String("http://forums.openpilot.org")));
    sites << qMakePair(tr("OpenPilot Code Reviews"), QString(QLatin1String("http://git.openpilot.org")));    
    sites << qMakePair(tr("OpenPilot Progress Tracker"), QString(QLatin1String("http://progress.openpilot.org")));    
    
    QListIterator<QPair<QString, QString> > it(sites);
    while (it.hasNext()) {
        QPair<QString, QString> pair = it.next();
        ui->sitesTreeWidget->addItem(pair.first, pair.second, pair.second);
    }

}


CommunityWelcomePageWidget::~CommunityWelcomePageWidget()
{
    delete m_rssFetcher;
    delete ui;
}

void CommunityWelcomePageWidget::slotUrlClicked(const QString &data)
{
    QDesktopServices::openUrl(QUrl(data));
}

} // namespace Internal
} // namespace Welcome
