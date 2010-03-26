/*
 * scopegadget.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "scopegadget.h"
#include "scopegadgetwidget.h"

ScopeGadget::ScopeGadget(QString classId, ScopeGadgetWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget)
{
}

ScopeGadget::~ScopeGadget()
{

}
