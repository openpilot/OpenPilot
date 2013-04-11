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
#include "pfdqmlgadgetfactory.h"
#include "pfdqmlgadgetwidget.h"
#include "pfdqmlgadget.h"
#include "pfdqmlgadgetconfiguration.h"
#include "pfdqmlgadgetoptionspage.h"
#include <coreplugin/iuavgadget.h>

PfdQmlGadgetFactory::PfdQmlGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("PfdQmlGadget"),
                          tr("PFD (qml)"),
                          parent)
{
}

PfdQmlGadgetFactory::~PfdQmlGadgetFactory()
{
}

Core::IUAVGadget* PfdQmlGadgetFactory::createGadget(QWidget *parent)
{
    PfdQmlGadgetWidget* gadgetWidget = new PfdQmlGadgetWidget(parent);
    return new PfdQmlGadget(QString("PfdQmlGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *PfdQmlGadgetFactory::createConfiguration(QSettings *qSettings)
{
    return new PfdQmlGadgetConfiguration(QString("PfdQmlGadget"), qSettings);
}

IOptionsPage *PfdQmlGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new PfdQmlGadgetOptionsPage(qobject_cast<PfdQmlGadgetConfiguration*>(config));
}

