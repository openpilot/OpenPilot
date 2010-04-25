/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtDesigner module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "qxtletterboxwidgetplugin.h"
#include "qxtletterboxwidget.h"
#include <QtPlugin>

QxtLetterBoxWidgetPlugin::QxtLetterBoxWidgetPlugin(QObject* parent)
        : QObject(parent), QxtDesignerPlugin("QxtLetterBoxWidget")
{
}

QWidget* QxtLetterBoxWidgetPlugin::createWidget(QWidget* parent)
{
    return new QxtLetterBoxWidget(parent);
}

QString QxtLetterBoxWidgetPlugin::domXml() const
{
    return "<widget class=\"QxtLetterBoxWidget\" name=\"qxtLetterBoxWidget\">\n"
           " <property name=\"geometry\">\n"
           "  <rect>\n"
           "   <x>0</x>\n"
           "   <y>0</y>\n"
           "   <width>120</width>\n"
           "   <height>80</height>\n"
           "  </rect>\n"
           " </property>\n"
           "</widget>\n";
}
