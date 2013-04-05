/**
 ******************************************************************************
 *
 * @file       pfdgadgetconfiguration.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin Primary Flight Display Plugin
 * @{
 * @brief The Primary Flight Display Gadget 
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

#ifndef PFDGADGETCONFIGURATION_H
#define PFDGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

class PFDGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit PFDGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    //set dial configuration functions
    void setDialFile(QString dialFile){m_defaultDial=dialFile;}
    void setUseOpenGL(bool flag) { useOpenGLFlag = flag; }
    void setHqFonts(bool flag) { hqFonts = flag; }
    void setBeSmooth(bool flag) { beSmooth = flag;}

    //get dial configuration functions
    QString dialFile() {return m_defaultDial;}
    bool useOpenGL() { return useOpenGLFlag; }
    bool getHqFonts() { return hqFonts; }
    bool getBeSmooth() { return beSmooth; }

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

private:
    QString m_defaultDial; // The name of the dial's SVG source file
    bool useOpenGLFlag;
    bool hqFonts;
    bool beSmooth;
};

#endif // PFDGADGETCONFIGURATION_H
