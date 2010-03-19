/*
 * emptygadget.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "emptygadget.h"
#include "emptygadgetwidget.h"
#include <QtGui/QToolBar>

EmptyGadget::EmptyGadget(EmptyGadgetWidget *widget) :
        IUAVGadget(widget),
        m_widget(widget),
        m_toolbar(new QToolBar())
{
}

EmptyGadget::~EmptyGadget()
{

}
