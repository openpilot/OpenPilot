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
#include <QMessageBox>

#define  Pi 3.14159265358979323846


ConfigccpmWidget::ConfigccpmWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    int i;
    m_ccpm = new Ui_ccpmWidget();
    m_ccpm->setupUi(this);
    SwashLvlConfigurationInProgress=0;
    SwashLvlState=0;
    SwashLvlServoInterlock=0;

    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    //ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    //UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    // Initialization of the swashplaye widget
    m_ccpm->SwashplateImage->setScene(new QGraphicsScene(this));

    m_ccpm->SwashLvlSwashplateImage->setScene(m_ccpm->SwashplateImage->scene());
    m_ccpm->SwashLvlSwashplateImage->setSceneRect(-50,-30,500,500);
    m_ccpm->SwashLvlSwashplateImage->scale(.85,.85);

    //m_ccpm->SwashplateImage->setSceneRect(SwashplateImg->boundingRect());
    m_ccpm->SwashplateImage->setSceneRect(-50,-30,500,500);
    m_ccpm->SwashplateImage->scale(.85,.85);



    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/ccpm_setup.svg"));


    SwashplateImg = new QGraphicsSvgItem();
    SwashplateImg->setSharedRenderer(renderer);
    SwashplateImg->setElementId("Swashplate");
    SwashplateImg->setObjectName("Swashplate");
    m_ccpm->SwashplateImage->scene()->addItem(SwashplateImg);

    QFont serifFont("Times", 16, QFont::Bold);
    QPen pen;  // creates a default pen

    pen.setStyle(Qt::DotLine);
    pen.setWidth(2);
    pen.setBrush(Qt::gray);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    QList<QString> ServoNames;
    ServoNames << "ServoW" << "ServoX" << "ServoY" << "ServoZ" ;

    for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
    {
        ServoLines[i] = m_ccpm->SwashLvlSwashplateImage->scene()->addLine(0,0,100*i,i*i*100,pen);

        Servos[i] = new QGraphicsSvgItem();
        Servos[i]->setSharedRenderer(renderer);
        Servos[i]->setElementId(ServoNames.at(i));
        m_ccpm->SwashplateImage->scene()->addItem(Servos[i]);

        ServosText[i] = new QGraphicsTextItem();
        ServosText[i]->setDefaultTextColor(Qt::red);
        ServosText[i]->setPlainText(QString("-"));
        ServosText[i]->setFont(serifFont);
        m_ccpm->SwashplateImage->scene()->addItem(ServosText[i]);


        SwashLvlSpinBoxes[i] = new QSpinBox(m_ccpm->SwashLvlSwashplateImage);       // use QGraphicsView
        m_ccpm->SwashLvlSwashplateImage->scene()->addWidget(SwashLvlSpinBoxes[i]);
        //SwashLvlSpinBoxes[i]->move(i*50+50,20);
        //SwashLvlSpinBoxes[i]->resize(40,20);
        //SwashLvlSpinBoxes[i]->heightForWidth()
        SwashLvlSpinBoxes[i]->setFixedSize(50,20);
        SwashLvlSpinBoxes[i]->setMaximum(10000);
        SwashLvlSpinBoxes[i]->setMinimum(0);
        SwashLvlSpinBoxes[i]->setValue(0);

    }


/*
    Servos[0] = new QGraphicsSvgItem();
    Servos[0]->setSharedRenderer(renderer);
    Servos[0]->setElementId("ServoW");
    m_ccpm->SwashplateImage->scene()->addItem(Servos[0]);

    Servos[1] = new QGraphicsSvgItem();
    Servos[1]->setSharedRenderer(renderer);
    Servos[1]->setElementId("ServoX");
    m_ccpm->SwashplateImage->scene()->addItem(Servos[1]);

    Servos[2] = new QGraphicsSvgItem();
    Servos[2]->setSharedRenderer(renderer);
    Servos[2]->setElementId("ServoY");
    m_ccpm->SwashplateImage->scene()->addItem(Servos[2]);

    Servos[3] = new QGraphicsSvgItem();
    Servos[3]->setSharedRenderer(renderer);
    Servos[3]->setElementId("ServoZ");
    m_ccpm->SwashplateImage->scene()->addItem(Servos[3]);


    ServosText[0] = new QGraphicsTextItem();
    ServosText[0]->setDefaultTextColor(Qt::red);
    ServosText[0]->setPlainText(QString("-"));
    ServosText[0]->setFont(serifFont);
    m_ccpm->SwashplateImage->scene()->addItem(ServosText[0]);

    ServosText[1] = new QGraphicsTextItem();
    ServosText[1]->setDefaultTextColor(Qt::red);
    ServosText[1]->setPlainText(QString("-"));
    ServosText[1]->setFont(serifFont);
    m_ccpm->SwashplateImage->scene()->addItem(ServosText[1]);

    ServosText[2] = new QGraphicsTextItem();
    ServosText[2]->setDefaultTextColor(Qt::red);
    ServosText[2]->setPlainText(QString("-"));
    ServosText[2]->setFont(serifFont);
    m_ccpm->SwashplateImage->scene()->addItem(ServosText[2]);

    ServosText[3] = new QGraphicsTextItem();
    ServosText[3]->setDefaultTextColor(Qt::red);
    ServosText[3]->setPlainText(QString("-"));
    ServosText[3]->setFont(serifFont);
    m_ccpm->SwashplateImage->scene()->addItem(ServosText[3]);
*/
    m_ccpm->PitchCurve->setMin(-1);

    resetMixer(m_ccpm->PitchCurve, 5);
    resetMixer(m_ccpm->ThrottleCurve, 5);





    QStringList channels;
    channels << "Channel1" << "Channel2" <<
            "Channel3" << "Channel4" << "Channel5" << "Channel6" << "Channel7" << "Channel8" << "None" ;
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
    m_ccpm->ccpmCollectiveChannel->addItems(channels);
    m_ccpm->ccpmCollectiveChannel->setCurrentIndex(8);

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
    connect(m_ccpm->TabObject, SIGNAL(currentChanged ( QWidget * )), this, SLOT(UpdateType()));

