/**
 ******************************************************************************
 *
 * @file       outputchannelform.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Servo output configuration form for the config output gadget
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

#include "outputchannelform.h"
#include "configoutputwidget.h"

OutputChannelForm::OutputChannelForm(const int index, QWidget *parent, const bool showLegend) :
        ConfigTaskWidget(parent),
        ui(),
        m_index(index),
        m_inChannelTest(false)
{
    ui.setupUi(this);
    if(!showLegend)
    {
        // Remove legend
        QGridLayout *grid_layout = dynamic_cast<QGridLayout*>(layout());
        Q_ASSERT(grid_layout);
        for (int col = 0; col < grid_layout->columnCount(); col++)
        { // remove every item in first row
            QLayoutItem *item = grid_layout->itemAtPosition(0, col);
            if (!item) continue;
            // get widget from layout item
            QWidget *legend_widget = item->widget();
            if (!legend_widget) continue;
            // delete widget
            grid_layout->removeWidget(legend_widget);
            delete legend_widget;
        }
    }

    // The convention for OP is Channel 1 to Channel 10.
    ui.actuatorNumber->setText(QString("%1:").arg(m_index+1));

    // Register for ActuatorSettings changes:
    connect(ui.actuatorMin, SIGNAL(editingFinished()),
            this, SLOT(setChannelRange()));
    connect(ui.actuatorMax, SIGNAL(editingFinished()),
            this, SLOT(setChannelRange()));
    connect(ui.actuatorRev, SIGNAL(toggled(bool)),
            this, SLOT(reverseChannel(bool)));
    // Now connect the channel out sliders to our signal to send updates in test mode
    connect(ui.actuatorNeutral, SIGNAL(valueChanged(int)),
            this, SLOT(sendChannelTest(int)));

    ui.actuatorLink->setChecked(false);
    connect(ui.actuatorLink, SIGNAL(toggled(bool)),
            this, SLOT(linkToggled(bool)));

    disableMouseWheelEvents();
}

OutputChannelForm::~OutputChannelForm()
{
    // Do nothing
}

/**
 * Restrict UI to protect users from accidental misuse.
 */
void OutputChannelForm::enableChannelTest(bool state)
{
    if (m_inChannelTest == state) return;
    m_inChannelTest = state;

    if(m_inChannelTest)
    {
        // Prevent stupid users from touching the minimum & maximum ranges while
        // moving the sliders. Thanks Ivan for the tip :)
        ui.actuatorMin->setEnabled(false);
        ui.actuatorMax->setEnabled(false);
        ui.actuatorRev->setEnabled(false);
    }
    else
    {
        ui.actuatorMin->setEnabled(true);
        ui.actuatorMax->setEnabled(true);
        ui.actuatorRev->setEnabled(true);
    }
}


/**
 * Toggles the channel linked state for use in testing mode
 */
void OutputChannelForm::linkToggled(bool state)
{
    Q_UNUSED(state)

    if (!m_inChannelTest)
        return;	// we are not in Test Output mode

    // find the minimum slider value for the linked ones
    if (!parent()) return;
    int min = 10000;
    int linked_count = 0;
    QList<OutputChannelForm*> outputChannelForms = parent()->findChildren<OutputChannelForm*>();
    // set the linked channels of the parent widget to the same value
    foreach(OutputChannelForm *outputChannelForm, outputChannelForms)
    {
        if (!outputChannelForm->ui.actuatorLink->checkState())
            continue;
        if (this == outputChannelForm)
            continue;
        int value = outputChannelForm->ui.actuatorNeutral->value();
        if(min > value) min = value;
        linked_count++;
    }

    if (linked_count <= 0)
        return;		// no linked channels

    // set the linked channels to the same value
    foreach(OutputChannelForm *outputChannelForm, outputChannelForms)
    {
        if (!outputChannelForm->ui.actuatorLink->checkState())
            continue;
        outputChannelForm->ui.actuatorNeutral->setValue(min);
    }
}

/**
 * Set maximal channel value.
 */
void OutputChannelForm::max(int maximum)
{
    minmax(ui.actuatorMin->value(), maximum);
}

/**
 * Set minimal channel value.
 */
void OutputChannelForm::min(int minimum)
{
    minmax(minimum, ui.actuatorMax->value());
}

/**
 * Set minimal and maximal channel value.
 */
void OutputChannelForm::minmax(int minimum, int maximum)
{
    ui.actuatorMin->setValue(minimum);
    ui.actuatorMax->setValue(maximum);
    setChannelRange();
    if(ui.actuatorMin->value() > ui.actuatorMax->value())
        ui.actuatorRev->setChecked(true);
    else
        ui.actuatorRev->setChecked(false);
}

/**
 * Set neutral of channel.
 */
void OutputChannelForm::neutral(int value)
{
    ui.actuatorNeutral->setValue(value);
}

