/**
 ******************************************************************************
 *
 * @file       uavgadgetview.h
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

#ifndef UAVGADGETVIEW_H
#define UAVGADGETVIEW_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QAction>
#include <QtGui/QSplitter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QStackedLayout>
#include <QtCore/QPointer>


QT_BEGIN_NAMESPACE
class QComboBox;
class QToolButton;
class QLabel;
class QVBoxLayout;
QT_END_NAMESPACE

namespace Utils {
class StyledBar;
}

namespace Core {

class IUAVGadget;
class UAVGadgetManager;

namespace Internal {


class UAVGadgetView : public QWidget
{
    Q_OBJECT

public:
    UAVGadgetView(UAVGadgetManager *uavGadgetManager, IUAVGadget *uavGadget = 0, QWidget *parent = 0);
    virtual ~UAVGadgetView();

    IUAVGadget *gadget() const;
    void setGadget(IUAVGadget *uavGadget);
    bool hasGadget(IUAVGadget *uavGadget) const;
    void removeGadget();

    void showToolbar(bool show);

    void saveState(QSettings *qSettings);
    void restoreState(QSettings *qSettings);

public slots:
    void closeView();

private slots:
    void listSelectionActivated(int index);
    void currentGadgetChanged(IUAVGadget *gadget);

private:
    int indexOfClassId(QString classId);
    void updateToolBar();

    QPointer<UAVGadgetManager> m_uavGadgetManager;
    QPointer<IUAVGadget> m_uavGadget;
    QWidget *m_toolBar;
    QComboBox *m_defaultToolBar;
    QWidget *m_currentToolBar;
    QWidget *m_activeToolBar;
    QComboBox *m_uavGadgetList;
    QToolButton *m_closeButton;
    Utils::StyledBar *m_top;
    QVBoxLayout *tl; // top layout
    int m_defaultIndex;
    QLabel *m_activeLabel;
};

}
}
#endif // UAVGADGETVIEW_H
