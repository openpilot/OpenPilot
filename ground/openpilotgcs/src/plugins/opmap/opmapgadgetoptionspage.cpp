/**
 ******************************************************************************
 *
 * @file       opmapgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin OpenPilot Map Plugin
 * @{
 * @brief The OpenPilot Map plugin 
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

#include "opmapgadgetoptionspage.h"
#include "opmapgadgetconfiguration.h"
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFileDialog>

#include "opmapcontrol/opmapcontrol.h"
#include "utils/pathutils.h"
#include "ui_opmapgadgetoptionspage.h"

// *********************************************

OPMapGadgetOptionsPage::OPMapGadgetOptionsPage(OPMapGadgetConfiguration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(config)
{
}

QWidget *OPMapGadgetOptionsPage::createPage(QWidget *parent)
{
    m_page = new Ui::OPMapGadgetOptionsPage();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);

    // populate the map provider combobox
    m_page->providerComboBox->clear();
    m_page->providerComboBox->addItems(mapcontrol::Helper::MapTypes());

    // populate the access mode combobox
    m_page->accessModeComboBox->clear();
    m_page->accessModeComboBox->addItems(mapcontrol::Helper::AccessModeTypes());

    int index = m_page->providerComboBox->findText(m_config->mapProvider());
    index = (index >= 0) ? index : 0;
    m_page->providerComboBox->setCurrentIndex(index);

    m_page->zoomSpinBox->setValue(m_config->zoom());
    m_page->latitudeSpinBox->setValue(m_config->latitude());
    m_page->longitudeSpinBox->setValue(m_config->longitude());

    m_page->checkBoxUseOpenGL->setChecked(m_config->useOpenGL());
    m_page->checkBoxShowTileGridLines->setChecked(m_config->showTileGridLines());

    index = m_page->accessModeComboBox->findText(m_config->accessMode());
    index = (index >= 0) ? index : 0;
    m_page->accessModeComboBox->setCurrentIndex(index);

    m_page->checkBoxUseMemoryCache->setChecked(m_config->useMemoryCache());

    m_page->lineEditCacheLocation->setExpectedKind(Utils::PathChooser::Directory);
    m_page->lineEditCacheLocation->setPromptDialogTitle(tr("Choose Cache Directory"));
    m_page->lineEditCacheLocation->setPath(m_config->cacheLocation());

    QDir dir(":/uavs/images/");
    QStringList list=dir.entryList();
    foreach(QString i,list)
    {
        QIcon icon(QPixmap(":/uavs/images/"+i));
        m_page->uavSymbolComboBox->addItem(icon,QString(),i);
    }
    for(int x=0;x<m_page->uavSymbolComboBox->count();++x)
    {
        if(m_page->uavSymbolComboBox->itemData(x).toString()==m_config->uavSymbol())
        {
            m_page->uavSymbolComboBox->setCurrentIndex(x);
        }
    }

    connect(m_page->pushButtonCacheDefaults, SIGNAL(clicked()), this, SLOT(on_pushButtonCacheDefaults_clicked()));

    return w;
}

void OPMapGadgetOptionsPage::on_pushButtonCacheDefaults_clicked()
{
    int index = m_page->accessModeComboBox->findText("ServerAndCache");
    index = (index >= 0) ? index : 0;
    m_page->accessModeComboBox->setCurrentIndex(index);

    m_page->checkBoxUseMemoryCache->setChecked(true);
    m_page->lineEditCacheLocation->setPath(Utils::PathUtils().GetStoragePath() + "mapscache" + QDir::separator());

}

void OPMapGadgetOptionsPage::apply()
{
    m_config->setMapProvider(m_page->providerComboBox->currentText());
    m_config->setZoom(m_page->zoomSpinBox->value());
    m_config->setLatitude(m_page->latitudeSpinBox->value());
    m_config->setLongitude(m_page->longitudeSpinBox->value());
    m_config->setUseOpenGL(m_page->checkBoxUseOpenGL->isChecked());
    m_config->setShowTileGridLines(m_page->checkBoxShowTileGridLines->isChecked());
    m_config->setAccessMode(m_page->accessModeComboBox->currentText());
    m_config->setUseMemoryCache(m_page->checkBoxUseMemoryCache->isChecked());
    m_config->setCacheLocation(m_page->lineEditCacheLocation->path());
    m_config->setUavSymbol(m_page->uavSymbolComboBox->itemData(m_page->uavSymbolComboBox->currentIndex()).toString());
}

void OPMapGadgetOptionsPage::finish()
{
    delete m_page;
}
