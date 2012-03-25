/**
 ******************************************************************************
 *
 * @file       configvehicletypewidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Airframe configuration panel
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
#include "configvehicletypewidget.h"

#include <QDebug>
#include <QStringList>
#include <QTimer>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <math.h>
#include <QDesktopServices>
#include <QUrl>
#include "systemsettings.h"
#include "mixersettings.h"
#include "actuatorsettings.h"
#include <QEventLoop>

/**
  Helper delegate for the custom mixer editor table.
  Taken straight from Qt examples, thanks!
  */
SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
     : QItemDelegate(parent)
 {
 }

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(-127);
    editor->setMaximum(127);

    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

/**********************************************************************************/


/**
 Constructor
 */
ConfigVehicleTypeWidget::ConfigVehicleTypeWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_aircraft = new Ui_AircraftWidget();
    m_aircraft->setupUi(this);

    addApplySaveButtons(m_aircraft->saveAircraftToRAM,m_aircraft->saveAircraftToSD);

    addUAVObject("SystemSettings");
    addUAVObject("MixerSettings");
    addUAVObject("ActuatorSettings");


    ffTuningInProgress = false;
    ffTuningPhase = false;

	//Generate list of channels
    QStringList channels;
    channels << "None";
    for (int i = 0; i < ActuatorSettings::CHANNELADDR_NUMELEM; i++) {
        mixerTypes << QString("Mixer%1Type").arg(i+1);
        mixerVectors << QString("Mixer%1Vector").arg(i+1);
        channels << QString("Channel%1").arg(i+1);
    }

    QStringList airframeTypes;
    airframeTypes << "Fixed Wing" << "Multirotor" << "Helicopter" << "Ground" << "Custom";
    m_aircraft->aircraftType->addItems(airframeTypes);
    m_aircraft->aircraftType->setCurrentIndex(1);

    QStringList fixedWingTypes;
    fixedWingTypes << "Elevator aileron rudder" << "Elevon" << "Vtail";
    m_aircraft->fixedWingType->addItems(fixedWingTypes);
    m_aircraft->fixedWingType->setCurrentIndex(0); //Set default model to "Elevator aileron rudder"

	QStringList groundVehicleTypes;
    groundVehicleTypes << "Turnable (car)" << "Differential (tank)" << "Motorcycle";
//    groundVehicleTypes << "Turnable (car)";
    m_aircraft->groundVehicleType->addItems(groundVehicleTypes);
    m_aircraft->groundVehicleType->setCurrentIndex(0); //Set default model to "Turnable (car)"

    QStringList multiRotorTypes;
    multiRotorTypes << "Tricopter Y"<< "Quad +" << "Quad X" <<
					"Hexacopter" << "Hexacopter X" << "Hexacopter Y6" <<
					"Octocopter" << "Octocopter V" << "Octo Coax +" << "Octo Coax X" ;
    m_aircraft->multirotorFrameType->addItems(multiRotorTypes);
    m_aircraft->multirotorFrameType->setCurrentIndex(1); //Set default model to "Quad +"

    // Now load all the channel assignements 
	//OLD STYLE: DO IT MANUALLY
//    m_aircraft->triYawChannelBox->addItems(channels);
//    m_aircraft->gvMotor1ChannelBox->addItems(channels);
//    m_aircraft->gvMotor2ChannelBox->addItems(channels);
//    m_aircraft->gvSteering1ChannelBox->addItems(channels);
//    m_aircraft->gvSteering2ChannelBox->addItems(channels);
//    m_aircraft->fwElevator1ChannelBox->addItems(channels);
//    m_aircraft->fwElevator2ChannelBox->addItems(channels);
//    m_aircraft->fwEngineChannelBox->addItems(channels);
//    m_aircraft->fwRudder1ChannelBox->addItems(channels);
//    m_aircraft->fwRudder2ChannelBox->addItems(channels);
//    m_aircraft->fwAileron1ChannelBox->addItems(channels);
//    m_aircraft->fwAileron2ChannelBox->addItems(channels);

	//NEW STYLE: Loop through the widgets looking for all widgets that have "ChannelBox" in their name
	//  The upshot of this is that ALL new ComboBox widgets for selecting the output channel must have "ChannelBox" in their name
	foreach(QComboBox *combobox, this->findChildren<QComboBox*>(QRegExp("\\S+ChannelBo\\S+")))//FOR WHATEVER REASON, THIS DOES NOT WORK WITH ChannelBox. ChannelBo is sufficiently accurate
	{
		combobox->addItems(channels);
	}
	
    // Setup the Multirotor picture in the Quad settings interface
    m_aircraft->quadShape->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_aircraft->quadShape->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/quad-shapes.svg"));
    quad = new QGraphicsSvgItem();
    quad->setSharedRenderer(renderer);
    quad->setElementId("quad-plus");
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(quad);
    scene->setSceneRect(quad->boundingRect());
    m_aircraft->quadShape->setScene(scene);

    // Put combo boxes in line one of the custom mixer table:
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("Mixer1Type"));
    QStringList list = field->getOptions();
    for (int i=0;i<8;i++) {
        QComboBox* qb = new QComboBox(m_aircraft->customMixerTable);
        qb->addItems(list);
        m_aircraft->customMixerTable->setCellWidget(0,i,qb);
    }

    SpinBoxDelegate *sbd = new SpinBoxDelegate();
    for (int i=1;i<8; i++) {
        m_aircraft->customMixerTable->setItemDelegateForRow(i, sbd);
    }

	//Connect aircraft type selection dropbox to callback function
    connect(m_aircraft->aircraftType, SIGNAL(currentIndexChanged(int)), this, SLOT(switchAirframeType(int)));
	
	//Connect airframe selection dropbox to callback functions
    connect(m_aircraft->fixedWingType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));
    connect(m_aircraft->multirotorFrameType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));
    connect(m_aircraft->groundVehicleType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));

	//Connect throttle curve reset pushbuttons to reset functions
    connect(m_aircraft->fwThrottleReset, SIGNAL(clicked()), this, SLOT(resetFwMixer()));
    connect(m_aircraft->mrThrottleCurveReset, SIGNAL(clicked()), this, SLOT(resetMrMixer()));
    connect(m_aircraft->gvThrottleCurve1Reset, SIGNAL(clicked()), this, SLOT(resetGvFrontMixer()));	
    connect(m_aircraft->gvThrottleCurve2Reset, SIGNAL(clicked()), this, SLOT(resetGvRearMixer()));	
    connect(m_aircraft->customReset1, SIGNAL(clicked()), this, SLOT(resetCt1Mixer()));
    connect(m_aircraft->customReset2, SIGNAL(clicked()), this, SLOT(resetCt2Mixer()));

	//Connect throttle curve manipulation points to output text
    connect(m_aircraft->fixedWingThrottle, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateFwThrottleCurveValue(QList<double>,double)));
    connect(m_aircraft->multiThrottleCurve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateMrThrottleCurveValue(QList<double>,double)));
    connect(m_aircraft->groundVehicleThrottle1, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateGvThrottle1CurveValue(QList<double>,double)));
    connect(m_aircraft->groundVehicleThrottle2, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateGvThrottle2CurveValue(QList<double>,double)));
    connect(m_aircraft->customThrottle1Curve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateCustomThrottle1CurveValue(QList<double>,double)));
    connect(m_aircraft->customThrottle2Curve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateCustomThrottle2CurveValue(QList<double>,double)));

//    connect(m_aircraft->fwAileron1Channel, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleAileron2(int)));
//    connect(m_aircraft->fwElevator1Channel, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleElevator2(int)));

    //Connect the three feed forward test checkboxes
    connect(m_aircraft->ffTestBox1, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox2, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox3, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));

	//WHAT DOES THIS DO?
    enableControls(false);
    refreshWidgetsValues();

    // Connect the help pushbutton
    connect(m_aircraft->airframeHelp, SIGNAL(clicked()), this, SLOT(openHelp()));
    addToDirtyMonitor();
	
	//Initialize GUI tabs //MOVING THIS FROM THE END OF THIS FUNCTION CAN CAUSE RUNTIME ERRORS DUE TO setupMultiRotorUI. WHY?
	setupMultiRotorUI( m_aircraft->multirotorFrameType->currentText() );
    setupGroundVehicleUI( m_aircraft->groundVehicleType->currentText() );
    setupFixedWingUI( m_aircraft->fixedWingType->currentText() );
	
}


/**
 Destructor
 */
ConfigVehicleTypeWidget::~ConfigVehicleTypeWidget()
{
   // Do nothing
}


/**
  Slot for switching the airframe type. We do it explicitely
  rather than a signal in the UI, because we want to force a fitInView of the quad shapes.
  This is because this method (fitinview) only works when the widget is shown.
  */
void ConfigVehicleTypeWidget::switchAirframeType(int index){
    m_aircraft->airframesWidget->setCurrentIndex(index);
    m_aircraft->quadShape->setSceneRect(quad->boundingRect());
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
    if (m_aircraft->aircraftType->findText("Custom")) {
        m_aircraft->customMixerTable->resizeColumnsToContents();
        for (int i=0;i<8;i++) {
            m_aircraft->customMixerTable->setColumnWidth(i,(m_aircraft->customMixerTable->width()-
                                                            m_aircraft->customMixerTable->verticalHeader()->width())/8);
        }
    }
}


/**
 WHAT DOES THIS DO???
 */
void ConfigVehicleTypeWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
    m_aircraft->customMixerTable->resizeColumnsToContents();
    for (int i=0;i<8;i++) {
        m_aircraft->customMixerTable->setColumnWidth(i,(m_aircraft->customMixerTable->width()-
                                                        m_aircraft->customMixerTable->verticalHeader()->width())/8);
    }
}

/**
 Resize the GUI contents when the user changes the window size
 */
void ConfigVehicleTypeWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
    // Make the custom table columns autostretch:
    m_aircraft->customMixerTable->resizeColumnsToContents();
    for (int i=0;i<8;i++) {
        m_aircraft->customMixerTable->setColumnWidth(i,(m_aircraft->customMixerTable->width()-
                                                        m_aircraft->customMixerTable->verticalHeader()->width())/8);
    }

}


void ConfigVehicleTypeWidget::toggleAileron2(int index)
{
    if (index) {
        m_aircraft->fwAileron2ChannelBox->setEnabled(true);
        m_aircraft->fwAileron2Label->setEnabled(true);
    } else {
        m_aircraft->fwAileron2ChannelBox->setEnabled(false);
        m_aircraft->fwAileron2Label->setEnabled(false);
    }
}

void ConfigVehicleTypeWidget::toggleElevator2(int index)
{
    if (index) {
        m_aircraft->fwElevator2ChannelBox->setEnabled(true);
        m_aircraft->fwElevator2Label->setEnabled(true);
    } else {
        m_aircraft->fwElevator2ChannelBox->setEnabled(false);
        m_aircraft->fwElevator2Label->setEnabled(false);
    }
}

void ConfigVehicleTypeWidget::toggleRudder2(int index)
{
    if (index) {
        m_aircraft->fwRudder2ChannelBox->setEnabled(true);
        m_aircraft->fwRudder2Label->setEnabled(true);
    } else {
        m_aircraft->fwRudder2ChannelBox->setEnabled(false);
        m_aircraft->fwRudder2Label->setEnabled(false);
    }
}


/////////////////////////////////////////////////////////
/// Feed Forward Testing
/////////////////////////////////////////////////////////
/**
  Enables and runs feed forward testing
  */
void ConfigVehicleTypeWidget::enableFFTest()
{
    // Role:
    // - Check if all three checkboxes are checked
    // - Every other timer event: toggle engine from 45% to 55%
    // - Every other time event: send FF settings to flight FW
    if (m_aircraft->ffTestBox1->isChecked() &&
        m_aircraft->ffTestBox2->isChecked() &&
        m_aircraft->ffTestBox3->isChecked()) {
        if (!ffTuningInProgress)
        {
            // Initiate tuning:
            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ManualControlCommand")));
            UAVObject::Metadata mdata = obj->getMetadata();
            accInitialData = mdata;
            mdata.flightAccess = UAVObject::ACCESS_READONLY;
            obj->setMetadata(mdata);
        }
        // Depending on phase, either move actuator or send FF settings:
        if (ffTuningPhase) {
            // Send FF settings to the board
            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            UAVObjectField* field = obj->getField(QString("FeedForward"));
            field->setDouble((double)m_aircraft->feedForwardSlider->value()/100);
            field = obj->getField(QString("AccelTime"));
            field->setDouble(m_aircraft->accelTime->value());
            field = obj->getField(QString("DecelTime"));
            field->setDouble(m_aircraft->decelTime->value());
            field = obj->getField(QString("MaxAccel"));
            field->setDouble(m_aircraft->maxAccelSlider->value());
            obj->updated();
        } else  {
            // Toggle motor state
            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ManualControlCommand")));
            double value = obj->getField("Throttle")->getDouble();
            double target = (value < 0.5) ? 0.55 : 0.45;
            obj->getField("Throttle")->setValue(target);
            obj->updated();
        }
        ffTuningPhase = !ffTuningPhase;
        ffTuningInProgress = true;
        QTimer::singleShot(1000, this, SLOT(enableFFTest()));
    } else {
        // - If no: disarm timer, restore actuatorcommand metadata
        // Disarm!
        if (ffTuningInProgress) {
            ffTuningInProgress = false;
            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ManualControlCommand")));
            UAVObject::Metadata mdata = obj->getMetadata();
            mdata = accInitialData; // Restore metadata
            obj->setMetadata(mdata);
        }
    }
}


