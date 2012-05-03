/**
 ******************************************************************************
 *
 * @file       configvehicletypewidget.cpp
 * @author     E. Lafargue, K. Sebesta & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012
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
    for (unsigned int i = 0; i < ActuatorSettings::CHANNELADDR_NUMELEM; i++) {
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
            UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READONLY);
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
		refreshFixedWingWidgetsValues(frameType);

    } else if (frameType == "Tri" || 
			   frameType == "QuadX" || frameType == "QuadP" ||
               frameType == "Hexa" || frameType == "HexaCoax" || frameType == "HexaX" || 
			   frameType == "Octo" || frameType == "OctoV" || frameType == "OctoCoaxP" || frameType == "OctoCoaxX" ) {
		
		 // Retrieve multirotor settings
		 refreshMultiRotorWidgetsValues(frameType);
    } else if (frameType == "HeliCP") {
        m_aircraft->widget_3->requestccpmUpdate();
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Helicopter"));//"Helicopter"
	} else if (frameType.startsWith("GroundVehicle")) {

		// Retrieve ground vehicle settings
		refreshGroundVehicleWidgetsValues(frameType);
		
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
    else if (frameType == "GroundVehicleCar" || frameType == "Turnable (car)" ||
			 frameType == "GroundVehicleDifferential" || frameType == "Differential (tank)" || 
			 frameType == "GroundVehicleMotorcyle" || frameType == "Motorcycle") {
		setupGroundVehicleUI(frameType);
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
		airframeType = updateFixedWingObjectsFromWidgets();
     } else if (m_aircraft->aircraftType->currentText() == "Multirotor") {		 
		 //update the mixer
		 airframeType = updateMultiRotorObjectsFromWidgets();
	 } else if (m_aircraft->aircraftType->currentText() == "Helicopter") {
		 airframeType = "HeliCP";
		 m_aircraft->widget_3->sendccpmUpdate();
	 } else if (m_aircraft->aircraftType->currentText() == "Ground") {
			 airframeType = updateGroundVehicleObjectsFromWidgets();
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

