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
#include <QEventLoop>

#include "systemsettings.h"
#include "mixersettings.h"
#include "actuatorsettings.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>


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
    
    ExtensionSystem::PluginManager *pm=ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings * settings=pm->getObject<Core::Internal::GeneralSettings>();
    if(!settings->useExpertMode())
        m_aircraft->saveAircraftToRAM->setVisible(false);

    addApplySaveButtons(m_aircraft->saveAircraftToRAM,m_aircraft->saveAircraftToSD);

    addUAVObject("SystemSettings");
    addUAVObject("MixerSettings");
    addUAVObject("ActuatorSettings");

    ffTuningInProgress = false;
    ffTuningPhase = false;

    //Generate lists of mixerTypeNames, mixerVectorNames, channelNames
    channelNames << "None";
    for (int i = 0; i < (int)ActuatorSettings::CHANNELADDR_NUMELEM; i++) {

        mixerTypes << QString("Mixer%1Type").arg(i+1);
        mixerVectors << QString("Mixer%1Vector").arg(i+1);
        channelNames << QString("Channel%1").arg(i+1);
    }

    QStringList airframeTypes;
    airframeTypes << "Fixed Wing" << "Multirotor" << "Helicopter" << "Ground" << "Custom";
    m_aircraft->aircraftType->addItems(airframeTypes);
    m_aircraft->aircraftType->setCurrentIndex(1);    //Set default vehicle to MultiRotor
    m_aircraft->airframesWidget->setCurrentIndex(1); // Force the tab index to match

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
    m_aircraft->multirotorFrameType->setCurrentIndex(2); //Set default model to "Quad X"


	//NEW STYLE: Loop through the widgets looking for all widgets that have "ChannelBox" in their name
	//  The upshot of this is that ALL new ComboBox widgets for selecting the output channel must have "ChannelBox" in their name
	foreach(QComboBox *combobox, this->findChildren<QComboBox*>(QRegExp("\\S+ChannelBo\\S+")))//FOR WHATEVER REASON, THIS DOES NOT WORK WITH ChannelBox. ChannelBo is sufficiently accurate
	{
        combobox->addItems(channelNames);
    }
	
    // Setup the Multirotor picture in the Quad settings interface
    m_aircraft->quadShape->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_aircraft->quadShape->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/multirotor-shapes.svg"));
    quad = new QGraphicsSvgItem();
    quad->setSharedRenderer(renderer);
    quad->setElementId("quad-x");
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(quad);
    scene->setSceneRect(quad->boundingRect());
    m_aircraft->quadShape->setScene(scene);

    // Put combo boxes in line one of the custom mixer table:
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("Mixer1Type"));
    QStringList list = field->getOptions();
    for (int i=0; i<(int)(VehicleConfig::CHANNEL_NUMELEM); i++) {
        QComboBox* qb = new QComboBox(m_aircraft->customMixerTable);
        qb->addItems(list);
        m_aircraft->customMixerTable->setCellWidget(0,i,qb);
    }

    SpinBoxDelegate *sbd = new SpinBoxDelegate();
    for (int i=1; i<(int)(VehicleConfig::CHANNEL_NUMELEM); i++) {
        m_aircraft->customMixerTable->setItemDelegateForRow(i, sbd);
    }

    // create and setup a MultiRotor config widget
    m_multirotor = new ConfigMultiRotorWidget(m_aircraft);
    m_multirotor->quad = quad;
    m_multirotor->uiowner = this;
    m_multirotor->setupUI(m_aircraft->multirotorFrameType->currentText());

    // create and setup a GroundVehicle config widget
    m_groundvehicle = new ConfigGroundVehicleWidget(m_aircraft);
    m_groundvehicle->setupUI(m_aircraft->groundVehicleType->currentText() );

    // create and setup a FixedWing config widget
    m_fixedwing = new ConfigFixedWingWidget(m_aircraft);
    m_fixedwing->setupUI(m_aircraft->fixedWingType->currentText() );

    // create and setup a Helicopter config widget
    m_heli = m_aircraft->widget_3;
    m_heli->setupUI(QString("HeliCP"));

	//Connect aircraft type selection dropbox to callback function
    connect(m_aircraft->aircraftType, SIGNAL(currentIndexChanged(int)), this, SLOT(switchAirframeType(int)));
	
	//Connect airframe selection dropbox to callback functions
    connect(m_aircraft->fixedWingType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));
    connect(m_aircraft->multirotorFrameType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));
    connect(m_aircraft->groundVehicleType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));
    //mdl connect(m_heli->m_ccpm->ccpmType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));

    //Connect the three feed forward test checkboxes
    connect(m_aircraft->ffTestBox1, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox2, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox3, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));

    //Connect the multirotor motor reverse checkbox
    connect(m_aircraft->MultirotorRevMixercheckBox, SIGNAL(clicked(bool)), this, SLOT(reverseMultirotorMotor()));

    // Connect the help pushbutton
    connect(m_aircraft->airframeHelp, SIGNAL(clicked()), this, SLOT(openHelp()));
    enableControls(false);
    refreshWidgetsValues();
    addToDirtyMonitor();

    disableMouseWheelEvents();
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
}


