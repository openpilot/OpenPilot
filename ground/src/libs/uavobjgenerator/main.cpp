/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      UAVObjectGenerator main.
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
#include <QtCore/QCoreApplication>
#include <QFile>
#include <QString>
#include <iostream>
#include "uavobjectgenerator.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString basepath;
    // Get arguments
    if (argc == 1)
    {
        basepath = QString("../../../../../");
    }
    else if (argc == 2)
    {
        basepath = QString(argv[1]);
    }
    else
    {
        std::cout << "Invalid arguments: uavobjectgenerator [base path to gcs and flight directories (as in svn)]" << std::endl;
    }
    // Start application
    UAVObjectGenerator gen(basepath);
    gen.processAll();
    return 0;
}




