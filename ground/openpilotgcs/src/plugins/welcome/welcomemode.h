/**
 ******************************************************************************
 *
 * @file       welcomemode.h
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

#ifndef WELCOMEMODE_H
#define WELCOMEMODE_H

#include "welcome_global.h"

#include <coreplugin/imode.h>


QT_BEGIN_NAMESPACE
class QWidget;
class QUrl;
QT_END_NAMESPACE

namespace Welcome {

struct WelcomeModePrivate;

class WELCOME_EXPORT WelcomeMode : public Core::IMode
{
    Q_OBJECT

public:
    WelcomeMode();
    ~WelcomeMode();

    // IMode
    QString name() const;
    QIcon icon() const;
    int priority() const;
    QWidget *widget();
    const char *uniqueModeName() const;
    QList<int> context() const;
    void activated();
    QString contextHelpId() const { return QLatin1String("OpenPilot GCS"); }
    void setPriority(int priority) { m_priority = priority; }

public slots:
    void openUrl(const QString &url);
    void openPage(const QString &page);
    void triggerAction(const QString &actionId);

private:
    WelcomeModePrivate *m_d;
    int m_priority;
};

} // namespace Welcome

#endif // WELCOMEMODE_H
