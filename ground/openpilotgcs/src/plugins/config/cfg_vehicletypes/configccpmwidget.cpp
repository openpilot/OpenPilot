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
#include <QBrush>
#include <math.h>
#include <QMessageBox>

#include "mixersettings.h"
#include "systemsettings.h"
#include "actuatorcommand.h"

#define  Pi 3.14159265358979323846


ConfigCcpmWidget::ConfigCcpmWidget(QWidget *parent) : VehicleConfig(parent)
{
    int i;
    SwashLvlConfigurationInProgress=0;
    SwashLvlState=0;
    SwashLvlServoInterlock=0;
    updatingFromHardware=FALSE;
    updatingToHardware=FALSE;

    m_ccpm = new Ui_ccpmWidget();
    m_ccpm->setupUi(this);

    // Initialization of the swashplaye widget
    m_ccpm->SwashplateImage->setScene(new QGraphicsScene(this));

    m_ccpm->SwashLvlSwashplateImage->setScene(m_ccpm->SwashplateImage->scene());
    m_ccpm->SwashLvlSwashplateImage->setSceneRect(-50,-50,500,500);
    //m_ccpm->SwashLvlSwashplateImage->scale(.85,.85);

    //m_ccpm->SwashplateImage->setSceneRect(SwashplateImg->boundingRect());
    m_ccpm->SwashplateImage->setSceneRect(-50,-30,500,500);
    //m_ccpm->SwashplateImage->scale(.85,.85);

    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/ccpm_setup.svg"));


    SwashplateImg = new QGraphicsSvgItem();
    SwashplateImg->setSharedRenderer(renderer);
    SwashplateImg->setElementId("Swashplate");
    SwashplateImg->setObjectName("Swashplate");
    //SwashplateImg->setScale(0.75);
    m_ccpm->SwashplateImage->scene()->addItem(SwashplateImg);

    QFont serifFont("Times", 24, QFont::Bold);
    QPen pen;  // creates a default pen

    pen.setStyle(Qt::DotLine);
    pen.setWidth(2);
    pen.setBrush(Qt::gray);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    
    QBrush brush(Qt::darkBlue);
    QPen pen2;  // creates a default pen
    
    //pen2.setStyle(Qt::DotLine);
    pen2.setWidth(1);
    pen2.setBrush(Qt::blue);
    //pen2.setCapStyle(Qt::RoundCap);
    //pen2.setJoinStyle(Qt::RoundJoin);
    
    
   //brush.setStyle(Qt::RadialGradientPattern);
    
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
        ServosText[i]->setDefaultTextColor(Qt::yellow);
        ServosText[i]->setPlainText(QString("-"));
        ServosText[i]->setFont(serifFont);
        
        ServosTextCircles[i] = new QGraphicsEllipseItem(1,1,30,30);
        ServosTextCircles[i]->setBrush(brush);
        ServosTextCircles[i]->setPen(pen2);
        m_ccpm->SwashplateImage->scene()->addItem(ServosTextCircles[i]);
        m_ccpm->SwashplateImage->scene()->addItem(ServosText[i]);
        


        SwashLvlSpinBoxes[i] = new QSpinBox(m_ccpm->SwashLvlSwashplateImage);       // use QGraphicsView
        m_ccpm->SwashLvlSwashplateImage->scene()->addWidget(SwashLvlSpinBoxes[i]);
        SwashLvlSpinBoxes[i]->setMaximum(10000);
        SwashLvlSpinBoxes[i]->setMinimum(0);
        SwashLvlSpinBoxes[i]->setValue(0);

    }

    //initialize our two mixer curves
    // mixercurve defaults to mixercurve_throttle
    m_ccpm->ThrottleCurve->initLinearCurve(5, 1.0, 0.0);

    // tell mixercurve this is a pitch curve
    m_ccpm->PitchCurve->setMixerType(MixerCurve::MIXERCURVE_PITCH);
    m_ccpm->PitchCurve->initLinearCurve(5, 1.0, -1.0);

    //initialize channel names
    m_ccpm->ccpmEngineChannel->addItems(channelNames);
    m_ccpm->ccpmEngineChannel->setCurrentIndex(0);
    m_ccpm->ccpmTailChannel->addItems(channelNames);
    m_ccpm->ccpmTailChannel->setCurrentIndex(0);
    m_ccpm->ccpmServoWChannel->addItems(channelNames);
    m_ccpm->ccpmServoWChannel->setCurrentIndex(0);
    m_ccpm->ccpmServoXChannel->addItems(channelNames);
    m_ccpm->ccpmServoXChannel->setCurrentIndex(0);
    m_ccpm->ccpmServoYChannel->addItems(channelNames);
    m_ccpm->ccpmServoYChannel->setCurrentIndex(0);
    m_ccpm->ccpmServoZChannel->addItems(channelNames);
    m_ccpm->ccpmServoZChannel->setCurrentIndex(0);

    QStringList Types;
    Types << QString::fromUtf8("CCPM 2 Servo 90º") << QString::fromUtf8("CCPM 3 Servo 90º") <<
             QString::fromUtf8("CCPM 4 Servo 90º") << QString::fromUtf8("CCPM 3 Servo 120º") <<
             QString::fromUtf8("CCPM 3 Servo 140º") << QString::fromUtf8("FP 2 Servo 90º")  <<
             QString::fromUtf8("Custom - User Angles") << QString::fromUtf8("Custom - Advanced Settings");
    m_ccpm->ccpmType->addItems(Types);
    m_ccpm->ccpmType->setCurrentIndex(m_ccpm->ccpmType->count() - 1);

    refreshWidgetsValues(QString("HeliCP"));

    UpdateType();

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
    connect(m_ccpm->TabObject, SIGNAL(currentChanged ( QWidget * )), this, SLOT(UpdateType()));

    connect(m_ccpm->SwashLvlStartButton, SIGNAL(clicked()), this, SLOT(SwashLvlStartButtonPressed()));
    connect(m_ccpm->SwashLvlNextButton, SIGNAL(clicked()), this, SLOT(SwashLvlNextButtonPressed()));
    connect(m_ccpm->SwashLvlCancelButton, SIGNAL(clicked()), this, SLOT(SwashLvlCancelButtonPressed()));
    connect(m_ccpm->SwashLvlFinishButton, SIGNAL(clicked()), this, SLOT(SwashLvlFinishButtonPressed()));

    connect(m_ccpm->ccpmCollectivePassthrough, SIGNAL(clicked()),this, SLOT(SetUIComponentVisibilities()));
    connect(m_ccpm->ccpmLinkCyclic, SIGNAL(clicked()), this, SLOT(SetUIComponentVisibilities()));
    connect(m_ccpm->ccpmLinkRoll, SIGNAL(clicked()), this, SLOT(SetUIComponentVisibilities()));


    
   ccpmSwashplateRedraw(); 
}