//    connect(m_ccpm->SwashLvlSwashplateImage, SIGNAL(valueChanged(double)), this, SLOT(ccpmSwashplateRedraw()));


    connect(m_ccpm->PitchCurve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updatePitchCurveValue(QList<double>,double)));
    connect(m_ccpm->ThrottleCurve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateThrottleCurveValue(QList<double>,double)));

    connect(m_ccpm->SwashLvlStartButton, SIGNAL(clicked()), this, SLOT(SwashLvlStartButtonPressed()));
    connect(m_ccpm->SwashLvlNextButton, SIGNAL(clicked()), this, SLOT(SwashLvlNextButtonPressed()));
    connect(m_ccpm->SwashLvlCancelButton, SIGNAL(clicked()), this, SLOT(SwashLvlCancelButtonPressed()));
    connect(m_ccpm->SwashLvlFinishButton, SIGNAL(clicked()), this, SLOT(SwashLvlFinishButtonPressed()));

    connect(m_ccpm->ccpmCollectivePassthrough, SIGNAL(clicked()), this, SLOT(UpdatCCPMUIOptions()));
    connect(m_ccpm->ccpmLinkCyclic, SIGNAL(clicked()), this, SLOT(UpdatCCPMUIOptions()));
    connect(m_ccpm->ccpmLinkRoll, SIGNAL(clicked()), this, SLOT(UpdatCCPMUIOptions()));


    
    
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

    UpdatCCPMUIOptions();
    
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


        m_ccpm->ccpmAdvancedSettingsTable->resizeColumnsToContents();
        for (int i=0;i<6;i++) {
            m_ccpm->ccpmAdvancedSettingsTable->setColumnWidth(i,(m_ccpm->ccpmAdvancedSettingsTable->width()-
                                                            m_ccpm->ccpmAdvancedSettingsTable->verticalHeader()->width())/6);
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

void ConfigccpmWidget::ccpmSwashplateRedraw()
{
    double angle[CCPM_MAX_SWASH_SERVOS],CorrectionAngle,x,y,w,h,radius,CenterX,CenterY;
    int used[CCPM_MAX_SWASH_SERVOS],i;

    CorrectionAngle=m_ccpm->ccpmCorrectionAngle->value();

    //CenterX=m_ccpm->SwashplateImage->scene()->sceneRect().center().x();
   // CenterY=m_ccpm->SwashplateImage->scene()->sceneRect().center().y();
    CenterX=200;
    CenterY=220;

    SwashplateImg->setPos(CenterX-200,CenterY-200);

    used[0]=((m_ccpm->ccpmServoWChannel->currentIndex()<8)&&(m_ccpm->ccpmServoWChannel->isEnabled()));
    used[1]=((m_ccpm->ccpmServoXChannel->currentIndex()<8)&&(m_ccpm->ccpmServoXChannel->isEnabled()));
    used[2]=((m_ccpm->ccpmServoYChannel->currentIndex()<8)&&(m_ccpm->ccpmServoYChannel->isEnabled()));
    used[3]=((m_ccpm->ccpmServoZChannel->currentIndex()<8)&&(m_ccpm->ccpmServoZChannel->isEnabled()));
    angle[0]=(CorrectionAngle+180+m_ccpm->ccpmAngleW->value())*Pi/180.00;
    angle[1]=(CorrectionAngle+180+m_ccpm->ccpmAngleX->value())*Pi/180.00;
    angle[2]=(CorrectionAngle+180+m_ccpm->ccpmAngleY->value())*Pi/180.00;
    angle[3]=(CorrectionAngle+180+m_ccpm->ccpmAngleZ->value())*Pi/180.00;


    for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
    {
        radius=210;
        x=CenterX-(radius*sin(angle[i]))-10.00;
        y=CenterY+(radius*cos(angle[i]))-10.00;
        Servos[i]->setPos(x, y);
        Servos[i]->setVisible(used[i]!=0);

        radius=170;
        x=CenterX-(radius*sin(angle[i]))-10.00;
        y=CenterY+(radius*cos(angle[i]))-10.00;
        ServosText[i]->setPos(x, y);
        ServosText[i]->setVisible(used[i]!=0);

        w=SwashLvlSpinBoxes[i]->width()/2;
        h=SwashLvlSpinBoxes[i]->height()/2;
        radius = (215.00+w+h);
        x=CenterX-(radius*sin(angle[i]))-w;
        y=CenterY+(radius*cos(angle[i]))-h;
        SwashLvlSpinBoxes[i]->move(m_ccpm->SwashLvlSwashplateImage->mapFromScene (x, y));
        SwashLvlSpinBoxes[i]->setVisible(used[i]!=0);

        radius=220;
        x=CenterX-(radius*sin(angle[i]));
        y=CenterY+(radius*cos(angle[i]));
        ServoLines[i]->setLine(CenterX,CenterY,x,y);
        ServoLines[i]->setVisible(used[i]!=0);
    }

    //m_ccpm->SwashplateImage->centerOn (CenterX, CenterY);

    //m_ccpm->SwashplateImage->fitInView(SwashplateImg, Qt::KeepAspectRatio);
}

void ConfigccpmWidget::ccpmSwashplateUpdate()
{
    ccpmSwashplateRedraw();
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

        ServosText[0]->setPlainText(QString("%1").arg( MixerChannelData[2]+1 ));
        ServosText[1]->setPlainText(QString("%1").arg( MixerChannelData[3]+1 ));
        ServosText[2]->setPlainText(QString("%1").arg( MixerChannelData[4]+1 ));
        ServosText[3]->setPlainText(QString("%1").arg( MixerChannelData[5]+1 ));


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
                m_ccpm->ccpmAdvancedSettingsTable->item(i,0)->setText(QString("%1").arg( MixerChannelData[i]+1 ));
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
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,3)->setText(QString("%1").arg((int)(127.0*(1-CollectiveConstant)*sin((180+CorrectionAngle + ThisAngle[i])*Pi/180.00))));//Roll
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
             if (Channel == "-") Channel = QString("9");
             MixerChannelData[i]= Channel.toInt();
         }
    }



}

