/*
 * scopegadget.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "scopegadget.h"
#include "scopegadgetwidget.h"
#include <QtGui/QToolBar>

ScopeGadget::ScopeGadget(ScopeGadgetWidget *widget) :
        IUAVGadget(widget),
        m_widget(widget),
        m_toolbar(new QToolBar())
{
}

ScopeGadget::~ScopeGadget()
{

}
