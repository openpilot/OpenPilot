
#include "configautotunewidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QList>
#include "relaytuningsettings.h"
#include "relaytuning.h"
#include "stabilizationsettings.h"
#include "hwsettings.h"

ConfigAutotuneWidget::ConfigAutotuneWidget(QWidget *parent) :
    ConfigTaskWidget(parent)
{
    m_autotune = new Ui_AutotuneWidget();
    m_autotune->setupUi(this);

    // Connect automatic signals
    autoLoadWidgets();
    disableMouseWheelEvents();

    // Whenever any value changes compute new potential stabilization settings
    connect(m_autotune->rateTuning, SIGNAL(valueChanged(int)), this, SLOT(recomputeStabilization()));
    connect(m_autotune->attitudeTuning, SIGNAL(valueChanged(int)), this, SLOT(recomputeStabilization()));

    addUAVObject("HwSettings");
    addWidget(m_autotune->enableAutoTune);

    RelayTuning *relayTuning = RelayTuning::GetInstance(getObjectManager());
    Q_ASSERT(relayTuning);
    if(relayTuning)
        connect(relayTuning, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(recomputeStabilization()));

    // Connect the apply button for the stabilization settings
    connect(m_autotune->useComputedValues, SIGNAL(pressed()), this, SLOT(saveStabilization()));
}

/**
  * Apply the stabilization settings computed
  */
void ConfigAutotuneWidget::saveStabilization()
{
    StabilizationSettings *stabilizationSettings = StabilizationSettings::GetInstance(getObjectManager());
    Q_ASSERT(stabilizationSettings);
    if(!stabilizationSettings)
        return;

    // Make sure to recompute in case the other stab settings changed since
    // the last time
    recomputeStabilization();

    // Apply this data to the board
    stabilizationSettings->setData(stabSettings);
    stabilizationSettings->updated();
}

/**
  * Called whenever the gain ratios or measured values
  * are changed
  */
void ConfigAutotuneWidget::recomputeStabilization()
{
    RelayTuningSettings *relayTuningSettings = RelayTuningSettings::GetInstance(getObjectManager());
    Q_ASSERT(relayTuningSettings);
    if (!relayTuningSettings)
        return;

    RelayTuning *relayTuning = RelayTuning::GetInstance(getObjectManager());
    Q_ASSERT(relayTuning);
    if(!relayTuning)
        return;

    StabilizationSettings *stabilizationSettings = StabilizationSettings::GetInstance(getObjectManager());
    Q_ASSERT(stabilizationSettings);
    if(!stabilizationSettings)
        return;

    RelayTuning::DataFields relayTuningData = relayTuning->getData();
    RelayTuningSettings::DataFields tuningSettingsData = relayTuningSettings->getData();
    stabSettings = stabilizationSettings->getData();

    // Need to divide these by 100 because that is what the .ui file does
    // to get the UAVO
    const double gain_ratio_r = m_autotune->rateTuning->value() / 100.0;
    const double zero_ratio_r = m_autotune->rateTuning->value() / 100.0;
    const double gain_ratio_p = m_autotune->attitudeTuning->value() / 100.0;
    const double zero_ratio_p = m_autotune->attitudeTuning->value() / 100.0;

    // For now just run over roll and pitch
    for (int i = 0; i < 2; i++) {
        if (relayTuningData.Period[i] == 0 || relayTuningData.Gain[i] == 0)
            continue;

        double wu = 1000.0 * 2 * M_PI / relayTuningData.Period[i]; // ultimate freq = output osc freq (rad/s)

        double wc = wu * gain_ratio_r;      // target openloop crossover frequency (rad/s)
        double zc = wc * zero_ratio_r;      // controller zero location (rad/s)
        double kpu = 4.0f / M_PI / relayTuningData.Gain[i];  // ultimate gain, i.e. the proportional gain for instablity
        double kp = kpu * gain_ratio_r;     // proportional gain
        double ki = zc * kp;                // integral gain

        // Now calculate gains for the next loop out knowing it is the integral of
        // the inner loop -- the plant is position/velocity = scale*1/s
        double wc2 = wc * gain_ratio_p;          // crossover of the attitude loop
        double kp2 = wc2;                       // kp of attitude
        double ki2 = wc2 * zero_ratio_p * kp2;  // ki of attitude

        switch(i) {
        case 0: // Roll

            stabSettings.RollRatePID[StabilizationSettings::ROLLRATEPID_KP] = kp;
            stabSettings.RollRatePID[StabilizationSettings::ROLLRATEPID_KI] = ki;
            stabSettings.RollPI[StabilizationSettings::ROLLPI_KP] = kp2;
            stabSettings.RollPI[StabilizationSettings::ROLLPI_KI] = ki2;
            break;
        case 1: // Pitch
            stabSettings.PitchRatePID[StabilizationSettings::PITCHRATEPID_KP] = kp;
            stabSettings.PitchRatePID[StabilizationSettings::PITCHRATEPID_KI] = ki;
            stabSettings.PitchPI[StabilizationSettings::PITCHPI_KP] = kp2;
            stabSettings.PitchPI[StabilizationSettings::PITCHPI_KI] = ki2;
            break;
        }
    }

    // Display these computed settings
    m_autotune->rollRateKp->setText(QString().number(stabSettings.RollRatePID[StabilizationSettings::ROLLRATEPID_KP]));
    m_autotune->rollRateKi->setText(QString().number(stabSettings.RollRatePID[StabilizationSettings::ROLLRATEPID_KI]));
    m_autotune->rollAttitudeKp->setText(QString().number(stabSettings.RollPI[StabilizationSettings::ROLLPI_KP]));
    m_autotune->rollAttitudeKi->setText(QString().number(stabSettings.RollPI[StabilizationSettings::ROLLPI_KI]));
    m_autotune->pitchRateKp->setText(QString().number(stabSettings.PitchRatePID[StabilizationSettings::PITCHRATEPID_KP]));
    m_autotune->pitchRateKi->setText(QString().number(stabSettings.PitchRatePID[StabilizationSettings::PITCHRATEPID_KI]));
    m_autotune->pitchAttitudeKp->setText(QString().number(stabSettings.PitchPI[StabilizationSettings::PITCHPI_KP]));
    m_autotune->pitchAttitudeKi->setText(QString().number(stabSettings.PitchPI[StabilizationSettings::PITCHPI_KI]));
}
void ConfigAutotuneWidget::refreshWidgetsValues(UAVObject *obj)
{
    HwSettings *hwSettings = HwSettings::GetInstance(getObjectManager());
    if(obj==hwSettings)
    {
        bool dirtyBack=isDirty();
        HwSettings::DataFields hwSettingsData = hwSettings->getData();
        m_autotune->enableAutoTune->setChecked(
            hwSettingsData.OptionalModules[HwSettings::OPTIONALMODULES_AUTOTUNE] == HwSettings::OPTIONALMODULES_ENABLED);
        setDirty(dirtyBack);
    }
    ConfigTaskWidget::refreshWidgetsValues(obj);
}
void ConfigAutotuneWidget::updateObjectsFromWidgets()
{
    HwSettings *hwSettings = HwSettings::GetInstance(getObjectManager());
    HwSettings::DataFields hwSettingsData = hwSettings->getData();
    hwSettingsData.OptionalModules[HwSettings::OPTIONALMODULES_AUTOTUNE] =
         m_autotune->enableAutoTune->isChecked() ? HwSettings::OPTIONALMODULES_ENABLED : HwSettings::OPTIONALMODULES_DISABLED;
    hwSettings->setData(hwSettingsData);
    ConfigTaskWidget::updateObjectsFromWidgets();
}
