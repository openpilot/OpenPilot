/**
 ******************************************************************************
 *
 * @file       multipage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup MultiPage
 * @{
 * @brief
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

#include "selectionpage.h"
#include "ui_selectionpage.h"
#include "setupwizard.h"

SelectionPage::SelectionPage(SetupWizard *wizard, QString shapeFile, QWidget *parent) :
    AbstractWizardPage(wizard, parent), Selection(),
    ui(new Ui::SelectionPage)
{
    ui->setupUi(this);

    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(shapeFile);
    m_shape = new QGraphicsSvgItem();
    m_shape->setSharedRenderer(renderer);
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(m_shape);
    ui->typeGraphicsView->setScene(scene);

    connect(ui->typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(selectionChanged(int)));
}

SelectionPage::~SelectionPage()
{
    while (!m_selectionItems.empty()) {
        delete m_selectionItems.takeFirst();
    }
    delete ui;
}

void SelectionPage::initializePage()
{
    // lazy init
    if (m_selectionItems.isEmpty()) {
        setupSelection(this);
        foreach(SelectionItem * item, m_selectionItems) {
            ui->typeCombo->addItem(item->name());
        }

        // Default to first item if any
        if (ui->typeCombo->count() > 0) {
            ui->typeCombo->setCurrentIndex(0);
        }
    }
    initializePage(getWizard());
}

bool SelectionPage::validatePage()
{
    return validatePage(m_selectionItems.at(ui->typeCombo->currentIndex()));
}

void SelectionPage::fitImage()
{
    if (m_shape) {
        ui->typeGraphicsView->setSceneRect(m_shape->boundingRect());
        ui->typeGraphicsView->fitInView(m_shape, Qt::KeepAspectRatio);
    }
}

void SelectionPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    fitImage();
}

void SelectionPage::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    fitImage();
}

void SelectionPage::selectionChanged(int index)
{
    SelectionItem *item = m_selectionItems.at(index);

    m_shape->setElementId(item->shapeId());
    ui->typeDescription->setText(item->description());
    fitImage();
}

void SelectionPage::addItem(QString name, QString description, QString shapeId, int id, bool disabled)
{
    m_selectionItems << new SelectionItem(name, description, shapeId, id, disabled);
}

void SelectionPage::setTitle(QString title)
{
    ui->label->setText(title);
}

void SelectionPage::setText(QString text)
{
    ui->text->setText(text);
}

void SelectionPage::setItemDisabled(int id, bool disabled)
{
    for (int i = 0; i < m_selectionItems.count(); i++) {
        if (id < 0 || m_selectionItems.at(i)->id() == id) {
            m_selectionItems.at(i)->setDisabled(disabled);
            ui->typeCombo->setItemData(i, disabled ? 0 : 33, Qt::UserRole - 1);
        }
    }
}

void SelectionPage::setSelectedItem(int id)
{
    for (int i = 0; i < m_selectionItems.count(); i++) {
        if (m_selectionItems.at(i)->id() == id) {
            ui->typeCombo->setCurrentIndex(i);
        }
    }
}

SelectionItem *SelectionPage::getSelectedItem()
{
    return m_selectionItems.at(ui->typeCombo->currentIndex());
}

SelectionItem::SelectionItem(QString name, QString description, QString shapeId, int id, bool disabled) :
    m_name(name), m_description(description), m_shapeId(shapeId), m_id(id), m_disabled(disabled)
{}
