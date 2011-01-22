/**
 ******************************************************************************
 *
 * @file       myinterfaces.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
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

#ifndef MYINTERFACES_H
#define MYINTERFACES_H

#include <aggregate.h>

#include <QtCore/QString>

class IComboEntry : public QObject
{
    Q_OBJECT

public:
    IComboEntry(QString title) : m_title(title) {}
    virtual ~IComboEntry() {}
    QString title() const { return m_title; }

private:
    QString m_title;
};

class IText1 : public QObject
{
    Q_OBJECT

public:
    IText1(QString text) : m_text(text) {}
    virtual ~IText1() {}
    QString text() const { return m_text; }

private:
    QString m_text;
};

class IText2 : public QObject
{
    Q_OBJECT

public:
    IText2(QString text) : m_text(text) {}
    QString text() const { return m_text; }

private:
    QString m_text;
};

class IText3 : public QObject
{
    Q_OBJECT

public:
    IText3(QString text) : m_text(text) {}
    virtual ~IText3() {}
    QString text() const { return m_text; }

private:
    QString m_text;
};

#endif // MYINTERFACES_H
