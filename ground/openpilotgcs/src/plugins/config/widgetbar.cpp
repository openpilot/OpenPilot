
 #include <QtGui>

 #include "widgetbar.h"

 WidgetBar::WidgetBar(QWidget *parent)
     : QWidget(parent)
 {
	m_maximum = 2000;
	m_minimum = 1000;
	m_value = 1500;

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

 void WidgetBar::paintEvent(QPaintEvent * /* event */)
 {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, false);

	int range = abs(m_maximum - m_minimum);
	if (range < 1) range = 1;

	float level = (float)(m_value - m_minimum) / range;	// 0 to +1

	int h = (int)((height() - 5) * level + 0.5f);

	QRect rect;
	rect.setLeft(2);
	rect.setTop(height() - 3 - h);
	rect.setWidth(width() - 5);
	rect.setHeight(h);

	// background
	painter.setPen(QColor(160, 160, 160));
	painter.setBrush(QColor(255, 255, 255));
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

	if ((m_maximum - m_minimum) > 0)
	{
		painter.setPen(QColor(128, 128, 255));
		painter.setBrush(QColor(128, 128, 255));
		painter.drawRoundRect(rect, 3, 3);
	}
}