/**
  Resets Fixed wing throttle mixer
  */
void ConfigVehicleTypeWidget::resetFwMixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve1"));
    resetMixer(m_aircraft->fixedWingThrottle, field->getNumElements(),1);
}

/**
  Resets Multirotor throttle mixer
  */
void ConfigVehicleTypeWidget::resetMrMixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve1"));
    resetMixer(m_aircraft->multiThrottleCurve, field->getNumElements(),0.9);
}

/**
 Resets Ground vehicle front throttle mixer
 */
void ConfigVehicleTypeWidget::resetGvFrontMixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve1"));
    resetMixer(m_aircraft->groundVehicleThrottle1, field->getNumElements(),1);
}

/**
 Resets Ground vehicle rear throttle mixer
 */
void ConfigVehicleTypeWidget::resetGvRearMixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve2"));
    resetMixer(m_aircraft->groundVehicleThrottle2, field->getNumElements(),1);
}

/**
  Resets Custom throttle 1 mixer
  */
void ConfigVehicleTypeWidget::resetCt1Mixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve1"));
    resetMixer(m_aircraft->customThrottle1Curve, field->getNumElements(),1);
}

/**
  Resets Custom throttle 2 mixer
  */
void ConfigVehicleTypeWidget::resetCt2Mixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve2"));
    resetMixer(m_aircraft->customThrottle2Curve, field->getNumElements(),1);
}


/**
  Resets a mixer curve
  */
void ConfigVehicleTypeWidget::resetMixer(MixerCurveWidget *mixer, int numElements, double maxvalue)
{
    // Setup all Throttle1 curves for all types of airframes
    mixer->initLinearCurve((quint32)numElements,maxvalue);
}

/**
  Updates the currently moved fixed wing throttle curve item value
  */
void ConfigVehicleTypeWidget::updateFwThrottleCurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->fwThrottleCurveItemValue->setText(QString().sprintf("Val: %.2f",value));
}

/**
  Updates the currently moved multi-rotor throttle curve item value
  */
void ConfigVehicleTypeWidget::updateMrThrottleCurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->mrThrottleCurveItemValue->setText(QString().sprintf("Val: %.2f",value));
}

/**
 Updates the moved ground vehicle front throttle curve item value
 */
void ConfigVehicleTypeWidget::updateGvThrottle1CurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->gvThrottleCurve1ItemValue->setText(QString().sprintf("Val: %.2f",value));
}

/**
 Updates the moved ground vehicle rear throttle curve item value
 */
void ConfigVehicleTypeWidget::updateGvThrottle2CurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->gvThrottleCurve2ItemValue->setText(QString().sprintf("Val: %.2f",value));
}

/**
  Updates the currently moved custom throttle curve item value (Custom throttle 1)
  */
void ConfigVehicleTypeWidget::updateCustomThrottle1CurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->customThrottleCurve1Value->setText(QString().sprintf("Val: %.2f",value));
}

/**
  Updates the currently moved custom throttle curve item value (Custom throttle 2)
  */
void ConfigVehicleTypeWidget::updateCustomThrottle2CurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->customThrottleCurve2Value->setText(QString().sprintf("Val: %.2f",value));
}


/**************************
  * Aircraft settings
  **************************/
/**
  Refreshes the current value of the SystemSettings which holds the aircraft type
  */
