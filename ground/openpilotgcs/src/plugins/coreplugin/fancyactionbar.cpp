/**
 ******************************************************************************
 *
 * @file       fancyactionbar.cpp
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

#include "fancyactionbar.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QPainter>
#include <QtGui/QPicture>
#include <QtGui/QVBoxLayout>
#include <QtSvg/QSvgRenderer>
#include <QtGui/QAction>

using namespace Core;
using namespace Internal;

static const char* const svgIdButtonBase =               "ButtonBase";
static const char* const svgIdButtonNormalBase =         "ButtonNormalBase";
static const char* const svgIdButtonNormalOverlay =      "ButtonNormalOverlay";
static const char* const svgIdButtonPressedBase =        "ButtonPressedBase";
static const char* const svgIdButtonPressedOverlay =     "ButtonPressedOverlay";
static const char* const svgIdButtonDisabledOverlay =    "ButtonDisabledOverlay";
static const char* const svgIdButtonHoverOverlay =       "ButtonHoverOverlay";

static const char* const elementsSvgIds[] = {
    svgIdButtonBase,
    svgIdButtonNormalBase,
    svgIdButtonNormalOverlay,
    svgIdButtonPressedBase,
    svgIdButtonPressedOverlay,
    svgIdButtonDisabledOverlay,
    svgIdButtonHoverOverlay
};

const QMap<QString, QPicture> &buttonElementsMap()
{
    static QMap<QString, QPicture> result;
    if (result.isEmpty()) {
        QSvgRenderer renderer(QLatin1String(":/fancyactionbar/images/fancytoolbutton.svg"));
        for (size_t i = 0; i < sizeof(elementsSvgIds)/sizeof(elementsSvgIds[0]); i++) {
            QString elementId(elementsSvgIds[i]);
            QPicture elementPicture;
            QPainter elementPainter(&elementPicture);
            renderer.render(&elementPainter, elementId);
            result.insert(elementId, elementPicture);
        }
    }
    return result;
}

FancyToolButton::FancyToolButton(QWidget *parent)
    : QToolButton(parent)
    , m_buttonElements(buttonElementsMap())
{
    setAttribute(Qt::WA_Hover, true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void FancyToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    QSize sh(sizeHint());
    double scale = (double)height() / sh.height();
    if (scale < 1) {
        p.save();
        p.scale(1, scale);
    }
    p.drawPicture(0, 0, m_buttonElements.value(svgIdButtonBase));
    p.drawPicture(0, 0, m_buttonElements.value(isDown() ? svgIdButtonPressedBase : svgIdButtonNormalBase));
#ifndef Q_WS_MAC // Mac UIs usually don't hover
    if (underMouse() && isEnabled())
        p.drawPicture(0, 0, m_buttonElements.value(svgIdButtonHoverOverlay));
#endif

    if (scale < 1)
        p.restore();

    if (!icon().isNull()) {
        icon().paint(&p, rect());
    } else {
        const int margin = 4;
        p.drawText(rect().adjusted(margin, margin, -margin, -margin), Qt::AlignCenter | Qt::TextWordWrap, text());

    }

    if (scale < 1) {
        p.scale(1, scale);
    }

    if (isEnabled()) {
        p.drawPicture(0, 0, m_buttonElements.value(isDown() ?
                                                   svgIdButtonPressedOverlay : svgIdButtonNormalOverlay));
    } else {
        p.drawPicture(0, 0, m_buttonElements.value(svgIdButtonDisabledOverlay));
    }
}

void FancyActionBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
}

QSize FancyToolButton::sizeHint() const
{
    return m_buttonElements.value(svgIdButtonBase).boundingRect().size();
}

QSize FancyToolButton::minimumSizeHint() const
{
    return QSize(8, 8);
}

FancyActionBar::FancyActionBar(QWidget *parent)
    : QWidget(parent)
{
    m_actionsLayout = new QVBoxLayout;

    QHBoxLayout *centeringLayout = new QHBoxLayout;
    centeringLayout->addStretch();
    centeringLayout->addLayout(m_actionsLayout);
    centeringLayout->addStretch();
    setLayout(centeringLayout);
}

void FancyActionBar::insertAction(int index, QAction *action, QMenu *menu)
{
    FancyToolButton *toolButton = new FancyToolButton(this);
    toolButton->setDefaultAction(action);
    if (menu) {
        toolButton->setMenu(menu);
        toolButton->setPopupMode(QToolButton::DelayedPopup);

        // execute action also if a context menu item is select
        connect(toolButton, SIGNAL(triggered(QAction*)),
                this, SLOT(toolButtonContextMenuActionTriggered(QAction*)),
                Qt::QueuedConnection);
    }
    m_actionsLayout->insertWidget(index, toolButton);
}

/*
  This slot is invoked when a context menu action of a tool button is triggered.
  In this case we also want to trigger the default action of the button.

  This allows the user e.g. to select and run a specific run configuration with one click.
  */
void FancyActionBar::toolButtonContextMenuActionTriggered(QAction* action)
{
    if (QToolButton *button = qobject_cast<QToolButton*>(sender())) {
        if (action != button->defaultAction())
            button->defaultAction()->trigger();
    }
}
