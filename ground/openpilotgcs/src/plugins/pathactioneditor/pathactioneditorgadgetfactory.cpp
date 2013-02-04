/**
 ******************************************************************************
 * @file       pathactioneditorgadgetfactor.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup PathAction Editor GCS Plugins
 * @{
 * @addtogroup PathActionEditorGadgetPlugin PathAction Editor Gadget Plugin
 * @{
 * @brief A gadget to edit a list of pathactions
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
#include "pathactioneditorgadgetfactory.h"
#include "pathactioneditorgadgetwidget.h"
#include "pathactioneditorgadget.h"
#include <coreplugin/iuavgadget.h>
#include <QDebug>

PathActionEditorGadgetFactory::PathActionEditorGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("PathActionEditorGadget"),
                          tr("PathAction Editor"),
                          parent)
{
}

PathActionEditorGadgetFactory::~PathActionEditorGadgetFactory()
{

}

IUAVGadget* PathActionEditorGadgetFactory::createGadget(QWidget *parent) {
    PathActionEditorGadgetWidget* gadgetWidget = new PathActionEditorGadgetWidget(parent);
    return new PathActionEditorGadget(QString("PathActionEditorGadget"), gadgetWidget, parent);
}
