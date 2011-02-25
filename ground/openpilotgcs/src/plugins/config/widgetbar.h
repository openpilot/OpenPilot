
 #ifndef WIDGETBAR_H
 #define WIDGETBAR_H

 #include <QBrush>
 #include <QPen>
 #include <QPixmap>
 #include <QWidget>

 class WidgetBar : public QWidget
 {
	Q_OBJECT

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

 protected:
	void paintEvent(QPaintEvent *event);

 private:
	int m_maximum;
	int m_minimum;
	int m_value;
 };

 #endif