void ConfigVehicleTypeWidget::refreshWidgetsValues()
{
    if(!allObjectsUpdated())
        return;
	
	//WHAT DOES THIS DO?
    bool dirty=isDirty(); //WHY IS THIS CALLED HERE AND THEN AGAIN SEVERAL LINES LATER IN setupAirframeUI()
	
    // Get the Airframe type from the system settings:
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(obj);
    UAVObjectField *field = obj->getField(QString("AirframeType"));
    Q_ASSERT(field);
    // At this stage, we will need to have some hardcoded settings in this code, this
    // is not ideal, but here you go.
    QString frameType = field->getValue().toString();
    setupAirframeUI(frameType);

    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    field = obj->getField(QString("ThrottleCurve1"));
    Q_ASSERT(field);
    QList<double> curveValues;
    // If the 1st element of the curve is <= -10, then the curve
    // is a straight line (that's how the mixer works on the mainboard):
    if (field->getValue(0).toInt() <= -10) {
        m_aircraft->multiThrottleCurve->initLinearCurve(field->getNumElements(),(double)1);
        m_aircraft->fixedWingThrottle->initLinearCurve(field->getNumElements(),(double)1);
        m_aircraft->groundVehicleThrottle1->initLinearCurve(field->getNumElements(),(double)1);
    }
    else {
        double temp=0;
        double value;
        for (unsigned int i=0; i < field->getNumElements(); i++) {
            value=field->getValue(i).toDouble();
            temp+=value;
            curveValues.append(value);
        }
        if(temp==0)
        {
            m_aircraft->multiThrottleCurve->initLinearCurve(field->getNumElements(),0.9);
            m_aircraft->fixedWingThrottle->initLinearCurve(field->getNumElements(),(double)1);
            m_aircraft->groundVehicleThrottle1->initLinearCurve(field->getNumElements(),(double)1);
        }
        else
        {
            m_aircraft->multiThrottleCurve->initCurve(curveValues);
            m_aircraft->fixedWingThrottle->initCurve(curveValues);
            m_aircraft->groundVehicleThrottle1->initCurve(curveValues);
        }
    }
	
    // Setup all Throttle2 curves for all types of airframes //AT THIS MOMENT, THAT MEANS ONLY GROUND VEHICLES
	Q_ASSERT(obj);
    field = obj->getField(QString("ThrottleCurve2"));
    Q_ASSERT(field);
    curveValues.clear();
    // If the 1st element of the curve is <= -10, then the curve
    // is a straight line (that's how the mixer works on the mainboard):
    if (field->getValue(0).toInt() <= -10) {
        m_aircraft->groundVehicleThrottle2->initLinearCurve(field->getNumElements(),(double)1);
    }
    else {
        double temp=0;
        double value;
        for (unsigned int i=0; i < field->getNumElements(); i++) {
            value=field->getValue(i).toDouble();
            temp+=value;
            curveValues.append(value);
        }
        if(temp==0)
        {
            m_aircraft->groundVehicleThrottle2->initLinearCurve(field->getNumElements(),(double)1);
        }
        else
        {
            m_aircraft->groundVehicleThrottle2->initCurve(curveValues);
        }
    }
    
    // Load the Settings for fixed wing frames:
    if (frameType.startsWith("FixedWing")) {
		
        // Retrieve fixed wing settings
		if(1){
		refreshFixedWingWidgetsValues(frameType);
		}
//         // Then retrieve how channels are setup
//        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
//        Q_ASSERT(obj);
//        field = obj->getField(QString("FixedWingThrottle"));
//        Q_ASSERT(field);
//        m_aircraft->fwEngineChannelBox->setCurrentIndex(m_aircraft->fwEngineChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingRoll1"));
//        Q_ASSERT(field);
//        m_aircraft->fwAileron1ChannelBox->setCurrentIndex(m_aircraft->fwAileron1ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingRoll2"));
//        Q_ASSERT(field);
//        m_aircraft->fwAileron2ChannelBox->setCurrentIndex(m_aircraft->fwAileron2ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingPitch1"));
//        Q_ASSERT(field);
//        m_aircraft->fwElevator1ChannelBox->setCurrentIndex(m_aircraft->fwElevator1ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingPitch2"));
//        Q_ASSERT(field);
//        m_aircraft->fwElevator2ChannelBox->setCurrentIndex(m_aircraft->fwElevator2ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingYaw1"));
//        Q_ASSERT(field);
//        m_aircraft->fwRudder1ChannelBox->setCurrentIndex(m_aircraft->fwRudder1ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingYaw2"));
//        Q_ASSERT(field);
//        m_aircraft->fwRudder2ChannelBox->setCurrentIndex(m_aircraft->fwRudder2ChannelBox->findText(field->getValue().toString()));
//
//        if (frameType == "FixedWingElevon") {
//        // If the airframe is elevon, restore the slider setting
//            // Find the channel number for Elevon1 (FixedWingRoll1)
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            Q_ASSERT(obj);
//            int chMixerNumber = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
//            if (chMixerNumber >= 0) { // If for some reason the actuators were incoherent, we might fail here, hence the check.
//                field = obj->getField(mixerVectors.at(chMixerNumber));
//                int ti = field->getElementNames().indexOf("Roll");
//                m_aircraft->elevonSlider1->setValue(field->getDouble(ti)*100);
//                ti = field->getElementNames().indexOf("Pitch");
//                m_aircraft->elevonSlider2->setValue(field->getDouble(ti)*100);
//            }
//        }
//        if (frameType == "FixedWingVtail") {
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            Q_ASSERT(obj);
//            int chMixerNumber = m_aircraft->fwElevator1ChannelBox->currentIndex()-1;
//            if (chMixerNumber >=0) {
//                field = obj->getField(mixerVectors.at(chMixerNumber));
//                int ti = field->getElementNames().indexOf("Yaw");
//                m_aircraft->elevonSlider1->setValue(field->getDouble(ti)*100);
//                ti = field->getElementNames().indexOf("Pitch");
//                m_aircraft->elevonSlider2->setValue(field->getDouble(ti)*100);
//            }
//        }

    } else if (frameType == "Tri" || 
			   frameType == "QuadX" || frameType == "QuadP" ||
               frameType == "Hexa" || frameType == "HexaCoax" || frameType == "HexaX" || 
			   frameType == "Octo" || frameType == "OctoV" || frameType == "OctoCoaxP" || frameType == "OctoCoaxX" ) {
		
        // Retrieve multirotor settings
		if(1){
		refreshMultiRotorWidgetsValues(frameType);
		}
		else{//
//        //////////////////////////////////////////////////////////////////
//        // Retrieve Multirotor settings
//        //////////////////////////////////////////////////////////////////
//
//        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
//        Q_ASSERT(obj);
//        if (frameType == "QuadP") {
//            // Motors 1/2/3/4 are: N / E / S / W
//            field = obj->getField(QString("VTOLMotorN"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorE"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorS"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorW"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//			
//            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
//            // This assumes that all vectors are identical - if not, the user should use the
//            // "custom" setting.
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            Q_ASSERT(obj);
//            int tmpVal= m_aircraft->multiMotor1->currentIndex()-1;
//            // tmpVal will be -1 if value is set to "None"
//            if (tmpVal > -1) {
//                field = obj->getField(mixerVectors.at(tmpVal));
//                int i = field->getElementNames().indexOf("Pitch");
//                double val = field->getDouble(i)/1.27;
//                m_aircraft->mrPitchMixLevel->setValue(val);
//                i = field->getElementNames().indexOf("Yaw");
//                val = (1-field->getDouble(i)/1.27);
//                m_aircraft->mrYawMixLevel->setValue(val);
//                tmpVal = m_aircraft->multiMotor2->currentIndex()-1;
//                field = obj->getField(mixerVectors.at(tmpVal));
//                i = field->getElementNames().indexOf("Roll");
//                val = -field->getDouble(i)/1.27;
//                m_aircraft->mrRollMixLevel->setValue(val);
//            }
//        } else if (frameType == "QuadX") {
//            // Motors 1/2/3/4 are: NW / NE / SE / SW
//            field = obj->getField(QString("VTOLMotorNW"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorNE"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorSE"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorSW"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//			
//            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
//            // This assumes that all vectors are identical - if not, the user should use the
//            // "custom" setting.
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            Q_ASSERT(obj);
//            int tmpVal= m_aircraft->multiMotor1->currentIndex()-1;
//            // tmpVal will be -1 if value is set to "None"
//            if (tmpVal > -1) {
//                field = obj->getField(mixerVectors.at(tmpVal));
//                int i = field->getElementNames().indexOf("Pitch");
//                double val = field->getDouble(i)/1.27;
//                m_aircraft->mrPitchMixLevel->setValue(val);
//                i = field->getElementNames().indexOf("Yaw");
//                val = 1-field->getDouble(i)/1.27;
//                m_aircraft->mrYawMixLevel->setValue(val);
//                i = field->getElementNames().indexOf("Roll");
//                val = field->getDouble(i)/1.27;
//                m_aircraft->mrRollMixLevel->setValue(val);
//            }
//        } else if (frameType == "Hexa") {
//            // Motors 1/2/3 4/5/6 are: N / NE / SE / S / SW / NW
//            field = obj->getField(QString("VTOLMotorN"));
//            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorNE"));
//            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorSE"));
//            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorS"));
//            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorSW"));
//            m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorNW"));
//            m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//			
//            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
//            // This assumes that all vectors are identical - if not, the user should use the
//            // "custom" setting.
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            int tmpVal= m_aircraft->multiMotor1->currentIndex()-1;
//            // tmpVal will be -1 if value is set to "None"
//            if (tmpVal > -1) {
//                field = obj->getField(mixerVectors.at(tmpVal));
//                int i = field->getElementNames().indexOf("Pitch");
//                double val = floor(field->getDouble(i)/1.27);
//                m_aircraft->mrPitchMixLevel->setValue(val);
//                i = field->getElementNames().indexOf("Yaw");
//                val = floor(-field->getDouble(i)/1.27);
//                m_aircraft->mrYawMixLevel->setValue(val);
//                tmpVal = m_aircraft->multiMotor2->currentIndex()-1;
//                if(tmpVal>-1)
//                {
//                    field = obj->getField(mixerVectors.at(tmpVal));
//                    i = field->getElementNames().indexOf("Roll");
//                    val = floor(1-field->getDouble(i)/1.27);
//                    m_aircraft->mrRollMixLevel->setValue(val);
//                }
//            }
//        } else if (frameType == "HexaX") {
//            // Motors 1/2/3 4/5/6 are: NE / E / SE / SW / W / NW
//            field = obj->getField(QString("VTOLMotorNE"));
//            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorE"));
//            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorSE"));
//            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorSW"));
//            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorW"));
//            m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorNW"));
//            m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//			
//            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
//            // This assumes that all vectors are identical - if not, the user should use the
//            // "custom" setting.
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            int tmpVal= m_aircraft->multiMotor1->currentIndex()-1;
//            // tmpVal will be -1 if value is set to "None"
//            if (tmpVal > -1) {
//                field = obj->getField(mixerVectors.at(tmpVal));
//                int i = field->getElementNames().indexOf("Pitch");
//                double val = floor(field->getDouble(i)/1.27);
//                m_aircraft->mrPitchMixLevel->setValue(val);
//                i = field->getElementNames().indexOf("Yaw");
//                val = floor(-field->getDouble(i)/1.27);
//                m_aircraft->mrYawMixLevel->setValue(val);
//                tmpVal = m_aircraft->multiMotor2->currentIndex()-1;
//                field = obj->getField(mixerVectors.at(tmpVal));
//                i = field->getElementNames().indexOf("Roll");
//                val = floor(1-field->getDouble(i)/1.27);
//                m_aircraft->mrRollMixLevel->setValue(val);
//            }
//        } else if (frameType == "HexaCoax") {
//            // Motors 1/2/3 4/5/6 are: NW/W NE/E S/SE
//            field = obj->getField(QString("VTOLMotorNW"));
//            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorW"));
//            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorNE"));
//            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorE"));
//            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorS"));
//            m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorSE"));
//            m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//			
//            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
//            // This assumes that all vectors are identical - if not, the user should use the
//            // "custom" setting.
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            int tmpVal= m_aircraft->multiMotor1->currentIndex()-1;
//            // tmpVal will be -1 if value is set to "None"
//            if (tmpVal > -1) {
//                field = obj->getField(mixerVectors.at(tmpVal));
//                int i = field->getElementNames().indexOf("Pitch");
//                double val = floor(2*field->getDouble(i)/1.27);
//                m_aircraft->mrPitchMixLevel->setValue(val);
//                i = field->getElementNames().indexOf("Yaw");
//                val = floor(-field->getDouble(i)/1.27);
//                m_aircraft->mrYawMixLevel->setValue(val);
//                i = field->getElementNames().indexOf("Roll");
//                val = floor(field->getDouble(i)/1.27);
//                m_aircraft->mrRollMixLevel->setValue(val);
//            }
//        }  else if (frameType == "Octo" || frameType == "OctoV" ||
//                    frameType == "OctoCoaxP") {
//            // Motors 1 to 8 are N / NE / E / etc
//            field = obj->getField(QString("VTOLMotorN"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorNE"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorE"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorSE"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorS"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorSW"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorW"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor7->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorNW"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor8->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//			
//            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
//            // This assumes that all vectors are identical - if not, the user should use the
//            // "custom" setting.
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            int tmpVal= m_aircraft->multiMotor1->currentIndex()-1;
//            // tmpVal will be -1 if value is set to "None"
//            if (tmpVal > -1) {
//                if (frameType == "Octo") {
//                    field = obj->getField(mixerVectors.at(tmpVal));
//                    int i = field->getElementNames().indexOf("Pitch");
//                    double val = floor(field->getDouble(i)/1.27);
//                    m_aircraft->mrPitchMixLevel->setValue(val);
//                    i = field->getElementNames().indexOf("Yaw");
//                    val = floor(-field->getDouble(i)/1.27);
//                    m_aircraft->mrYawMixLevel->setValue(val);
//                    tmpVal = m_aircraft->multiMotor2->currentIndex()-1;
//                    field = obj->getField(mixerVectors.at(tmpVal));
//                    i = field->getElementNames().indexOf("Roll");
//                    val = floor(-field->getDouble(i)/1.27);
//                    m_aircraft->mrRollMixLevel->setValue(val);
//                } else if (frameType == "OctoV") {
//                    field = obj->getField(mixerVectors.at(tmpVal));
//                    int i = field->getElementNames().indexOf("Yaw");
//                    double val = floor(-field->getDouble(i)/1.27);
//                    m_aircraft->mrYawMixLevel->setValue(val);
//                    i = field->getElementNames().indexOf("Roll");
//                    val = floor(-field->getDouble(i)/1.27);
//                    m_aircraft->mrRollMixLevel->setValue(val);
//                    tmpVal = m_aircraft->multiMotor2->currentIndex()-1;
//                    field = obj->getField(mixerVectors.at(tmpVal));
//                    i = field->getElementNames().indexOf("Pitch");
//                    val = floor(field->getDouble(i)/1.27);
//                    m_aircraft->mrPitchMixLevel->setValue(val);
//
//                } else if (frameType == "OctoCoaxP") {
//                    field = obj->getField(mixerVectors.at(tmpVal));
//                    int i = field->getElementNames().indexOf("Pitch");
//                    double val = floor(field->getDouble(i)/1.27);
//                    m_aircraft->mrPitchMixLevel->setValue(val);
//                    i = field->getElementNames().indexOf("Yaw");
//                    val = floor(-field->getDouble(i)/1.27);
//                    m_aircraft->mrYawMixLevel->setValue(val);
//                    tmpVal = m_aircraft->multiMotor3->currentIndex()-1;
//                    field = obj->getField(mixerVectors.at(tmpVal));
//                    i = field->getElementNames().indexOf("Roll");
//                    val = floor(-field->getDouble(i)/1.27);
//                    m_aircraft->mrRollMixLevel->setValue(val);
//
//                }
//            }
//        } else if (frameType == "OctoCoaxX") {
//                 // Motors 1 to 8 are N / NE / E / etc
//                 field = obj->getField(QString("VTOLMotorNW"));
//                 Q_ASSERT(field);
//                 m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
//                 field = obj->getField(QString("VTOLMotorN"));
//                 Q_ASSERT(field);
//                 m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
//                 field = obj->getField(QString("VTOLMotorNE"));
//                 Q_ASSERT(field);
//                 m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
//                 field = obj->getField(QString("VTOLMotorE"));
//                 Q_ASSERT(field);
//                 m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//                 field = obj->getField(QString("VTOLMotorSE"));
//                 Q_ASSERT(field);
//                 m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//                 field = obj->getField(QString("VTOLMotorS"));
//                 Q_ASSERT(field);
//                 m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//                 field = obj->getField(QString("VTOLMotorSW"));
//                 Q_ASSERT(field);
//                 m_aircraft->multiMotor7->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//                 field = obj->getField(QString("VTOLMotorW"));
//                 Q_ASSERT(field);
//                 m_aircraft->multiMotor8->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
//			
//                 // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
//                 // This assumes that all vectors are identical - if not, the user should use the
//                 // "custom" setting.
//                 obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//                 int tmpVal= m_aircraft->multiMotor1->currentIndex()-1;
//                 // tmpVal will be -1 if value is set to "None"
//                 if (tmpVal > -1) {
//                     field = obj->getField(mixerVectors.at(tmpVal));
//                     int i = field->getElementNames().indexOf("Pitch");
//                     double val = floor(field->getDouble(i)/1.27);
//                     m_aircraft->mrPitchMixLevel->setValue(val);
//                     i = field->getElementNames().indexOf("Yaw");
//                     val = floor(-field->getDouble(i)/1.27);
//                     m_aircraft->mrYawMixLevel->setValue(val);
//                     i = field->getElementNames().indexOf("Roll");
//                     val = floor(field->getDouble(i)/1.27);
//                     m_aircraft->mrRollMixLevel->setValue(val);
//                 }
//        } else if (frameType == "Tri") {
//            // Motors 1 to 8 are N / NE / E / etc
//            field = obj->getField(QString("VTOLMotorNW"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorNE"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
//            field = obj->getField(QString("VTOLMotorS"));
//            Q_ASSERT(field);
//            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
//            field = obj->getField(QString("FixedWingYaw1"));
//            Q_ASSERT(field);
//            m_aircraft->triYawChannelBoxBox->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
//
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            int tmpVal= m_aircraft->multiMotor1->currentIndex()-1;
//            // tmpVal will be -1 if value is set to "None"
//            if (tmpVal > -1) {
//                field = obj->getField(mixerVectors.at(tmpVal));
//                int i = field->getElementNames().indexOf("Pitch");
//                double val = floor(2*field->getDouble(i)/1.27);
//                m_aircraft->mrPitchMixLevel->setValue(val);
//                i = field->getElementNames().indexOf("Roll");
//                val = floor(field->getDouble(i)/1.27);
//                m_aircraft->mrRollMixLevel->setValue(val);
//            }
//
//        }
//        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//        Q_ASSERT(obj);
//        // Now, retrieve the Feedforward values:
//        field = obj->getField(QString("FeedForward"));
//        Q_ASSERT(field);
//        m_aircraft->feedForwardSlider->setValue(field->getDouble()*100);
//        field = obj->getField(QString("AccelTime"));
//        Q_ASSERT(field);
//        m_aircraft->accelTime->setValue(field->getDouble());
//        field = obj->getField(QString("DecelTime"));
//        Q_ASSERT(field);
//        m_aircraft->decelTime->setValue(field->getDouble());
//        field = obj->getField(QString("MaxAccel"));
//        Q_ASSERT(field);
//        m_aircraft->maxAccelSlider->setValue(field->getDouble());
	}
    } else if (frameType == "HeliCP") {
        m_aircraft->widget_3->requestccpmUpdate();
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Helicopter"));//"Helicopter"
	} else if (frameType.startsWith("GroundVehicle")) {

		// Retrieve ground vehicle settings
		if(1){
			refreshGroundVehicleWidgetsValues(frameType);
		}
		
//		//THIS SECTION STILL NEEDS WORK. FOR THE MOMENT, USE THE FIXED-WING ONBOARD SETTING IN ORDER TO MINIMIZE CHANCES OF BOLLOXING REAL CODE
//		// Then retrieve channel setup values
//        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
//        Q_ASSERT(obj);
//        field = obj->getField(QString("FixedWingThrottle"));
//        Q_ASSERT(field);
//        m_aircraft->fwEngineChannelBox->setCurrentIndex(m_aircraft->fwEngineChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingRoll1"));
//        Q_ASSERT(field);
//        m_aircraft->fwAileron1ChannelBox->setCurrentIndex(m_aircraft->fwAileron1ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingRoll2"));
//        Q_ASSERT(field);
//        m_aircraft->fwAileron2ChannelBox->setCurrentIndex(m_aircraft->fwAileron2ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingPitch1"));
//        Q_ASSERT(field);
//        m_aircraft->fwElevator1ChannelBox->setCurrentIndex(m_aircraft->fwElevator1ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingPitch2"));
//        Q_ASSERT(field);
//        m_aircraft->fwElevator2ChannelBox->setCurrentIndex(m_aircraft->fwElevator2ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingYaw1"));
//        Q_ASSERT(field);
//        m_aircraft->fwRudder1ChannelBox->setCurrentIndex(m_aircraft->fwRudder1ChannelBox->findText(field->getValue().toString()));
//        field = obj->getField(QString("FixedWingYaw2"));
//        Q_ASSERT(field);
//        m_aircraft->fwRudder2ChannelBox->setCurrentIndex(m_aircraft->fwRudder2ChannelBox->findText(field->getValue().toString()));
//		
//        if (frameType == "GroundDifferential") {
//			//CURRENTLY BROKEN UNTIL WE DECIDE HOW DIFFERENTIAL SHOULD BEHAVE
//			// If the airframe is differential, restore the slider setting
//            // Find the channel number for Elevon1 (FixedWingRoll1)
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            Q_ASSERT(obj);
//            int chMixerNumber = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
//            if (chMixerNumber >= 0) { // If for some reason the actuators were incoherent, we might fail here, hence the check.
//                field = obj->getField(mixerVectors.at(chMixerNumber));
//                int ti = field->getElementNames().indexOf("Roll");
//                m_aircraft->differentialSteeringSlider1->setValue(field->getDouble(ti)*100);
//				
//                ti = field->getElementNames().indexOf("Pitch");
//                m_aircraft->differentialSteeringSlider2->setValue(field->getDouble(ti)*100);
//            }
//        }
//        if (frameType == "GroundMotorcycle") {
//			//CURRENTLY BROKEN UNTIL WE DECIDE HOW MOTORCYCLE SHOULD BEHAVE
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//            Q_ASSERT(obj);
//            int chMixerNumber = m_aircraft->gvMotor1ChannelBox->currentIndex()-1;
//            if (chMixerNumber >=0) {
//                field = obj->getField(mixerVectors.at(chMixerNumber));
//                int ti = field->getElementNames().indexOf("Yaw");
//                m_aircraft->differentialSteeringSlider1->setValue(field->getDouble(ti)*100);
//				
//                ti = field->getElementNames().indexOf("Pitch");
//                m_aircraft->differentialSteeringSlider2->setValue(field->getDouble(ti)*100);
//            }
//        }
	} else if (frameType == "Custom") {
		m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Custom"));
	}
	
	
    updateCustomAirframeUI();
    setDirty(dirty);
}

/**
  \brief Sets up the mixer depending on Airframe type. Accepts either system settings or
  combo box entry from airframe type, as those do not overlap.
  */
