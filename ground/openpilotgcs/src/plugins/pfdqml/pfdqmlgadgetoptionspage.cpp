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

#include "pfdqmlgadgetoptionspage.h"
#include "pfdqmlgadgetconfiguration.h"
#include "ui_pfdqmlgadgetoptionspage.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavdataobject.h"


#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

PfdQmlGadgetOptionsPage::PfdQmlGadgetOptionsPage(PfdQmlGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *PfdQmlGadgetOptionsPage::createPage(QWidget *parent)
{
    options_page = new Ui::PfdQmlGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget(parent);
    //main layout
    options_page->setupUi(optionsPageWidget);

    // Restore the contents from the settings:
    options_page->qmlSourceFile->setExpectedKind(Utils::PathChooser::File);
    options_page->qmlSourceFile->setPromptDialogFilter(tr("QML file (*.qml)"));
    options_page->qmlSourceFile->setPromptDialogTitle(tr("Choose QML file"));
    options_page->qmlSourceFile->setPath(m_config->qmlFile());

    // Restore the contents from the settings:
    options_page->earthFile->setExpectedKind(Utils::PathChooser::File);
    options_page->earthFile->setPromptDialogFilter(tr("OsgEarth (*.earth)"));
    options_page->earthFile->setPromptDialogTitle(tr("Choose OsgEarth terrain file"));
    options_page->earthFile->setPath(m_config->earthFile());

    options_page->useOpenGL->setChecked(m_config->openGLEnabled());
    options_page->showTerrain->setChecked(m_config->terrainEnabled());

    options_page->useActualLocation->setChecked(m_config->actualPositionUsed());
    options_page->usePredefinedLocation->setChecked(!m_config->actualPositionUsed());
    options_page->latitude->setText(QString::number(m_config->latitude()));
    options_page->longitude->setText(QString::number(m_config->longitude()));
    options_page->altitude->setText(QString::number(m_config->altitude()));
    options_page->useOnlyCache->setChecked(m_config->cacheOnly());

#ifndef USE_OSG
    options_page->showTerrain->setChecked(false);
    options_page->showTerrain->setVisible(false);
#endif

    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void PfdQmlGadgetOptionsPage::apply()
{
    m_config->setQmlFile(options_page->qmlSourceFile->path());
    m_config->setEarthFile(options_page->earthFile->path());
    m_config->setOpenGLEnabled(options_page->useOpenGL->isChecked());

#ifdef USE_OSG
    m_config->setTerrainEnabled(options_page->showTerrain->isChecked());
#else
    m_config->setTerrainEnabled(false);
#endif

    m_config->setActualPositionUsed(options_page->useActualLocation->isChecked());
    m_config->setLatitude(options_page->latitude->text().toDouble());
    m_config->setLongitude(options_page->longitude->text().toDouble());
    m_config->setAltitude(options_page->altitude->text().toDouble());
    m_config->setCacheOnly(options_page->useOnlyCache->isChecked());
}

void PfdQmlGadgetOptionsPage::finish()
{
}
