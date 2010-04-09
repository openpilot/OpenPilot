#ifndef BASICDIAL_H
#define BASICDIAL_H

#include <QSvgWidget>
#include <QSvgRenderer>
#include <QPen>

class BasicDial : public QWidget
{
    Q_OBJECT

public:
    BasicDial(QWidget *parent);
    virtual void paintEvent(QPaintEvent * event);

    void setAngles(qreal bottom, qreal top);
    void setRange(qreal bottom, qreal top);
    qreal getValue(void);
    void setBackgroundFile(QString file);

    void setPen(QPen p); //scrap later

public slots:
    void  setAngle(int i);
    void  setValue(qreal value);

private:
    void  renderBackground(void);
    void  renderNeedle(qreal angle);
    qreal value2angle(qreal value);

    QSvgRenderer *renderer;
    QPixmap       bg;
    qreal         angle;
    QColor        needleColor;
    QString       backgroundFile;
    QPen          pen; // scrap later

    qreal         topValue, topAngle;
    qreal         bottomValue, bottomAngle;
    qreal         angleSpan;
    qreal         currentValue;
};

#endif // BASICDIAL_H