void ConfigVehicleTypeWidget::setupAirframeUI(QString frameType)
{
	
    bool dirty=isDirty();
	if(frameType == "FixedWing" || frameType == "Elevator aileron rudder" || 
	   frameType == "FixedWingElevon" || frameType == "Elevon" ||
	   frameType == "FixedWingVtail" || frameType == "Vtail"){
		setupFixedWingUI(frameType);
//	}
//    else if (frameType == "FixedWing" || frameType == "Elevator aileron rudder") {
//        // Setup the UI
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Fixed Wing"));
//        m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Elevator aileron rudder"));
//        m_aircraft->fwRudder1ChannelBox->setEnabled(true);
//        m_aircraft->fwRudder1Label->setEnabled(true);
//        m_aircraft->fwRudder2ChannelBox->setEnabled(true);
//        m_aircraft->fwRudder2Label->setEnabled(true);
//        m_aircraft->fwElevator1ChannelBox->setEnabled(true);
//        m_aircraft->fwElevator1Label->setEnabled(true);
//        m_aircraft->fwElevator2ChannelBox->setEnabled(true);
//        m_aircraft->fwElevator2Label->setEnabled(true);
//        m_aircraft->fwAileron1ChannelBox->setEnabled(true);
//        m_aircraft->fwAileron1Label->setEnabled(true);
//        m_aircraft->fwAileron2ChannelBox->setEnabled(true);
//        m_aircraft->fwAileron2Label->setEnabled(true);
//
//        m_aircraft->fwAileron1Label->setText("Aileron 1");
//        m_aircraft->fwAileron2Label->setText("Aileron 2");
//        m_aircraft->fwElevator1Label->setText("Elevator 1");
//        m_aircraft->fwElevator2Label->setText("Elevator 2");
//        m_aircraft->elevonMixBox->setHidden(true);
//
//    } else if (frameType == "FixedWingElevon" || frameType == "Elevon") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Fixed Wing"));
//        m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Elevon"));
//        m_aircraft->fwAileron1Label->setText("Elevon 1");
//        m_aircraft->fwAileron2Label->setText("Elevon 2");
//        m_aircraft->fwElevator1ChannelBox->setEnabled(false);
//        m_aircraft->fwElevator1Label->setEnabled(false);
//        m_aircraft->fwElevator2ChannelBox->setEnabled(false);
//        m_aircraft->fwElevator2Label->setEnabled(false);
//        m_aircraft->fwRudder1ChannelBox->setEnabled(true);
//        m_aircraft->fwRudder1Label->setEnabled(true);
//        m_aircraft->fwRudder2ChannelBox->setEnabled(true);
//        m_aircraft->fwRudder2Label->setEnabled(true);
//        m_aircraft->fwElevator1Label->setText("Elevator 1");
//        m_aircraft->fwElevator2Label->setText("Elevator 2");
//        m_aircraft->elevonMixBox->setHidden(false);
//        m_aircraft->elevonLabel1->setText("Roll");
//        m_aircraft->elevonLabel2->setText("Pitch");
//
//     } else if (frameType == "FixedWingVtail" || frameType == "Vtail") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Fixed Wing"));
//        m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Vtail"));
//        m_aircraft->fwRudder1ChannelBox->setEnabled(false);
//        m_aircraft->fwRudder1Label->setEnabled(false);
//        m_aircraft->fwRudder2ChannelBox->setEnabled(false);
//        m_aircraft->fwRudder2Label->setEnabled(false);
//        m_aircraft->fwElevator1ChannelBox->setEnabled(true);
//        m_aircraft->fwElevator1Label->setEnabled(true);
//        m_aircraft->fwElevator1Label->setText("Vtail 1");
//        m_aircraft->fwElevator2Label->setText("Vtail 2");
//        m_aircraft->elevonMixBox->setHidden(false);
//        m_aircraft->fwElevator2ChannelBox->setEnabled(true);
//        m_aircraft->fwElevator2Label->setEnabled(true);
//        m_aircraft->fwAileron1Label->setText("Aileron 1");
//        m_aircraft->fwAileron2Label->setText("Aileron 2");
//        m_aircraft->elevonLabel1->setText("Rudder");
//        m_aircraft->elevonLabel2->setText("Pitch");

	 } else if (frameType == "Tri" || frameType == "Tricopter Y" ||
				frameType == "QuadX" || frameType == "Quad X" ||
				frameType == "QuadP" || frameType == "Quad +" ||
				frameType == "Hexa" || frameType == "Hexacopter" ||
				frameType == "HexaX" || frameType == "Hexacopter X" ||
				frameType == "HexaCoax" || frameType == "Hexacopter Y6" ||
				frameType == "Octo" || frameType == "Octocopter" ||
				frameType == "OctoV" || frameType == "Octocopter V" ||
				frameType == "OctoCoaxP" || frameType == "Octo Coax +" ) {
		 
		 //Call multi-rotor setup UI
		 setupMultiRotorUI(frameType);
	 }	 
//	 } else if (frameType == "QuadX" || frameType == "Quad X") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Quad X"));
//        quad->setElementId("quad-X");
//        m_aircraft->multiMotor4->setEnabled(true);
//        m_aircraft->multiMotor5->setEnabled(false);
//        m_aircraft->multiMotor6->setEnabled(false);
//        m_aircraft->multiMotor7->setEnabled(false);
//        m_aircraft->multiMotor8->setEnabled(false);
//        m_aircraft->triYawChannelBox->setEnabled(false);
//        m_aircraft->mrRollMixLevel->setValue(50);
//        m_aircraft->mrPitchMixLevel->setValue(50);
//        m_aircraft->mrYawMixLevel->setValue(50);
//    } else if (frameType == "QuadP" || frameType == "Quad +") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Quad +"));
//        quad->setElementId("quad-plus");
//        m_aircraft->multiMotor4->setEnabled(true);
//        m_aircraft->multiMotor5->setEnabled(false);
//        m_aircraft->multiMotor6->setEnabled(false);
//        m_aircraft->multiMotor7->setEnabled(false);
//        m_aircraft->multiMotor8->setEnabled(false);
//        m_aircraft->triYawChannelBox->setEnabled(false);
//                m_aircraft->mrRollMixLevel->setValue(100);
//        m_aircraft->mrPitchMixLevel->setValue(100);
//        m_aircraft->mrYawMixLevel->setValue(50);
//    } else if (frameType == "Hexa" || frameType == "Hexacopter") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Hexacopter"));
//        quad->setElementId("quad-hexa");
//        m_aircraft->multiMotor4->setEnabled(true);
//        m_aircraft->multiMotor5->setEnabled(true);
//        m_aircraft->multiMotor6->setEnabled(true);
//        m_aircraft->multiMotor7->setEnabled(false);
//        m_aircraft->multiMotor8->setEnabled(false);
//        m_aircraft->triYawChannelBox->setEnabled(false);
//        m_aircraft->mrRollMixLevel->setValue(50);
//        m_aircraft->mrPitchMixLevel->setValue(33);
//        m_aircraft->mrYawMixLevel->setValue(33);
//    } else if (frameType == "Octo" || frameType == "Octocopter") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Octocopter"));
//        quad->setElementId("quad-octo");
//        m_aircraft->multiMotor4->setEnabled(true);
//        m_aircraft->multiMotor5->setEnabled(true);
//        m_aircraft->multiMotor6->setEnabled(true);
//        m_aircraft->multiMotor7->setEnabled(true);
//        m_aircraft->multiMotor8->setEnabled(true);
//        m_aircraft->triYawChannelBox->setEnabled(false);
//        m_aircraft->mrRollMixLevel->setValue(33);
//        m_aircraft->mrPitchMixLevel->setValue(33);
//        m_aircraft->mrYawMixLevel->setValue(25);
//    } else if (frameType == "HexaX" || frameType == "Hexacopter X" ) {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Hexacopter X"));
//        quad->setElementId("quad-hexa-H");
//        m_aircraft->multiMotor4->setEnabled(true);
//        m_aircraft->multiMotor5->setEnabled(true);
//        m_aircraft->multiMotor6->setEnabled(true);
//        m_aircraft->multiMotor7->setEnabled(false);
//        m_aircraft->multiMotor8->setEnabled(false);
//        m_aircraft->triYawChannelBox->setEnabled(false);
//        m_aircraft->mrRollMixLevel->setValue(33);
//        m_aircraft->mrPitchMixLevel->setValue(50);
//        m_aircraft->mrYawMixLevel->setValue(33);
//
//    } else if (frameType == "OctoV" || frameType == "Octocopter V") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Octocopter V"));
//        quad->setElementId("quad-octo-v");
//        m_aircraft->multiMotor4->setEnabled(true);
//        m_aircraft->multiMotor5->setEnabled(true);
//        m_aircraft->multiMotor6->setEnabled(true);
//        m_aircraft->multiMotor7->setEnabled(true);
//        m_aircraft->multiMotor8->setEnabled(true);
//        m_aircraft->triYawChannelBox->setEnabled(false);
//        m_aircraft->mrRollMixLevel->setValue(25);
//        m_aircraft->mrPitchMixLevel->setValue(25);
//        m_aircraft->mrYawMixLevel->setValue(25);
//
//    } else if (frameType == "OctoCoaxP" || frameType == "Octo Coax +") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Octo Coax +"));
//        quad->setElementId("octo-coax-P");
//        m_aircraft->multiMotor4->setEnabled(true);
//        m_aircraft->multiMotor5->setEnabled(true);
//        m_aircraft->multiMotor6->setEnabled(true);
//        m_aircraft->multiMotor7->setEnabled(true);
//        m_aircraft->multiMotor8->setEnabled(true);
//        m_aircraft->triYawChannelBox->setEnabled(false);
//        m_aircraft->mrRollMixLevel->setValue(100);
//        m_aircraft->mrPitchMixLevel->setValue(100);
//        m_aircraft->mrYawMixLevel->setValue(50);
//
//    } else if (frameType == "OctoCoaxX" || frameType == "Octo Coax X") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Octo Coax X"));
//        quad->setElementId("octo-coax-X");
//        m_aircraft->multiMotor4->setEnabled(true);
//        m_aircraft->multiMotor5->setEnabled(true);
//        m_aircraft->multiMotor6->setEnabled(true);
//        m_aircraft->multiMotor7->setEnabled(true);
//        m_aircraft->multiMotor8->setEnabled(true);
//        m_aircraft->triYawChannelBox->setEnabled(false);
//        m_aircraft->mrRollMixLevel->setValue(50);
//        m_aircraft->mrPitchMixLevel->setValue(50);
//        m_aircraft->mrYawMixLevel->setValue(50);
//
//    } else if (frameType == "HexaCoax" || frameType == "Hexacopter Y6") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Hexacopter Y6"));
//        quad->setElementId("hexa-coax");
//        m_aircraft->multiMotor4->setEnabled(true);
//        m_aircraft->multiMotor5->setEnabled(true);
//        m_aircraft->multiMotor6->setEnabled(true);
//        m_aircraft->multiMotor7->setEnabled(false);
//        m_aircraft->multiMotor8->setEnabled(false);
//        m_aircraft->triYawChannelBox->setEnabled(false);
//        m_aircraft->mrRollMixLevel->setValue(100);
//        m_aircraft->mrPitchMixLevel->setValue(50);
//        m_aircraft->mrYawMixLevel->setValue(66);
//
//    } else if (frameType == "Tri" || frameType == "Tricopter Y") {
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
//        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Tricopter Y"));
//        quad->setElementId("tri");
//        m_aircraft->multiMotor4->setEnabled(false);
//        m_aircraft->multiMotor5->setEnabled(false);
//        m_aircraft->multiMotor6->setEnabled(false);
//        m_aircraft->multiMotor7->setEnabled(false);
//        m_aircraft->multiMotor8->setEnabled(false);
//        m_aircraft->triYawChannelBox->setEnabled(true);
//
    else if (frameType == "GroundVehicleCar" || frameType == "Turnable (car)" ||
			 frameType == "GroundVehicleDifferential" || frameType == "Differential (tank)" || 
			 frameType == "GroundVehicleMotorcyle" || frameType == "Motorcycle") {
		setupGroundVehicleUI(frameType);
		
////    } else if (m_aircraft->aircraftType->currentText() == "Ground") {
//		m_aircraft->differentialSteeringMixBox->setHidden(true);			
//		//STILL NEEDS WORK
//        // Setup the UI
//        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Ground"));
//        
//		m_aircraft->gvSteering1Label->setEnabled(true);
//        m_aircraft->gvSteering1Label->setEnabled(true);       
//		m_aircraft->gvSteering2Label->setEnabled(true);
//        m_aircraft->gvSteering2Label->setEnabled(true);
//        m_aircraft->gvMotor1ChannelBox->setEnabled(true);
//        m_aircraft->gvMotor1Label->setEnabled(true);
//		m_aircraft->gvAileron1ChannelBox->setEnabled(true);
//        m_aircraft->gvAileron1Label->setEnabled(true);
//		m_aircraft->gvAileron2ChannelBox->setEnabled(true);
//        m_aircraft->gvAileron2Label->setEnabled(true);
//		
//		if (frameType == "GroundVehicleDifferential" || frameType == "Differential (tank)"){
//			m_aircraft->groundVehicleType->setCurrentIndex(m_aircraft->groundVehicleType->findText("Differential (tank)"));
//			m_aircraft->gvMotor2ChannelBox->setEnabled(true);
//			m_aircraft->gvMotor2Label->setEnabled(true);
//			
//			m_aircraft->gvMotor1Label->setText("Left motor");
//			m_aircraft->gvMotor2Label->setText("Right motor");
//			m_aircraft->differentialSteeringMixBox->setHidden(false);
//		}
//		else if (frameType == "GroundVehicleMotorcyle" || frameType == "Motorcycle"){
//			m_aircraft->groundVehicleType->setCurrentIndex(m_aircraft->groundVehicleType->findText("Motorcycle"));
//			m_aircraft->gvMotor2ChannelBox->setEnabled(false);
//			m_aircraft->gvMotor2Label->setEnabled(false);
//
//			m_aircraft->gvMotor1Label->setText("Motor");
//			m_aircraft->gvMotor2Label->setText("Elevator 2");
//			m_aircraft->differentialSteeringMixBox->setHidden(true);
//		}
//		else {
//			m_aircraft->groundVehicleType->setCurrentIndex(m_aircraft->groundVehicleType->findText("Turnable (car)"));
//			
//			m_aircraft->gvMotor2ChannelBox->setEnabled(true);
//			m_aircraft->gvMotor2Label->setEnabled(true);
//
//			m_aircraft->gvMotor1Label->setText("Front motor");
//			m_aircraft->gvMotor2Label->setText("Rear motor");
//			m_aircraft->differentialSteeringMixBox->setHidden(true);			
//		}
				
    }
	
	//SHOULDN'T THIS BE DONE ONLY IN QUAD SETUP, AND NOT ALL THE REST???
    m_aircraft->quadShape->setSceneRect(quad->boundingRect());
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);

    setDirty(dirty);
}