ConfigCcpmWidget::~ConfigCcpmWidget()
{
   // Do nothing
}

void ConfigCcpmWidget::setupUI(QString frameType)
{
    Q_UNUSED(frameType);
}

void ConfigCcpmWidget::ResetActuators(GUIConfigDataUnion* configData)
{
    configData->heli.Throttle = 0;
    configData->heli.Tail = 0;
    configData->heli.ServoIndexW = 0;
    configData->heli.ServoIndexX = 0;
    configData->heli.ServoIndexY = 0;
    configData->heli.ServoIndexZ = 0;
}

QStringList ConfigCcpmWidget::getChannelDescriptions()
{
    int i;
    QStringList channelDesc;

    // init a channel_numelem list of channel desc defaults
    for (i=0; i < (int)(ConfigCcpmWidget::CHANNEL_NUMELEM); i++)
    {
        channelDesc.append(QString("-"));
    }

    // get the gui config data
    GUIConfigDataUnion configData = GetConfigData();
    heliGUISettingsStruct heli = configData.heli;

    if (heli.Throttle > 0)
        channelDesc[heli.Throttle - 1] = QString("Throttle");
    if (heli.Tail > 0)
        channelDesc[heli.Tail - 1] = QString("Tail");

    switch(heli.FirstServoIndex)
    {
        case 0:  //front
            if (heli.ServoIndexW > 0)
                channelDesc[heli.ServoIndexW - 1] = QString("Elevator");
            if (heli.ServoIndexX > 0)
                channelDesc[heli.ServoIndexX - 1] = QString("Roll1");
            if (heli.ServoIndexY > 0)
                channelDesc[heli.ServoIndexY - 1] = QString("Roll2");
        break;

        case 1:  //right
            if (heli.ServoIndexW > 0)
                channelDesc[heli.ServoIndexW - 1] = QString("ServoW");
            if (heli.ServoIndexX > 0)
                channelDesc[heli.ServoIndexX - 1] = QString("ServoX");
            if (heli.ServoIndexY > 0)
                channelDesc[heli.ServoIndexY - 1] = QString("ServoY");
        break;

        case 2:  //rear
            if (heli.ServoIndexW > 0)
                channelDesc[heli.ServoIndexW - 1] = QString("Elevator");
            if (heli.ServoIndexX > 0)
                channelDesc[heli.ServoIndexX - 1] = QString("Roll1");
            if (heli.ServoIndexY > 0)
                channelDesc[heli.ServoIndexY - 1] = QString("Roll2");
        break;

        case 3:  //left
            if (heli.ServoIndexW > 0)
                channelDesc[heli.ServoIndexW - 1] = QString("ServoW");
            if (heli.ServoIndexX > 0)
                channelDesc[heli.ServoIndexX - 1] = QString("ServoX");
            if (heli.ServoIndexY > 0)
                channelDesc[heli.ServoIndexY - 1] = QString("ServoY");
        break;

    }
    if (heli.ServoIndexZ > 0)
        channelDesc[heli.ServoIndexZ - 1] = QString("ServoZ");

    return channelDesc;
}

