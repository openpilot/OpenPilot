/**
 ******************************************************************************
 *
 * @file       qmlviewgadget.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin QML Viewer Plugin
 * @{
 * @brief The QML Viewer Gadget
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

#ifndef QMLVIEWGADGET_H_
#define QMLVIEWQMLGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "qmlviewgadgetwidget.h"

class IUAVGadget;
class QWidget;
class QString;
class QmlViewGadgetWidget;

using namespace Core;

class QmlViewGadget : public Core::IUAVGadget {
    Q_OBJECT
public:
    QmlViewGadget(QString classId, QmlViewGadgetWidget *widget, QWidget *parent = 0);
    ~QmlViewGadget();

    QWidget *widget()
    {
        if (!m_container) {
            m_container = QWidget::createWindowContainer(m_widget, m_parent);
            m_container->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            m_container->setMinimumSize(64, 64);
        }
        return m_container;
    }
    void loadConfiguration(IUAVGadgetConfiguration *config);

private:
    QWidget *m_container;
    QWidget *m_parent;
    QmlViewGadgetWidget *m_widget;
};


#endif // QMLVIEWQMLGADGET_H_