/**************************
  * ccpm settings
  **************************/
/*
 Get the state of the UI check boxes and change the visibility of sliders
 */
void ConfigccpmWidget::UpdatCCPMUIOptions()
{
    int ccpmCollectivePassthroughState;
    int ccpmLinkCyclicState;
    int ccpmLinkRollState;
    
    //get the user options
    ccpmCollectivePassthroughState=m_ccpm->ccpmCollectivePassthrough->isChecked();
    ccpmLinkCyclicState=m_ccpm->ccpmLinkCyclic->isChecked();
    ccpmLinkRollState=m_ccpm->ccpmLinkRoll->isChecked();
    
    //set which sliders are user...
    m_ccpm->ccpmRevoMixingBox->setVisible(0);
    
    m_ccpm->ccpmPitchMixingBox->setVisible(!ccpmCollectivePassthroughState&&ccpmLinkCyclicState);
    m_ccpm->ccpmCollectiveScalingBox->setVisible(ccpmCollectivePassthroughState||!ccpmLinkCyclicState);

    m_ccpm->ccpmCollectiveChLabel->setVisible(ccpmCollectivePassthroughState);
    m_ccpm->ccpmCollectiveChannel->setVisible(ccpmCollectivePassthroughState);

    
    
    m_ccpm->ccpmCyclicScalingBox->setVisible((ccpmCollectivePassthroughState||!ccpmLinkCyclicState)&&ccpmLinkRollState);
    if (ccpmLinkCyclicState)
    {
        m_ccpm->ccpmPitchScalingBox->setVisible(0);
        m_ccpm->ccpmRollScalingBox->setVisible(0);
        m_ccpm->ccpmLinkRoll->setVisible(0);
        
    }
    else
    {
        m_ccpm->ccpmPitchScalingBox->setVisible(!ccpmLinkRollState);
        m_ccpm->ccpmRollScalingBox->setVisible(!ccpmLinkRollState);
        m_ccpm->ccpmLinkRoll->setVisible(1);
    }

}
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

    if (SwashLvlConfigurationInProgress)return;

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
            field = obj->getField(tr( "Mixer%1Vector" ).arg(i+1));
            //config the vector
            for (j=0;j<5;j++)
            {
                MixerDataFromHeli[i][j] = field->getValue(j).toInt();
                //field->setValue(m_ccpm->ccpmAdvancedSettingsTable->item(i,j+1)->text().toInt(),j);
            }
    }
    for (i=0;i<8;i++)
    {
            field = obj->getField(tr( "Mixer%1Type" ).arg(i+1));
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
        ServoAngles[j]=fmod(360.0+atan2(-a1,a2)/(Pi/180.00),360.0);
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

    if (SwashLvlConfigurationInProgress)return;
    ShowDisclaimer(1);
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("MixerSettings")));
        Q_ASSERT(obj);

        UpdateMixer();

        //clear the output types
        for (i=0;i<8;i++)
        {
            field = obj->getField(tr( "Mixer%1Type" ).arg( i+1 ));
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
                field = obj->getField(tr( "Mixer%1Type" ).arg( MixerChannelData[i]+1 ));
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
                field = obj->getField(tr( "Mixer%1Vector" ).arg( MixerChannelData[i]+1 ));
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
    if (SwashLvlConfigurationInProgress)return;
    ShowDisclaimer(0);
    // Send update so that the latest value is saved
    sendccpmUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);
}

void ConfigccpmWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    // Make the custom table columns autostretch:
    m_ccpm->ccpmAdvancedSettingsTable->resizeColumnsToContents();
    for (int i=0;i<6;i++) {
        m_ccpm->ccpmAdvancedSettingsTable->setColumnWidth(i,(m_ccpm->ccpmAdvancedSettingsTable->width()-
                                                        m_ccpm->ccpmAdvancedSettingsTable->verticalHeader()->width())/6);
    }
    ccpmSwashplateRedraw();

}
void ConfigccpmWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    m_ccpm->ccpmAdvancedSettingsTable->resizeColumnsToContents();
    for (int i=0;i<6;i++) {
        m_ccpm->ccpmAdvancedSettingsTable->setColumnWidth(i,(m_ccpm->ccpmAdvancedSettingsTable->width()-
                                                        m_ccpm->ccpmAdvancedSettingsTable->verticalHeader()->width())/6);
    }
}


void ConfigccpmWidget::SwashLvlStartButtonPressed()
{
    QMessageBox msgBox;
    int i;
     msgBox.setText("<h1>Swashplate Leveling Routine</h1>");
     msgBox.setInformativeText("<b>You are about to start the Swashplate levelling routine.</b><p>This process will start by downloading the current configuration from the GCS to the OP hardware and will adjust your configuration at various stages.<p>The final state of your system should match the current configuration in the GCS config gadget.<p>Please ensure all ccpm settings in the GCS are correct before continuing.<p>If this process is interrupted, then the state of your OP board may not match the GCS configuration.<p><i>After completing this process, please check all settings before attempting to fly.</i><p><font color=red><b>Please disconnect your motor to ensure it will not spin up.</b></font><p><hr><i>Do you wish to proceed?</i>");
     msgBox.setStandardButtons(QMessageBox::Yes |  QMessageBox::Cancel);
     msgBox.setDefaultButton(QMessageBox::Cancel);
     msgBox.setIcon(QMessageBox::Information);
     int ret = msgBox.exec();

     UAVObjectField* MinField;
     UAVObjectField* NeutralField;
     UAVObjectField* MaxField;
     UAVDataObject* obj;
     ExtensionSystem::PluginManager *pm;
     UAVObjectManager *objManager;

     switch (ret) {
        case QMessageBox::Yes:
            // Yes was clicked
            SwashLvlState=0;
            //remove Flight control of ActuatorCommand
            enableSwashplateLevellingControl(true);

            m_ccpm->SwashLvlStartButton->setEnabled(false);
            m_ccpm->SwashLvlNextButton->setEnabled(true);
            m_ccpm->SwashLvlCancelButton->setEnabled(true);
            m_ccpm->SwashLvlFinishButton->setEnabled(false);
            //clear status check boxes
            m_ccpm->SwashLvlStepList->item(0)->setCheckState(Qt::Unchecked);
            m_ccpm->SwashLvlStepList->item(1)->setCheckState(Qt::Unchecked);
            m_ccpm->SwashLvlStepList->item(2)->setCheckState(Qt::Unchecked);
            m_ccpm->SwashLvlStepList->item(3)->setCheckState(Qt::Unchecked);


            //download the current settings to the OP hw
            sendccpmUpdate();

            //change control mode to gcs control / disarmed
            //set throttle to 0


            //save off the old ActuatorSettings for the swashplate servos
            pm = ExtensionSystem::PluginManager::instance();
            objManager = pm->getObject<UAVObjectManager>();


            // Get the channel assignements:
            obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
            Q_ASSERT(obj);
            obj->requestUpdate();
            MinField = obj->getField(QString("ChannelMin"));
            NeutralField = obj->getField(QString("ChannelNeutral"));
            MaxField = obj->getField(QString("ChannelMax"));

            //channel assignments
            oldSwashLvlConfiguration.ServoChannels[0]=m_ccpm->ccpmServoWChannel->currentIndex();
            oldSwashLvlConfiguration.ServoChannels[1]=m_ccpm->ccpmServoXChannel->currentIndex();
            oldSwashLvlConfiguration.ServoChannels[2]=m_ccpm->ccpmServoYChannel->currentIndex();
            oldSwashLvlConfiguration.ServoChannels[3]=m_ccpm->ccpmServoZChannel->currentIndex();
            //if servos are used
            oldSwashLvlConfiguration.Used[0]=((m_ccpm->ccpmServoWChannel->currentIndex()<8)&&(m_ccpm->ccpmServoWChannel->isEnabled()));
            oldSwashLvlConfiguration.Used[1]=((m_ccpm->ccpmServoXChannel->currentIndex()<8)&&(m_ccpm->ccpmServoXChannel->isEnabled()));
            oldSwashLvlConfiguration.Used[2]=((m_ccpm->ccpmServoYChannel->currentIndex()<8)&&(m_ccpm->ccpmServoYChannel->isEnabled()));
            oldSwashLvlConfiguration.Used[3]=((m_ccpm->ccpmServoZChannel->currentIndex()<8)&&(m_ccpm->ccpmServoZChannel->isEnabled()));
            //min,neutral,max values for the servos
            for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
            {
                oldSwashLvlConfiguration.Min[i]=MinField->getValue(oldSwashLvlConfiguration.ServoChannels[i]).toInt();
                oldSwashLvlConfiguration.Neutral[i]=NeutralField->getValue(oldSwashLvlConfiguration.ServoChannels[i]).toInt();
                oldSwashLvlConfiguration.Max[i]=MaxField->getValue(oldSwashLvlConfiguration.ServoChannels[i]).toInt();
            }

            //copy to new Actuator settings.
            memcpy((void*)&newSwashLvlConfiguration,(void*)&oldSwashLvlConfiguration,sizeof(SwashplateServoSettingsStruct));

             //goto the first step
            SwashLvlNextButtonPressed();
        break;
        case QMessageBox::Cancel:
            // Cancel was clicked
            SwashLvlState=0;
            //restore Flight control of ActuatorCommand
            enableSwashplateLevellingControl(false);

            m_ccpm->SwashLvlStartButton->setEnabled(true);
            m_ccpm->SwashLvlNextButton->setEnabled(false);
            m_ccpm->SwashLvlCancelButton->setEnabled(false);
            m_ccpm->SwashLvlFinishButton->setEnabled(false);
            break;
        default:
            // should never be reached
            break;
      }


}
void ConfigccpmWidget::SwashLvlNextButtonPressed()
{
    //ShowDisclaimer(2);
    SwashLvlState++;
    int i;




    switch (SwashLvlState)
    {
    case 0:
        break;
    case 1: //Neutral levelling
        m_ccpm->SwashLvlStepList->setCurrentRow(0);
        //set spin boxes and swashplate servos to Neutral values
        setSwashplateLevel(50);
        //disable position slider
        m_ccpm->SwashLvlPositionSlider->setEnabled(false);
        m_ccpm->SwashLvlPositionSpinBox->setEnabled(false);
        //set position slider to 50%
        m_ccpm->SwashLvlPositionSlider->setValue(50);
        m_ccpm->SwashLvlPositionSpinBox->setValue(50);
        //connect spinbox signals to slots and ebnable them
        for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
        {
            connect(SwashLvlSpinBoxes[i], SIGNAL(valueChanged(int)), this, SLOT(SwashLvlSpinBoxChanged(int)));
            SwashLvlSpinBoxes[i]->setEnabled(true);
        }
        //issue user instructions
        m_ccpm->SwashLvlStepInstruction->setHtml("<h2>Neutral levelling</h2><p>Using adjustment of:<ul><li>servo horns<li>link lengths and<li>Neutral timing spinboxes to the right</ul><br>ensure that the swashplate is in the center of desired travel range and is level.");
        break;
    case 2: //Max levelling
        //check Neutral status as complete
        m_ccpm->SwashLvlStepList->item(0)->setCheckState(Qt::Checked);
        m_ccpm->SwashLvlStepList->setCurrentRow(1);
        //set spin boxes and swashplate servos to Max values
        setSwashplateLevel(100);
        //set position slider to 100%
        m_ccpm->SwashLvlPositionSlider->setValue(100);
        m_ccpm->SwashLvlPositionSpinBox->setValue(100);
        //issue user instructions
        m_ccpm->SwashLvlStepInstruction->setText("<h2>Max levelling</h2><p>Using adjustment of:<ul><li>Max timing spinboxes to the right ONLY</ul><br>ensure that the swashplate is at the top of desired travel range and is level.");
        break;
    case 3: //Min levelling
        //check Max status as complete
        m_ccpm->SwashLvlStepList->item(1)->setCheckState(Qt::Checked);
        m_ccpm->SwashLvlStepList->setCurrentRow(2);
        //set spin boxes and swashplate servos to Min values
        setSwashplateLevel(0);
        //set position slider to 0%
        m_ccpm->SwashLvlPositionSlider->setValue(0);
        m_ccpm->SwashLvlPositionSpinBox->setValue(0);
        //issue user instructions
        m_ccpm->SwashLvlStepInstruction->setText("<h2>Min levelling</h2><p>Using adjustment of:<ul><li>Min timing spinboxes to the right ONLY</ul><br>ensure that the swashplate is at the bottom of desired travel range and is level.");
         break;
    case 4: //levelling verification
        //check Min status as complete
        m_ccpm->SwashLvlStepList->item(2)->setCheckState(Qt::Checked);
        m_ccpm->SwashLvlStepList->setCurrentRow(3);
        //enable position slider
        m_ccpm->SwashLvlPositionSlider->setEnabled(true);
        m_ccpm->SwashLvlPositionSpinBox->setEnabled(true);
        //make heli respond to slider movement
        connect(m_ccpm->SwashLvlPositionSlider, SIGNAL(valueChanged(int)), this, SLOT(setSwashplateLevel(int)));
        //disable spin boxes
        for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
        {
            SwashLvlSpinBoxes[i]->setEnabled(false);
        }

        //issue user instructions
        m_ccpm->SwashLvlStepInstruction->setText("<h2>levelling verification</h2><p>Adjust the slider to the right over it's full range and observe the swashplate motion. It should remain level over the entire range of travel.");
         break;
    case 5: //levelling complete
        //check verify status as complete
        m_ccpm->SwashLvlStepList->item(3)->setCheckState(Qt::Checked);
        //issue user instructions
        m_ccpm->SwashLvlStepInstruction->setText("<h2>levelling complete</h2><p>Press the Finish button to save these settings to the SD card<p>Press the cancel button to return to the pre-levelling settings");
        //disable position slider
        m_ccpm->SwashLvlPositionSlider->setEnabled(false);
        m_ccpm->SwashLvlPositionSpinBox->setEnabled(false);
        //disconnect levelling slots from signals
        disconnect(m_ccpm->SwashLvlPositionSlider, SIGNAL(valueChanged(int)), this, SLOT(setSwashplateLevel(int)));
        for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
        {
            disconnect(SwashLvlSpinBoxes[i], SIGNAL(valueChanged(int)), this, SLOT(SwashLvlSpinBoxChanged(int)));
        }

        m_ccpm->SwashLvlStartButton->setEnabled(false);
        m_ccpm->SwashLvlNextButton->setEnabled(false);
        m_ccpm->SwashLvlCancelButton->setEnabled(true);
        m_ccpm->SwashLvlFinishButton->setEnabled(true);

    default:
        //restore collective/cyclic setting
        //restore pitch curve
        //clear spin boxes
        //change control mode to gcs control (OFF) / disarmed
        //issue user confirmation
        break;
    }
}
void ConfigccpmWidget::SwashLvlCancelButtonPressed()
{
    int i;
    SwashLvlState=0;

    UAVObjectField* MinField;
    UAVObjectField* NeutralField;
    UAVObjectField* MaxField;

    m_ccpm->SwashLvlStartButton->setEnabled(true);
    m_ccpm->SwashLvlNextButton->setEnabled(false);
    m_ccpm->SwashLvlCancelButton->setEnabled(false);
    m_ccpm->SwashLvlFinishButton->setEnabled(false);

    m_ccpm->SwashLvlStepList->item(0)->setCheckState(Qt::Unchecked);
    m_ccpm->SwashLvlStepList->item(1)->setCheckState(Qt::Unchecked);
    m_ccpm->SwashLvlStepList->item(2)->setCheckState(Qt::Unchecked);
    m_ccpm->SwashLvlStepList->item(3)->setCheckState(Qt::Unchecked);

    //restore old Actuator Settings
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    //update settings to match our changes.
    MinField = obj->getField(QString("ChannelMin"));
    NeutralField = obj->getField(QString("ChannelNeutral"));
    MaxField = obj->getField(QString("ChannelMax"));

    //min,neutral,max values for the servos
    for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
    {
        MinField->setValue(oldSwashLvlConfiguration.Min[i],oldSwashLvlConfiguration.ServoChannels[i]);
        NeutralField->setValue(oldSwashLvlConfiguration.Neutral[i],oldSwashLvlConfiguration.ServoChannels[i]);
        MaxField->setValue(oldSwashLvlConfiguration.Max[i],oldSwashLvlConfiguration.ServoChannels[i]);
    }

    obj->updated();


    //restore Flight control of ActuatorCommand
    enableSwashplateLevellingControl(false);

    m_ccpm->SwashLvlStepInstruction->setText("<h2>Levelling Cancelled</h2><p>Previous settings have been restored.");

}