void ConfigCcpmWidget::UpdateType()
{
    int TypeInt,SingleServoIndex,NumServosDefined;
    QString TypeText;
    double AdjustmentAngle=0;

    SetUIComponentVisibilities();
    
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
    m_ccpm->ccpmCorrectionAngle->setEnabled(TypeInt!=0);

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

    m_ccpm->PitchCurve->setVisible(1);
    //m_ccpm->customThrottleCurve2Value->setVisible(1);
    //m_ccpm->label_41->setVisible(1);

    NumServosDefined=4;
    //set values for pre defined heli types
        if (TypeText.compare(QString::fromUtf8("CCPM 2 Servo 90º"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 90,360));
            m_ccpm->ccpmAngleY->setValue(0);
            m_ccpm->ccpmAngleZ->setValue(0);
            m_ccpm->ccpmAngleY->setEnabled(0);
            m_ccpm->ccpmAngleZ->setEnabled(0);
            m_ccpm->ccpmServoYChannel->setCurrentIndex(0);
            m_ccpm->ccpmServoZChannel->setCurrentIndex(0);
            m_ccpm->ccpmServoYChannel->setEnabled(0);
            m_ccpm->ccpmServoZChannel->setEnabled(0);
            //m_ccpm->ccpmCorrectionAngle->setValue(0);
            NumServosDefined=2;

        }
        if (TypeText.compare(QString::fromUtf8("CCPM 3 Servo 90º"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 90,360));
            m_ccpm->ccpmAngleY->setValue(fmod(AdjustmentAngle + 180,360));
            m_ccpm->ccpmAngleZ->setValue(0);
            m_ccpm->ccpmAngleZ->setEnabled(0);
            m_ccpm->ccpmServoZChannel->setCurrentIndex(0);
            m_ccpm->ccpmServoZChannel->setEnabled(0);
            //m_ccpm->ccpmCorrectionAngle->setValue(0);
            NumServosDefined=3;
        
        }
        if (TypeText.compare(QString::fromUtf8("CCPM 4 Servo 90º"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 90,360));
            m_ccpm->ccpmAngleY->setValue(fmod(AdjustmentAngle + 180,360));
            m_ccpm->ccpmAngleZ->setValue(fmod(AdjustmentAngle + 270,360));
            //m_ccpm->ccpmCorrectionAngle->setValue(0);
            m_ccpm->ccpmSingleServo->setEnabled(0);
            m_ccpm->ccpmSingleServo->setCurrentIndex(0);
            NumServosDefined=4;
        
        }
        if (TypeText.compare(QString::fromUtf8("CCPM 3 Servo 120º"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 120,360));
            m_ccpm->ccpmAngleY->setValue(fmod(AdjustmentAngle + 240,360));
            m_ccpm->ccpmAngleZ->setValue(0);
            m_ccpm->ccpmAngleZ->setEnabled(0);
            m_ccpm->ccpmServoZChannel->setCurrentIndex(0);
            m_ccpm->ccpmServoZChannel->setEnabled(0);
            //m_ccpm->ccpmCorrectionAngle->setValue(0);
            NumServosDefined=3;
            
        }
        if (TypeText.compare(QString::fromUtf8("CCPM 3 Servo 140º"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 140,360));
            m_ccpm->ccpmAngleY->setValue(fmod(AdjustmentAngle + 220,360));
            m_ccpm->ccpmAngleZ->setValue(0);
            m_ccpm->ccpmAngleZ->setEnabled(0);
            m_ccpm->ccpmServoZChannel->setCurrentIndex(0);
            m_ccpm->ccpmServoZChannel->setEnabled(0);
            //m_ccpm->ccpmCorrectionAngle->setValue(0);
            NumServosDefined=3;

        }
        if (TypeText.compare(QString::fromUtf8("FP 2 Servo 90º"), Qt::CaseInsensitive)==0)
        {
            m_ccpm->ccpmAngleW->setValue(AdjustmentAngle + 0);
            m_ccpm->ccpmAngleX->setValue(fmod(AdjustmentAngle + 90,360));
            m_ccpm->ccpmAngleY->setValue(0);
            m_ccpm->ccpmAngleZ->setValue(0);
            m_ccpm->ccpmAngleY->setEnabled(0);
            m_ccpm->ccpmAngleZ->setEnabled(0);
            m_ccpm->ccpmServoYChannel->setCurrentIndex(0);
            m_ccpm->ccpmServoZChannel->setCurrentIndex(0);
            m_ccpm->ccpmServoYChannel->setEnabled(0);
            m_ccpm->ccpmServoZChannel->setEnabled(0);
            //m_ccpm->ccpmCorrectionAngle->setValue(0);

            m_ccpm->ccpmCollectivespinBox->setEnabled(0);
            m_ccpm->ccpmCollectiveSlider->setEnabled(0);
            m_ccpm->ccpmCollectivespinBox->setValue(0);
            m_ccpm->ccpmCollectiveSlider->setValue(0);
            m_ccpm->PitchCurve->setVisible(0);
            //m_ccpm->customThrottleCurve2Value->setVisible(0);
            //m_ccpm->label_41->setVisible(0);
            NumServosDefined=2;
        }

    //set the visibility of the swashplate servo selection boxes
    m_ccpm->ccpmServoWLabel->setVisible(NumServosDefined>=1);
    m_ccpm->ccpmServoXLabel->setVisible(NumServosDefined>=2);
    m_ccpm->ccpmServoYLabel->setVisible(NumServosDefined>=3);
    m_ccpm->ccpmServoZLabel->setVisible(NumServosDefined>=4);
    m_ccpm->ccpmServoWChannel->setVisible(NumServosDefined>=1);
    m_ccpm->ccpmServoXChannel->setVisible(NumServosDefined>=2);
    m_ccpm->ccpmServoYChannel->setVisible(NumServosDefined>=3);
    m_ccpm->ccpmServoZChannel->setVisible(NumServosDefined>=4);
    
    //set the visibility of the swashplate angle selection boxes
    m_ccpm->ccpmServoWLabel_2->setVisible(NumServosDefined>=1);
    m_ccpm->ccpmServoXLabel_2->setVisible(NumServosDefined>=2);
    m_ccpm->ccpmServoYLabel_2->setVisible(NumServosDefined>=3);
    m_ccpm->ccpmServoZLabel_2->setVisible(NumServosDefined>=4);
    m_ccpm->ccpmAngleW->setVisible(NumServosDefined>=1);
    m_ccpm->ccpmAngleX->setVisible(NumServosDefined>=2);
    m_ccpm->ccpmAngleY->setVisible(NumServosDefined>=3);
    m_ccpm->ccpmAngleZ->setVisible(NumServosDefined>=4);
    

        m_ccpm->ccpmAdvancedSettingsTable->resizeColumnsToContents();
        for (int i=0;i<6;i++) {
            m_ccpm->ccpmAdvancedSettingsTable->setColumnWidth(i,(m_ccpm->ccpmAdvancedSettingsTable->width()-
                                                            m_ccpm->ccpmAdvancedSettingsTable->verticalHeader()->width())/6);
        }

    
    
    
    //update UI
    ccpmSwashplateUpdate();
    
}



