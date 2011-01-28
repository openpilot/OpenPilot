/**
 ******************************************************************************
 *
 * @file       lineardialgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup LinearDialPlugin Linear Dial Plugin
 * @{
 * @brief Impliments a gadget that displays linear gauges 
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

#ifndef LINEARDIALGADGETCONFIGURATION_H
#define LINEARDIALGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

/* This is a generic bargraph dial
   supporting one indicator.
  */
class LineardialGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit LineardialGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    //set dial configuration functions
    void setDialFile(QString filename){dialFile=filename;}
    void setRange(double min, double max) { minValue = min; maxValue = max;}
    void setGreenRange(double min, double max) { greenMin = min; greenMax = max;}
    void setYellowRange(double min, double max) { yellowMin = min; yellowMax = max;}
    void setRedRange(double min, double max) { redMin = min; redMax = max;}

    void setFont(QString text) { font = text; }

    void setFactor(double val) { factor = val; }
    void setDecimalPlaces (int val) { decimalPlaces = val; }

    void setSourceDataObject(QString text) {sourceDataObject = text; }
    void setSourceObjField(QString text) { sourceObjectField = text; }

	void setUseOpenGL(bool flag) { useOpenGLFlag = flag; }

    //get dial configuration functions
    QString getDialFile() {return dialFile;}
    double getMin() { return minValue;}
    double getMax() { return maxValue;}
    double getGreenMin(){ return greenMin;}
    double getGreenMax(){ return greenMax;}
    double getYellowMin(){ return yellowMin;}
    double getYellowMax(){ return yellowMax;}
    double getRedMin(){ return redMin;}
    double getRedMax(){ return redMax;}
    QString getSourceDataObject() { return sourceDataObject;}
    QString getSourceObjectField() { return sourceObjectField;}
    QString getFont() { return font;}
    int getDecimalPlaces() { return decimalPlaces; }
    double getFactor() { return factor; }
	bool useOpenGL() { return useOpenGLFlag; }

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

private:
    // A linear or "bargraph" dial contains:
    // * A SVG background file
    // The source file should have at least the following IDs
    // defined: "background", "green", "yellow", "red", "needle"
    QString dialFile;
    // * The name of the UAVObject field to display
    QString sourceDataObject;
    QString sourceObjectField;
    // The font used for the dial
    QString font;
    // * The minimum and maximum values to be displayed
    double minValue;
    double maxValue;
    // * Three start-stop values for green/yellow/red
    double redMin;
    double redMax;
    double yellowMin;
    double yellowMax;
    double greenMin;
    double greenMax;

    double factor;
	bool useOpenGLFlag;

    int decimalPlaces;
};

#endif // LINEARDIALGADGETCONFIGURATION_H
