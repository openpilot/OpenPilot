/**
 ******************************************************************************
 *
 * @file       generalsettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#include "generalsettings.h"

#include <utils/stylehelper.h>
#include <utils/qtcolorbutton.h>
#include <utils/consoleprocess.h>
#include <coreplugin/icore.h>
#include <QtGui/QMessageBox>
#include <QtCore/QDir>

#include <QtCore/QLibraryInfo>
#include <QtCore/QSettings>

#include "ui_generalsettings.h"

using namespace Utils;
using namespace Core::Internal;

GeneralSettings::GeneralSettings():
    m_saveSettingsOnExit(true),
    m_dialog(0),
    m_autoConnect(true),
    m_autoSelect(true),
    m_useUDPMirror(false),
    m_useExpertMode(false)
{
}

QString GeneralSettings::id() const
{
    return QLatin1String("General");
}

QString GeneralSettings::trName() const
{
    return tr("General");
}

QString GeneralSettings::category() const
{
    return QLatin1String("Environment");
}

QString GeneralSettings::trCategory() const
{
    return tr("Environment");
}

static bool hasQmFilesForLocale(const QString &locale, const QString &creatorTrPath)
{
    static const QString qtTrPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    const QString trFile = QLatin1String("qt_") + locale + QLatin1String(".qm");
    return QFile::exists(qtTrPath+'/'+trFile) || QFile::exists(creatorTrPath+'/'+trFile);
}

void GeneralSettings::fillLanguageBox() const
{
    const QString currentLocale = language();

    m_page->languageBox->addItem(tr("<System Language>"), QString());
    // need to add this explicitly, since there is no qm file for English
    m_page->languageBox->addItem(QLatin1String("English"), QLatin1String("C"));
    if (currentLocale == QLatin1String("C"))
        m_page->languageBox->setCurrentIndex(m_page->languageBox->count() - 1);

    const QString creatorTrPath =
            Core::ICore::instance()->resourcePath() + QLatin1String("/translations");
    const QStringList languageFiles = QDir(creatorTrPath).entryList(QStringList(QLatin1String("openpilotgcs*.qm")));

    Q_FOREACH(const QString &languageFile, languageFiles)
    {
        int start = languageFile.indexOf(QLatin1Char('_'))+1;
        int end = languageFile.lastIndexOf(QLatin1Char('.'));
        const QString locale = languageFile.mid(start, end-start);
        // no need to show a language that creator will not load anyway
        if (hasQmFilesForLocale(locale, creatorTrPath)) {
            m_page->languageBox->addItem(QLocale::languageToString(QLocale(locale).language()), locale);
            if (locale == currentLocale)
                m_page->languageBox->setCurrentIndex(m_page->languageBox->count() - 1);
        }
    }
}

QWidget *GeneralSettings::createPage(QWidget *parent)
{
    m_page = new Ui::GeneralSettings();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);
    fillLanguageBox();
    connect(m_page->checkAutoConnect,SIGNAL(stateChanged(int)),this,SLOT(slotAutoConnect(int)));
    m_page->checkBoxSaveOnExit->setChecked(m_saveSettingsOnExit);
    m_page->checkAutoConnect->setChecked(m_autoConnect);
    m_page->checkAutoSelect->setChecked(m_autoSelect);
    m_page->cbUseUDPMirror->setChecked(m_useUDPMirror);
    m_page->cbExpertMode->setChecked(m_useExpertMode);
    m_page->colorButton->setColor(StyleHelper::baseColor());

    connect(m_page->resetButton, SIGNAL(clicked()),
            this, SLOT(resetInterfaceColor()));

    return w;
}

void GeneralSettings::apply()
{
    int currentIndex = m_page->languageBox->currentIndex();
    setLanguage(m_page->languageBox->itemData(currentIndex, Qt::UserRole).toString());
    // Apply the new base color if accepted
    StyleHelper::setBaseColor(m_page->colorButton->color());

    m_saveSettingsOnExit = m_page->checkBoxSaveOnExit->isChecked();
    m_useUDPMirror=m_page->cbUseUDPMirror->isChecked();
    m_useExpertMode=m_page->cbExpertMode->isChecked();
    m_autoConnect = m_page->checkAutoConnect->isChecked();
    m_autoSelect = m_page->checkAutoSelect->isChecked();
}

void GeneralSettings::finish()
{
    delete m_page;
}

void GeneralSettings::readSettings(QSettings* qs)
{
    qs->beginGroup(QLatin1String("General"));
    m_language = qs->value(QLatin1String("OverrideLanguage"),QLocale::system().name()).toString();
    m_saveSettingsOnExit = qs->value(QLatin1String("SaveSettingsOnExit"),m_saveSettingsOnExit).toBool();
    m_autoConnect = qs->value(QLatin1String("AutoConnect"),m_autoConnect).toBool();
    m_autoSelect = qs->value(QLatin1String("AutoSelect"),m_autoSelect).toBool();
    m_useUDPMirror = qs->value(QLatin1String("UDPMirror"),m_useUDPMirror).toBool();
    m_useExpertMode = qs->value(QLatin1String("ExpertMode"),m_useExpertMode).toBool();
    qs->endGroup();
}

void GeneralSettings::saveSettings(QSettings* qs)
{
    qs->beginGroup(QLatin1String("General"));

    if (m_language.isEmpty())
        qs->remove(QLatin1String("OverrideLanguage"));
    else
        qs->setValue(QLatin1String("OverrideLanguage"), m_language);

    qs->setValue(QLatin1String("SaveSettingsOnExit"), m_saveSettingsOnExit);
    qs->setValue(QLatin1String("AutoConnect"), m_autoConnect);
    qs->setValue(QLatin1String("AutoSelect"), m_autoSelect);
    qs->setValue(QLatin1String("UDPMirror"), m_useUDPMirror);
    qs->setValue(QLatin1String("ExpertMode"), m_useExpertMode);
    qs->endGroup();
}

void GeneralSettings::resetInterfaceColor()
{
    m_page->colorButton->setColor(0x666666);
}

void GeneralSettings::showHelpForExternalEditor()
{
    if (m_dialog) {
        m_dialog->show();
        m_dialog->raise();
        m_dialog->activateWindow();
        return;
    }
#if 0
    QMessageBox *mb = new QMessageBox(QMessageBox::Information,
                                  tr("Variables"),
                                  EditorManager::instance()->externalEditorHelpText(),
                                  QMessageBox::Cancel,
                                  m_page->helpExternalEditorButton);
    mb->setWindowModality(Qt::NonModal);
    m_dialog = mb;
    mb->show();
#endif
}

void GeneralSettings::resetLanguage()
{
    // system language is default
    m_page->languageBox->setCurrentIndex(0);
}

QString GeneralSettings::language() const
{
    return m_language;
}

void GeneralSettings::setLanguage(const QString &locale)
{
    if (m_language != locale) {
        if (!locale.isEmpty()) {
        QMessageBox::information((QWidget*)Core::ICore::instance()->mainWindow(), tr("Restart required"),
                                 tr("The language change will take effect after a restart of the OpenPilot GCS."));
        }
        m_language = locale;
    }
}

bool GeneralSettings::saveSettingsOnExit() const
{
    return m_saveSettingsOnExit;
}

bool GeneralSettings::autoConnect() const
{
    return m_autoConnect;
}

bool GeneralSettings::autoSelect() const
{
    return m_autoSelect;
}

bool GeneralSettings::useUDPMirror() const
{
    return m_useUDPMirror;
}

bool GeneralSettings::useExpertMode() const
{
    return m_useExpertMode;
}

void GeneralSettings::slotAutoConnect(int value)
{
    if (value==Qt::Checked)
        m_page->checkAutoSelect->setEnabled(false);
    else
        m_page->checkAutoSelect->setEnabled(true);
}