void ConfigCcpmWidget::ccpmSwashplateRedraw()
{
    double angle[CCPM_MAX_SWASH_SERVOS],CorrectionAngle,x,y,w,h,radius,CenterX,CenterY;
    int used[CCPM_MAX_SWASH_SERVOS],defined[CCPM_MAX_SWASH_SERVOS],i;
    QRectF bounds;
    QRect size;
    double scale,xscale,yscale;


    size = m_ccpm->SwashplateImage->rect();
    xscale=size.width();
    yscale=size.height();
    scale=xscale;
    if (yscale<scale)scale=yscale;
    scale/=460.00;
    m_ccpm->SwashplateImage->resetTransform ();
    m_ccpm->SwashplateImage->scale(scale,scale);  
    
    size = m_ccpm->SwashLvlSwashplateImage->rect();
    xscale=size.width();
    yscale=size.height();
    scale=xscale;
    if (yscale<scale)scale=yscale;
    scale/=590.00;
    m_ccpm->SwashLvlSwashplateImage->resetTransform ();
    m_ccpm->SwashLvlSwashplateImage->scale(scale,scale);  
    
    CorrectionAngle=m_ccpm->ccpmCorrectionAngle->value();

    CenterX=200;
    CenterY=200;

    bounds=SwashplateImg->boundingRect();
    
    SwashplateImg->setPos(CenterX-bounds.width()/2,CenterY-bounds.height()/2);

    defined[0]=(m_ccpm->ccpmServoWChannel->isEnabled());
    defined[1]=(m_ccpm->ccpmServoXChannel->isEnabled());
    defined[2]=(m_ccpm->ccpmServoYChannel->isEnabled());
    defined[3]=(m_ccpm->ccpmServoZChannel->isEnabled());
    used[0]=((m_ccpm->ccpmServoWChannel->currentIndex()>0)&&(m_ccpm->ccpmServoWChannel->isEnabled()));
    used[1]=((m_ccpm->ccpmServoXChannel->currentIndex()>0)&&(m_ccpm->ccpmServoXChannel->isEnabled()));
    used[2]=((m_ccpm->ccpmServoYChannel->currentIndex()>0)&&(m_ccpm->ccpmServoYChannel->isEnabled()));
    used[3]=((m_ccpm->ccpmServoZChannel->currentIndex()>0)&&(m_ccpm->ccpmServoZChannel->isEnabled()));
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

        radius=150;
        bounds=ServosText[i]->boundingRect();
        x=CenterX-(radius*sin(angle[i]))-bounds.width()/2;
        y=CenterY+(radius*cos(angle[i]))-bounds.height()/2;
        
        ServosText[i]->setPos(x, y);
        ServosText[i]->setVisible(used[i]!=0);
 
        if (bounds.width()>bounds.height())
        {
            bounds.setHeight(bounds.width());
        }
        else
        {
            bounds.setWidth(bounds.height());
        }
        x=CenterX-(radius*sin(angle[i]))-bounds.width()/2;
        y=CenterY+(radius*cos(angle[i]))-bounds.height()/2;
        
        ServosTextCircles[i]->setRect(bounds);
        ServosTextCircles[i]->setPos(x, y);
        ServosTextCircles[i]->setVisible(used[i]!=0);

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
        ServoLines[i]->setVisible(defined[i]!=0);
    }

    //m_ccpm->SwashplateImage->centerOn (CenterX, CenterY);

    //m_ccpm->SwashplateImage->fitInView(SwashplateImg, Qt::KeepAspectRatio);
}

void ConfigCcpmWidget::ccpmSwashplateUpdate()
{
    ccpmSwashplateRedraw();
    SetUIComponentVisibilities();
    UpdateMixer();
}

