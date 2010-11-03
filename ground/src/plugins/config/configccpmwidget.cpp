/**
 ******************************************************************************
 *
 * @file       configccpmwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief ccpm configuration panel
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
#include "configccpmwidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <math.h>

#define  Pi 3.14159265358979323846


ConfigccpmWidget::ConfigccpmWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_ccpm = new Ui_ccpmWidget();
    m_ccpm->setupUi(this);

    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    //ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    //UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    // Initialization of the swashplaye widget
    m_ccpm->SwashplateImage->setScene(new QGraphicsScene(this));

    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/ccpm_setup.svg"));


    SwashplateImg = new QGraphicsSvgItem();
    SwashplateImg->setSharedRenderer(renderer);
    SwashplateImg->setElementId("Swashplate");
    SwashplateImg->setObjectName("Swashplate");
    m_ccpm->SwashplateImage->scene()->addItem(SwashplateImg);
    m_ccpm->SwashplateImage->setSceneRect(SwashplateImg->boundingRect());

    m_ccpm->SwashplateImage->scale(.75,.75);

    ServoW = new QGraphicsSvgItem();
    ServoW->setSharedRenderer(renderer);
    ServoW->setElementId("ServoW");
    m_ccpm->SwashplateImage->scene()->addItem(ServoW);

    ServoX = new QGraphicsSvgItem();
    ServoX->setSharedRenderer(renderer);
    ServoX->setElementId("ServoX");
    m_ccpm->SwashplateImage->scene()->addItem(ServoX);

    ServoY = new QGraphicsSvgItem();
    ServoY->setSharedRenderer(renderer);
    ServoY->setElementId("ServoY");
    m_ccpm->SwashplateImage->scene()->addItem(ServoY);

    ServoZ = new QGraphicsSvgItem();
    ServoZ->setSharedRenderer(renderer);
    ServoZ->setElementId("ServoZ");
    m_ccpm->SwashplateImage->scene()->addItem(ServoZ);

    QFont serifFont("Times", 16, QFont::Bold);

    ServoWText = new QGraphicsTextItem();
    ServoWText->setDefaultTextColor(Qt::red);
    ServoWText->setPlainText(QString("-"));
    ServoWText->setFont(serifFont);
    m_ccpm->SwashplateImage->scene()->addItem(ServoWText);

    ServoXText = new QGraphicsTextItem();
    ServoXText->setDefaultTextColor(Qt::red);
    ServoXText->setPlainText(QString("-"));
    ServoXText->setFont(serifFont);
    m_ccpm->SwashplateImage->scene()->addItem(ServoXText);

    ServoYText = new QGraphicsTextItem();
    ServoYText->setDefaultTextColor(Qt::red);
    ServoYText->setPlainText(QString("-"));
    ServoYText->setFont(serifFont);
    m_ccpm->SwashplateImage->scene()->addItem(ServoYText);

    ServoZText = new QGraphicsTextItem();
    ServoZText->setDefaultTextColor(Qt::red);
    ServoZText->setPlainText(QString("-"));
    ServoZText->setFont(serifFont);
    m_ccpm->SwashplateImage->scene()->addItem(ServoZText);

    m_ccpm->PitchCurve->setMin(-1);

    resetMixer(m_ccpm->PitchCurve, 5);
    resetMixer(m_ccpm->ThrottleCurve, 5);





    QStringList channels;
    channels << "Channel0" << "Channel1" << "Channel2" <<
            "Channel3" << "Channel4" << "Channel5" << "Channel6" << "Channel7" << "None" ;
    m_ccpm->ccpmEngineChannel->addItems(channels);
    m_ccpm->ccpmEngineChannel->setCurrentIndex(8);
    m_ccpm->ccpmTailChannel->addItems(channels);
    m_ccpm->ccpmTailChannel->setCurrentIndex(8);
    m_ccpm->ccpmServoWChannel->addItems(channels);
    m_ccpm->ccpmServoWChannel->setCurrentIndex(8);
    m_ccpm->ccpmServoXChannel->addItems(channels);
    m_ccpm->ccpmServoXChannel->setCurrentIndex(8);
    m_ccpm->ccpmServoYChannel->addItems(channels);
    m_ccpm->ccpmServoYChannel->setCurrentIndex(8);
    m_ccpm->ccpmServoZChannel->addItems(channels);
    m_ccpm->ccpmServoZChannel->setCurrentIndex(8);

    QStringList Types;
    Types << "CCPM 2 Servo 90�" << "CCPM 3 Servo 120�" << "CCPM 3 Servo 140�" << "FP 2 Servo 90�"  << "Custom - User Angles" << "Custom - Advanced Settings"  ;
    m_ccpm->ccpmType->addItems(Types);
    m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->count() - 1);
    requestccpmUpdate();
    UpdateCurveSettings();


    //disable changing number of points in curves until UAVObjects have more than 5
    m_ccpm->NumCurvePoints->setEnabled(0);

    UpdateType();

    //connect(m_ccpm->saveccpmToSD, SIGNAL(clicked()), this, SLOT(saveccpmUpdate()));
    //connect(m_ccpm->saveccpmToRAM, SIGNAL(clicked()), this, SLOT(sendccpmUpdate()));
    //connect(m_ccpm->getccpmCurrent, SIGNAL(clicked()), this, SLOT(requestccpmUpdate()));
    connect(m_ccpm->ccpmGenerateCurve, SIGNAL(clicked()), this, SLOT(GenerateCurve()));
    connect(m_ccpm->NumCurvePoints, SIGNAL(valueChanged(int)), this, SLOT(UpdateCurveSettings()));
    connect(m_ccpm->CurveToGenerate, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateCurveSettings()));
    connect(m_ccpm->CurveType, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateCurveSettings()));
    connect(m_ccpm->ccpmAngleW, SIGNAL(valueChanged(double)), this, SLOT(ccpmSwashplateUpdate()));
    connect(m_ccpm->ccpmAngleX, SIGNAL(valueChanged(double)), this, SLOT(ccpmSwashplateUpdate()));
    connect(m_ccpm->ccpmAngleY, SIGNAL(valueChanged(double)), this, SLOT(ccpmSwashplateUpdate()));
    connect(m_ccpm->ccpmAngleZ, SIGNAL(valueChanged(double)), this, SLOT(ccpmSwashplateUpdate()));
    connect(m_ccpm->ccpmCorrectionAngle, SIGNAL(valueChanged(double)), this, SLOT(ccpmSwashplateUpdate()));
    connect(m_ccpm->ccpmServoWChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(ccpmSwashplateUpdate()));
    connect(m_ccpm->ccpmServoXChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(ccpmSwashplateUpdate()));
    connect(m_ccpm->ccpmServoYChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(ccpmSwashplateUpdate()));
    connect(m_ccpm->ccpmServoZChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(ccpmSwashplateUpdate()));
    connect(m_ccpm->ccpmEngineChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMixer()));
    connect(m_ccpm->ccpmTailChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMixer()));
    connect(m_ccpm->ccpmRevoSlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateMixer()));
    connect(m_ccpm->ccpmREVOspinBox, SIGNAL(valueChanged(int)), this, SLOT(UpdateMixer()));
    connect(m_ccpm->ccpmCollectiveSlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateMixer()));
    connect(m_ccpm->ccpmCollectivespinBox, SIGNAL(valueChanged(int)), this, SLOT(UpdateMixer()));
    connect(m_ccpm->ccpmType, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateType()));
    connect(m_ccpm->ccpmSingleServo, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateType()));
    connect(m_ccpm->CurveSettings, SIGNAL(cellChanged (int, int)), this, SLOT(UpdateCurveWidgets()));



    connect(m_ccpm->PitchCurve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updatePitchCurveValue(QList<double>,double)));
    connect(m_ccpm->ThrottleCurve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateThrottleCurveValue(QList<double>,double)));



   // connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestccpmUpdate()));

}

ConfigccpmWidget::~ConfigccpmWidget()
{
   // Do nothing
}

void ConfigccpmWidget::UpdateType()
{
    int TypeInt,SingleServoIndex;
    QString TypeText;
    double AdjustmentAngle=0;


    TypeInt = m_ccpm->ccpmType->count() - m_ccpm->ccpmType->currentIndex()-1;
    TypeText = m_ccpm->ccpmType->currentText();
    SingleServoIndex = m_ccpm->ccpmSingleServo->currentIndex();

    //set visibility of user settings
    m_ccpm->ccpmAdvancedSettingsTable->setEnabled(TypeInt==0);
    m_ccpm->ccpmAdvancedSettingsTable->clearFocus();;

    m_ccpm->ccpmAngleW->setEnabled(TypeInt==1);
    m_ccpm->ccpmAngleX->setEnabled(TypeInt==1);
    m_ccpm->ccpmAngleY->setEnabled(TypeInt==1);
    m_ccpm->ccpmAngleZ->setEnabled(TypeInt==1);
    m_ccpm->ccpmCorrectionAngle->setEnabled(TypeInt==1);

    m_ccpm->ccpmServoWChannel->setEnabled(TypeInt>0);
    m_ccpm->ccpmServoXChannel->setEnabled(TypeInt>0);
    m_ccpm->ccpmServoYChannel->setEnabled(TypeInt>0);
    m_ccpm->ccpmServoZChannel->setEnabled(TypeInt>0);
    m_ccpm->ccpmSingleServo->setEnabled(TypeInt>1);

    m_ccpm->ccpmEngineChannel->setEnabled(TypeInt>0);
    m_ccpm->ccpmTailChannel->setEnabled(TypeInt>0);
    m_ccpm->ccpmCollectiveSlider->setEnabled(TypeInt>0);
    m_ccpm->ccpmCollectivespinBox->setEnabled(TypeInt>0);
    m_ccpm->ccpmRevoSlider->setEnabled(TypeInt>0);
    m_ccpm->ccpmREVOspinBox->setEnabled(TypeInt>0);

    AdjustmentAngle=SingleServoIndex*90;

    m_ccpm->CurveToGenerate->setEnabled(1);
    m_ccpm->CurveSettings->setColumnHidden(1,0);
    m_ccpm->PitchCurve->setVisible(1);
    //m_ccpm->customThrottleCurve2Value->setVisible(1);
    //m_ccpm->label_41->setVisible(1);

    //set values for pre defined heli types
        if (TypeText.compare(QString("CCPM 2 Servo 90�"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 90,360));
            m_ccpm->ccpmAngleY->setValue(0);
            m_ccpm->ccpmAngleZ->setValue(0);
            m_ccpm->ccpmAngleY->setEnabled(0);
            m_ccpm->ccpmAngleZ->setEnabled(0);
            m_ccpm->ccpmServoYChannel->setCurrentIndex(8);
            m_ccpm->ccpmServoZChannel->setCurrentIndex(8);
            m_ccpm->ccpmServoYChannel->setEnabled(0);
            m_ccpm->ccpmServoZChannel->setEnabled(0);
            m_ccpm->ccpmCorrectionAngle->setValue(0);

        }
        if (TypeText.compare(QString("CCPM 3 Servo 120�"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 120,360));
            m_ccpm->ccpmAngleY->setValue(fmod(AdjustmentAngle + 240,360));
            m_ccpm->ccpmAngleZ->setValue(0);
            m_ccpm->ccpmAngleZ->setEnabled(0);
            m_ccpm->ccpmServoZChannel->setCurrentIndex(8);
            m_ccpm->ccpmServoZChannel->setEnabled(0);
            m_ccpm->ccpmCorrectionAngle->setValue(0);

        }
        if (TypeText.compare(QString("CCPM 3 Servo 140�"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 140,360));
            m_ccpm->ccpmAngleY->setValue(fmod(AdjustmentAngle + 220,360));
            m_ccpm->ccpmAngleZ->setValue(0);
            m_ccpm->ccpmAngleZ->setEnabled(0);
            m_ccpm->ccpmServoZChannel->setCurrentIndex(8);
            m_ccpm->ccpmServoZChannel->setEnabled(0);
            m_ccpm->ccpmCorrectionAngle->setValue(0);

        }
        if (TypeText.compare(QString("FP 2 Servo 90�"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 90,360));
            m_ccpm->ccpmAngleY->setValue(0);
            m_ccpm->ccpmAngleZ->setValue(0);
            m_ccpm->ccpmAngleY->setEnabled(0);
            m_ccpm->ccpmAngleZ->setEnabled(0);
            m_ccpm->ccpmServoYChannel->setCurrentIndex(8);
            m_ccpm->ccpmServoZChannel->setCurrentIndex(8);
            m_ccpm->ccpmServoYChannel->setEnabled(0);
            m_ccpm->ccpmServoZChannel->setEnabled(0);
            m_ccpm->ccpmCorrectionAngle->setValue(0);

            m_ccpm->ccpmCollectivespinBox->setEnabled(0);
            m_ccpm->ccpmCollectiveSlider->setEnabled(0);
            m_ccpm->ccpmCollectivespinBox->setValue(0);
            m_ccpm->ccpmCollectiveSlider->setValue(0);
            m_ccpm->CurveToGenerate->setCurrentIndex(0);
            m_ccpm->CurveToGenerate->setEnabled(0);
            m_ccpm->CurveSettings->setColumnHidden(1,1);
            m_ccpm->PitchCurve->setVisible(0);
            //m_ccpm->customThrottleCurve2Value->setVisible(0);
            //m_ccpm->label_41->setVisible(0);
        }



    //update UI
    ccpmSwashplateUpdate();

}

/**
  Resets a mixer curve
  */
