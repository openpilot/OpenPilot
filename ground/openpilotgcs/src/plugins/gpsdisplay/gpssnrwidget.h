#ifndef GPSSNRWIDGET_H
#define GPSSNRWIDGET_H

#include <QGraphicsView>
#include <QtGui/QGraphicsRectItem>

class GpsSnrWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GpsSnrWidget(QWidget *parent = 0);
    ~GpsSnrWidget();

signals:

public slots:
    void updateSat(int index, int prn, int elevation, int azimuth, int snr);

private:
    static const int MAX_SATTELITES = 16;
    int satellites[MAX_SATTELITES][4];
    QGraphicsScene *scene;
    QGraphicsRectItem *boxes[MAX_SATTELITES];
    QGraphicsSimpleTextItem* satTexts[MAX_SATTELITES];
    QGraphicsSimpleTextItem* satSNRs[MAX_SATTELITES];

    void drawSat(int index);

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

};

#endif // GPSSNRWIDGET_H
