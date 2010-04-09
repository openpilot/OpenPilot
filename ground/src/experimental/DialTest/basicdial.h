#ifndef BASICDIAL_H
#define BASICDIAL_H

#include <QSvgWidget>
#include <QSvgRenderer>

class BasicDial : public QWidget
{
    Q_OBJECT

public:
    BasicDial(QWidget *parent);
    virtual void paintEvent(QPaintEvent * event);

public slots:
    void setAngle(int i);

private:
    void renderBackground(void);
    void renderNeedle(qreal angle);

    QSvgRenderer *renderer;
    QPixmap       bg;
    qreal         angle;

};

#endif // BASICDIAL_H
