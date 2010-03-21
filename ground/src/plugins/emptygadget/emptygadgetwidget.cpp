/*
 * emptygadgetwidget.cpp
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */
#include "emptygadgetwidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

EmptyGadgetWidget::EmptyGadgetWidget(QWidget *parent) : QLabel(parent)
{
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->
    setText(tr("Choose a gadget to display in this view.\n") +
            tr("You can also split this view in two.\n\n") +
            tr("Maybe you first have to choose Show Toolbars in the Window menu."));
}

EmptyGadgetWidget::~EmptyGadgetWidget()
{
   // Do nothing
}