void ConfigCcpmWidget::UpdateMixer()
{
    bool useCCPM;
    bool useCyclic;
    int i,j,ThisEnable[6];
    float CollectiveConstant,PitchConstant,RollConstant,ThisAngle[6];
    QString Channel;

    if (throwConfigError(QString("HeliCP")))
        return;

    GUIConfigDataUnion config = GetConfigData();

    useCCPM = !(config.heli.ccpmCollectivePassthroughState || !config.heli.ccpmLinkCyclicState);
    useCyclic = config.heli.ccpmLinkRollState;

    CollectiveConstant = (float)config.heli.SliderValue0 / 100.00;

    if (useCCPM) 
    {//cyclic = 1 - collective
        PitchConstant = 1-CollectiveConstant;
        RollConstant = PitchConstant;
    }
    else
    {
        PitchConstant = (float)config.heli.SliderValue1 / 100.00;;
        if (useCyclic) 
        {
            RollConstant = PitchConstant;
        }   
        else
        {
            RollConstant = (float)config.heli.SliderValue2 / 100.00;;
        }               
    }

    if (config.heli.SwashplateType>0)
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

        ServosText[0]->setPlainText(QString("%1").arg( MixerChannelData[2] ));
        ServosText[1]->setPlainText(QString("%1").arg( MixerChannelData[3] ));
        ServosText[2]->setPlainText(QString("%1").arg( MixerChannelData[4] ));
        ServosText[3]->setPlainText(QString("%1").arg( MixerChannelData[5] ));


        //go through the user data and update the mixer matrix
        for (i=0;i<6;i++)
        {
            if ((MixerChannelData[i]>0)&&((ThisEnable[i])||(i<2)))
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
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,3)->setText(QString("%1").arg((int)(127.0*(RollConstant)*sin((180+config.heli.CorrectionAngle + ThisAngle[i])*Pi/180.00))));//Roll
                    m_ccpm->ccpmAdvancedSettingsTable->item(i,4)->setText(QString("%1").arg((int)(127.0*(PitchConstant)*cos((config.heli.CorrectionAngle + ThisAngle[i])*Pi/180.00))));//Pitch
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
QString ConfigCcpmWidget::updateConfigObjects()
{
    QString airframeType = "HeliCP";

    bool useCCPM;
    bool useCyclic;

    if (updatingFromHardware == TRUE) return airframeType;

    updatingFromHardware = TRUE;

    //get the user options
    GUIConfigDataUnion config = GetConfigData();

    //swashplate config
    config.heli.SwashplateType = m_ccpm->ccpmType->count() - m_ccpm->ccpmType->currentIndex()-1;
    config.heli.FirstServoIndex = m_ccpm->ccpmSingleServo->currentIndex();

    //ccpm mixing options
    config.heli.ccpmCollectivePassthroughState = m_ccpm->ccpmCollectivePassthrough->isChecked();
    config.heli.ccpmLinkCyclicState = m_ccpm->ccpmLinkCyclic->isChecked();
    config.heli.ccpmLinkRollState = m_ccpm->ccpmLinkRoll->isChecked();
    useCCPM = !(config.heli.ccpmCollectivePassthroughState || !config.heli.ccpmLinkCyclicState);
    useCyclic = config.heli.ccpmLinkRollState;

    //correction angle
    config.heli.CorrectionAngle = m_ccpm->ccpmCorrectionAngle->value();

    //update sliders
    if (useCCPM)
    {
        config.heli.SliderValue0 = m_ccpm->ccpmCollectiveSlider->value();
    }
    else
    {
        config.heli.SliderValue0 = m_ccpm->ccpmCollectiveScale->value();
    }
    if (useCyclic)
    {
        config.heli.SliderValue1 = m_ccpm->ccpmCyclicScale->value();
    }
    else
    {
        config.heli.SliderValue1 = m_ccpm->ccpmPitchScale->value();
    }
    config.heli.SliderValue2 = m_ccpm->ccpmRollScale->value();

    //servo assignments
    config.heli.ServoIndexW = m_ccpm->ccpmServoWChannel->currentIndex();
    config.heli.ServoIndexX = m_ccpm->ccpmServoXChannel->currentIndex();
    config.heli.ServoIndexY = m_ccpm->ccpmServoYChannel->currentIndex();
    config.heli.ServoIndexZ = m_ccpm->ccpmServoZChannel->currentIndex();

    //throttle
    config.heli.Throttle = m_ccpm->ccpmEngineChannel->currentIndex();
    //tail
    config.heli.Tail = m_ccpm->ccpmTailChannel->currentIndex();

    SetConfigData(config);

    updatingFromHardware = FALSE;
    return airframeType;
}

QString ConfigCcpmWidget::updateConfigObjectsFromWidgets() //UpdateCCPMOptionsFromUI()
{
    QString airframeType = updateConfigObjects();

    setMixer();

    return airframeType;
}

void ConfigCcpmWidget::refreshWidgetsValues(QString frameType) //UpdateCCPMUIFromOptions()
{
    Q_UNUSED(frameType);

    GUIConfigDataUnion config = GetConfigData();

    //swashplate config
    setComboCurrentIndex( m_ccpm->ccpmType, m_ccpm->ccpmType->count() - (config.heli.SwashplateType +1));
    setComboCurrentIndex(m_ccpm->ccpmSingleServo, config.heli.FirstServoIndex);
    
    //ccpm mixing options
    m_ccpm->ccpmCollectivePassthrough->setChecked(config.heli.ccpmCollectivePassthroughState);
    m_ccpm->ccpmLinkCyclic->setChecked(config.heli.ccpmLinkCyclicState);
    m_ccpm->ccpmLinkRoll->setChecked(config.heli.ccpmLinkRollState);
    
    //correction angle
    m_ccpm->ccpmCorrectionAngle->setValue(config.heli.CorrectionAngle);
        
    //update sliders
    m_ccpm->ccpmCollectiveScale->setValue(config.heli.SliderValue0);
    m_ccpm->ccpmCollectiveScaleBox->setValue(config.heli.SliderValue0);
    m_ccpm->ccpmCyclicScale->setValue(config.heli.SliderValue1);
    m_ccpm->ccpmCyclicScaleBox->setValue(config.heli.SliderValue1);
    m_ccpm->ccpmPitchScale->setValue(config.heli.SliderValue1);
    m_ccpm->ccpmPitchScaleBox->setValue(config.heli.SliderValue1);
    m_ccpm->ccpmRollScale->setValue(config.heli.SliderValue2);
    m_ccpm->ccpmRollScaleBox->setValue(config.heli.SliderValue2);
    m_ccpm->ccpmCollectiveSlider->setValue(config.heli.SliderValue0);
    m_ccpm->ccpmCollectivespinBox->setValue(config.heli.SliderValue0);
    
    //servo assignments
    setComboCurrentIndex(m_ccpm->ccpmServoWChannel, config.heli.ServoIndexW);
    setComboCurrentIndex( m_ccpm->ccpmServoXChannel,config.heli.ServoIndexX);
    setComboCurrentIndex( m_ccpm->ccpmServoYChannel,config.heli.ServoIndexY);
    setComboCurrentIndex( m_ccpm->ccpmServoZChannel,config.heli.ServoIndexZ);

    //throttle
    setComboCurrentIndex( m_ccpm->ccpmEngineChannel, config.heli.Throttle);
    //tail
    setComboCurrentIndex( m_ccpm->ccpmTailChannel, config.heli.Tail);

    getMixer();
}


void ConfigCcpmWidget::SetUIComponentVisibilities()
{
    m_ccpm->ccpmRevoMixingBox->setVisible(0);
    
    m_ccpm->ccpmPitchMixingBox->setVisible(!m_ccpm->ccpmCollectivePassthrough->isChecked() &&
                                           m_ccpm->ccpmLinkCyclic->isChecked());

    m_ccpm->ccpmCollectiveScalingBox->setVisible(m_ccpm->ccpmCollectivePassthrough->isChecked() || !m_ccpm->ccpmLinkCyclic->isChecked());

    m_ccpm->ccpmLinkCyclic->setVisible(!m_ccpm->ccpmCollectivePassthrough->isChecked());

    m_ccpm->ccpmCyclicScalingBox->setVisible((m_ccpm->ccpmCollectivePassthrough->isChecked() || !m_ccpm->ccpmLinkCyclic->isChecked()) &&
                                              m_ccpm->ccpmLinkRoll->isChecked());

    if (!m_ccpm->ccpmCollectivePassthrough->checkState() && m_ccpm->ccpmLinkCyclic->isChecked())
    {
        m_ccpm->ccpmPitchScalingBox->setVisible(0);
        m_ccpm->ccpmRollScalingBox->setVisible(0);
        m_ccpm->ccpmLinkRoll->setVisible(0);

    }
    else
    {
        m_ccpm->ccpmPitchScalingBox->setVisible(!m_ccpm->ccpmLinkRoll->isChecked());
        m_ccpm->ccpmRollScalingBox->setVisible(!m_ccpm->ccpmLinkRoll->isChecked());
        m_ccpm->ccpmLinkRoll->setVisible(1);
    }
}
/**
  Request the current value of the SystemSettings which holds the ccpm type
  */
void ConfigCcpmWidget::getMixer()
{
    if (SwashLvlConfigurationInProgress)return;
    if (updatingToHardware)return;

    updatingFromHardware=TRUE;
    
    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QPointer<VehicleConfig> vconfig = new VehicleConfig();

    QList<double> curveValues;
    vconfig->getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, &curveValues);

    // is at least one of the curve values != 0?
    if (vconfig->isValidThrottleCurve(&curveValues)) {
        m_ccpm->ThrottleCurve->setCurve(&curveValues);
    }
    else {
        m_ccpm->ThrottleCurve->ResetCurve();
    }


    vconfig->getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, &curveValues);
    // is at least one of the curve values != 0?
    if (vconfig->isValidThrottleCurve(&curveValues)) {
        m_ccpm->PitchCurve->setCurve(&curveValues);
    }
    else {
        m_ccpm->PitchCurve->ResetCurve();
    }

    updatingFromHardware=FALSE;

    ccpmSwashplateUpdate();
}