void ConfigccpmWidget::SwashLvlFinishButtonPressed()
{
    int i;

    UAVObjectField* MinField;
    UAVObjectField* NeutralField;
    UAVObjectField* MaxField;

    m_ccpm->SwashLvlStartButton->setEnabled(true);
    m_ccpm->SwashLvlNextButton->setEnabled(false);
    m_ccpm->SwashLvlCancelButton->setEnabled(false);
    m_ccpm->SwashLvlFinishButton->setEnabled(false);

    //save new Actuator Settings to memory and SD card
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    //update settings to match our changes.
    MinField = obj->getField(QString("ChannelMin"));
    NeutralField = obj->getField(QString("ChannelNeutral"));
    MaxField = obj->getField(QString("ChannelMax"));

    //min,neutral,max values for the servos
    for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
    {
        MinField->setValue(newSwashLvlConfiguration.Min[i],newSwashLvlConfiguration.ServoChannels[i]);
        NeutralField->setValue(newSwashLvlConfiguration.Neutral[i],newSwashLvlConfiguration.ServoChannels[i]);
        MaxField->setValue(newSwashLvlConfiguration.Max[i],newSwashLvlConfiguration.ServoChannels[i]);
    }

    obj->updated();

    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);

    //restore Flight control of ActuatorCommand
    enableSwashplateLevellingControl(false);

    m_ccpm->SwashLvlStepInstruction->setText("<h2>Levelling Completed</h2><p>New settings have been saved to the SD card");

    ShowDisclaimer(0);
    //ShowDisclaimer(2);

}

