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

#ifndef PFDQMLGADGET_H_
#define PFDQMLQMLGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "pfdqmlgadgetwidget.h"

class IUAVGadget;
class QWidget;
class QString;
class PfdQmlGadgetWidget;

using namespace Core;

class PfdQmlGadget : public Core::IUAVGadget {
    Q_OBJECT
public:
    PfdQmlGadget(QString classId, PfdQmlGadgetWidget *widget, QWidget *parent = 0);
    ~PfdQmlGadget();

    QWidget *widget()
    {
        if (!m_container) {
            m_container = QWidget::createWindowContainer(m_widget, m_parent);
            m_container->setMinimumSize(64, 64);
            m_container->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        }
        return m_container;
    }
    void loadConfiguration(IUAVGadgetConfiguration *config);

private:
    QWidget *m_container;
    QWidget *m_parent;
    PfdQmlGadgetWidget *m_widget;
};


#endif // PFDQMLQMLGADGET_H_