/**
  Reset the contents of a field
  */
void ConfigVehicleTypeWidget::resetField(UAVObjectField * field)
{
    for (unsigned int i=0;i<field->getNumElements();i++) {
        field->setValue(0,i);
    }
}

/**
  Reset actuator values
  */
void ConfigVehicleTypeWidget::resetActuators()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    QList<UAVObjectField*> fieldList = obj->getFields();
    // Reset all assignements first:
    foreach (UAVObjectField* field, fieldList) {
        // NOTE: we assume that all options in ActuatorSettings are a channel assignement
        // except for the options called "ChannelBoxXXX"
        if (field->getUnits().contains("channel")) {
            field->setValue(field->getOptions().last());
        }
    }
}
//
///**
//  Setup Elevator/Aileron/Rudder airframe.
//
//   If both Aileron channels are set to 'None' (EasyStar), do Pitch/Rudder mixing
//
//   Returns False if impossible to create the mixer.
//  */
//bool ConfigVehicleTypeWidget::setupFrameFixedWing()
//{
//    // Check coherence:
//    // - At least Pitch and either Roll or Yaw
//    if (m_aircraft->fwEngineChannelBox->currentText() == "None" ||
//        m_aircraft->fwElevator1ChannelBox->currentText() == "None" ||
//        ((m_aircraft->fwAileron1ChannelBox->currentText() == "None") &&
//        (m_aircraft->fwRudder1ChannelBox->currentText() == "None"))) {
//        // TODO: explain the problem in the UI
//        m_aircraft->fwStatusLabel->setText("ERROR: check channel assignment");
//        return false;
//    }
//    // Now setup the channels:
//    resetActuators();
//
//    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
//    Q_ASSERT(obj);
//
//    // Elevator
//    UAVObjectField *field = obj->getField("FixedWingPitch1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwElevator1ChannelBox->currentText());
//    field = obj->getField("FixedWingPitch2");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwElevator2ChannelBox->currentText());
//    // Aileron
//    field = obj->getField("FixedWingRoll1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron1ChannelBox->currentText());
//    field = obj->getField("FixedWingRoll2");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron2ChannelBox->currentText());
//    // Rudder
//    field = obj->getField("FixedWingYaw1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwRudder1ChannelBox->currentText());
//    // Throttle
//    field = obj->getField("FixedWingThrottle");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwEngineChannelBox->currentText());
//
//    obj->updated();
//
//    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//    Q_ASSERT(obj);
//    // ... and compute the matrix:
//    // In order to make code a bit nicer, we assume:
//    // - ChannelBox dropdowns start with 'None', then 0 to 7
//
//    // 1. Assign the servo/motor/none for each channel
//    // Disable all
//    foreach(QString mixer, mixerTypes) {
//        field = obj->getField(mixer);
//        Q_ASSERT(field);
//        field->setValue("Disabled");
//    }
//    // and set only the relevant channels:
//    // Engine
//    int tmpVal = m_aircraft->fwEngineChannelBox->currentIndex()-1;
//    field = obj->getField(mixerTypes.at(tmpVal));
//    field->setValue("Motor");
//    field = obj->getField(mixerVectors.at(tmpVal));
//    // First of all reset the vector
//    resetField(field);
//    int ti = field->getElementNames().indexOf("ThrottleCurve1");
//    field->setValue(127, ti);
//
//    // Rudder
//    tmpVal = m_aircraft->fwRudder1ChannelBox->currentIndex()-1;
//    // tmpVal will be -1 if rudder is set to "None"
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Yaw");
//        field->setValue(127, ti);
//    } // Else: we have no rudder, only ailerons, we're fine with it.
//
//    // Ailerons
//    tmpVal = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Roll");
//        field->setValue(127, ti);
//        // Only set Aileron 2 if Aileron 1 is defined
//        tmpVal = m_aircraft->fwAileron2ChannelBox->currentIndex()-1;
//        if (tmpVal > -1) {
//            field = obj->getField(mixerTypes.at(tmpVal));
//            field->setValue("Servo");
//            field = obj->getField(mixerVectors.at(tmpVal));
//            resetField(field);
//            ti = field->getElementNames().indexOf("Roll");
//            field->setValue(127, ti);
//        }
//    } // Else we have no ailerons. Our consistency check guarantees we have
//      // rudder in this case, so we're fine with it too.
//
//    // Elevator
//    tmpVal = m_aircraft->fwElevator1ChannelBox->currentIndex()-1;
//    field = obj->getField(mixerTypes.at(tmpVal));
//    field->setValue("Servo");
//    field = obj->getField(mixerVectors.at(tmpVal));
//    resetField(field);
//    ti = field->getElementNames().indexOf("Pitch");
//    field->setValue(127, ti);
//    // Only set Elevator 2 if it is defined
//    tmpVal = m_aircraft->fwElevator2ChannelBox->currentIndex()-1;
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Pitch");
//        field->setValue(127, ti);
//    }
//
//    obj->updated();
//    m_aircraft->fwStatusLabel->setText("Mixer generated");
//
//    return true;
//}
//
///**
//  Setup Elevon
//  */
//bool ConfigVehicleTypeWidget::setupFrameElevon()
//{
//    // Check coherence:
//    // - At least Aileron1 and Aileron 2, and engine
//    if (m_aircraft->fwEngineChannelBox->currentText() == "None" ||
//        m_aircraft->fwAileron1ChannelBox->currentText() == "None" ||
//        m_aircraft->fwAileron2ChannelBox->currentText() == "None") {
//        // TODO: explain the problem in the UI
//        m_aircraft->fwStatusLabel->setText("ERROR: check channel assignment");
//        return false;
//    }
//
//    resetActuators();
//    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
//    Q_ASSERT(obj);
//
//    // Elevons
//    UAVObjectField *field = obj->getField("FixedWingRoll1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron1ChannelBox->currentText());
//    field = obj->getField("FixedWingRoll2");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron2ChannelBox->currentText());
//    // Rudder 1 (can be None)
//    field = obj->getField("FixedWingYaw1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwRudder1ChannelBox->currentText());
//    // Rudder 2 (can be None)
//    field = obj->getField("FixedWingYaw2");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwRudder2ChannelBox->currentText());
//    // Throttle
//    field = obj->getField("FixedWingThrottle");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwEngineChannelBox->currentText());
//
//    obj->updated();
//
//    // Save the curve:
//    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//    Q_ASSERT(obj);
//    // ... and compute the matrix:
//    // In order to make code a bit nicer, we assume:
//    // - ChannelBox dropdowns start with 'None', then 0 to 7
//
//    // 1. Assign the servo/motor/none for each channel
//    // Disable all
//    foreach(QString mixer, mixerTypes) {
//        field = obj->getField(mixer);
//        Q_ASSERT(field);
//        field->setValue("Disabled");
//    }
//    // and set only the relevant channels:
//    // Engine
//    int tmpVal = m_aircraft->fwEngineChannelBox->currentIndex()-1;
//    field = obj->getField(mixerTypes.at(tmpVal));
//    field->setValue("Motor");
//    field = obj->getField(mixerVectors.at(tmpVal));
//    // First of all reset the vector
//    resetField(field);
//    int ti = field->getElementNames().indexOf("ThrottleCurve1");
//    field->setValue(127, ti);
//
//    // Rudder 1
//    tmpVal = m_aircraft->fwRudder1ChannelBox->currentIndex()-1;
//    // tmpVal will be -1 if rudder 1 is set to "None"
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Yaw");
//        field->setValue(127, ti);
//    } // Else: we have no rudder, only elevons, we're fine with it.
//
//    // Rudder 2
//    tmpVal = m_aircraft->fwRudder2ChannelBox->currentIndex()-1;
//    // tmpVal will be -1 if rudder 2 is set to "None"
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Yaw");
//        field->setValue(-127, ti);
//    } // Else: we have no rudder, only elevons, we're fine with it.
//
//    tmpVal = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Pitch");
//        field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
//        ti = field->getElementNames().indexOf("Roll");
//        field->setValue((double)m_aircraft->elevonSlider1->value()*1.27,ti);
//    }
//
//    tmpVal = m_aircraft->fwAileron2ChannelBox->currentIndex()-1;
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Pitch");
//        field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
//        ti = field->getElementNames().indexOf("Roll");
//        field->setValue(-(double)m_aircraft->elevonSlider1->value()*1.27,ti);
//    }
//
//    obj->updated();
//    m_aircraft->fwStatusLabel->setText("Mixer generated");
//    return true;
//}
//
//
///**
//  Setup VTail
//  */
//bool ConfigVehicleTypeWidget::setupFrameVtail()
//{
//    // Check coherence:
//    // - At least Pitch1 and Pitch2, and engine
//    if (m_aircraft->fwEngineChannelBox->currentText() == "None" ||
//        m_aircraft->fwElevator1ChannelBox->currentText() == "None" ||
//        m_aircraft->fwElevator2ChannelBox->currentText() == "None") {
//        // TODO: explain the problem in the UI
//        m_aircraft->fwStatusLabel->setText("WARNING: check channel assignment");
//        return false;
//    }
//
//    resetActuators();
//    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
//    Q_ASSERT(obj);
//
//    // Elevons
//    UAVObjectField *field = obj->getField("FixedWingPitch1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwElevator1ChannelBox->currentText());
//    field = obj->getField("FixedWingPitch2");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwElevator2ChannelBox->currentText());
//    field = obj->getField("FixedWingRoll1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron1ChannelBox->currentText());
//    field = obj->getField("FixedWingRoll2");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron2ChannelBox->currentText());
//
//    // Throttle
//    field = obj->getField("FixedWingThrottle");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwEngineChannelBox->currentText());
//
//    obj->updated();
//
//    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//    Q_ASSERT(obj);
//    // ... and compute the matrix:
//    // In order to make code a bit nicer, we assume:
//    // - ChannelBox dropdowns start with 'None', then 0 to 7
//
//    // 1. Assign the servo/motor/none for each channel
//    // Disable all
//    foreach(QString mixer, mixerTypes) {
//        field = obj->getField(mixer);
//        Q_ASSERT(field);
//        field->setValue("Disabled");
//    }
//    // and set only the relevant channels:
//    // Engine
//    int tmpVal = m_aircraft->fwEngineChannelBox->currentIndex()-1;
//    field = obj->getField(mixerTypes.at(tmpVal));
//    field->setValue("Motor");
//    field = obj->getField(mixerVectors.at(tmpVal));
//    // First of all reset the vector
//    resetField(field);
//    int ti = field->getElementNames().indexOf("ThrottleCurve1");
//    field->setValue(127, ti);
//
//    tmpVal = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Roll");
//        field->setValue(127,ti);
//    }
//
//    tmpVal = m_aircraft->fwAileron2ChannelBox->currentIndex()-1;
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Roll");
//        field->setValue(-127,ti);
//    }
//
//    // Now compute the VTail
//    tmpVal = m_aircraft->fwElevator1ChannelBox->currentIndex()-1;
//    field = obj->getField(mixerTypes.at(tmpVal));
//    field->setValue("Servo");
//    field = obj->getField(mixerVectors.at(tmpVal));
//    resetField(field);
//    ti = field->getElementNames().indexOf("Pitch");
//    field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
//    ti = field->getElementNames().indexOf("Yaw");
//    field->setValue((double)m_aircraft->elevonSlider1->value()*1.27,ti);
//
//    tmpVal = m_aircraft->fwElevator2ChannelBox->currentIndex()-1;
//    field = obj->getField(mixerTypes.at(tmpVal));
//    field->setValue("Servo");
//    field = obj->getField(mixerVectors.at(tmpVal));
//    resetField(field);
//    ti = field->getElementNames().indexOf("Pitch");
//    field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
//    ti = field->getElementNames().indexOf("Yaw");
//    field->setValue(-(double)m_aircraft->elevonSlider1->value()*1.27,ti);
//
//    obj->updated();
//    m_aircraft->fwStatusLabel->setText("Mixer generated");
//    return true;
//}


