/**
 ******************************************************************************
 *
 * @file       generator_io.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      I/O Code for the generator
 *
 * @see        The GNU Public License (GPL) Version 3
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

#include "generator_io.h"

using namespace std;

/**
 * Read a file and return its contents as a string
 */
QString readFile(QString name,bool do_warn)
{
    QFile file(name);
    if (!file.open(QFile::ReadOnly))   {
        if (do_warn)
            std::cout << "Warning: Could not open " << name.toStdString() << endl;
        return QString();
    }

    QTextStream fileStr(&file);
    QString str = fileStr.readAll();
    file.close();
    return str;
}

/**
 * Read a file and return its contents as a string
 */
QString readFile(QString name)
{
    return readFile(name,true);
}

/**
 * Write contents of string to file
 */
bool writeFile(QString name, QString& str)
{
    QFile file(name);
    if (!file.open(QFile::WriteOnly))
        return false;
    QTextStream fileStr(&file);
    fileStr << str;
    file.close();
    return true;
}

/**
 * Write contents of string to file if the content changes
 */
bool writeFileIfDiffrent(QString name, QString& str)
{
    if (str==readFile(name,false))
        return true;
    return writeFile(name,str);
}