void ConfigccpmWidget::resetMixer(MixerCurveWidget *mixer, int numElements)
{
    QList<double> curveValues;
    for (double i=0; i<numElements; i++) {
        curveValues.append(i/(numElements-1));
    }
    // Setup all Throttle1 curves for all types of airframes
    mixer->initCurve(curveValues);
}

void ConfigccpmWidget::UpdateCurveWidgets()
{
    int NumCurvePoints,i,Changed;
    QList<double> curveValues;
    QList<double> OldCurveValues;
    double ThisValue;
    //get the user settings
    NumCurvePoints=m_ccpm->NumCurvePoints->value();

    curveValues.clear();
    Changed=0;
    OldCurveValues=m_ccpm->ThrottleCurve->getCurve();
    for (i=0; i<NumCurvePoints; i++)
    {
        ThisValue=m_ccpm->CurveSettings->item(i, 0 )->text().toDouble();
        curveValues.append(ThisValue);
        if (ThisValue!=OldCurveValues.at(i))Changed=1;
    }
    // Setup all Throttle1 curves for all types of airframes
    if (Changed==1)m_ccpm->ThrottleCurve->setCurve(curveValues);

    curveValues.clear();
    Changed=0;
    OldCurveValues=m_ccpm->PitchCurve->getCurve();
    for (i=0; i<NumCurvePoints; i++)
    {
        ThisValue=m_ccpm->CurveSettings->item(i, 1 )->text().toDouble();
        curveValues.append(ThisValue);
        if (ThisValue!=OldCurveValues.at(i))Changed=1;
    }
    // Setup all Throttle1 curves for all types of airframes
    if (Changed==1)m_ccpm->PitchCurve->setCurve(curveValues);
}