/**
 Destructor
 */
ConfigVehicleTypeWidget::~ConfigVehicleTypeWidget()
{
   // Do nothing
}

/**
  Static function to get currently assigned channelDescriptions
  for all known vehicle types;  instantiates the appropriate object
  then asks it to supply channel descs
  */
QStringList ConfigVehicleTypeWidget::getChannelDescriptions()
{    
    int i;
    QStringList channelDesc;

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);

    // get an instance of systemsettings
    SystemSettings * systemSettings = SystemSettings::GetInstance(objMngr);
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    switch (systemSettingsData.AirframeType)
    {
        // fixed wing
        case SystemSettings::AIRFRAMETYPE_FIXEDWING:
        case SystemSettings::AIRFRAMETYPE_FIXEDWINGELEVON:
        case SystemSettings::AIRFRAMETYPE_FIXEDWINGVTAIL:
        {
            channelDesc = ConfigFixedWingWidget::getChannelDescriptions();
        }
        break;

        // helicp
        case SystemSettings::AIRFRAMETYPE_HELICP:
        {
            channelDesc = ConfigCcpmWidget::getChannelDescriptions();
        }
        break;

        //multirotor
        case SystemSettings::AIRFRAMETYPE_VTOL:
        case SystemSettings::AIRFRAMETYPE_TRI:
        case SystemSettings::AIRFRAMETYPE_QUADX:
        case SystemSettings::AIRFRAMETYPE_QUADP:
        case SystemSettings::AIRFRAMETYPE_OCTOV:
        case SystemSettings::AIRFRAMETYPE_OCTOCOAXX:
        case SystemSettings::AIRFRAMETYPE_OCTOCOAXP:
        case SystemSettings::AIRFRAMETYPE_OCTO:
        case SystemSettings::AIRFRAMETYPE_HEXAX:
        case SystemSettings::AIRFRAMETYPE_HEXACOAX:
        case SystemSettings::AIRFRAMETYPE_HEXA:
        {
            channelDesc = ConfigMultiRotorWidget::getChannelDescriptions();
        }
        break;

        // ground
        case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLECAR:
        case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEDIFFERENTIAL:
        case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEMOTORCYCLE:
        {
            channelDesc = ConfigGroundVehicleWidget::getChannelDescriptions();
        }
        break;

        default:
        {
            for (i=0; i < (int)(VehicleConfig::CHANNEL_NUMELEM); i++)
                channelDesc.append(QString("-"));
        }
        break;
    }

//    for (i=0; i < channelDesc.count(); i++)
//        qDebug() << QString("Channel %0 = %1").arg(i).arg(channelDesc[i]);

    return channelDesc;
}


/**
  Slot for switching the airframe type. We do it explicitely
  rather than a signal in the UI, because we want to force a fitInView of the quad shapes.
  This is because this method (fitinview) only works when the widget is shown.
  */
