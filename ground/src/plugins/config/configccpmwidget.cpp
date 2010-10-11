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
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

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


    QStringList channels;
    channels << "Channel0" << "Channel1" << "Channel2" <<
            "Channel3" << "Channel4" << "Channel5" << "Channel6" << "Channel7" << "None" ;
    m_ccpm->ccpmEngineChannel->addItems(channels);
    m_ccpm->ccpmTailChannel->addItems(channels);
    m_ccpm->ccpmServoWChannel->addItems(channels);
    m_ccpm->ccpmServoXChannel->addItems(channels);
    m_ccpm->ccpmServoYChannel->addItems(channels);
    m_ccpm->ccpmServoZChannel->addItems(channels);

    QStringList Types;
    Types << "CCPM 90º" << "CCPM 120º" << "CCPM 140º" ;
    m_ccpm->ccpmType->addItems(Types);

    requestccpmUpdate();
    UpdateCurveSettings();

    ccpmSwashplateUpdate();
    connect(m_ccpm->saveccpmToSD, SIGNAL(clicked()), this, SLOT(saveccpmUpdate()));
    connect(m_ccpm->saveccpmToRAM, SIGNAL(clicked()), this, SLOT(sendccpmUpdate()));
    connect(m_ccpm->getccpmCurrent, SIGNAL(clicked()), this, SLOT(requestccpmUpdate()));
    connect(m_ccpm->ccpmGenerateCurve, SIGNAL(clicked()), this, SLOT(GenerateCurve()));
    connect(m_ccpm->NumCurvePoints, SIGNAL(valueChanged(int)), this, SLOT(UpdateCurveSettings()));
    connect(m_ccpm->CurveType, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateCurveSettings()));
    connect(m_ccpm->ServoSettings, SIGNAL(cellChanged(int,int)), this, SLOT(ccpmSwashplateUpdate()));

    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestccpmUpdate()));

}

ConfigccpmWidget::~ConfigccpmWidget()
{
   // Do nothing
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

}
void ConfigccpmWidget::GenerateCurve()
{
   int NumCurvePoints,CurveToGenerate,i;
   double value1, value2, value3, scale;
   QString CurveType;
   QTableWidgetItem *item;


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
           item->setText( tr( "%1" ).arg( value1 ) );
       }
       if ( CurveType.compare("Linear")==0)
       {
           item->setText( tr( "%1" ).arg(value1 +(scale*(value2-value1))) );
       }
       if ( CurveType.compare("Step")==0)
       {
           if (scale*100<value3)
           {
               item->setText( tr( "%1" ).arg(value1) );
           }
           else
           {
               item->setText( tr( "%1" ).arg(value2) );
           }
       }
   }
   for (i=NumCurvePoints;i<10;i++)
   {
       item =m_ccpm->CurveSettings->item(i, CurveToGenerate );
       item->setText( tr( "" ) );
   }


}

void ConfigccpmWidget::ccpmSwashplateUpdate()
{
    double angle,x,y;
    int used;


    used=m_ccpm->ServoSettings->item(0,1)->text().compare("None");
    ServoW->setVisible(used!=0);
    angle=(180+m_ccpm->ServoSettings->item(0,0)->text().toDouble())*Pi/180.00;
    x=200.00-(200.00*sin(angle))-10.00;
    y=200.00+(200.00*cos(angle))-10.00;
    ServoW->setPos(x, y);

    used=m_ccpm->ServoSettings->item(1,1)->text().compare("None");
    ServoX->setVisible(used!=0);
    angle=(180+m_ccpm->ServoSettings->item(1,0)->text().toDouble())*Pi/180.00;
    x=200.00-(200.00*sin(angle))-10.00;
    y=200.00+(200.00*cos(angle))-10.00;
    ServoX->setPos(x, y);

    used=m_ccpm->ServoSettings->item(2,1)->text().compare("None");
    ServoY->setVisible(used!=0);
    angle=(180+m_ccpm->ServoSettings->item(2,0)->text().toDouble())*Pi/180.00;
    x=200.00-(200.00*sin(angle))-10.00;
    y=200.00+(200.00*cos(angle))-10.00;
    ServoY->setPos(x, y);

    used=m_ccpm->ServoSettings->item(3,1)->text().compare("None");
    ServoZ->setVisible(used!=0);
    angle=(180+m_ccpm->ServoSettings->item(3,0)->text().toDouble())*Pi/180.00;
    x=200.00-(200.00*sin(angle))-10.00;
    y=200.00+(200.00*cos(angle))-10.00;
    ServoZ->setPos(x, y);

}


