/**
 ******************************************************************************
 *
 * @file       mixercurve.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief A MixerCurve Gadget used to update settings in the firmware
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

#include <QtGui/QWidget>
#include <QResizeEvent>
#include <math.h>
#include "mixercurve.h"
#include "dblspindelegate.h"

MixerCurve::MixerCurve(QWidget *parent) :
    QFrame(parent),
    m_mixerUI(new Ui::MixerCurve)
{
    m_mixerUI->setupUi(this);

    // setup some convienence pointers
    m_curve = m_mixerUI->CurveWidget;
    m_settings = m_mixerUI->CurveSettings;


    m_mixerUI->SettingsGroup->hide();
    m_curve->showCommands(false);
    m_curve->showCommand("Reset", false);
    m_curve->showCommand("Popup", false);
    m_curve->showCommand("Commands", false);

    // create our spin delegate
    m_spinDelegate = new DoubleSpinDelegate();

    // set the default mixer type
    setMixerType(MixerCurve::MIXERCURVE_THROTTLE);

    // setup and turn off advanced mode
    CommandActivated();

    // paint the ui
    UpdateCurveUI();

    // wire up our signals

    connect(m_mixerUI->CurveType, SIGNAL(currentIndexChanged(int)), this, SLOT(CurveTypeChanged()));
    connect(m_mixerUI->ResetCurve, SIGNAL(clicked()), this, SLOT(ResetCurve()));
    connect(m_mixerUI->PopupCurve, SIGNAL(clicked()),this,SLOT(PopupCurve()));
    connect(m_mixerUI->GenerateCurve, SIGNAL(clicked()), this, SLOT(GenerateCurve()));
    connect(m_curve, SIGNAL(curveUpdated()), this, SLOT(UpdateSettingsTable()));
    connect(m_curve, SIGNAL(commandActivated(MixerNode*)),this, SLOT(CommandActivated(MixerNode*)));
    connect(m_settings, SIGNAL(cellChanged(int,int)), this, SLOT(SettingsTableChanged()));
    connect(m_mixerUI->CurveMin, SIGNAL(valueChanged(double)), this, SLOT(CurveMinChanged(double)));
    connect(m_mixerUI->CurveMax, SIGNAL(valueChanged(double)), this, SLOT(CurveMaxChanged(double)));
    connect(m_mixerUI->CurveStep, SIGNAL(valueChanged(double)), this, SLOT(GenerateCurve()));



}

MixerCurve::~MixerCurve()
{
    delete m_mixerUI;
    delete m_spinDelegate;
}

void MixerCurve::setMixerType(MixerCurveType curveType)
{
    m_curveType = curveType;

    m_mixerUI->CurveMin->setMaximum(1.0);
    m_mixerUI->CurveMax->setMaximum(1.0);

    switch (m_curveType) {
        case MixerCurve::MIXERCURVE_THROTTLE:
        {
            m_mixerUI->SettingsGroup->setTitle("Throttle Curve");
            m_curve->setRange(0.0, 1.0);
            m_mixerUI->CurveMin->setMinimum(0.0);
            m_mixerUI->CurveMax->setMinimum(0.0);
            break;
        }
        case MixerCurve::MIXERCURVE_PITCH:
        {
            m_mixerUI->SettingsGroup->setTitle("Pitch Curve");
            m_curve->setRange(-1.0, 1.0);            
            m_curve->setPositiveColor("#0000aa", "#0000aa");
            m_mixerUI->CurveMin->setMinimum(-1.0);
            m_mixerUI->CurveMax->setMinimum(-1.0);
            break;
        }
    }

    m_spinDelegate->setRange(m_mixerUI->CurveMin->minimum(), m_mixerUI->CurveMax->maximum());
    for (int i=0; i<MixerCurveWidget::NODE_NUMELEM; i++) {
        m_settings->setItemDelegateForRow(i, m_spinDelegate);
    }

    ResetCurve();
}

void MixerCurve::ResetCurve()
{
    m_mixerUI->CurveMin->setValue(m_mixerUI->CurveMin->minimum());
    m_mixerUI->CurveMax->setValue(m_mixerUI->CurveMax->maximum());
    m_mixerUI->CurveType->setCurrentIndex(m_mixerUI->CurveType->findText("Linear"));

    initLinearCurve(MixerCurveWidget::NODE_NUMELEM, getCurveMax(), getCurveMin());

    m_curve->activateCommand("Linear");

    UpdateSettingsTable();
}

void MixerCurve::PopupCurve()
{
    if (!m_curve->isCommandActive("Popup")) {

        m_mixerUI->SettingsGroup->show();
        m_mixerUI->PopupCurve->hide();

        PopupWidget* popup = new PopupWidget();
        popup->popUp(this);

        m_mixerUI->SettingsGroup->hide();
        m_mixerUI->PopupCurve->show();
        m_curve->showCommands(false);
    }
}
void MixerCurve::UpdateCurveUI()
{
    //get the user settings
    QString curveType = m_mixerUI->CurveType->currentText();

    m_curve->activateCommand(curveType);
    bool cmdsActive = m_curve->isCommandActive("Commands");

    m_curve->showCommand("StepPlus", cmdsActive && curveType != "Linear");
    m_curve->showCommand("StepMinus", cmdsActive && curveType != "Linear");
    m_curve->showCommand("StepValue", cmdsActive && curveType != "Linear");

    m_mixerUI->CurveStep->setMinimum(0.0);
    m_mixerUI->CurveStep->setMaximum(100.0);
    m_mixerUI->CurveStep->setSingleStep(1.00);

    //set default visible
    m_mixerUI->minLabel->setVisible(true);
    m_mixerUI->CurveMin->setVisible(true);
    m_mixerUI->maxLabel->setVisible(false);
    m_mixerUI->CurveMax->setVisible(false);
    m_mixerUI->stepLabel->setVisible(false);
    m_mixerUI->CurveStep->setVisible(false);

    if ( curveType.compare("Flat")==0)
    {
        m_mixerUI->minLabel->setVisible(false);
        m_mixerUI->CurveMin->setVisible(false);
        m_mixerUI->stepLabel->setVisible(true);
        m_mixerUI->CurveStep->setVisible(true);
        m_mixerUI->CurveStep->setMinimum(m_mixerUI->CurveMin->minimum());
        m_mixerUI->CurveStep->setMaximum(m_mixerUI->CurveMax->maximum());
        m_mixerUI->CurveStep->setSingleStep(0.01);
        m_mixerUI->CurveStep->setValue(m_mixerUI->CurveMax->value() / 2);
    }
    if ( curveType.compare("Linear")==0)
    {
        m_mixerUI->maxLabel->setVisible(true);
        m_mixerUI->CurveMax->setVisible(true);
    }
    if ( curveType.compare("Step")==0)
    {
        m_mixerUI->maxLabel->setVisible(true);
        m_mixerUI->CurveMax->setVisible(true);
        m_mixerUI->stepLabel->setText("Step at");
        m_mixerUI->stepLabel->setVisible(true);
        m_mixerUI->CurveStep->setVisible(true);

        m_mixerUI->CurveStep->setMinimum(1.0);
    }
    if ( curveType.compare("Exp")==0)
    {
        m_mixerUI->maxLabel->setVisible(true);
        m_mixerUI->CurveMax->setVisible(true);
        m_mixerUI->stepLabel->setText("Power");
        m_mixerUI->stepLabel->setVisible(true);
        m_mixerUI->CurveStep->setVisible(true);

        m_mixerUI->CurveStep->setMinimum(1.0);
    }
    if ( curveType.compare("Log")==0)
    {
        m_mixerUI->maxLabel->setVisible(true);
        m_mixerUI->CurveMax->setVisible(true);
        m_mixerUI->stepLabel->setText("Power");
        m_mixerUI->stepLabel->setVisible(true);
        m_mixerUI->CurveStep->setVisible(true);        
        m_mixerUI->CurveStep->setMinimum(1.0);
    }

    GenerateCurve();
}

void MixerCurve::GenerateCurve()
{
   double scale;
   double newValue;

   //get the user settings
   double value1 = getCurveMin();
   double value2 = getCurveMax();
   double value3 = getCurveStep();

   m_curve->setCommandText("StepValue", QString("%0").arg(value3));

   QString CurveType = m_mixerUI->CurveType->currentText();

   QList<double> points;

   for (int i=0; i<MixerCurveWidget::NODE_NUMELEM; i++)
   {
       scale =((double)i/(double)(MixerCurveWidget::NODE_NUMELEM - 1));

       if ( CurveType.compare("Flat")==0)
       {
           points.append(value3);
       }
       if ( CurveType.compare("Linear")==0)
       {
           newValue =value1 +(scale*(value2-value1));
           points.append(newValue);
       }
       if ( CurveType.compare("Step")==0)
       {
           if (scale*100<value3)
           {
               points.append(value1);
           }
           else
           {
               points.append(value2);
           }
       }
       if ( CurveType.compare("Exp")==0)
       {
           newValue =value1 +(((exp(scale*(value3/10))-1))/(exp((value3/10))-1)*(value2-value1));
           points.append(newValue);
       }
       if ( CurveType.compare("Log")==0)
       {
           newValue = value1 +(((log(scale*(value3*2)+1))/(log(1+(value3*2))))*(value2-value1));
           points.append(newValue);
       }
   }

   setCurve(&points);
}

/**
  Wrappers for mixercurvewidget.
  */