void ConfigccpmWidget::updatePitchCurveValue(QList<double> curveValues0,double Value0)
{
    int NumCurvePoints,i;
    double CurrentValue;
    QList<double> internalCurveValues;
    //get the user settings
    NumCurvePoints=m_ccpm->NumCurvePoints->value();
    internalCurveValues=m_ccpm->PitchCurve->getCurve();

    for (i=0; i<internalCurveValues.length(); i++)
    {
        CurrentValue=m_ccpm->CurveSettings->item(i, 1 )->text().toDouble();
        if (CurrentValue!=internalCurveValues[i])
        {
            m_ccpm->CurveSettings->item(i, 1)->setText(QString().sprintf("%.3f",internalCurveValues.at(i)));
        }

    }

}

void ConfigccpmWidget::updateThrottleCurveValue(QList<double> curveValues0,double Value0)
{
    int NumCurvePoints,i;
    double CurrentValue;
    QList<double> internalCurveValues;
    //get the user settings
    NumCurvePoints=m_ccpm->NumCurvePoints->value();
    internalCurveValues=m_ccpm->ThrottleCurve->getCurve();

    for (i=0; i<internalCurveValues.length(); i++)
    {
        CurrentValue=m_ccpm->CurveSettings->item(i, 1 )->text().toDouble();
        if (CurrentValue!=internalCurveValues[i])
        {
            m_ccpm->CurveSettings->item(i, 0)->setText(QString().sprintf("%.3f",internalCurveValues.at(i)));
        }

    }

}