/**************************
  * ccpm settings
  **************************/
/**
  Request the current value of the SystemSettings which holds the ccpm type
  */
void ConfigccpmWidget::requestccpmUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    int i;

    UAVObjectField *field;
    QTableWidgetItem *newItem;// = new QTableWidgetItem();


	//not doing anything yet


    ccpmSwashplateUpdate();

}


/**
  Sends the config to the board (ccpm type)
  */
void ConfigccpmWidget::sendccpmUpdate()
{
    int i,ThisChannel[6];
float CollectiveConstant,ThisAngle[6];
    UAVObjectField *field;
    QTableWidgetItem *newItem;// = new QTableWidgetItem();
    UAVDataObject* obj;

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("MixerSettings")));
        Q_ASSERT(obj);

        CollectiveConstant=m_ccpm->CollectiveConstant->value();


		//get the channel data from the ui
        ThisChannel[0] = m_ccpm->ccpmThrottle->currentText().toInt();
        ThisChannel[1] = m_ccpm->ccpmTailRotor->currentText().toInt();
        newItem =m_ccpm->ServoSettings->item(0, 1);
        ThisChannel[2] = newItem->text().toInt();
        newItem =m_ccpm->ServoSettings->item(1, 1);
        ThisChannel[3] = newItem->text().toInt();
        newItem =m_ccpm->ServoSettings->item(2, 1);
        ThisChannel[4] = newItem->text().toInt();
        newItem =m_ccpm->ServoSettings->item(3, 1);
        ThisChannel[5] = newItem->text().toInt();


		//get the angle data from the ui
        newItem =m_ccpm->ServoSettings->item(0, 0);
        ThisAngle[2] = newItem->text().toFloat();
        newItem =m_ccpm->ServoSettings->item(1, 0);
        ThisAngle[3] = newItem->text().toFloat();
        newItem =m_ccpm->ServoSettings->item(2, 0);
        ThisAngle[4] = newItem->text().toFloat();
        newItem =m_ccpm->ServoSettings->item(3, 0);
        ThisAngle[5] = newItem->text().toFloat();

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
            if (ThisChannel[i]<8)
            {
                 //select the correct mixer for this config element
               field = obj->getField(tr( "Mixer%1Type" ).arg( ThisChannel[i] ));
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
                field = obj->getField(tr( "Mixer%1Vector" ).arg( ThisChannel[i] ));
                 //config the vector
               if (i==0)
                {//motor
                    field->setValue(127.0,0);//ThrottleCurve1
                    field->setValue(0,1);//ThrottleCurve2
                    field->setValue(0,2);//Roll
                    field->setValue(0,3);//Pitch
                    field->setValue(0,4);//Yaw
                }
                if (i==1)
                {//tailrotor
                    field->setValue(0,0);//ThrottleCurve1
                    field->setValue(0,1);//ThrottleCurve2
                    field->setValue(0,2);//Roll
                    field->setValue(0,3);//Pitch
                    field->setValue(127.0,4);//Yaw
                }
                if (i>1)
                {//Swashplate
                    field->setValue(0,0);//ThrottleCurve1
                    field->setValue(127.0*CollectiveConstant,1);//ThrottleCurve2
                    field->setValue(127.0*(1-CollectiveConstant)*sin(ThisAngle[i]*Pi/180.00),2);//Roll
                    field->setValue(127.0*(1-CollectiveConstant)*cos(ThisAngle[i]*Pi/180.00),3);//Pitch
                    field->setValue(0,4);//Yaw

                }
            }

        }


 		//get the user data for the curve into the mixer settings
       field = obj->getField(QString("ThrottleCurve1"));
        for (i=0;i<5;i++)
        {
            newItem =m_ccpm->CurveSettings->item(i, 0);
            field->setValue(newItem->text().toDouble(),i);
        }
        field = obj->getField(QString("ThrottleCurve2"));
        for (i=0;i<5;i++)
        {
            newItem =m_ccpm->CurveSettings->item(i, 1);
            field->setValue(newItem->text().toDouble(),i);
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