void ConfigVehicleTypeWidget::switchAirframeType(int index)
{
    m_aircraft->airframesWidget->setCurrentIndex(index);
    m_aircraft->quadShape->setSceneRect(quad->boundingRect());
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
    if (m_aircraft->aircraftType->findText("Custom")) {
        m_aircraft->customMixerTable->resizeColumnsToContents();
        for (int i=0;i<(int)(VehicleConfig::CHANNEL_NUMELEM);i++) {
            m_aircraft->customMixerTable->setColumnWidth(i,(m_aircraft->customMixerTable->width()-
                                                            m_aircraft->customMixerTable->verticalHeader()->width())/10);
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
    for (int i=0;i<(int)(VehicleConfig::CHANNEL_NUMELEM);i++) {
        m_aircraft->customMixerTable->setColumnWidth(i,(m_aircraft->customMixerTable->width()-
                                                        m_aircraft->customMixerTable->verticalHeader()->width())/ 10);
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
    for (int i=0;i<(int)(VehicleConfig::CHANNEL_NUMELEM);i++) {
        m_aircraft->customMixerTable->setColumnWidth(i,(m_aircraft->customMixerTable->width()-
                                                        m_aircraft->customMixerTable->verticalHeader()->width())/ 10);
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
            UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            Q_ASSERT(mixer);

            QPointer<VehicleConfig> vconfig = new VehicleConfig();

            // Update feed forward settings
            vconfig->setMixerValue(mixer, "FeedForward", m_aircraft->feedForwardSlider->value() / 100.0);
            vconfig->setMixerValue(mixer, "AccelTime", m_aircraft->accelTime->value());
            vconfig->setMixerValue(mixer, "DecelTime", m_aircraft->decelTime->value());
            vconfig->setMixerValue(mixer, "MaxAccel", m_aircraft->maxAccelSlider->value());
            mixer->updated();
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

/**************************
  * Aircraft settings
  **************************/
/**
  Refreshes the current value of the SystemSettings which holds the aircraft type
  */
void ConfigVehicleTypeWidget::refreshWidgetsValues(UAVObject * o)
{
    Q_UNUSED(o);

    if(!allObjectsUpdated())
        return;
	
    bool dirty=isDirty();
	
    // Get the Airframe type from the system settings:
    UAVDataObject* system = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(system);

    UAVObjectField *field = system->getField(QString("AirframeType"));
    Q_ASSERT(field);
    // At this stage, we will need to have some hardcoded settings in this code, this
    // is not ideal, but there you go.
    QString frameType = field->getValue().toString();
    setupAirframeUI(frameType);

    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QPointer<VehicleConfig> vconfig = new VehicleConfig();

    QList<double> curveValues;    
    vconfig->getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, &curveValues);

    // is at least one of the curve values != 0?
    if (vconfig->isValidThrottleCurve(&curveValues)) {
        // yes, use the curve we just read from mixersettings
        m_aircraft->multiThrottleCurve->initCurve(&curveValues);
        m_aircraft->fixedWingThrottle->initCurve(&curveValues);
        m_aircraft->groundVehicleThrottle1->initCurve(&curveValues);
    }
    else {
        // no, init a straight curve
        m_aircraft->multiThrottleCurve->initLinearCurve(curveValues.count(), 0.9);
        m_aircraft->fixedWingThrottle->initLinearCurve(curveValues.count(), 1.0);
        m_aircraft->groundVehicleThrottle1->initLinearCurve(curveValues.count(), 1.0);
    }
	
    // Setup all Throttle2 curves for all types of airframes //AT THIS MOMENT, THAT MEANS ONLY GROUND VEHICLES
    vconfig->getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, &curveValues);

    if (vconfig->isValidThrottleCurve(&curveValues)) {
        m_aircraft->groundVehicleThrottle2->initCurve(&curveValues);
    }
    else {
        m_aircraft->groundVehicleThrottle2->initLinearCurve(curveValues.count(), 1.0);
    }

    // Load the Settings for fixed wing frames:
    if (frameType.startsWith("FixedWing")) {
		
        // Retrieve fixed wing settings
        m_fixedwing->refreshWidgetsValues(frameType);

    } else if (frameType == "Tri" || 
			   frameType == "QuadX" || frameType == "QuadP" ||
               frameType == "Hexa" || frameType == "HexaCoax" || frameType == "HexaX" || 
			   frameType == "Octo" || frameType == "OctoV" || frameType == "OctoCoaxP" || frameType == "OctoCoaxX" ) {
		
		 // Retrieve multirotor settings
         m_multirotor->refreshWidgetsValues(frameType);
    } else if (frameType == "HeliCP") {
        setComboCurrentIndex(m_aircraft->aircraftType, m_aircraft->aircraftType->findText("Helicopter"));
        m_heli->refreshWidgetsValues(frameType);

	} else if (frameType.startsWith("GroundVehicle")) {

        // Retrieve ground vehicle settings
        m_groundvehicle->refreshWidgetsValues(frameType);
		
	} else if (frameType == "Custom") {
        setComboCurrentIndex(m_aircraft->aircraftType, m_aircraft->aircraftType->findText("Custom"));
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
        m_fixedwing->setupUI(frameType);
     }
    else if (frameType == "Tri" || frameType == "Tricopter Y" ||
				frameType == "QuadX" || frameType == "Quad X" ||
				frameType == "QuadP" || frameType == "Quad +" ||
				frameType == "Hexa" || frameType == "Hexacopter" ||
				frameType == "HexaX" || frameType == "Hexacopter X" ||
				frameType == "HexaCoax" || frameType == "Hexacopter Y6" ||
				frameType == "Octo" || frameType == "Octocopter" ||
				frameType == "OctoV" || frameType == "Octocopter V" ||
				frameType == "OctoCoaxP" || frameType == "Octo Coax +" || 
				frameType == "OctoCoaxX" || frameType == "Octo Coax X" ) {
		 
         //Call multi-rotor setup UI
         m_multirotor->setupUI(frameType);
	 }	 
    else if (frameType == "HeliCP") {
        m_heli->setupUI(frameType);
    }
    else if (frameType == "GroundVehicleCar" || frameType == "Turnable (car)" ||
			 frameType == "GroundVehicleDifferential" || frameType == "Differential (tank)" || 
             frameType == "GroundVehicleMotorcyle" || frameType == "Motorcycle") {
        m_groundvehicle->setupUI(frameType);
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
  Updates the custom airframe settings based on the current airframe.

  Note: does NOT ask for an object refresh itself!
  */
void ConfigVehicleTypeWidget::updateCustomAirframeUI()
{    
    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QPointer<VehicleConfig> vconfig = new VehicleConfig();

    QList<double> curveValues;
    vconfig->getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, &curveValues);

    // is at least one of the curve values != 0?
    if (vconfig->isValidThrottleCurve(&curveValues)) {
        m_aircraft->customThrottle1Curve->initCurve(&curveValues);
    }
    else {
        // no, init a straight curve
        m_aircraft->customThrottle1Curve->initLinearCurve(curveValues.count(), 1.0);
    }

    if (MixerSettings* mxr = qobject_cast<MixerSettings *>(mixer)) {
        MixerSettings::DataFields mixerSettingsData = mxr->getData();
        if (mixerSettingsData.Curve2Source == MixerSettings::CURVE2SOURCE_THROTTLE)
            m_aircraft->customThrottle2Curve->setMixerType(MixerCurve::MIXERCURVE_THROTTLE);
        else {
            m_aircraft->customThrottle2Curve->setMixerType(MixerCurve::MIXERCURVE_PITCH);
        }
    }

    // Setup all Throttle2 curves for all types of airframes
    vconfig->getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, &curveValues);

    if (vconfig->isValidThrottleCurve(&curveValues)) {
        m_aircraft->customThrottle2Curve->initCurve(&curveValues);
    }
    else {
        m_aircraft->customThrottle2Curve->initLinearCurve(curveValues.count(), 1.0, m_aircraft->customThrottle2Curve->getMin());
    }

    // Update the mixer table:
    for (int channel=0; channel<(int)(VehicleConfig::CHANNEL_NUMELEM); channel++) {
        UAVObjectField* field = mixer->getField(mixerTypes.at(channel));
        if (field)
        {
            QComboBox* q = (QComboBox*)m_aircraft->customMixerTable->cellWidget(0,channel);
            if (q)
            {
                QString s = field->getValue().toString();
                setComboCurrentIndex(q, q->findText(s));
            }

            m_aircraft->customMixerTable->item(1,channel)->setText(
                QString::number(vconfig->getMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_THROTTLECURVE1)));
            m_aircraft->customMixerTable->item(2,channel)->setText(
                QString::number(vconfig->getMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_THROTTLECURVE2)));
            m_aircraft->customMixerTable->item(3,channel)->setText(
                QString::number(vconfig->getMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_ROLL)));
            m_aircraft->customMixerTable->item(4,channel)->setText(
                QString::number(vconfig->getMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_PITCH)));
            m_aircraft->customMixerTable->item(5,channel)->setText(
                QString::number(vconfig->getMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_YAW)));
        }
    }

    // Update feed forward settings
    m_aircraft->feedForwardSlider->setValue(vconfig->getMixerValue(mixer,"FeedForward") * 100);
    m_aircraft->accelTime->setValue(vconfig->getMixerValue(mixer,"AccelTime"));
    m_aircraft->decelTime->setValue(vconfig->getMixerValue(mixer,"DecelTime"));
    m_aircraft->maxAccelSlider->setValue(vconfig->getMixerValue(mixer,"MaxAccel"));
}


