/**
 ******************************************************************************
 *
 * @file       uavgadgetview.cpp
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

#include "uavgadgetview.h"
#include "uavgadgetmanager.h"
#include "uavgadgetinstancemanager.h"
#include "iuavgadget.h"
#include "coreimpl.h"
#include "minisplitter.h"
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>

#include <utils/qtcassert.h>
#include <utils/styledbar.h>

#include <QtCore/QDebug>

#include <QtWidgets/QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QMenu>
#include <QClipboard>

#ifdef Q_WS_MAC
#include <qmacstyle_mac.h>
#endif

Q_DECLARE_METATYPE(Core::IUAVGadget *)

using namespace Core;
using namespace Core::Internal;

UAVGadgetView::UAVGadgetView(Core::UAVGadgetManager *uavGadgetManager, IUAVGadget *uavGadget, QWidget *parent) :
    QWidget(parent),
    m_uavGadgetManager(uavGadgetManager),
    m_uavGadget(uavGadget),
    m_toolBar(new QWidget(this)),
    m_defaultToolBar(new QComboBox(this)),
    m_uavGadgetList(new QComboBox(this)),
    m_closeButton(new QToolButton(this)),
    m_defaultIndex(0),
    m_activeLabel(new QLabel)
{
    tl = new QVBoxLayout(this);
    tl->setSpacing(0);
    tl->setMargin(0);
    {
        m_uavGadgetList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_uavGadgetList->setMinimumContentsLength(15);
        m_uavGadgetList->setMaxVisibleItems(40);
        m_uavGadgetList->setContextMenuPolicy(Qt::CustomContextMenu);
        UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
        QStringList sl    = im->classIds();
        int index = 0;
        bool startFromOne = false;
        foreach(QString classId, sl) {
            if (classId == QString("EmptyGadget")) {
                m_defaultIndex = 0;
                startFromOne   = true;
                m_uavGadgetList->insertItem(0, im->gadgetName(classId), classId);
                m_uavGadgetList->setItemIcon(0, im->gadgetIcon(classId));
                m_uavGadgetList->insertSeparator(1);
            } else {
                int i = startFromOne ? 1 : 0;
                for (; i < m_uavGadgetList->count(); i++) {
                    if (QString::localeAwareCompare(m_uavGadgetList->itemText(i), im->gadgetName(classId)) > 0) {
                        break;
                    }
                }
                m_uavGadgetList->insertItem(i, im->gadgetName(classId), classId);
                m_uavGadgetList->setItemIcon(i, im->gadgetIcon(classId));
            }
            ++index;
        }

        m_defaultToolBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        m_activeToolBar = m_defaultToolBar;

        QHBoxLayout *toolBarLayout = new QHBoxLayout(m_toolBar);
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        toolBarLayout->addWidget(m_defaultToolBar);
        m_toolBar->setLayout(toolBarLayout);
        m_toolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

        QWidget *spacerWidget = new QWidget(this);
        spacerWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        m_activeLabel->setTextFormat(Qt::RichText);
        m_activeLabel->setText("<font color=red><b>" + tr("Active") + "</b></font>");

        m_closeButton->setAutoRaise(true);
        m_closeButton->setIcon(QIcon(":/core/images/closebutton.png"));

        m_top = new Utils::StyledBar(this);
        m_top->setMaximumHeight(35);
        QHBoxLayout *toplayout = new QHBoxLayout(m_top);
        toplayout->setSpacing(4);
        toplayout->setMargin(4);
        toplayout->addWidget(m_uavGadgetList);
        toplayout->addWidget(m_toolBar); // Custom toolbar stretches
        toplayout->addWidget(spacerWidget);
        toplayout->addWidget(m_activeLabel);
        toplayout->addWidget(m_closeButton);

        m_top->setLayout(toplayout);
        tl->addWidget(m_top);

        connect(m_uavGadgetList, SIGNAL(activated(int)), this, SLOT(listSelectionActivated(int)));
        connect(m_closeButton, SIGNAL(clicked()), this, SLOT(closeView()), Qt::QueuedConnection);
        connect(m_uavGadgetManager, SIGNAL(currentGadgetChanged(IUAVGadget *)), this, SLOT(currentGadgetChanged(IUAVGadget *)));
    }
    if (m_uavGadget) {
        setGadget(m_uavGadget);
    } else {
        listSelectionActivated(m_defaultIndex);
    }
}

UAVGadgetView::~UAVGadgetView()
{
    removeGadget();
}

bool UAVGadgetView::hasGadget(IUAVGadget *uavGadget) const
{
    return m_uavGadget == uavGadget;
}

void UAVGadgetView::showToolbar(bool show)
{
    m_top->setHidden(!show);
}

void UAVGadgetView::closeView()
{
    m_uavGadgetManager->closeView(this);
}

void UAVGadgetView::removeGadget()
{
    if (!m_uavGadget) {
        return;
    }
    tl->removeWidget(m_uavGadget->widget());

    m_uavGadget->setParent(0);
    m_uavGadget->widget()->setParent(0);
    QWidget *toolBar = m_uavGadget->toolBar();
    if (toolBar != 0) {
        if (m_activeToolBar == toolBar) {
            m_activeToolBar = m_defaultToolBar;
            m_activeToolBar->setVisible(true);
        }
        m_toolBar->layout()->removeWidget(toolBar);
        toolBar->setParent(0);
    }
    m_uavGadget = 0;
}

IUAVGadget *UAVGadgetView::gadget() const
{
    return m_uavGadget;
}

void UAVGadgetView::setGadget(IUAVGadget *uavGadget)
{
    if (!uavGadget) {
        return;
    }
    removeGadget();
    m_uavGadget = uavGadget;
    tl->addWidget(m_uavGadget->widget());
    m_uavGadget->widget()->setParent(this);
    m_uavGadget->widget()->show();
    int index = indexOfClassId(m_uavGadget->classId());
    Q_ASSERT(index >= 0);
    m_uavGadgetList->setCurrentIndex(index);

    updateToolBar();
}

void UAVGadgetView::updateToolBar()
{
    if (!m_uavGadget) {
        return;
    }
    QComboBox *toolBar = m_uavGadget->toolBar();
    if (!toolBar) {
        toolBar = m_defaultToolBar;
    }
    if (m_activeToolBar == toolBar) {
        return;
    }
    if (toolBar->count() == 0) {
        toolBar->hide();
    }
    m_toolBar->layout()->addWidget(toolBar);
    m_activeToolBar->setVisible(false);
    m_activeToolBar = toolBar;
}

void UAVGadgetView::listSelectionActivated(int index)
{
    if (index < 0 || index >= m_uavGadgetList->count()) {
        index = m_defaultIndex;
    }

    QString classId = m_uavGadgetList->itemData(index).toString();
    if (m_uavGadget && (m_uavGadget->classId() == classId)) {
        return;
    }

    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    IUAVGadget *gadget = im->createGadget(classId, this);

    IUAVGadget *gadgetToRemove   = m_uavGadget;
    setGadget(gadget);
    m_uavGadgetManager->setCurrentGadget(gadget);
    im->removeGadget(gadgetToRemove);
}

int UAVGadgetView::indexOfClassId(QString classId)
{
    return m_uavGadgetList->findData(classId);
}

void UAVGadgetView::currentGadgetChanged(IUAVGadget *gadget)
{
    m_activeLabel->setVisible(m_uavGadget == gadget);
}

void UAVGadgetView::saveState(QSettings *qSettings)
{
    qSettings->setValue("type", "uavGadget");
    qSettings->setValue("classId", gadget()->classId());
    qSettings->beginGroup("gadget");
    gadget()->saveState(qSettings);
    qSettings->endGroup();
}

void UAVGadgetView::restoreState(QSettings *qSettings)
{
    QString classId = qSettings->value("classId").toString();
    int index = indexOfClassId(classId);

    if (index < 0) {
        index = m_defaultIndex;
    }
    classId = m_uavGadgetList->itemData(index).toString();

    IUAVGadget *newGadget;
    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    if (qSettings->childGroups().contains("gadget")) {
        newGadget = im->createGadget(classId, this, false);
        qSettings->beginGroup("gadget");
        newGadget->restoreState(qSettings);
        qSettings->endGroup();
    } else {
        newGadget = im->createGadget(classId, this);
    }

    IUAVGadget *gadgetToRemove = m_uavGadget;
    setGadget(newGadget);
    m_uavGadgetManager->setCurrentGadget(newGadget);
    im->removeGadget(gadgetToRemove);
}
