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

#ifndef LOWPASSFILTER_H_
#define LOWPASSFILTER_H_

#include <QObject>
#include <QTime>
#include <QBasicTimer>

class LowPassFilter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal input READ input WRITE setInput NOTIFY inputChanged)
    Q_PROPERTY(qreal value READ value NOTIFY valueChanged)
    Q_PROPERTY(qreal timeConstant READ timeConstant WRITE setTimeConstant NOTIFY timeConstantChanged)
    Q_PROPERTY(qreal updatePeriod READ updatePeriod WRITE setUpdatePeriod NOTIFY updatePeriodChanged)
public:
   LowPassFilter(QObject *parent = 0);
   ~LowPassFilter();

   qreal input() const { return m_input; }
   qreal value() const { return m_value; }
   qreal timeConstant() const { return m_timeConstant; }
   qreal updatePeriod() const { return m_updatePeriod; }

public slots:
   void setInput(qreal arg);
   void setTimeConstant(qreal arg);
   void setUpdatePeriod(qreal arg);

signals:
   void inputChanged(qreal arg);
   void valueChanged(qreal arg);
   void timeConstantChanged(qreal arg);
   void updatePeriodChanged(qreal arg);

private slots:
   void updateValue();

protected:
   void timerEvent(QTimerEvent *ev);

private:
   bool m_loaded;
   qreal m_input;
   qreal m_value;
   qreal m_timeConstant;
   qreal m_updatePeriod;
   QTime m_lastUpdateTime;
   QBasicTimer m_updateTimer;
};

#endif
