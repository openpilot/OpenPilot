/**
 ******************************************************************************
 *
 * @file       widgetbar.cpp
 * @author     Cathy Moss Copyright (C) 2011.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Configuration Plugin
 * @{
 * @brief A bar display widget
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

 #include <QtGui>

 #include "widgetbar.h"

 WidgetBar::WidgetBar(QWidget *parent)
     : QWidget(parent)
 {
	m_maximum = 2000;
	m_minimum = 1000;
	m_value = 1500;

	m_orientation = Qt::Vertical;

	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
 }

 QSize WidgetBar::minimumSizeHint() const
 {
	 return QSize(8, 8);
 }

 QSize WidgetBar::sizeHint() const
 {
	 return QSize(200, 400);
 }

 void WidgetBar::setMaximum(int value)
 {
	m_maximum = value;
	update();
 }

 void WidgetBar::setMinimum(int value)
 {
	m_minimum = value;
	update();
 }

 void WidgetBar::setValue(int value)
 {
	if (value < m_minimum) value = m_minimum;
	else
	if (value > m_maximum) value = m_maximum;

	m_value = value;
	update();
 }

void WidgetBar::setOrientation(Qt::Orientation orientation)
{
	m_orientation = orientation;
	update();
}

void WidgetBar::paintEvent(QPaintEvent * /* event */)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, false);

	int range = abs(m_maximum - m_minimum);
	if (range < 1) range = 1;

	float level = (float)(m_value - m_minimum) / range;	// 0 to +1

	int length = 0;
	QRect rect;

	switch (m_orientation)
	{
		case Qt::Horizontal:
			{
				length = (int)((width() - 5) * level + 0.5f);
				rect.setLeft(2);
				rect.setTop(2);
				rect.setWidth(length);
				rect.setHeight(height() - 5);
			}
			break;

		case Qt::Vertical:
			{
				length = (int)((height() - 5) * level + 0.5f);
				rect.setLeft(2);
				rect.setTop(height() - 3 - length);
				rect.setWidth(width() - 5);
				rect.setHeight(length);
			}
			break;
	}

	// background
//	painter.setPen(QColor(160, 160, 160));
//	painter.setBrush(QColor(255, 255, 255));
	painter.setPen(QColor(80, 80, 80));
	painter.setBrush(QColor(160, 160, 160));
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

	if ((m_maximum - m_minimum) > 0)
	{
		// solid bar
//		painter.setPen(QColor(128, 128, 255));
//		painter.setBrush(QColor(128, 128, 255));
//		painter.drawRoundRect(rect, 3, 3);

		// colourful bar
		for (int i = 0; i < length; i++)
		{
			if (!(i & 1))
				painter.setPen(QColor(0, 0, 0));		// black
			else
//				painter.setPen(QColor(0, 255, 0));		// green
				painter.setPen(QColor(128, 128, 255));	// blue
			if (m_orientation == Qt::Vertical)
				painter.drawLine(rect.left(), rect.bottom() + 1 - i, rect.right() + 1, rect.bottom() + 1 - i);	// vertical bar
			else
				painter.drawLine(rect.top(), rect.left() + i, rect.bottom(), rect.left() + i);					// horizontal bar
		}
	}
}