void MixerCurve::initCurve (const QList<double>* points)
{
    m_curve->setCurve(points);
    UpdateSettingsTable();
}
QList<double> MixerCurve::getCurve()
{
    return m_curve->getCurve();
}
void MixerCurve::initLinearCurve(int numPoints, double maxValue, double minValue)
{
    setMin(minValue);
    setMax(maxValue);

    m_curve->initLinearCurve(numPoints, maxValue, minValue);

    if (m_spinDelegate)
        m_spinDelegate->setRange(minValue, maxValue);
}
void MixerCurve::setCurve(const QList<double>* points)
{
    m_curve->setCurve(points);
    UpdateSettingsTable();
}
void MixerCurve::setMin(double value)
{
    //m_curve->setMin(value);
    m_mixerUI->CurveMin->setMinimum(value);
}
double MixerCurve::getMin()
{
    return m_curve->getMin();
}
void MixerCurve::setMax(double value)
{
    //m_curve->setMax(value);
    m_mixerUI->CurveMax->setMaximum(value);
}
double MixerCurve::getMax()
{
    return m_curve->getMax();
}
double MixerCurve::setRange(double min, double max)
{
    return m_curve->setRange(min, max);
}


double MixerCurve::getCurveMin()
{
    return m_mixerUI->CurveMin->value();
}
double MixerCurve::getCurveMax()
{
    return m_mixerUI->CurveMax->value();
}