///**
// Setup steerable ground vehicle.
// 
// If both Aileron channels are set to 'None' (EasyStar), do Pitch/Rudder mixing
// 
// Returns False if impossible to create the mixer.
// */
//bool ConfigVehicleTypeWidget::setupGroundVehicle()
//{
//    // Check coherence:
//    // - At least Pitch and either Roll or Yaw
//    if ((m_aircraft->gvMotor1ChannelBox->currentText() == "None" &&
//		m_aircraft->gvMotor2ChannelBox->currentText() == "None") ||
//        (m_aircraft->gvSteering1ChannelBox->currentText() == "None" &&
//        m_aircraft->gvSteering2ChannelBox->currentText() == "None")) {
//			// TODO: explain the problem in the UI
//			m_aircraft->gvStatusLabel->setText("<font color='red'>ERROR: check channel assignments</font>");
//			if(m_aircraft->gvMotor1ChannelBox->currentText() == "None" && m_aircraft->gvMotor2ChannelBox->currentText() == "None"){
//				m_aircraft->gvStatusLabel->setText("<font color='red'>ERROR: check motor channel assignment</font>");
//				m_aircraft->gvMotor1Label->setText("<font color='red'>" + m_aircraft->gvMotor1Label->text() + "</font>");
//				m_aircraft->gvMotor2Label->setText("<font color='red'>" + m_aircraft->gvMotor2Label->text() + "</font>");
//				
//			}
//			else{
//				QTextEdit* htmlText=new QTextEdit(m_aircraft->gvMotor1Label->text());  // HtmlText is any QString with html tags.
//				m_aircraft->gvMotor1Label->setText(htmlText->toPlainText());
//				delete htmlText;
//				
//				htmlText=new QTextEdit(m_aircraft->gvMotor2Label->text());  // HtmlText is any QString with html tags.
//				m_aircraft->gvMotor2Label->setText(htmlText->toPlainText());
//			}
//			
//			if (m_aircraft->gvSteering1ChannelBox->currentText() == "None" && m_aircraft->gvSteering2ChannelBox->currentText() == "None") {
//				m_aircraft->gvStatusLabel->setText("<font color='red'>ERROR: check steering channel assignment</font>");
//				m_aircraft->gvSteering1Label->setText("<font color='red'>" + m_aircraft->gvSteering1Label->text() + "</font>");
//				m_aircraft->gvSteering2Label->setText("<font color='red'>" + m_aircraft->gvSteering2Label->text() + "</font>");
//			}
//			else{
//				QTextEdit* htmlText=new QTextEdit(m_aircraft->gvSteering1Label->text());  // HtmlText is any QString with html tags.
//				m_aircraft->gvSteering1Label->setText(htmlText->toPlainText());
//				delete htmlText;
//				
//				htmlText=new QTextEdit(m_aircraft->gvSteering2Label->text());  // HtmlText is any QString with html tags.
//				m_aircraft->gvSteering2Label->setText(htmlText->toPlainText());
//			}
//			return false;
//		}
//	else{
////		m_aircraft->gvStatusLabel->setText("Mixer generated");
//		QTextEdit* htmlText=new QTextEdit(m_aircraft->gvSteering1Label->text());  // HtmlText is any QString with html tags.
//		m_aircraft->gvSteering1Label->setText(htmlText->toPlainText());
//		delete htmlText;
//		
//		htmlText=new QTextEdit(m_aircraft->gvSteering2Label->text());  // HtmlText is any QString with html tags.
//		m_aircraft->gvSteering2Label->setText(htmlText->toPlainText());
//		delete htmlText;
//
//		htmlText=new QTextEdit(m_aircraft->gvMotor1Label->text());  // HtmlText is any QString with html tags.
//		m_aircraft->gvMotor1Label->setText(htmlText->toPlainText());
//		delete htmlText;
//		
//		htmlText=new QTextEdit(m_aircraft->gvMotor2Label->text());  // HtmlText is any QString with html tags.
//		m_aircraft->gvMotor2Label->setText(htmlText->toPlainText());
//		
//		
//	
//		
//	}
//	
////#if 0	
//	// Now setup the channels:
//    resetActuators();
//	
//    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
//    Q_ASSERT(obj);
//	
//    // Elevator
//    UAVObjectField *field = obj->getField("FixedWingPitch1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->gvMotor1ChannelBox->currentText());
//    field = obj->getField("FixedWingPitch2");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->gvMotor2ChannelBox->currentText());
//    // Aileron
//    field = obj->getField("FixedWingRoll1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron1ChannelBox->currentText());
//    field = obj->getField("FixedWingRoll2");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron2ChannelBox->currentText());
//    // Rudder
//    field = obj->getField("FixedWingYaw1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->gvSteering1ChannelBox->currentText());
//    // Throttle
//    field = obj->getField("FixedWingThrottle");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->gvSteering2ChannelBox->currentText());
//	
//    obj->updated();
//	
//    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//    Q_ASSERT(obj);
//    // ... and compute the matrix:
//    // In order to make code a bit nicer, we assume:
//    // - ChannelBox dropdowns start with 'None', then 0 to 7
//	
//    // 1. Assign the servo/motor/none for each channel
//    // Disable all
//    foreach(QString mixer, mixerTypes) {
//        field = obj->getField(mixer);
//        Q_ASSERT(field);
//        field->setValue("Disabled");
//    }
//	
//	int tmpVal, ti;
//#if 0	
//    // and set only the relevant channels:
//    // Engine
//    tmpVal = m_aircraft->fwEngineChannelBox->currentIndex()-1;
//    field = obj->getField(mixerTypes.at(tmpVal));
//    field->setValue("Motor");
//    field = obj->getField(mixerVectors.at(tmpVal));
//    // First of all reset the vector
//    resetField(field);
//    ti = field->getElementNames().indexOf("ThrottleCurve1");
//    field->setValue(127, ti);
//#endif	
//    // Steering
//    tmpVal = m_aircraft->gvSteering1ChannelBox->currentIndex()-1;
//    // tmpVal will be -1 if steering is set to "None"
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Yaw");
//        field->setValue(127, ti);
//    } // Else: we have no rudder, only ailerons, we're fine with it.
//
//#if 0	
//    // Ailerons
//    tmpVal = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("Roll");
//        field->setValue(127, ti);
//        // Only set Aileron 2 if Aileron 1 is defined
//        tmpVal = m_aircraft->fwAileron2ChannelBox->currentIndex()-1;
//        if (tmpVal > -1) {
//            field = obj->getField(mixerTypes.at(tmpVal));
//            field->setValue("Servo");
//            field = obj->getField(mixerVectors.at(tmpVal));
//            resetField(field);
//            ti = field->getElementNames().indexOf("Roll");
//            field->setValue(127, ti);
//        }
//    } // Else we have no ailerons. Our consistency check guarantees we have
//	// rudder in this case, so we're fine with it too.
//#endif	
//	
//    // Motor
//    tmpVal = m_aircraft->gvMotor1ChannelBox->currentIndex()-1;
//    field = obj->getField(mixerTypes.at(tmpVal));
//    field->setValue("Servo");
//    field = obj->getField(mixerVectors.at(tmpVal));
//    resetField(field);
//    ti = field->getElementNames().indexOf("ThrottleCurve1");
//    field->setValue(127, ti);
//	
//    // Only set Motor 2 if it is defined
//    tmpVal = m_aircraft->gvMotor2ChannelBox->currentIndex()-1;
//    if (tmpVal > -1) {
//        field = obj->getField(mixerTypes.at(tmpVal));
//        field->setValue("Servo");
//        field = obj->getField(mixerVectors.at(tmpVal));
//        resetField(field);
//        ti = field->getElementNames().indexOf("ThrottleCurve2");
//        field->setValue(127, ti);
//    }
//	
//    obj->updated();
////#endif	
//    m_aircraft->gvStatusLabel->setText("Mixer generated");
//	
//    return true;
//}


/**
  Set up a complete mixer, taking a table of factors. The factors
  shoudl mainly be +/- 1 factors, since they will be weighted by the
  value of the Pitch/Roll/Yaw sliders.

  Example:
    double xMixer [8][3] =  {
           P   R   Y
         { 1,  1, -1},  Motor 1
         { 1, -1,  1},  Motor 2
         {-1, -1, -1},  Motor 3
         {-1,  1,  1},  ...
         { 0,  0,  0},
         { 0,  0,  0},
         { 0,  0,  0},
         { 0,  0,  0}
     };
  */
//bool ConfigVehicleTypeWidget::setupMultiRotorMixer(double mixerFactors[8][3])
//{
//    qDebug()<<"Mixer factors";
//    qDebug()<<mixerFactors[0][0]<<" "<<mixerFactors[0][1]<<" "<<mixerFactors[0][2];
//    qDebug()<<mixerFactors[1][0]<<" "<<mixerFactors[1][1]<<" "<<mixerFactors[1][2];
//    qDebug()<<mixerFactors[2][0]<<" "<<mixerFactors[2][1]<<" "<<mixerFactors[2][2];
//    qDebug()<<mixerFactors[3][0]<<" "<<mixerFactors[3][1]<<" "<<mixerFactors[3][2];
//    qDebug()<<mixerFactors[4][0]<<" "<<mixerFactors[4][1]<<" "<<mixerFactors[4][2];
//    qDebug()<<mixerFactors[5][0]<<" "<<mixerFactors[5][1]<<" "<<mixerFactors[5][2];
//    qDebug()<<mixerFactors[6][0]<<" "<<mixerFactors[6][1]<<" "<<mixerFactors[6][2];
//    qDebug()<<mixerFactors[7][0]<<" "<<mixerFactors[7][1]<<" "<<mixerFactors[7][2];
//
//    UAVObjectField *field;
//    QList<QComboBox*> mmList;
//    mmList << m_aircraft->multiMotorChannelBox1 << m_aircraft->multiMotorChannelBox2 << m_aircraft->multiMotorChannelBox3
//            << m_aircraft->multiMotorChannelBox4 << m_aircraft->multiMotorChannelBox5 << m_aircraft->multiMotorChannelBox6
//            << m_aircraft->multiMotorChannelBox7 << m_aircraft->multiMotorChannelBox8;
//    UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//    // 1. Assign the servo/motor/none for each channel
//    // Disable all
//    foreach(QString mixer, mixerTypes) {
//        field = obj->getField(mixer);
//        Q_ASSERT(field);
//        field->setValue("Disabled");
//    }
//    // and enable only the relevant channels:
//    double pFactor = (double)m_aircraft->mrPitchMixLevel->value()/100;
//    double rFactor = (double)m_aircraft->mrRollMixLevel->value()/100;
//    double yFactor = (double)m_aircraft->mrYawMixLevel->value()/100;
//    qDebug()<<QString("pFactor=%0 rFactor=%1 yFactor=%2").arg(pFactor).arg(rFactor).arg(yFactor);
//    for (int i=0 ; i<8; i++) {
//        if(mmList.at(i)->isEnabled())
//        {
//            int channel = mmList.at(i)->currentIndex()-1;
//            if (channel > -1)
//                setupQuadMotor(channel, mixerFactors[i][0]*pFactor,
//                           rFactor*mixerFactors[i][1], yFactor*mixerFactors[i][2]);
//        }
//    }
////    obj->updated();
//    return true;
//}


///**
//  Helper function: setupQuadMotor
//  */
//void ConfigVehicleTypeWidget::setupQuadMotor(int channel, double pitch, double roll, double yaw)
//{
//    qDebug()<<QString("Setup quad motor channel=%0 pitch=%1 roll=%2 yaw=%3").arg(channel).arg(pitch).arg(roll).arg(yaw);
//    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//    Q_ASSERT(obj);
//    UAVObjectField *field = obj->getField(mixerTypes.at(channel));
//    field->setValue("Motor");
//    field = obj->getField(mixerVectors.at(channel));
//    // First of all reset the vector
//    resetField(field);
//    int ti = field->getElementNames().indexOf("ThrottleCurve1");
//    field->setValue(127, ti);
//    ti = field->getElementNames().indexOf("Roll");
//    field->setValue(roll*127,ti);
//    qDebug()<<"Set roll="<<roll*127;
//    ti = field->getElementNames().indexOf("Pitch");
//    field->setValue(pitch*127,ti);
//    qDebug()<<"Set pitch="<<pitch*127;
//    ti = field->getElementNames().indexOf("Yaw");
//    field->setValue(yaw*127,ti);
//    qDebug()<<"Set yaw="<<yaw*127;
//}

///**
//  Helper function: setup motors. Takes a list of channel names in input.
//  */
//void ConfigVehicleTypeWidget::setupMotors(QList<QString> motorList)
//{
//    resetActuators();
//    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
//    Q_ASSERT(obj);
//    UAVObjectField *field;
//    QList<QComboBox*> mmList;
//    mmList << m_aircraft->multiMotor1 << m_aircraft->multiMotor2 << m_aircraft->multiMotor3
//            << m_aircraft->multiMotor4 << m_aircraft->multiMotor5 << m_aircraft->multiMotor6
//            << m_aircraft->multiMotor7 << m_aircraft->multiMotor8;
//    foreach (QString motor, motorList) {
//        field = obj->getField(motor);
//        field->setValue(mmList.takeFirst()->currentText());
//    }
//    //obj->updated(); // Save...
//}