void ConfigccpmWidget::UpdateCurveSettings()
{
    int NumCurvePoints,i;
    double scale;
    QString CurveType;
    QStringList vertHeaders;

    //get the user settings
    NumCurvePoints=m_ccpm->NumCurvePoints->value();
    CurveType=m_ccpm->CurveType->currentText();

    vertHeaders << "-" << "-" << "-" << "-" << "-" << "-" << "-" << "-" << "-" << "-" ;
    for (i=0;i<NumCurvePoints;i++)
    {
        scale =((double)i/(double)(NumCurvePoints-1));
        vertHeaders[i] = tr( "%1%" ).arg(100.00*scale, 0, 'f', 1);
    }
    m_ccpm->CurveSettings->setVerticalHeaderLabels( vertHeaders );

    if (m_ccpm->CurveToGenerate->currentIndex()==0)
    {
        m_ccpm->CurveValue1->setMinimum(0.0);
        m_ccpm->CurveValue2->setMinimum(0.0);
        m_ccpm->CurveValue3->setMinimum(0.0);
    }
    else
    {
        m_ccpm->CurveValue1->setMinimum(-1.0);
        m_ccpm->CurveValue2->setMinimum(-1.0);
        m_ccpm->CurveValue3->setMinimum(0.0);
    }
    m_ccpm->CurveValue1->setMaximum(1.0);
    m_ccpm->CurveValue2->setMaximum(1.0);
    m_ccpm->CurveValue3->setMaximum(100.0);
    m_ccpm->CurveValue1->setSingleStep(0.1);
    m_ccpm->CurveValue2->setSingleStep(0.1);
    m_ccpm->CurveValue3->setSingleStep(1.0);
    m_ccpm->CurveValue1->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);;
    m_ccpm->CurveValue2->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    m_ccpm->CurveValue3->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);

    if ( CurveType.compare("Flat")==0)
    {
        m_ccpm->CurveLabel1->setText("Value");
        m_ccpm->CurveLabel1->setVisible(true);
        m_ccpm->CurveValue1->setVisible(true);
        m_ccpm->CurveLabel2->setVisible(false);
        m_ccpm->CurveValue2->setVisible(false);
        m_ccpm->CurveLabel3->setVisible(false);
        m_ccpm->CurveValue3->setVisible(false);
        m_ccpm->ccpmGenerateCurve->setVisible(true);
        m_ccpm->CurveToGenerate->setVisible(true);
    }
    if ( CurveType.compare("Linear")==0)
    {
        m_ccpm->CurveLabel1->setText("Min");
        m_ccpm->CurveLabel1->setVisible(true);
        m_ccpm->CurveValue1->setVisible(true);
        m_ccpm->CurveLabel2->setText("Max");
        m_ccpm->CurveLabel2->setVisible(true);
        m_ccpm->CurveValue2->setVisible(true);
        m_ccpm->CurveLabel3->setVisible(false);
        m_ccpm->CurveValue3->setVisible(false);
        m_ccpm->ccpmGenerateCurve->setVisible(true);
        m_ccpm->CurveToGenerate->setVisible(true);
    }
    if ( CurveType.compare("Step")==0)
    {
        m_ccpm->CurveLabel1->setText("Min");
        m_ccpm->CurveLabel1->setVisible(true);
        m_ccpm->CurveValue1->setVisible(true);
        m_ccpm->CurveLabel2->setText("Max");
        m_ccpm->CurveLabel2->setVisible(true);
        m_ccpm->CurveValue2->setVisible(true);
        m_ccpm->CurveLabel3->setText("Step at");
        m_ccpm->CurveLabel3->setVisible(true);
        m_ccpm->CurveValue3->setVisible(true);
        m_ccpm->ccpmGenerateCurve->setVisible(true);
        m_ccpm->CurveToGenerate->setVisible(true);
    }
    if ( CurveType.compare("Exp")==0)
    {
        m_ccpm->CurveLabel1->setText("Min");
        m_ccpm->CurveLabel1->setVisible(true);
        m_ccpm->CurveValue1->setVisible(true);
        m_ccpm->CurveLabel2->setText("Max");
        m_ccpm->CurveLabel2->setVisible(true);
        m_ccpm->CurveValue2->setVisible(true);
        m_ccpm->CurveLabel3->setText("Strength");
        m_ccpm->CurveLabel3->setVisible(true);
        m_ccpm->CurveValue3->setVisible(true);
        m_ccpm->CurveValue3->setMinimum(1.0);
        m_ccpm->CurveValue3->setMaximum(100.0);
        m_ccpm->CurveValue3->setSingleStep(1.0);
        m_ccpm->CurveValue3->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);;
        m_ccpm->ccpmGenerateCurve->setVisible(true);
        m_ccpm->CurveToGenerate->setVisible(true);
    }
    if ( CurveType.compare("Log")==0)
    {
        m_ccpm->CurveLabel1->setText("Min");
        m_ccpm->CurveLabel1->setVisible(true);
        m_ccpm->CurveValue1->setVisible(true);
        m_ccpm->CurveLabel2->setText("Max");
        m_ccpm->CurveLabel2->setVisible(true);
        m_ccpm->CurveValue2->setVisible(true);
        m_ccpm->CurveLabel3->setText("Strength");
        m_ccpm->CurveLabel3->setVisible(true);
        m_ccpm->CurveValue3->setVisible(true);
        m_ccpm->CurveValue3->setMinimum(1.0);
        m_ccpm->CurveValue3->setMaximum(100.0);
        m_ccpm->CurveValue3->setSingleStep(1.0);
        m_ccpm->CurveValue3->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);;
        m_ccpm->ccpmGenerateCurve->setVisible(true);
        m_ccpm->CurveToGenerate->setVisible(true);
    }
    if ( CurveType.compare("Custom")==0)
    {
        m_ccpm->CurveLabel1->setVisible(false);
        m_ccpm->CurveValue1->setVisible(false);
        m_ccpm->CurveLabel2->setVisible(false);
        m_ccpm->CurveValue2->setVisible(false);
        m_ccpm->CurveLabel3->setVisible(false);
        m_ccpm->CurveValue3->setVisible(false);
        m_ccpm->ccpmGenerateCurve->setVisible(false);
        m_ccpm->CurveToGenerate->setVisible(false);
    }
