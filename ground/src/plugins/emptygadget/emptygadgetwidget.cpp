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
    setText(QString("Empty gadget"));

}

EmptyGadgetWidget::~EmptyGadgetWidget()
{
   // Do nothing
}