///**
//  Set up a Quad-X or Quad-P
//*/
//bool ConfigVehicleTypeWidget::setupQuad(bool pLayout)
//{
//    // Check coherence:
//    // - Four engines have to be defined
//    if (m_aircraft->multiMotor1->currentText() == "None" ||
//        m_aircraft->multiMotor2->currentText() == "None" ||
//        m_aircraft->multiMotor3->currentText() == "None" ||
//        m_aircraft->multiMotor4->currentText() == "None") {
//        m_aircraft->mrStatusLabel->setText("ERROR: Assign 4 motor channels");
//        return false;
//    }
//
//
//    QList<QString> motorList;
//    if (pLayout) {
//        motorList << "VTOLMotorN" << "VTOLMotorE" << "VTOLMotorS"
//                << "VTOLMotorW";
//    } else {
//        motorList << "VTOLMotorNW" << "VTOLMotorNE" << "VTOLMotorSE"
//                << "VTOLMotorSW";
//    }
//    setupMotors(motorList);
//
//    // Now, setup the mixer:
//    // Motor 1 to 4, X Layout:
//    //     pitch   roll    yaw
//    //    {0.5    ,0.5    ,-0.5     //Front left motor (CW)
//    //    {0.5    ,-0.5   ,0.5   //Front right motor(CCW)
//    //    {-0.5  ,-0.5    ,-0.5    //rear right motor (CW)
//    //    {-0.5   ,0.5    ,0.5   //Rear left motor  (CCW)
//    double xMixer [8][3] =  {
//         { 1,  1, -1},
//         { 1, -1,  1},
//         {-1, -1, -1},
//         {-1,  1,  1},
//         { 0,  0,  0},
//         { 0,  0,  0},
//         { 0,  0,  0},
//         { 0,  0,  0}
//     };
//    //
//    // Motor 1 to 4, P Layout:
//    // pitch   roll    yaw
//    //  {1      ,0      ,-0.5    //Front motor (CW)
//    //  {0      ,-1     ,0.5   //Right  motor(CCW)
//    //  {-1     ,0      ,-0.5    //Rear motor  (CW)
//    //  {0      ,1      ,0.5   //Left motor  (CCW)
//    double pMixer [8][3] =  {
//         { 1,  0, -1},
//         { 0, -1,  1},
//         {-1,  0, -1},
//         { 0,  1,  1},
//         { 0,  0,  0},
//         { 0,  0,  0},
//         { 0,  0,  0},
//         { 0,  0,  0}
//     };
//
//    if (pLayout) {
//        setupMultiRotorMixer(pMixer);
//    } else {
//        setupMultiRotorMixer(xMixer);
//    }
//    m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
//    return true;
//}
//
//
//
///**
//  Set up a Hexa-X or Hexa-P
//*/
//bool ConfigVehicleTypeWidget::setupHexa(bool pLayout)
//{
//    // Check coherence:
//    // - Four engines have to be defined
//    if (m_aircraft->multiMotor1->currentText() == "None" ||
//        m_aircraft->multiMotor2->currentText() == "None" ||
//        m_aircraft->multiMotor3->currentText() == "None" ||
//        m_aircraft->multiMotor4->currentText() == "None" ||
//        m_aircraft->multiMotor5->currentText() == "None" ||
//        m_aircraft->multiMotor6->currentText() == "None") {
//        m_aircraft->mrStatusLabel->setText("ERROR: Assign 6 motor channels");
//        return false;
//    }
//
//    QList<QString> motorList;
//    if (pLayout) {
//        motorList << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorSE"
//                << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorNW";
//    } else {
//        motorList << "VTOLMotorNE" << "VTOLMotorE" << "VTOLMotorSE"
//                << "VTOLMotorSW" << "VTOLMotorW" << "VTOLMotorNW";
//    }
//    setupMotors(motorList);
//
//    // and set only the relevant channels:
//
//    // Motor 1 to 6, P Layout:
//    //     pitch   roll    yaw
//    //  1 { 0.3  , 0      ,-0.3 // N   CW
//    //  2 { 0.3  ,-0.5    , 0.3 // NE CCW
//    //  3 {-0.3  ,-0.5    ,-0.3 // SE  CW
//    //  4 {-0.3  , 0      , 0.3 // S  CCW
//    //  5 {-0.3  , 0.5    ,-0.3 // SW  CW
//    //  6 { 0.3  , 0.5    , 0.3 // NW CCW
//
//   double pMixer [8][3] =  {
//        { 1,  0, -1},
//        { 1, -1,  1},
//        {-1, -1, -1},
//        {-1,  0,  1},
//        {-1,  1, -1},
//        { 1,  1,  1},
//        { 0,  0,  0},
//        { 0,  0,  0}
//    };
//
//   //
//    // Motor 1 to 6, X Layout:
//    // 1 [  0.5, -0.3, -0.3 ] NE
//    // 2 [  0  , -0.3,  0.3 ] E
//    // 3 [ -0.5, -0.3, -0.3 ] SE
//    // 4 [ -0.5,  0.3,  0.3 ] SW
//    // 5 [  0  ,  0.3, -0.3 ] W
//    // 6 [  0.5,  0.3,  0.3 ] NW
//   double xMixer [8][3] = {
//       {  1, -1, -1},
//       {  0, -1,  1},
//       { -1, -1, -1},
//       { -1,  1,  1},
//       {  0,  1, -1},
//       {  1,  1,  1},
//       {  0,  0,  0},
//       {  0,  0,  0}
//   };
//
//   if (pLayout) {
//       setupMultiRotorMixer(pMixer);
//   } else {
//       setupMultiRotorMixer(xMixer);
//   }
//   m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
//   return true;
//}



/**
  Updates the custom airframe settings based on the current airframe.

  Note: does NOT ask for an object refresh itself!
  */
void ConfigVehicleTypeWidget::updateCustomAirframeUI()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve1"));
    QList<double> curveValues;
    // If the 1st element of the curve is <= -10, then the curve
    // is a straight line (that's how the mixer works on the mainboard):
    if (field->getValue(0).toInt() <= -10) {
        m_aircraft->customThrottle1Curve->initLinearCurve(field->getNumElements(),(double)1);
    } else {
        double temp=0;
        double value;
        for (unsigned int i=0; i < field->getNumElements(); i++) {
            value=field->getValue(i).toDouble();
            temp+=value;
            curveValues.append(value);
        }
        if(temp==0)
            m_aircraft->customThrottle1Curve->initLinearCurve(field->getNumElements(),(double)1);
        else
            m_aircraft->customThrottle1Curve->initCurve(curveValues);
    }
	
    field = obj->getField(QString("ThrottleCurve2"));
    curveValues.clear();
    // If the 1st element of the curve is <= -10, then the curve
    // is a straight line (that's how the mixer works on the mainboard):
    if (field->getValue(0).toInt() <= -10) {
        m_aircraft->customThrottle2Curve->initLinearCurve(field->getNumElements(),(double)1);
    } else {
        for (unsigned int i=0; i < field->getNumElements(); i++) {
            curveValues.append(field->getValue(i).toDouble());
        }
        m_aircraft->customThrottle2Curve->initCurve(curveValues);
    }

    // Update the table:
    for (int i=0; i<8; i++) {
        field = obj->getField(mixerTypes.at(i));
        QComboBox* q = (QComboBox*)m_aircraft->customMixerTable->cellWidget(0,i);
        QString s = field->getValue().toString();
        q->setCurrentIndex(q->findText(s));
        //bool en = (s != "Disabled");
        field = obj->getField(mixerVectors.at(i));
        int ti = field->getElementNames().indexOf("ThrottleCurve1");
        m_aircraft->customMixerTable->item(1,i)->setText(field->getValue(ti).toString());
        ti = field->getElementNames().indexOf("ThrottleCurve2");
        m_aircraft->customMixerTable->item(2,i)->setText(field->getValue(ti).toString());
        ti = field->getElementNames().indexOf("Roll");
        m_aircraft->customMixerTable->item(3,i)->setText(field->getValue(ti).toString());
        ti = field->getElementNames().indexOf("Pitch");
        m_aircraft->customMixerTable->item(4,i)->setText(field->getValue(ti).toString());
        ti = field->getElementNames().indexOf("Yaw");
        m_aircraft->customMixerTable->item(5,i)->setText(field->getValue(ti).toString());
    }
}