int ConfigccpmWidget::ShowDisclaimer(int messageID)
{
     QMessageBox msgBox;
     msgBox.setText("<font color=red><h1>Warning!!!</h2></font>");
     int ret;
     switch (messageID) {
        case 0:
            // Basic disclaimer
             msgBox.setInformativeText("<h2>This code has many configurations.</h2><p>Please double check all settings before attempting flight!");
             msgBox.setStandardButtons(QMessageBox::Ok);
             msgBox.setDefaultButton(QMessageBox::Ok);
             msgBox.setIcon(QMessageBox::Information);
             ret = msgBox.exec();
             return 0;
            break;
        case 1:
            // Not Tested disclaimer
             msgBox.setInformativeText("<h2>The CCPM mixer code has not been used to fly a helicopter!</h2><p><font color=red>Use it at your own risk!</font><p>Do you wish to continue?");
             msgBox.setStandardButtons(QMessageBox::Yes |  QMessageBox::Cancel);
             msgBox.setDefaultButton(QMessageBox::Cancel);
             msgBox.setIcon(QMessageBox::Warning);
             ret = msgBox.exec();
             switch (ret)
             {
             case QMessageBox::Cancel: return -1;
             case QMessageBox::Yes: return 0;
             }
            break;
        case 2:
            // DO NOT use
            msgBox.setInformativeText("<h2>The CCPM swashplate levelling code is NOT complete!</h2><p><font color=red>DO NOT use it for flight!</font>");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Critical);
            ret = msgBox.exec();
            return 0;
            break;
       default:
            // should never be reached
            break;
        }
    return -1;
}