/**
  Sends the config to the board (ccpm type)
  */
void ConfigCcpmWidget::setMixer()
{
    int i,j;

    if (SwashLvlConfigurationInProgress)return;
    if (updatingToHardware == TRUE) return;

    updatingToHardware=TRUE;

    MixerSettings * mixerSettings = MixerSettings::GetInstance(getObjectManager());
    Q_ASSERT(mixerSettings);
    MixerSettings::DataFields mixerSettingsData = mixerSettings->getData();

    UpdateMixer();

    // Set up some helper pointers
    qint8 * mixers[8] = {mixerSettingsData.Mixer1Vector,
                         mixerSettingsData.Mixer2Vector,
                         mixerSettingsData.Mixer3Vector,
                         mixerSettingsData.Mixer4Vector,
                         mixerSettingsData.Mixer5Vector,
                         mixerSettingsData.Mixer6Vector,
                         mixerSettingsData.Mixer7Vector,
                         mixerSettingsData.Mixer8Vector
    };

    quint8 * mixerTypes[8] = {
        &mixerSettingsData.Mixer1Type,
        &mixerSettingsData.Mixer2Type,
        &mixerSettingsData.Mixer3Type,
        &mixerSettingsData.Mixer4Type,
        &mixerSettingsData.Mixer5Type,
        &mixerSettingsData.Mixer6Type,
        &mixerSettingsData.Mixer7Type,
        &mixerSettingsData.Mixer8Type
    };

    //reset all to Disabled
    for (i=0; i<8; i++)
        *mixerTypes[i] = 0;

    //go through the user data and update the mixer matrix
    for (i=0;i<6;i++)
    {
        if (MixerChannelData[i]>0)
        {
            //set the mixer type
            *(mixerTypes[MixerChannelData[i] - 1]) = i==0 ?
                        MixerSettings::MIXER1TYPE_MOTOR :
                        MixerSettings::MIXER1TYPE_SERVO;

            //config the vector
            for (j=0;j<5;j++)
                mixers[MixerChannelData[i] - 1][j] = m_ccpm->ccpmAdvancedSettingsTable->item(i,j+1)->text().toInt();
        }
    }

    //get the user data for the curve into the mixer settings
    QList<double> curve1 = m_ccpm->ThrottleCurve->getCurve();
    QList<double> curve2 = m_ccpm->PitchCurve->getCurve();
    for (i=0;i<5;i++) {
        mixerSettingsData.ThrottleCurve1[i] = curve1.at(i);
        mixerSettingsData.ThrottleCurve2[i] = curve2.at(i);
    }
    
    //mapping of collective input to curve 2...
    //MixerSettings.Curve2Source = Throttle,Roll,Pitch,Yaw,Accessory0,Accessory1,Accessory2,Accessory3,Accessory4,Accessory5
    //check if we are using throttle or directly from a channel...
    if (m_ccpm->ccpmCollectivePassthrough->isChecked())
        mixerSettingsData.Curve2Source = MixerSettings::CURVE2SOURCE_COLLECTIVE;
    else
        mixerSettingsData.Curve2Source = MixerSettings::CURVE2SOURCE_THROTTLE;
    
    mixerSettings->setData(mixerSettingsData);
    mixerSettings->updated();
    updatingToHardware=FALSE;

}

