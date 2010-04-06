/**
 ******************************************************************************
 *
 * @file       uavgadgetdecorator.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   coreplugin
 * @{
 * 
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

#ifndef UAVGADGETDECORATOR_H
#define UAVGADGETDECORATOR_H
#include <coreplugin/iuavgadget.h>

namespace Core {

class IUAVGadgetConfiguration;

class UAVGadgetDecorator : public IUAVGadget
{
Q_OBJECT
public:
    explicit UAVGadgetDecorator(IUAVGadget *gadget, QList<IUAVGadgetConfiguration*> *configurations);
    ~UAVGadgetDecorator();

    QWidget *widget() { return m_gadget->widget(); }
    QComboBox *toolBar() { return m_toolbar; }
    IUAVGadgetConfiguration *activeConfiguration() { return m_activeConfiguration; }
    void loadConfiguration(IUAVGadgetConfiguration *config);
    QByteArray saveState();
    void restoreState(QByteArray state);
public slots:
    void configurationChanged(IUAVGadgetConfiguration* config);
    void configurationAdded(IUAVGadgetConfiguration* config);
    void configurationToBeDeleted(IUAVGadgetConfiguration* config);
    void configurationNameChanged(IUAVGadgetConfiguration* config, QString oldName, QString newName);
private slots:
    void loadConfiguration(int index);
private:
    void updateToolbar();
    IUAVGadget *m_gadget;
    QComboBox *m_toolbar;
    IUAVGadgetConfiguration *m_activeConfiguration;
    QList<IUAVGadgetConfiguration*> *m_configurations;

};

} // namespace Core

#endif // UAVGADGETDECORATOR_H
