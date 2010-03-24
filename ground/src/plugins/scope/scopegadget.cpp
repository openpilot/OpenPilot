/*
 * scopegadget.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "scopegadget.h"
#include "scopegadgetwidget.h"

ScopeGadget::ScopeGadget(QString classId, QList<IUAVGadgetConfiguration*> *configurations, ScopeGadgetWidget *widget) :
        IUAVGadget(classId, configurations, widget),
        m_widget(widget)
{
}

ScopeGadget::~ScopeGadget()
{

}