UpdateCurveWidgets();

}
void ConfigccpmWidget::GenerateCurve()
{
   int NumCurvePoints,CurveToGenerate,i;
   double value1, value2, value3, scale;
   QString CurveType;
   QTableWidgetItem *item;
   double newValue;


   //get the user settings
   NumCurvePoints=m_ccpm->NumCurvePoints->value();
   value1=m_ccpm->CurveValue1->value();
   value2=m_ccpm->CurveValue2->value();
   value3=m_ccpm->CurveValue3->value();
   CurveToGenerate=m_ccpm->CurveToGenerate->currentIndex();
   CurveType=m_ccpm->CurveType->currentText();



   for (i=0;i<NumCurvePoints;i++)
   {
       scale =((double)i/(double)(NumCurvePoints-1));
       item =m_ccpm->CurveSettings->item(i, CurveToGenerate );

       if ( CurveType.compare("Flat")==0)
       {
           //item->setText( tr( "%1" ).arg( value1 ) );
           item->setText(QString().sprintf("%.3f",value1));
       }
       if ( CurveType.compare("Linear")==0)
       {
           newValue =value1 +(scale*(value2-value1));
           //item->setText( tr( "%1" ).arg(value1 +(scale*(value2-value1))) );
           item->setText(QString().sprintf("%.3f",newValue));
       }
       if ( CurveType.compare("Step")==0)
       {
           if (scale*100<value3)
           {
               //item->setText( tr( "%1" ).arg(value1) );
               item->setText(QString().sprintf("%.3f",value1));
           }
           else
           {
               //item->setText( tr( "%1" ).arg(value2) );
               item->setText(QString().sprintf("%.3f",value2));
           }
       }
       if ( CurveType.compare("Exp")==0)
       {
           newValue =value1 +(((exp(scale*(value3/10))-1))/(exp((value3/10))-1)*(value2-value1));
           //item->setText( tr( "%1" ).arg(value1 +(((exp(scale*(value3/10))-1))/(exp((value3/10))-1)*(value2-value1))) );
           item->setText(QString().sprintf("%.3f",newValue));
       }
       if ( CurveType.compare("Log")==0)
       {
           newValue = value1 +(((log(scale*(value3*2)+1))/(log(1+(value3*2))))*(value2-value1));
           //item->setText( tr( "%1" ).arg(value1 +(((log(scale*(value3*2)+1))/(log(1+(value3*2))))*(value2-value1))) );
           item->setText(QString().sprintf("%.3f",newValue));
       }
   }
   for (i=NumCurvePoints;i<10;i++)
   {
       item =m_ccpm->CurveSettings->item(i, CurveToGenerate );
       item->setText( tr( "" ) );
   }
    UpdateCurveWidgets();

}

void ConfigccpmWidget::ccpmSwashplateUpdate()
{
    double angle,CorrectionAngle,x,y,CenterX,CenterY;
    int used;

    CorrectionAngle=m_ccpm->ccpmCorrectionAngle->value();

    //CenterX=m_ccpm->SwashplateImage->scene()->sceneRect().center().x();
   // CenterY=m_ccpm->SwashplateImage->scene()->sceneRect().center().y();
    CenterX=200;
    CenterY=220;

    SwashplateImg->setPos(CenterX-200,CenterY-200);

    used=((m_ccpm->ccpmServoWChannel->currentIndex()<8)&&(m_ccpm->ccpmServoWChannel->isEnabled()));
    ServoW->setVisible(used!=0);
    ServoWText->setVisible(used!=0);
    angle=(CorrectionAngle+180+m_ccpm->ccpmAngleW->value())*Pi/180.00;
    x=CenterX-(200.00*sin(angle))-10.00;
    y=CenterY+(200.00*cos(angle))-10.00;
    ServoW->setPos(x, y);
    x=CenterX-(170.00*sin(angle))-10.00;
    y=CenterY+(170.00*cos(angle))-10.00;
    ServoWText->setPos(x, y);

    used=((m_ccpm->ccpmServoXChannel->currentIndex()<8)&&(m_ccpm->ccpmServoXChannel->isEnabled()));
    ServoX->setVisible(used!=0);
    ServoXText->setVisible(used!=0);
    angle=(CorrectionAngle+180+m_ccpm->ccpmAngleX->value())*Pi/180.00;
    x=CenterX-(200.00*sin(angle))-10.00;
    y=CenterY+(200.00*cos(angle))-10.00;
    ServoX->setPos(x, y);
    x=CenterX-(170.00*sin(angle))-10.00;
    y=CenterY+(170.00*cos(angle))-10.00;
    ServoXText->setPos(x, y);

    used=((m_ccpm->ccpmServoYChannel->currentIndex()<8)&&(m_ccpm->ccpmServoYChannel->isEnabled()));
    ServoY->setVisible(used!=0);
    ServoYText->setVisible(used!=0);
    angle=(CorrectionAngle+180+m_ccpm->ccpmAngleY->value())*Pi/180.00;
    x=CenterX-(200.00*sin(angle))-10.00;
    y=CenterY+(200.00*cos(angle))-10.00;
    ServoY->setPos(x, y);
    x=CenterX-(170.00*sin(angle))-10.00;
    y=CenterY+(170.00*cos(angle))-10.00;
    ServoYText->setPos(x, y);

    used=((m_ccpm->ccpmServoZChannel->currentIndex()<8)&&(m_ccpm->ccpmServoZChannel->isEnabled()));
    ServoZ->setVisible(used!=0);
    ServoZText->setVisible(used!=0);
    angle=(CorrectionAngle+180+m_ccpm->ccpmAngleZ->value())*Pi/180.00;
    x=CenterX-(200.00*sin(angle))-10.00;
    y=CenterY+(200.00*cos(angle))-10.00;
    ServoZ->setPos(x, y);
    x=CenterX-(170.00*sin(angle))-10.00;
    y=CenterY+(170.00*cos(angle))-10.00;
    ServoZText->setPos(x, y);

    //m_ccpm->SwashplateImage->centerOn (CenterX, CenterY);

    //m_ccpm->SwashplateImage->fitInView(SwashplateImg, Qt::KeepAspectRatio);

    UpdateMixer();

}

