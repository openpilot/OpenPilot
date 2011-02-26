/**
 ******************************************************************************
 *
 * @file       widgetbar.h
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

#ifndef WIDGETBAR_H
#define WIDGETBAR_H

#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>

QT_MODULE(Gui)

class WidgetBar : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
	Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
	Q_PROPERTY(int value READ value WRITE setValue)

	Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

public:
	WidgetBar(QWidget *parent = 0);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void setMaximum(int value);
	void setMinimum(int value);
	void setValue(int value);

	int maximum() { return m_maximum; }
	int minimum() { return m_minimum; }
	int value() { return m_value; }

	Qt::Orientation orientation() const { return m_orientation; }
	void setOrientation(Qt::Orientation orientation);

protected:
	void paintEvent(QPaintEvent *event);

private:
	int m_maximum;
	int m_minimum;
	int m_value;

	Qt::Orientation m_orientation;
};

#endif