/**
 * Set the channel assignment label.
 */
void OutputChannelForm::setAssignment(const QString &assignment)
{
    ui.actuatorName->setText(assignment);
    QFontMetrics metrics(ui.actuatorName->font());
    int width=metrics.width(assignment)+1;
    foreach(OutputChannelForm * form,parent()->findChildren<OutputChannelForm*>())
    {
        if(form==this)
            continue;
        if(form->ui.actuatorName->minimumSize().width()<width)
            form->ui.actuatorName->setMinimumSize(width,0);
        else
            width=form->ui.actuatorName->minimumSize().width();
    }
    ui.actuatorName->setMinimumSize(width,0);
}

/**
 * Sets the minimum/maximum value of the channel output sliders.
 * Have to do it here because setMinimum is not a slot.
 *
 * One added trick: if the slider is at its min when the value
 * is changed, then keep it on the min.
 */
void OutputChannelForm::setChannelRange()
{
    int oldMini = ui.actuatorNeutral->minimum();
//    int oldMaxi = ui.actuatorNeutral->maximum();

    if (ui.actuatorMin->value() < ui.actuatorMax->value())
    {
        ui.actuatorNeutral->setRange(ui.actuatorMin->value(), ui.actuatorMax->value());
        ui.actuatorRev->setChecked(false);
    }
    else
    {
        ui.actuatorNeutral->setRange(ui.actuatorMax->value(), ui.actuatorMin->value());
        ui.actuatorRev->setChecked(true);
    }

    if (ui.actuatorNeutral->value() == oldMini)
        ui.actuatorNeutral->setValue(ui.actuatorNeutral->minimum());

//    if (ui.actuatorNeutral->value() == oldMaxi)
//        ui.actuatorNeutral->setValue(ui.actuatorNeutral->maximum());  // this can be dangerous if it happens to be controlling a motor at the time!
}

/**
 * Reverses the channel when the checkbox is clicked
 */
void OutputChannelForm::reverseChannel(bool state)
{
    // Sanity check: if state became true, make sure the Maxvalue was higher than Minvalue
    // the situations below can happen!
    if (state && (ui.actuatorMax->value() < ui.actuatorMin->value()))
        return;
    if (!state && (ui.actuatorMax->value() > ui.actuatorMin->value()))
        return;

    // Now, swap the min & max values (only on the spinboxes, the slider
    // does not change!
    int temp = ui.actuatorMax->value();
    ui.actuatorMax->setValue(ui.actuatorMin->value());
    ui.actuatorMin->setValue(temp);

    // Also update the channel value
    // This is a trick to force the slider to update its value and
    // emit the right signal itself, because our sendChannelTest(int) method
    // relies on the object sender's identity.
    if (ui.actuatorNeutral->value() < ui.actuatorNeutral->maximum())
    {
        ui.actuatorNeutral->setValue(ui.actuatorNeutral->value()+1);
        ui.actuatorNeutral->setValue(ui.actuatorNeutral->value()-1);
    }
    else
    {
        ui.actuatorNeutral->setValue(ui.actuatorNeutral->value()-1);
        ui.actuatorNeutral->setValue(ui.actuatorNeutral->value()+1);
    }
}

/**
 * Emits the channel value which will be send to the UAV to move the servo.
 * Returns immediately if we are not in testing mode.
 */
void OutputChannelForm::sendChannelTest(int value)
{
    int in_value = value;

    QSlider *ob = (QSlider *)QObject::sender();
    if (!ob) return;

    if (ui.actuatorRev->isChecked())
            value = ui.actuatorMin->value() - value + ui.actuatorMax->value();	// the channel is reversed

    // update the label
    ui.actuatorValue->setText(QString::number(value));

    if (ui.actuatorLink->checkState() && parent())
    {	// the channel is linked to other channels
        QList<OutputChannelForm*> outputChannelForms = parent()->findChildren<OutputChannelForm*>();
        // set the linked channels of the parent widget to the same value
        foreach(OutputChannelForm *outputChannelForm, outputChannelForms)
        {
            if (this == outputChannelForm) continue;
            if (!outputChannelForm->ui.actuatorLink->checkState()) continue;

            int val = in_value;
            if (val < outputChannelForm->ui.actuatorNeutral->minimum())
                val = outputChannelForm->ui.actuatorNeutral->minimum();
            if (val > outputChannelForm->ui.actuatorNeutral->maximum())
                val = outputChannelForm->ui.actuatorNeutral->maximum();

            if (outputChannelForm->ui.actuatorNeutral->value() == val) continue;

            outputChannelForm->ui.actuatorNeutral->setValue(val);
            outputChannelForm->ui.actuatorValue->setText(QString::number(val));
        }
    }

    if (!m_inChannelTest)
        return;	// we are not in Test Output mode

    emit channelChanged(index(), value);
}