void ConfigccpmWidget::UpdateMixer()
{
    int i,j,Type,ThisEnable[6];
    float CollectiveConstant,CorrectionAngle,ThisAngle[6];
    //QTableWidgetItem *newItem;// = new QTableWidgetItem();
    QString Channel;

    Type = m_ccpm->ccpmType->count() - m_ccpm->ccpmType->currentIndex()-1;
    CollectiveConstant=m_ccpm->ccpmCollectiveSlider->value()/100.0;
    CorrectionAngle=m_ccpm->ccpmCorrectionAngle->value();


    if (Type>0)
    {//not advanced settings
        //get the channel data from the ui
        MixerChannelData[0] = m_ccpm->ccpmEngineChannel->currentIndex();
        MixerChannelData[1] = m_ccpm->ccpmTailChannel->currentIndex();
        MixerChannelData[2] = m_ccpm->ccpmServoWChannel->currentIndex();
        MixerChannelData[3] = m_ccpm->ccpmServoXChannel->currentIndex();
        MixerChannelData[4] = m_ccpm->ccpmServoYChannel->currentIndex();
        MixerChannelData[5] = m_ccpm->ccpmServoZChannel->currentIndex();

        //get the angle data from the ui
        ThisAngle[2] = m_ccpm->ccpmAngleW->value();
        ThisAngle[3] = m_ccpm->ccpmAngleX->value();
        ThisAngle[4] = m_ccpm->ccpmAngleY->value();
        ThisAngle[5] = m_ccpm->ccpmAngleZ->value();

        //get the angle data from the ui
        ThisEnable[2] = m_ccpm->ccpmServoWChannel->isEnabled();
        ThisEnable[3] = m_ccpm->ccpmServoXChannel->isEnabled();
        ThisEnable[4] = m_ccpm->ccpmServoYChannel->isEnabled();
        ThisEnable[5] = m_ccpm->ccpmServoZChannel->isEnabled();

        ServoWText->setPlainText(QString("%1").arg( MixerChannelData[2] ));
        ServoXText->setPlainText(QString("%1").arg( MixerChannelData[3] ));
        ServoYText->setPlainText(QString("%1").arg( MixerChannelData[4] ));
        ServoZText->setPlainText(QString("%1").arg( MixerChannelData[5] ));


        //go through the user data and update the mixer matrix
        for (i=0;i<6;i++)
        {
            /*
                data.Mixer0Type = 0;//Disabled,Motor,Servo
                data.Mixer0Vector[0] = 0;//ThrottleCurve1
                data.Mixer0Vector[1] = 0;//ThrottleCurve2
                data.Mixer0Vector[2] = 0;//Roll
                data.Mixer0Vector[3] = 0;//Pitch
                data.Mixer0Vector[4] = 0;//Yaw

            */
            if ((MixerChannelData[i]<8)&&((ThisEnable[i])||(i<2)))
            {
                m_ccpm->ccpmAdvancedSettingsTable->item(i,0)->setText(QString("%1").arg( MixerChannelData[i] ));
                 //config the vector
               if (i==0)
                {//motor-engine
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,1)->setText(QString("%1").arg(127));//ThrottleCurve1
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,2)->setText(QString("%1").arg(0));//ThrottleCurve2
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,3)->setText(QString("%1").arg(0));//Roll
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,4)->setText(QString("%1").arg(0));//Pitch
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,5)->setText(QString("%1").arg(0));//Yaw
                }
                if (i==1)
                {//tailrotor
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,1)->setText(QString("%1").arg(0));//ThrottleCurve1
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,2)->setText(QString("%1").arg(0));//ThrottleCurve2
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,3)->setText(QString("%1").arg(0));//Roll
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,4)->setText(QString("%1").arg(0));//Pitch
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,5)->setText(QString("%1").arg(127));//Yaw
                }
                if (i>1)
                {//Swashplate
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,1)->setText(QString("%1").arg(0));//ThrottleCurve1
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,2)->setText(QString("%1").arg((int)(127.0*CollectiveConstant)));//ThrottleCurve2
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,3)->setText(QString("%1").arg((int)(127.0*(1-CollectiveConstant)*sin((CorrectionAngle + ThisAngle[i])*Pi/180.00))));//Roll
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,4)->setText(QString("%1").arg((int)(127.0*(1-CollectiveConstant)*cos((CorrectionAngle + ThisAngle[i])*Pi/180.00))));//Pitch
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,5)->setText(QString("%1").arg(0));//Yaw

                }
            }
            else
            {
                for (j=0;j<6;j++) m_ccpm->ccpmAdvancedSettingsTable->item(i,j)->setText(QString("-"));
            }

        }
    }
    else
    {//advanced settings
         for (i=0;i<6;i++)
         {
             Channel =m_ccpm->ccpmAdvancedSettingsTable->item(i,0)->text();
             if (Channel == "-") Channel = QString("8");
             MixerChannelData[i]= Channel.toInt();
         }
    }



}

/**************************
  * ccpm settings
  **************************/
/**
  Request the current value of the SystemSettings which holds the ccpm type
  */