/**
  Send ccpm type to the board and request saving to SD card
  */
void ConfigCcpmWidget::saveccpmUpdate()
{
    if (SwashLvlConfigurationInProgress)return;
    ShowDisclaimer(0);
    // Send update so that the latest value is saved
    //sendccpmUpdate();
    setMixer();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    saveObjectToSD(obj);
}

void ConfigCcpmWidget::resizeEvent(QResizeEvent* event)
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
void ConfigCcpmWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    m_ccpm->ccpmAdvancedSettingsTable->resizeColumnsToContents();
    for (int i=0;i<6;i++) {
        m_ccpm->ccpmAdvancedSettingsTable->setColumnWidth(i,(m_ccpm->ccpmAdvancedSettingsTable->width()-
                                                        m_ccpm->ccpmAdvancedSettingsTable->verticalHeader()->width())/6);
    }
    ccpmSwashplateRedraw();
}


void ConfigCcpmWidget::SwashLvlStartButtonPressed()
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
            //sendccpmUpdate();
            setMixer();

            //change control mode to gcs control / disarmed
            //set throttle to 0


            //save off the old ActuatorSettings for the swashplate servos
            pm = ExtensionSystem::PluginManager::instance();
            objManager = pm->getObject<UAVObjectManager>();


            // Get the channel assignements:
            obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
            Q_ASSERT(obj);
            // obj->requestUpdate();
            MinField = obj->getField(QString("ChannelMin"));
            NeutralField = obj->getField(QString("ChannelNeutral"));
            MaxField = obj->getField(QString("ChannelMax"));

            //channel assignments
            oldSwashLvlConfiguration.ServoChannels[0]=m_ccpm->ccpmServoWChannel->currentIndex();
            oldSwashLvlConfiguration.ServoChannels[1]=m_ccpm->ccpmServoXChannel->currentIndex();
            oldSwashLvlConfiguration.ServoChannels[2]=m_ccpm->ccpmServoYChannel->currentIndex();
            oldSwashLvlConfiguration.ServoChannels[3]=m_ccpm->ccpmServoZChannel->currentIndex();
            //if servos are used
            oldSwashLvlConfiguration.Used[0]=((m_ccpm->ccpmServoWChannel->currentIndex()>0)&&(m_ccpm->ccpmServoWChannel->isEnabled()));
            oldSwashLvlConfiguration.Used[1]=((m_ccpm->ccpmServoXChannel->currentIndex()>0)&&(m_ccpm->ccpmServoXChannel->isEnabled()));
            oldSwashLvlConfiguration.Used[2]=((m_ccpm->ccpmServoYChannel->currentIndex()>0)&&(m_ccpm->ccpmServoYChannel->isEnabled()));
            oldSwashLvlConfiguration.Used[3]=((m_ccpm->ccpmServoZChannel->currentIndex()>0)&&(m_ccpm->ccpmServoZChannel->isEnabled()));
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
void ConfigCcpmWidget::SwashLvlNextButtonPressed()
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
void ConfigCcpmWidget::SwashLvlCancelButtonPressed()
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


void ConfigCcpmWidget::SwashLvlFinishButtonPressed()
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
    saveObjectToSD(obj);

    //restore Flight control of ActuatorCommand
    enableSwashplateLevellingControl(false);

    m_ccpm->SwashLvlStepInstruction->setText("<h2>Levelling Completed</h2><p>New settings have been saved to the SD card");

    ShowDisclaimer(0);
    //ShowDisclaimer(2);

}