/**
  Sends the config to the board (airframe type)

  We do all the tasks common to all airframes, or family of airframes, and
  we call additional methods for specific frames, so that we do not have a code
  that is too heavy.
*/
void ConfigVehicleTypeWidget::updateObjectsFromWidgets()
{
    qDebug()<<"updateObjectsFromWidgets";
    QString airframeType = "Custom"; //Sets airframe type default to "Custom"
    if (m_aircraft->aircraftType->currentText() == "Fixed Wing") {
		if(1){
			airframeType = updateFixedWingObjectsFromWidgets();
		}
		else{
		//        // Save the curve (common to all Fixed wing frames)
//        UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//        // Remove Feed Forward, it is pointless on a plane:
//        UAVObjectField* field = obj->getField(QString("FeedForward"));
//        field->setDouble(0);
//        field = obj->getField("ThrottleCurve1");
//        QList<double> curve = m_aircraft->fixedWingThrottle->getCurve();
//        for (int i=0;i<curve.length();i++) {
//            field->setValue(curve.at(i),i);
//        }
//
//        if (m_aircraft->fixedWingType->currentText() == "Elevator aileron rudder" ) {
//            airframeType = "FixedWing";
//            setupFrameFixedWing();
//        } else if (m_aircraft->fixedWingType->currentText() == "Elevon") {
//            airframeType = "FixedWingElevon";
//            setupFrameElevon();
//        } else { // "Vtail"
//            airframeType = "FixedWingVtail";
//            setupFrameVtail();
//        }
//        // Now reflect those settings in the "Custom" panel as well
//        updateCustomAirframeUI();
		}
     } else if (m_aircraft->aircraftType->currentText() == "Multirotor") {
		 
		 //update the mixer
		 if(1){
		 airframeType = updateMultiRotorObjectsFromWidgets();
		 }
		 else{
//		 
//         QList<QString> motorList;
//
//        // We can already setup the feedforward here, as it is common to all platforms
//        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//        UAVObjectField* field = obj->getField(QString("FeedForward"));
//        field->setDouble((double)m_aircraft->feedForwardSlider->value()/100);
//        field = obj->getField(QString("AccelTime"));
//        field->setDouble(m_aircraft->accelTime->value());
//        field = obj->getField(QString("DecelTime"));
//        field->setDouble(m_aircraft->decelTime->value());
//        field = obj->getField(QString("MaxAccel"));
//        field->setDouble(m_aircraft->maxAccelSlider->value());
//
//        // Curve is also common to all quads:
//        field = obj->getField("ThrottleCurve1");
//        QList<double> curve = m_aircraft->multiThrottleCurve->getCurve();
//        for (int i=0;i<curve.length();i++) {
//            field->setValue(curve.at(i),i);
//        }
//
//        if (m_aircraft->multirotorFrameType->currentText() == "Quad +") {
//            airframeType = "QuadP";
//            setupQuad(true);
//        } else if (m_aircraft->multirotorFrameType->currentText() == "Quad X") {
//            airframeType = "QuadX";
//            setupQuad(false);
//        } else if (m_aircraft->multirotorFrameType->currentText() == "Hexacopter") {
//            airframeType = "Hexa";
//            setupHexa(true);
//        } else if (m_aircraft->multirotorFrameType->currentText() == "Octocopter") {
//            airframeType = "Octo";
//            if (m_aircraft->multiMotor1->currentText() == "None" ||
//                m_aircraft->multiMotor2->currentText() == "None" ||
//                m_aircraft->multiMotor3->currentText() == "None" ||
//                m_aircraft->multiMotor4->currentText() == "None" ||
//                m_aircraft->multiMotor5->currentText() == "None" ||
//                m_aircraft->multiMotor6->currentText() == "None" ||
//                m_aircraft->multiMotor7->currentText() == "None" ||
//                m_aircraft->multiMotor8->currentText() == "None") {
//                m_aircraft->mrStatusLabel->setText("ERROR: Assign 8 motor channels");
//                return;
//            }
//            motorList << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorE" << "VTOLMotorSE"
//                    << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorW" << "VTOLMotorNW";
//            setupMotors(motorList);
//            // Motor 1 to 8:
//            //     pitch   roll    yaw
//           double mixer [8][3] = {
//               {  1,  0, -1},
//               {  1, -1,  1},
//               {  0, -1, -1},
//               { -1, -1,  1},
//               { -1,  0, -1},
//               { -1,  1,  1},
//               {  0,  1, -1},
//               {  1,  1,  1}
//           };
//           setupMultiRotorMixer(mixer);
//           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
//
//        } else if (m_aircraft->multirotorFrameType->currentText() == "Hexacopter X") {
//            airframeType = "HexaX";
//            setupHexa(false);
//        } else if (m_aircraft->multirotorFrameType->currentText() == "Octocopter V") {
//            airframeType = "OctoV";
//            if (m_aircraft->multiMotor1->currentText() == "None" ||
//                m_aircraft->multiMotor2->currentText() == "None" ||
//                m_aircraft->multiMotor3->currentText() == "None" ||
//                m_aircraft->multiMotor4->currentText() == "None" ||
//                m_aircraft->multiMotor5->currentText() == "None" ||
//                m_aircraft->multiMotor6->currentText() == "None" ||
//                m_aircraft->multiMotor7->currentText() == "None" ||
//                m_aircraft->multiMotor8->currentText() == "None") {
//                m_aircraft->mrStatusLabel->setText("ERROR: Assign 8 motor channels");
//                return;
//            }
//            motorList << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorE" << "VTOLMotorSE"
//                    << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorW" << "VTOLMotorNW";
//            setupMotors(motorList);
//            // Motor 1 to 8:
//            // IMPORTANT: Assumes evenly spaced engines
//            //     pitch   roll    yaw
//           double mixer [8][3] = {
//               {  0.33, -1, -1},
//               {  1   , -1,  1},
//               { -1   , -1, -1},
//               { -0.33, -1,  1},
//               { -0.33,  1, -1},
//               { -1   ,  1,  1},
//               {  1   ,  1, -1},
//               {  0.33,  1,  1}
//           };
//           setupMultiRotorMixer(mixer);
//           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
//
//        } else if (m_aircraft->multirotorFrameType->currentText() == "Octo Coax +") {
//            airframeType = "OctoCoaxP";
//            if (m_aircraft->multiMotor1->currentText() == "None" ||
//                m_aircraft->multiMotor2->currentText() == "None" ||
//                m_aircraft->multiMotor3->currentText() == "None" ||
//                m_aircraft->multiMotor4->currentText() == "None" ||
//                m_aircraft->multiMotor5->currentText() == "None" ||
//                m_aircraft->multiMotor6->currentText() == "None" ||
//                m_aircraft->multiMotor7->currentText() == "None" ||
//                m_aircraft->multiMotor8->currentText() == "None") {
//                m_aircraft->mrStatusLabel->setText("ERROR: Assign 8 motor channels");
//                return;
//            }
//            motorList << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorE" << "VTOLMotorSE"
//                    << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorW" << "VTOLMotorNW";
//            setupMotors(motorList);
//            // Motor 1 to 8:
//            //     pitch   roll    yaw
//           double mixer [8][3] = {
//               {  1,  0, -1},
//               {  1,  0,  1},
//               {  0, -1, -1},
//               {  0, -1,  1},
//               { -1,  0, -1},
//               { -1,  0,  1},
//               {  0,  1, -1},
//               {  0,  1,  1}
//           };
//           setupMultiRotorMixer(mixer);
//           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
//
//        } else if (m_aircraft->multirotorFrameType->currentText() == "Octo Coax X") {
//            airframeType = "OctoCoaxX";
//            if (m_aircraft->multiMotor1->currentText() == "None" ||
//                m_aircraft->multiMotor2->currentText() == "None" ||
//                m_aircraft->multiMotor3->currentText() == "None" ||
//                m_aircraft->multiMotor4->currentText() == "None" ||
//                m_aircraft->multiMotor5->currentText() == "None" ||
//                m_aircraft->multiMotor6->currentText() == "None" ||
//                m_aircraft->multiMotor7->currentText() == "None" ||
//                m_aircraft->multiMotor8->currentText() == "None") {
//                m_aircraft->mrStatusLabel->setText("ERROR: Assign 8 motor channels");
//                return;
//            }
//            motorList << "VTOLMotorNW" << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorE"
//                    << "VTOLMotorSE" << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorW";
//            setupMotors(motorList);
//            // Motor 1 to 8:
//            //     pitch   roll    yaw
//           double mixer [8][3] = {
//               {  1,  1, -1},
//               {  1,  1,  1},
//               {  1, -1, -1},
//               {  1, -1,  1},
//               { -1, -1, -1},
//               { -1, -1,  1},
//               { -1,  1, -1},
//               { -1,  1,  1}
//           };
//           setupMultiRotorMixer(mixer);
//           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
//
//        } else if (m_aircraft->multirotorFrameType->currentText() == "Hexacopter Y6") {
//            airframeType = "HexaCoax";
//            if (m_aircraft->multiMotor1->currentText() == "None" ||
//                m_aircraft->multiMotor2->currentText() == "None" ||
//                m_aircraft->multiMotor3->currentText() == "None" ||
//                m_aircraft->multiMotor4->currentText() == "None" ||
//                m_aircraft->multiMotor5->currentText() == "None" ||
//                m_aircraft->multiMotor6->currentText() == "None" ) {
//                m_aircraft->mrStatusLabel->setText("ERROR: Assign 6 motor channels");
//                return;
//            }
//            motorList << "VTOLMotorNW" << "VTOLMotorW" << "VTOLMotorNE" << "VTOLMotorE"
//                    << "VTOLMotorS" << "VTOLMotorSE";
//            setupMotors(motorList);
//
//            // Motor 1 to 6, Y6 Layout:
//            //     pitch   roll    yaw
//           double mixer [8][3] = {
//               {  0.5,  1, -1},
//               {  0.5,  1,  1},
//               {  0.5, -1, -1},
//               {  0.5, -1,  1},
//               { -1,    0, -1},
//               { -1,    0,  1},
//               {  0,    0,  0},
//               {  0,    0,  0}
//           };
//           setupMultiRotorMixer(mixer);
//           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
//
//        } else if (m_aircraft->multirotorFrameType->currentText() == "Tricopter Y") {
//            airframeType = "Tri";
//            if (m_aircraft->multiMotor1->currentText() == "None" ||
//                m_aircraft->multiMotor2->currentText() == "None" ||
//                m_aircraft->multiMotor3->currentText() == "None" ) {
//                m_aircraft->mrStatusLabel->setText("ERROR: Assign 3 motor channels");
//                return;
//            }
//            if (m_aircraft->triYawChannelBoxBox->currentText() == "None") {
//                m_aircraft->mrStatusLabel->setText("Error: Assign a Yaw channel");
//                return;
//            }
//            motorList << "VTOLMotorNW" << "VTOLMotorNE" << "VTOLMotorS";
//            setupMotors(motorList);
//            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
//            Q_ASSERT(obj);
//            field = obj->getField("FixedWingYaw1");
//            field->setValue(m_aircraft->triYawChannelBoxBox->currentText());
//
//            // Motor 1 to 6, Y6 Layout:
//            //     pitch   roll    yaw
//           double mixer [8][3] = {
//               {  0.5,  1,  0},
//               {  0.5, -1,  0},
//               { -1,  0,  0},
//               {  0,  0,  0},
//               {  0,  0,  0},
//               {  0,  0,  0},
//               {  0,  0,  0},
//               {  0,  0,  0}
//           };
//           setupMultiRotorMixer(mixer);
//
//           int tmpVal = m_aircraft->triYawChannelBoxBox->currentIndex()-1;
//           obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//           field = obj->getField(mixerTypes.at(tmpVal));
//           field->setValue("Servo");
//           field = obj->getField(mixerVectors.at(tmpVal));
//           resetField(field);
//           int ti = field->getElementNames().indexOf("Yaw");
//           field->setValue(127,ti);
//
//           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
//
//        }
//        // Now reflect those settings in the "Custom" panel as well
//        updateCustomAirframeUI();
		 }

	 } else  if (m_aircraft->aircraftType->currentText() == "Helicopter") {
		 airframeType = "HeliCP";
		 m_aircraft->widget_3->sendccpmUpdate();
	 } else  if (m_aircraft->aircraftType->currentText() == "Ground") {
		 if(1){
			 airframeType = updateGroundVehicleObjectsFromWidgets();
		 }
		 else{
//		 airframeType = "Ground";
//		 // Save the curve (common to all ground vehicle frames)
//		 UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//		 UAVObjectField* field;
//		 
//		 if (0){ //DON'T REMOVE FEEDFORWARD
//			 // Remove Feed Forward, it is pointless on a plane:
//			 field = obj->getField(QString("FeedForward"));
//			 field->setDouble(0);
//		 }
//		 
//		 field = obj->getField("ThrottleCurve1");
//		 QList<double> curve = m_aircraft->groundVehicleThrottle->getCurve();
//		 for (int i=0;i<curve.length();i++) {
//			 field->setValue(curve.at(i),i);
//		 }
//		 
//		 if (m_aircraft->groundVehicleType->currentText() == "Turnable (car)" ) {
//			 airframeType = "GroundVehicleCar";
//			 setupGroundVehicle();
//		 } else if (m_aircraft->groundVehicleType->currentText() == "Differential (tank)") {
//			 airframeType = "GroundVehicleDifferential";
//			 setupGroundVehicle();
//		 } else { // "Motorcycle"
//			 airframeType = "GroundVehicleMotorcycle";
//			 setupGroundVehicle();
//		 }
//		 
//		 
//		 // Now reflect those settings in the "Custom" panel as well
//		 updateCustomAirframeUI();
//		 
////		 m_aircraft->widget_3->sendccpmUpdate();
		 }
	 } else {
        airframeType = "Custom";

        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
        UAVObjectField* field = obj->getField(QString("FeedForward"));

        // Curve is also common to all quads:
        field = obj->getField("ThrottleCurve1");
        QList<double> curve = m_aircraft->customThrottle1Curve->getCurve();
        for (int i=0;i<curve.length();i++) {
            field->setValue(curve.at(i),i);
        }

        field = obj->getField("ThrottleCurve2");
        curve.clear();
        curve = m_aircraft->customThrottle2Curve->getCurve();
        for (int i=0;i<curve.length();i++) {
            field->setValue(curve.at(i),i);
        }

        // Update the table:
        for (int i=0; i<8; i++) {
            field = obj->getField(mixerTypes.at(i));
            QComboBox* q = (QComboBox*)m_aircraft->customMixerTable->cellWidget(0,i);
            field->setValue(q->currentText());
            field = obj->getField(mixerVectors.at(i));
            int ti = field->getElementNames().indexOf("ThrottleCurve1");
            field->setValue(m_aircraft->customMixerTable->item(1,i)->text(),ti);
            ti = field->getElementNames().indexOf("ThrottleCurve2");
            field->setValue(m_aircraft->customMixerTable->item(2,i)->text(),ti);
            ti = field->getElementNames().indexOf("Roll");
            field->setValue(m_aircraft->customMixerTable->item(3,i)->text(),ti);
            ti = field->getElementNames().indexOf("Pitch");
            field->setValue(m_aircraft->customMixerTable->item(4,i)->text(),ti);
            ti = field->getElementNames().indexOf("Yaw");
            field->setValue(m_aircraft->customMixerTable->item(5,i)->text(),ti);
        }

    }
	
	//WHAT DOES THIS DO?
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("SystemSettings")));
    UAVObjectField* field = obj->getField(QString("AirframeType"));
    field->setValue(airframeType);

}

/**
 Opens the wiki from the user's default browser
 */
void ConfigVehicleTypeWidget::openHelp()
{

    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/display/Doc/Airframe+configuration", QUrl::StrictMode) );
}


/**
 WHAT DOES THIS DO???
 */
void ConfigVehicleTypeWidget::addToDirtyMonitor()
{
    addWidget(m_aircraft->customMixerTable);
    addWidget(m_aircraft->customThrottle1Curve);
    addWidget(m_aircraft->customThrottle2Curve);
    addWidget(m_aircraft->multiThrottleCurve);
    addWidget(m_aircraft->fixedWingThrottle);
    addWidget(m_aircraft->fixedWingType);
    addWidget(m_aircraft->groundVehicleThrottle1);
    addWidget(m_aircraft->groundVehicleThrottle2);
    addWidget(m_aircraft->groundVehicleType);
    addWidget(m_aircraft->feedForwardSlider);
    addWidget(m_aircraft->accelTime);
    addWidget(m_aircraft->decelTime);
    addWidget(m_aircraft->maxAccelSlider);
    addWidget(m_aircraft->multirotorFrameType);
    addWidget(m_aircraft->multiMotorChannelBox1);
    addWidget(m_aircraft->multiMotorChannelBox2);
    addWidget(m_aircraft->multiMotorChannelBox3);
    addWidget(m_aircraft->multiMotorChannelBox4);
    addWidget(m_aircraft->multiMotorChannelBox5);
    addWidget(m_aircraft->multiMotorChannelBox6);
    addWidget(m_aircraft->multiMotorChannelBox7);
    addWidget(m_aircraft->multiMotorChannelBox8);
    addWidget(m_aircraft->triYawChannelBox);
    addWidget(m_aircraft->aircraftType);
    addWidget(m_aircraft->fwEngineChannelBox);
    addWidget(m_aircraft->fwAileron1ChannelBox);
    addWidget(m_aircraft->fwAileron2ChannelBox);
    addWidget(m_aircraft->fwElevator1ChannelBox);
    addWidget(m_aircraft->fwElevator2ChannelBox);
    addWidget(m_aircraft->fwRudder1ChannelBox);
    addWidget(m_aircraft->fwRudder2ChannelBox);
    addWidget(m_aircraft->elevonSlider1);
    addWidget(m_aircraft->elevonSlider2);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmType);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmTailChannel);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmEngineChannel);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmServoWChannel);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmServoXChannel);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmServoYChannel);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmSingleServo);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmServoZChannel);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmAngleW);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmAngleX);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmCorrectionAngle);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmAngleZ);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmAngleY);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmCollectivePassthrough);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmLinkRoll);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmLinkCyclic);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmRevoSlider);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmREVOspinBox);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmCollectiveSlider);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmCollectivespinBox);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmCollectiveScale);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmCollectiveScaleBox);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmCyclicScale);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmPitchScale);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmPitchScaleBox);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmRollScale);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmRollScaleBox);
    addWidget(m_aircraft->widget_3->m_ccpm->SwashLvlPositionSlider);
    addWidget(m_aircraft->widget_3->m_ccpm->SwashLvlPositionSpinBox);
    addWidget(m_aircraft->widget_3->m_ccpm->CurveType);
    addWidget(m_aircraft->widget_3->m_ccpm->NumCurvePoints);
    addWidget(m_aircraft->widget_3->m_ccpm->CurveValue1);
    addWidget(m_aircraft->widget_3->m_ccpm->CurveValue2);
    addWidget(m_aircraft->widget_3->m_ccpm->CurveValue3);
    addWidget(m_aircraft->widget_3->m_ccpm->CurveToGenerate);
    addWidget(m_aircraft->widget_3->m_ccpm->CurveSettings);
    addWidget(m_aircraft->widget_3->m_ccpm->ThrottleCurve);
    addWidget(m_aircraft->widget_3->m_ccpm->PitchCurve);
    addWidget(m_aircraft->widget_3->m_ccpm->ccpmAdvancedSettingsTable);
}