void ConfigccpmWidget::requestccpmUpdate()
{
#define MaxAngleError 2
    int MixerDataFromHeli[8][5];
    QString MixerOutputType[8];
    int EngineChannel,TailRotorChannel,ServoChannels[4],ServoAngles[4],SortAngles[4],CalcAngles[4],ServoCurve2[4];
    int NumServos=0;
    double Collective=0.0;
    double a1,a2;
    int HeadRotation,temp;
    int isCCPM=0;

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    int i,j;
    UAVObjectField *field;
    UAVDataObject* obj;
    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);

    //go through the user data and update the mixer matrix
    for (i=0;i<8;i++)
    {
            field = obj->getField(tr( "Mixer%1Vector" ).arg(i));
            //config the vector
            for (j=0;j<5;j++)
            {
                MixerDataFromHeli[i][j] = field->getValue(j).toInt();
                //field->setValue(m_ccpm->ccpmAdvancedSettingsTable->item(i,j+1)->text().toInt(),j);
            }
    }
    for (i=0;i<8;i++)
    {
            field = obj->getField(tr( "Mixer%1Type" ).arg(i));
            MixerOutputType[i] = field->getValue().toString();
    }

    EngineChannel =-1;
    TailRotorChannel =-1;
    for (j=0;j<5;j++)
    {
        ServoChannels[j]=8;
        ServoCurve2[j]=0;
        ServoAngles[j]=0;
        SortAngles[j]=j;
    }

    NumServos=0;
    //process the data from Heli and try to figure out the settings...
    for (i=0;i<8;i++)
    {
        //check if this is the engine... Throttle only
        if ((MixerOutputType[i].compare("Motor")==0)&&
            (MixerDataFromHeli[i][0]>0)&&//ThrottleCurve1
            (MixerDataFromHeli[i][1]==0)&&//ThrottleCurve2
            (MixerDataFromHeli[i][2]==0)&&//Roll
            (MixerDataFromHeli[i][3]==0)&&//Pitch
            (MixerDataFromHeli[i][4]==0))//Yaw
        {
            EngineChannel = i;
            m_ccpm->ccpmEngineChannel->setCurrentIndex(i);

        }
        //check if this is the tail rotor... REVO and YAW
        if ((MixerOutputType[i].compare("Servo")==0)&&
            //(MixerDataFromHeli[i][0]!=0)&&//ThrottleCurve1
            (MixerDataFromHeli[i][1]==0)&&//ThrottleCurve2
            (MixerDataFromHeli[i][2]==0)&&//Roll
            (MixerDataFromHeli[i][3]==0)&&//Pitch
            (MixerDataFromHeli[i][4]!=0))//Yaw
        {
            TailRotorChannel = i;
            m_ccpm->ccpmTailChannel->setCurrentIndex(i);
            m_ccpm->ccpmRevoSlider->setValue((MixerDataFromHeli[i][0]*100)/127);
            m_ccpm->ccpmREVOspinBox->setValue((MixerDataFromHeli[i][0]*100)/127);
        }
        //check if this is a swashplate servo... Throttle is zero
        if ((MixerOutputType[i].compare("Servo")==0)&&
            (MixerDataFromHeli[i][0]==0)&&//ThrottleCurve1
            //(MixerDataFromHeli[i][1]==0)&&//ThrottleCurve2
            //(MixerDataFromHeli[i][2]==0)&&//Roll
            //(MixerDataFromHeli[i][3]==0)&&//Pitch
            (MixerDataFromHeli[i][4]==0))//Yaw
        {
            ServoChannels[NumServos] = i;//record the channel for this servo
            ServoCurve2[NumServos]=MixerDataFromHeli[i][1];//record the ThrottleCurve2 contribution to this servo
            ServoAngles[NumServos]=NumServos*45;//make this 0 for the final version

            //if (NumServos==0)m_ccpm->ccpmServoWChannel->setCurrentIndex(i);
            //if (NumServos==1)m_ccpm->ccpmServoXChannel->setCurrentIndex(i);
            //if (NumServos==2)m_ccpm->ccpmServoYChannel->setCurrentIndex(i);
            //if (NumServos==3)m_ccpm->ccpmServoZChannel->setCurrentIndex(i);
            NumServos++;
        }

    }




    //just call it user angles for now....
    m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->findText("Custom - User Angles"));

    if (NumServos>1)
    {
        if((ServoCurve2[0]==0)&&(ServoCurve2[1]==0)&&(ServoCurve2[2]==0)&&(ServoCurve2[3]==0))
        {
            //fixed pitch heli
            isCCPM=0;
            m_ccpm->ccpmCollectiveSlider->setValue(0);
            Collective = 0.0;
        }
        if(ServoCurve2[0]==ServoCurve2[1])
        {
            if ((NumServos<3)||(ServoCurve2[1]==ServoCurve2[2]))
            {
                if ((NumServos<4)||(ServoCurve2[2]==ServoCurve2[3]))
                {//all the servos have the same ThrottleCurve2 setting so this must be a CCPM config
                    isCCPM=1;
                    Collective = ((double)ServoCurve2[0]*100.00)/127.00;
                    m_ccpm->ccpmCollectiveSlider->setValue((int)Collective);
                    m_ccpm->ccpmCollectivespinBox->setValue((int)Collective);

                 }
            }
        }
    }
    else
    {//must be a custom config... "Custom - Advanced Settings"
        m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->findText("Custom - Advanced Settings"));
    }

    HeadRotation=0;
    //calculate the angles
    for(j=0;j<NumServos;j++)
    {
        //MixerDataFromHeli[i][2]=(127.0*(1-CollectiveConstant)*sin((CorrectionAngle + ThisAngle[i])*Pi/180.00))));//Roll
        //MixerDataFromHeli[i][3]=(127.0*(1-CollectiveConstant)*cos((CorrectionAngle + ThisAngle[i])*Pi/180.00))));//Pitch
        a1=((double)MixerDataFromHeli[ServoChannels[j]][2]/(1.27*(100.0-Collective)));
        a2=((double)MixerDataFromHeli[ServoChannels[j]][3]/(1.27*(100.0-Collective)));
        ServoAngles[j]=fmod(360.0+atan2(a1,a2)/(Pi/180.00),360.0);
        //check the angles for one being a multiple of 90deg
        if (fmod(ServoAngles[j],90)<MaxAngleError)
        {
            HeadRotation=ServoAngles[j]/90;
        }

    }
    //set the head rotation
    m_ccpm->ccpmSingleServo->setCurrentIndex(HeadRotation);

    //calculate the un rotated angles
    for(j=0;j<NumServos;j++)
    {
      CalcAngles[j] = fmod(360.0+ServoAngles[j]-(double)HeadRotation*90.0,360.0);
    }
    //sort the calc angles do the smallest is first...brute force...
    for(i=0;i<5;i++)
    for(j=0;j<NumServos-1;j++)
    {
      if (CalcAngles[SortAngles[j]] > CalcAngles[SortAngles[j+1]])
      {//swap the sorted angles
        temp = SortAngles[j];
        SortAngles[j]=SortAngles[j+1];
        SortAngles[j+1]=temp;
      }

    }

    m_ccpm->ccpmAngleW->setValue(ServoAngles[SortAngles[0]]);
    m_ccpm->ccpmAngleX->setValue(ServoAngles[SortAngles[1]]);
    m_ccpm->ccpmAngleY->setValue(ServoAngles[SortAngles[2]]);
    m_ccpm->ccpmAngleZ->setValue(ServoAngles[SortAngles[3]]);

    m_ccpm->ccpmServoWChannel->setCurrentIndex(ServoChannels[SortAngles[0]]);
    m_ccpm->ccpmServoXChannel->setCurrentIndex(ServoChannels[SortAngles[1]]);
    m_ccpm->ccpmServoYChannel->setCurrentIndex(ServoChannels[SortAngles[2]]);
    m_ccpm->ccpmServoZChannel->setCurrentIndex(ServoChannels[SortAngles[3]]);


    //Types << "CCPM 2 Servo 90�" << "CCPM 3 Servo 120�" << "CCPM 3 Servo 140�" << "FP 2 Servo 90�"  << "Custom - User Angles" << "Custom - Advanced Settings"  ;


    //check this against known combinations
    if (NumServos==2)
    {
        if ((fabs(CalcAngles[SortAngles[0]])<MaxAngleError)&&
            (fabs(CalcAngles[SortAngles[1]]-90)<MaxAngleError))
        {// two servo 90�
            if (isCCPM)
            {
                m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->findText("CCPM 2 Servo 90�"));
                UpdateType();
            }
            else
            {
                m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->findText("FP 2 Servo 90�"));
                UpdateType();
            }

        }
    }
    if (NumServos==3)
    {
        if ((fabs(CalcAngles[SortAngles[0]])<MaxAngleError)&&
            (fabs(CalcAngles[SortAngles[1]]-120)<MaxAngleError)&&
            (fabs(CalcAngles[SortAngles[2]]-240)<MaxAngleError))
        {// three servo 120�
            if (isCCPM)
            {
                m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->findText("CCPM 3 Servo 120�"));
                UpdateType();

            }
            else
            {
                m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->findText("FP 3 Servo 120�"));
                UpdateType();
            }

        }
        else if ((fabs(CalcAngles[SortAngles[0]])<MaxAngleError)&&
            (fabs(CalcAngles[SortAngles[1]]-140)<MaxAngleError)&&
            (fabs(CalcAngles[SortAngles[2]]-220)<MaxAngleError))
        {// three servo 140�
            if (isCCPM)
            {
                m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->findText("CCPM 3 Servo 140�"));
                UpdateType();
            }
            else
            {
                m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->findText("FP 3 Servo 140�"));
                UpdateType();
            }

        }

    }
    if (NumServos==4)
    {

    }




    //get the settings for the curve from the mixer settings
    field = obj->getField(QString("ThrottleCurve1"));
    for (i=0;i<5;i++)
    {
        m_ccpm->CurveSettings->item(i, 0)->setText(QString().sprintf("%.3f",field->getValue(i).toDouble()));
        //m_ccpm->CurveSettings->item(i, 0)->setText(field->getValue(i).toString());
    }
    field = obj->getField(QString("ThrottleCurve2"));
    for (i=0;i<5;i++)
    {
        m_ccpm->CurveSettings->item(i, 1)->setText(QString().sprintf("%.3f",field->getValue(i).toDouble()));
        //m_ccpm->CurveSettings->item(i, 1)->setText(field->getValue(i).toString());
    }





    ccpmSwashplateUpdate();

}