/**
  Toggles the channel testing mode by making the GCS take over
  the ActuatorCommand objects
  */
void ConfigccpmWidget::enableSwashplateLevellingControl(bool state)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorCommand")));
    UAVObject::Metadata mdata = obj->getMetadata();
    if (state)
    {
        SwashLvlaccInitialData = mdata;
        mdata.flightAccess = UAVObject::ACCESS_READONLY;
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
        mdata.gcsTelemetryAcked = false;
        mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
        mdata.gcsTelemetryUpdatePeriod = 100;
        SwashLvlConfigurationInProgress=1;
        connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),this, SLOT(FocusChanged(QWidget*,QWidget*)));
        m_ccpm->TabObject->setTabEnabled(0,0);
        m_ccpm->TabObject->setTabEnabled(2,0);
        m_ccpm->TabObject->setTabEnabled(3,0);
        m_ccpm->ccpmType->setEnabled(0);
    }
    else
    {
        mdata = SwashLvlaccInitialData; // Restore metadata
        SwashLvlConfigurationInProgress=0;

        disconnect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),this, SLOT(FocusChanged(QWidget*,QWidget*)));
        m_ccpm->TabObject->setTabEnabled(0,1);
        m_ccpm->TabObject->setTabEnabled(2,1);
        m_ccpm->TabObject->setTabEnabled(3,1);
        m_ccpm->ccpmType->setEnabled(1);

    }
    obj->setMetadata(mdata);

}