/**
  Sends the config to the board (airframe type)

  We do all the tasks common to all airframes, or family of airframes, and
  we call additional methods for specific frames, so that we do not have a code
  that is too heavy.
*/
void ConfigVehicleTypeWidget::updateObjectsFromWidgets()
{
    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QPointer<VehicleConfig> vconfig = new VehicleConfig();

    // Update feed forward settings
    vconfig->setMixerValue(mixer, "FeedForward", m_aircraft->feedForwardSlider->value() / 100.0);
    vconfig->setMixerValue(mixer, "AccelTime", m_aircraft->accelTime->value());
    vconfig->setMixerValue(mixer, "DecelTime", m_aircraft->decelTime->value());
    vconfig->setMixerValue(mixer, "MaxAccel", m_aircraft->maxAccelSlider->value());

    QString airframeType = "Custom"; //Sets airframe type default to "Custom"
    if (m_aircraft->aircraftType->currentText() == "Fixed Wing") {
        airframeType = m_fixedwing->updateConfigObjectsFromWidgets();
    }
    else if (m_aircraft->aircraftType->currentText() == "Multirotor") {
         airframeType = m_multirotor->updateConfigObjectsFromWidgets();
    }
    else if (m_aircraft->aircraftType->currentText() == "Helicopter") {
         airframeType = m_heli->updateConfigObjectsFromWidgets();
    }
    else if (m_aircraft->aircraftType->currentText() == "Ground") {
         airframeType = m_groundvehicle->updateConfigObjectsFromWidgets();
    }
    else {
        vconfig->setThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, m_aircraft->customThrottle1Curve->getCurve());
        vconfig->setThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, m_aircraft->customThrottle2Curve->getCurve());

        // Update the table:
        for (int channel=0; channel<(int)(VehicleConfig::CHANNEL_NUMELEM); channel++) {
            QComboBox* q = (QComboBox*)m_aircraft->customMixerTable->cellWidget(0,channel);
            if(q->currentText()=="Disabled")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_DISABLED);
            else if(q->currentText()=="Motor")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_MOTOR);
            else if(q->currentText()=="Servo")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_SERVO);
            else if(q->currentText()=="CameraRoll")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_CAMERAROLL);
            else if(q->currentText()=="CameraPitch")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_CAMERAPITCH);
            else if(q->currentText()=="CameraYaw")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_CAMERAYAW);
            else if(q->currentText()=="Accessory0")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_ACCESSORY0);
            else if(q->currentText()=="Accessory1")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_ACCESSORY1);
            else if(q->currentText()=="Accessory2")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_ACCESSORY2);
            else if(q->currentText()=="Accessory3")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_ACCESSORY3);
            else if(q->currentText()=="Accessory4")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_ACCESSORY4);
            else if(q->currentText()=="Accessory5")
                vconfig->setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_ACCESSORY5);

            vconfig->setMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_THROTTLECURVE1,
                                            m_aircraft->customMixerTable->item(1,channel)->text().toDouble());
            vconfig->setMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_THROTTLECURVE2,
                                            m_aircraft->customMixerTable->item(2,channel)->text().toDouble());
            vconfig->setMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_ROLL,
                                            m_aircraft->customMixerTable->item(3,channel)->text().toDouble());
            vconfig->setMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_PITCH,
                                            m_aircraft->customMixerTable->item(4,channel)->text().toDouble());
            vconfig->setMixerVectorValue(mixer,channel,VehicleConfig::MIXERVECTOR_YAW,
                                            m_aircraft->customMixerTable->item(5,channel)->text().toDouble());
        }
    }

    // set the airframe type
    UAVDataObject* system = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(system);

    QPointer<UAVObjectField> field = system->getField(QString("AirframeType"));
    if (field)
        field->setValue(airframeType);

    updateCustomAirframeUI();
}