double MixerCurve::getCurveStep()
{
    return m_mixerUI->CurveStep->value();
}

void MixerCurve::UpdateSettingsTable()
{
    QList<double> points = m_curve->getCurve();
    int ptCnt = points.count();

    for (int i=0; i<ptCnt; i++)
    {
        QTableWidgetItem* item = m_settings->item(i, 0);
        if (item)
            item->setText(QString().sprintf("%.2f",points.at( (ptCnt - 1) - i )));
    }
}

void MixerCurve::SettingsTableChanged()
{
    QList<double> points;

    for (int i=0; i < m_settings->rowCount(); i++)
    {
        QTableWidgetItem* item = m_settings->item(i, 0);

        if (item)
            points.push_front(item->text().toDouble());
    }

    m_mixerUI->CurveMin->setValue(points.first());
    m_mixerUI->CurveMax->setValue(points.last());

    m_curve->setCurve(&points);
}

void MixerCurve::CommandActivated(MixerNode* node)
{
    QString name = (node) ? node->getName() : "Reset";

    if (name == "Reset") {        
        ResetCurve();
        m_curve->showCommands(false);
    }
    else if (name == "Commands") {

    }
    else if (name == "Popup" ) {
        PopupCurve();
    }
    else if (name == "Linear") {
        m_mixerUI->CurveType->setCurrentIndex(m_mixerUI->CurveType->findText("Linear"));
    }
    else if (name == "Log") {
        m_mixerUI->CurveType->setCurrentIndex(m_mixerUI->CurveType->findText("Log"));
    }
    else if (name == "Exp") {
        m_mixerUI->CurveType->setCurrentIndex(m_mixerUI->CurveType->findText("Exp"));
    }
    else if (name == "Flat") {
        m_mixerUI->CurveType->setCurrentIndex(m_mixerUI->CurveType->findText("Flat"));
    }
    else if (name == "Step") {
        m_mixerUI->CurveType->setCurrentIndex(m_mixerUI->CurveType->findText("Step"));
    }
    else if (name ==  "MinPlus") {
        m_mixerUI->CurveMin->stepUp();
    }
    else if (name ==  "MinMinus") {
        m_mixerUI->CurveMin->stepDown();
    }
    else if (name == "MaxPlus") {
        m_mixerUI->CurveMax->stepUp();
    }
    else if (name == "MaxMinus"){
        m_mixerUI->CurveMax->stepDown();
    }
    else if (name ==  "StepPlus") {
        m_mixerUI->CurveStep->stepUp();
        m_curve->setCommandText("StepValue", QString("%0").arg(getCurveStep()));
    }
    else if (name == "StepMinus") {
        m_mixerUI->CurveStep->stepDown();
        m_curve->setCommandText("StepValue", QString("%0").arg(getCurveStep()));
    }

    GenerateCurve();
}

void MixerCurve::CurveTypeChanged()
{
    // setup the ui for this curvetype
    UpdateCurveUI();
}

void MixerCurve::CurveMinChanged(double value)
{
    QList<double> points = m_curve->getCurve();
    points.removeFirst();
    points.push_front(value);
    setCurve(&points);
}

void MixerCurve::CurveMaxChanged(double value)
{
    // the max changed so redraw the curve
    //  mixercurvewidget::setCurve will trim any points above max
    QList<double> points = m_curve->getCurve();
    points.removeLast();
    points.append(value);
    setCurve(&points);
}

void MixerCurve::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    m_settings->resizeColumnsToContents();
    m_settings->setColumnWidth(0,(m_settings->width()-  m_settings->verticalHeader()->width()));

    int h = (m_settings->height() -  m_settings->horizontalHeader()->height()) / m_settings->rowCount();
    for (int i=0; i<m_settings->rowCount(); i++)
        m_settings->setRowHeight(i, h);

    m_curve->showEvent(event);
}

void MixerCurve::resizeEvent(QResizeEvent* event)
{
    m_settings->resizeColumnsToContents();
    m_settings->setColumnWidth(0,(m_settings->width() -  m_settings->verticalHeader()->width()));

    int h = (m_settings->height() -  m_settings->horizontalHeader()->height()) / m_settings->rowCount();
    for (int i=0; i<m_settings->rowCount(); i++)
        m_settings->setRowHeight(i, h);

    m_curve->resizeEvent(event);
}