/**
  Sets the swashplate level to a given value based on current settings for Max, Neutral and Min values.
  level ranges -1 to +1
  */
void ConfigccpmWidget::setSwashplateLevel(int percent)
{
    if (percent<0)return;// -1;
    if (percent>100)return;// -1;
    if (SwashLvlConfigurationInProgress!=1)return;// -1;
    int i;
    double value;
    double level = ((double)percent /50.00) - 1.00;

    SwashLvlServoInterlock=1;

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorCommand")));
    UAVObjectField * channel = obj->getField("Channel");



    if (level==0)
    {
        for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
        {
            channel->setValue(newSwashLvlConfiguration.Neutral[i],newSwashLvlConfiguration.ServoChannels[i]);
            SwashLvlSpinBoxes[i]->setValue(newSwashLvlConfiguration.Neutral[i]);
        }

    }
    else if (level>0)
    {
        for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
        {
            value = (newSwashLvlConfiguration.Max[i] - newSwashLvlConfiguration.Neutral[i])*level + newSwashLvlConfiguration.Neutral[i];
            channel->setValue(value,newSwashLvlConfiguration.ServoChannels[i]);
            SwashLvlSpinBoxes[i]->setValue(value);
        }

    }
    else if (level<0)
    {
        for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
        {
            value = (newSwashLvlConfiguration.Neutral[i] - newSwashLvlConfiguration.Min[i])*level + newSwashLvlConfiguration.Neutral[i];
            channel->setValue(value,newSwashLvlConfiguration.ServoChannels[i]);
            SwashLvlSpinBoxes[i]->setValue(value);
        }

    }
    obj->updated();
    SwashLvlServoInterlock=0;

return;
}


void ConfigccpmWidget::SwashLvlSpinBoxChanged(int value)
{
    int i;
    if (SwashLvlServoInterlock==1)return;
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorCommand")));
    UAVObjectField * channel = obj->getField("Channel");

    switch (SwashLvlState)
    {
    case 0:
        break;
    case 1: //Neutral levelling
        for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
        {
            newSwashLvlConfiguration.Neutral[i]=SwashLvlSpinBoxes[i]->value();
            channel->setValue(newSwashLvlConfiguration.Neutral[i],newSwashLvlConfiguration.ServoChannels[i]);
        }
        obj->updated();
        break;
    case 2: //Max levelling
        for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
        {
            newSwashLvlConfiguration.Max[i]=SwashLvlSpinBoxes[i]->value();
            channel->setValue(newSwashLvlConfiguration.Max[i],newSwashLvlConfiguration.ServoChannels[i]);
        }
        obj->updated();
        break;
    case 3: //Min levelling
        for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++)
        {
            newSwashLvlConfiguration.Min[i]=SwashLvlSpinBoxes[i]->value();
            channel->setValue(newSwashLvlConfiguration.Min[i],newSwashLvlConfiguration.ServoChannels[i]);
        }
        obj->updated();
        break;
    case 4: //levelling verification
        break;
    case 5: //levelling complete
        break;
    default:
        break;
    }
    return;
}


void ConfigccpmWidget::FocusChanged(QWidget *oldFocus, QWidget *newFocus)
{
    if (SwashLvlConfigurationInProgress!=1) return;
    QMessageBox msgBox;
    int ret;
    msgBox.setText("<h1>Warning!!!</h1>");

    if ((this->isAncestorOf(oldFocus))&&(!this->isAncestorOf(newFocus)))
    {
        msgBox.setInformativeText("<b>You are in the middle of the levelling routine</b><br>Changing focus will cancel all levelling and return the OP hardware to the state it was in before levelling began.<p>Do you want to continue the levelling routine?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setIcon(QMessageBox::Information);
        ret = msgBox.exec();

        if (ret == QMessageBox::Yes)
        {

            //m_ccpm->TabObject->setCurrentIndex(1);
            //m_ccpm->SwashPlateLevel->setFocus(Qt::MouseFocusReason);
            //m_ccpm->SwashLvlInstructionsBox->setFocus(Qt::MouseFocusReason);
            oldFocus->setFocus(Qt::MouseFocusReason);
        }
        if (ret == QMessageBox::No)
        {
            SwashLvlCancelButtonPressed();
        }
    }
}