/**
 Opens the wiki from the user's default browser
 */
void ConfigVehicleTypeWidget::openHelp()
{

    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/x/44Cf", QUrl::StrictMode) );
}

/**
  Helper function:
  Sets the current index on supplied combobox to index
  if it is within bounds 0 <= index < combobox.count()
 */
void ConfigVehicleTypeWidget::setComboCurrentIndex(QComboBox* box, int index)
{
    if (index >= 0 && index < box->count())
        box->setCurrentIndex(index);
}

void ConfigVehicleTypeWidget::reverseMultirotorMotor(){
    QString frameType = m_aircraft->multirotorFrameType->currentText();
    m_multirotor->drawAirframe(frameType);
}


/**
 WHAT DOES THIS DO???
 */
void ConfigVehicleTypeWidget::addToDirtyMonitor()
{
    addWidget(m_aircraft->customMixerTable);
    addWidget(m_aircraft->customThrottle1Curve->getCurveWidget());
    addWidget(m_aircraft->customThrottle2Curve->getCurveWidget());
    addWidget(m_aircraft->multiThrottleCurve->getCurveWidget());
    addWidget(m_aircraft->fixedWingThrottle->getCurveWidget());
    addWidget(m_aircraft->fixedWingType);
    addWidget(m_aircraft->groundVehicleThrottle1->getCurveWidget());
    addWidget(m_aircraft->groundVehicleThrottle2->getCurveWidget());
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
    addWidget(m_aircraft->mrPitchMixLevel);
    addWidget(m_aircraft->mrRollMixLevel);
    addWidget(m_aircraft->mrYawMixLevel);
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
    addWidget(m_heli->m_ccpm->ccpmType);
    addWidget(m_heli->m_ccpm->ccpmTailChannel);
    addWidget(m_heli->m_ccpm->ccpmEngineChannel);
    addWidget(m_heli->m_ccpm->ccpmServoWChannel);
    addWidget(m_heli->m_ccpm->ccpmServoXChannel);
    addWidget(m_heli->m_ccpm->ccpmServoYChannel);
    addWidget(m_heli->m_ccpm->ccpmSingleServo);
    addWidget(m_heli->m_ccpm->ccpmServoZChannel);
    addWidget(m_heli->m_ccpm->ccpmAngleW);
    addWidget(m_heli->m_ccpm->ccpmAngleX);
    addWidget(m_heli->m_ccpm->ccpmCorrectionAngle);
    addWidget(m_heli->m_ccpm->ccpmAngleZ);
    addWidget(m_heli->m_ccpm->ccpmAngleY);
    addWidget(m_heli->m_ccpm->ccpmCollectivePassthrough);
    addWidget(m_heli->m_ccpm->ccpmLinkRoll);
    addWidget(m_heli->m_ccpm->ccpmLinkCyclic);
    addWidget(m_heli->m_ccpm->ccpmRevoSlider);
    addWidget(m_heli->m_ccpm->ccpmREVOspinBox);
    addWidget(m_heli->m_ccpm->ccpmCollectiveSlider);
    addWidget(m_heli->m_ccpm->ccpmCollectivespinBox);
    addWidget(m_heli->m_ccpm->ccpmCollectiveScale);
    addWidget(m_heli->m_ccpm->ccpmCollectiveScaleBox);
    addWidget(m_heli->m_ccpm->ccpmCyclicScale);
    addWidget(m_heli->m_ccpm->ccpmPitchScale);
    addWidget(m_heli->m_ccpm->ccpmPitchScaleBox);
    addWidget(m_heli->m_ccpm->ccpmRollScale);
    addWidget(m_heli->m_ccpm->ccpmRollScaleBox);
    addWidget(m_heli->m_ccpm->SwashLvlPositionSlider);
    addWidget(m_heli->m_ccpm->SwashLvlPositionSpinBox);
    addWidget(m_heli->m_ccpm->ThrottleCurve->getCurveWidget());
    addWidget(m_heli->m_ccpm->PitchCurve->getCurveWidget());
    addWidget(m_heli->m_ccpm->ccpmAdvancedSettingsTable);
}