/**
  Sends the config to the board (ccpm type)
  */
void ConfigccpmWidget::sendccpmUpdate()
{
    int i,j;
    UAVObjectField *field;
    UAVDataObject* obj;

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("MixerSettings")));
        Q_ASSERT(obj);

        UpdateMixer();

        //clear the output types
        for (i=0;i<8;i++)
        {
            field = obj->getField(tr( "Mixer%1Type" ).arg( i ));
            //clear the mixer type
            field->setValue("Disabled");
        }


        //go through the user data and update the mixer matrix
        for (i=0;i<6;i++)
        {
            /*
                data.Mixer0Type = 0;//Disabled,Motor,Servo
                data.Mixer0Vector[0] = 0;//ThrottleCurve1
                data.Mixer0Vector[1] = 0;//ThrottleCurve2
                data.Mixer0Vector[2] = 0;//Roll
                data.Mixer0Vector[3] = 0;//Pitch
                data.Mixer0Vector[4] = 0;//Yaw

            */
            if (MixerChannelData[i]<8)
            {
                //select the correct mixer for this config element
                field = obj->getField(tr( "Mixer%1Type" ).arg( MixerChannelData[i] ));
                //set the mixer type
                if (i==0)
                {
                    field->setValue("Motor");
                }
                else
                {
                    field->setValue("Servo");
                }

                //select the correct mixer for this config element
                field = obj->getField(tr( "Mixer%1Vector" ).arg( MixerChannelData[i] ));
                //config the vector
                for (j=0;j<5;j++)
                {
                    field->setValue(m_ccpm->ccpmAdvancedSettingsTable->item(i,j+1)->text().toInt(),j);
                }

            }

        }


 		//get the user data for the curve into the mixer settings
       field = obj->getField(QString("ThrottleCurve1"));
        for (i=0;i<5;i++)
        {
            field->setValue(m_ccpm->CurveSettings->item(i, 0)->text().toDouble(),i);
        }
        field = obj->getField(QString("ThrottleCurve2"));
        for (i=0;i<5;i++)
        {
            field->setValue(m_ccpm->CurveSettings->item(i, 1)->text().toDouble(),i);
        }

        obj->updated();


}

/**
  Send ccpm type to the board and request saving to SD card
  */
void ConfigccpmWidget::saveccpmUpdate()
{
    // Send update so that the latest value is saved
    sendccpmUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);
}

