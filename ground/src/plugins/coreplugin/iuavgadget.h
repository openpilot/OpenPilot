/**
 ******************************************************************************
 *
 * @file       iuavgadget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#ifndef IUAVGADGET_H
#define IUAVGADGET_H

#include <coreplugin/icontext.h>
#include <coreplugin/core_global.h>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Core {

class CORE_EXPORT IUAVGadget : public IContext
{
    Q_OBJECT
public:
    IUAVGadget(QObject *parent = 0) : IContext(parent) {}
    virtual ~IUAVGadget() {}

    virtual QList<int> context() const = 0;
    virtual QWidget *widget() = 0;
    virtual QString contextHelpId() const { return QString(); }

//    virtual void saveConfiguration() = 0;
//    virtual void loadConfiguration(QString ) = 0;
//    virtual QStringList getConfigurationNames() = 0;
    virtual QWidget *toolBar() = 0;
    virtual void setIndex(int index) { m_index = index; }
    virtual int index() { return m_index; }
private:
    int m_index;
};

} // namespace Core

#endif // IUAVGADGET_H