int ConfigCcpmWidget::ShowDisclaimer(int messageID)
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
             msgBox.setInformativeText("<h2>The CCPM mixer code needs more testing!</h2><p><font color=red>Use it at your own risk!</font><p>Do you wish to continue?");
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
void ConfigCcpmWidget::enableSwashplateLevellingControl(bool state)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorCommand")));
    UAVObject::Metadata mdata = obj->getMetadata();
    if (state)
    {
        SwashLvlaccInitialData = mdata;
        UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READONLY);
        UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_ONCHANGE);
        UAVObject::SetGcsTelemetryAcked(mdata, false);
        UAVObject::SetGcsTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_ONCHANGE);
        mdata.gcsTelemetryUpdatePeriod = 100;
        SwashLvlConfigurationInProgress=1;
        m_ccpm->TabObject->setTabEnabled(0,0);
        m_ccpm->TabObject->setTabEnabled(2,0);
        m_ccpm->TabObject->setTabEnabled(3,0);
        m_ccpm->ccpmType->setEnabled(0);
    }
    else
    {
        mdata = SwashLvlaccInitialData; // Restore metadata
        SwashLvlConfigurationInProgress=0;

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
void ConfigCcpmWidget::setSwashplateLevel(int percent)
{
    if (percent<0)return;// -1;
    if (percent>100)return;// -1;
    if (SwashLvlConfigurationInProgress!=1)return;// -1;
    int i;
    double value;
    double level = ((double)percent /50.00) - 1.00;

    SwashLvlServoInterlock=1;

    ActuatorCommand * actuatorCommand = ActuatorCommand::GetInstance(getObjectManager());
    ActuatorCommand::DataFields actuatorCommandData = actuatorCommand->getData();

    for (i=0;i<CCPM_MAX_SWASH_SERVOS;i++) {
        if (level==0)
            value = newSwashLvlConfiguration.Neutral[i];
        else if (level > 0)
            value = (newSwashLvlConfiguration.Max[i] - newSwashLvlConfiguration.Neutral[i])*level + newSwashLvlConfiguration.Neutral[i];
        else if (level < 0)
            value = (newSwashLvlConfiguration.Neutral[i] - newSwashLvlConfiguration.Min[i])*level + newSwashLvlConfiguration.Neutral[i];

        actuatorCommandData.Channel[newSwashLvlConfiguration.ServoChannels[i]] = value;
        SwashLvlSpinBoxes[i]->setValue(value);
    }

    actuatorCommand->setData(actuatorCommandData);
    actuatorCommand->updated();

    SwashLvlServoInterlock=0;

return;
}


void ConfigCcpmWidget::SwashLvlSpinBoxChanged(int value)
{
    Q_UNUSED(value);
    int i;
    if (SwashLvlServoInterlock==1)return;

    ActuatorCommand * actuatorCommand = ActuatorCommand::GetInstance(getObjectManager());
    ActuatorCommand::DataFields actuatorCommandData = actuatorCommand->getData();

    for (i = 0; i < CCPM_MAX_SWASH_SERVOS; i++) {
        value = SwashLvlSpinBoxes[i]->value();

        switch (SwashLvlState)
        {
        case 1: //Neutral levelling
            newSwashLvlConfiguration.Neutral[i]=value;
            break;
        case 2: //Max levelling
            newSwashLvlConfiguration.Max[i] = value;
            break;
        case 3: //Min levelling
            newSwashLvlConfiguration.Min[i]= value;
            break;
        case 4: //levelling verification
            break;
        case 5: //levelling complete
            break;
        default:
            break;
        }

        actuatorCommandData.Channel[newSwashLvlConfiguration.ServoChannels[i]] = value;
    }


    actuatorCommand->setData(actuatorCommandData);
    actuatorCommand->updated();

    return;
}

/**
 This function displays text and color formatting in order to help the user understand what channels have not yet been configured.
 */
bool ConfigCcpmWidget::throwConfigError(QString airframeType)
{
    Q_UNUSED(airframeType);

    bool error = false;

    if((m_ccpm->ccpmServoWChannel->currentIndex()==0)&&(m_ccpm->ccpmServoWChannel->isEnabled()))
    {
        m_ccpm->ccpmServoWLabel->setText("<font color=red>Servo W</font>");
        error = true;
    }
    else
    {
        m_ccpm->ccpmServoWLabel->setText("<font color=black>Servo W</font>");
    }
    if((m_ccpm->ccpmServoXChannel->currentIndex()==0)&&(m_ccpm->ccpmServoXChannel->isEnabled()))
    {
        m_ccpm->ccpmServoXLabel->setText("<font color=red>Servo X</font>");
        error = true;
    }
    else
    {
        m_ccpm->ccpmServoXLabel->setText("<font color=black>Servo X</font>");
    }
    if((m_ccpm->ccpmServoYChannel->currentIndex()==0)&&(m_ccpm->ccpmServoYChannel->isEnabled()))
    {
        m_ccpm->ccpmServoYLabel->setText("<font color=red>Servo Y</font>");
        error = true;
    }
    else
    {
        m_ccpm->ccpmServoYLabel->setText("<font color=black>Servo Y</font>");
    }
    if((m_ccpm->ccpmServoZChannel->currentIndex()==0)&&(m_ccpm->ccpmServoZChannel->isEnabled()))
    {
        m_ccpm->ccpmServoZLabel->setText("<font color=red>Servo Z</font>");
        error = true;
    }
    else
    {
        m_ccpm->ccpmServoZLabel->setText("<font color=black>Servo Z</font>");
    }

    if((m_ccpm->ccpmEngineChannel->currentIndex()==0)&&(m_ccpm->ccpmEngineChannel->isEnabled()))
    {
        m_ccpm->ccpmEngineLabel->setText("<font color=red>Engine</font>");
    }
    else
    {
        m_ccpm->ccpmEngineLabel->setText("<font color=black>Engine</font>");
    }

    if((m_ccpm->ccpmTailChannel->currentIndex()==0)&&(m_ccpm->ccpmTailChannel->isEnabled()))
    {
        m_ccpm->ccpmTailLabel->setText("<font color=red>Tail Rotor</font>");
        error = true;
    }
    else
    {
        m_ccpm->ccpmTailLabel->setText("<font color=black>Tail Rotor</font>");
    }

    return error;
}
