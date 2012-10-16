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

#include "lowpassfilter.h"
#include <QDebug>
#include <QTimerEvent>

LowPassFilter::LowPassFilter(QObject *parent) :
    QObject(parent),
    m_loaded(false),
    m_input(0),
    m_value(0),
    m_timeConstant(0.5), //500ms
    m_updatePeriod(1.0/30) //30 fps
{
}

LowPassFilter::~LowPassFilter()
{
}

/*!
  \property LowPassFilter::value
  Filtered value.
*/

/*!
  \property LowPassFilter::input
  Input sample value
*/
void LowPassFilter::setInput(qreal arg)
{
    bool changed = !qFuzzyCompare(m_input, arg);

    if (!m_loaded) {
        m_loaded = true;
        m_input = m_value = arg;
        m_lastUpdateTime.start();
    }

    m_input = arg;

    if (changed)
        emit inputChanged(arg);

    updateValue();
}

/*!
  \property LowPassFilter::timeConstant
  Low pass filter time constant, in seconds.
  The default value is 0.5 seconds.
*/
void LowPassFilter::setTimeConstant(qreal arg)
{
    if (!qFuzzyCompare(m_timeConstant, arg)) {
        m_timeConstant = arg;
        emit timeConstantChanged(arg);
    }
}

/*!
  \property LowPassFilter::updatePeriod
  Low pass filter update period, in seconds.
  The default value is 1/30s.
*/
void LowPassFilter::setUpdatePeriod(qreal arg)
{
    if (!qFuzzyCompare(m_updatePeriod, arg)) {
        m_updatePeriod = arg;
        if (m_updateTimer.isActive()) {
            m_updateTimer.stop();
            m_updateTimer.start(qRound(m_updatePeriod*1000), this);
        }
        emit updatePeriodChanged(arg);
    }
}

void LowPassFilter::updateValue()
{
    //check if the update timer should be running
    bool reached = qFuzzyIsNull(qAbs(m_input)+qAbs(m_value)) ||
            (qAbs(m_input-m_value)/(qAbs(m_input)+qAbs(m_value)) < 1e-3);

    if (reached && m_updateTimer.isActive())
        m_updateTimer.stop();
    else if (!reached && !m_updateTimer.isActive())
        m_updateTimer.start(qRound(m_updatePeriod*1000), this);

    int dT = m_lastUpdateTime.elapsed();
    m_lastUpdateTime.restart();
    if (dT == 0)
        return;

    m_value += (m_input - m_value)*dT/(m_timeConstant*1000 + dT);
    emit valueChanged(m_value);
}

void LowPassFilter::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == m_updateTimer.timerId())
        updateValue();
}

