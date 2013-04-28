#ifndef MONITORWIDGET_H
#define MONITORWIDGET_H

#include <QWidget>
#include <QObject>
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>
#include <QtCore/QPointer>

class MonitorWidget: public QGraphicsView {
Q_OBJECT
public:
    explicit MonitorWidget(QWidget *parent = 0);
    ~MonitorWidget();

    void setMin(double min)
    {
        minValue = min;
    }

    double getMin()
    {
        return minValue;
    }

    void setMax(double max)
    {
        maxValue = max;
    }

    double getMax()
    {
        return maxValue;
    }

public slots:
    void telemetryConnected();
    void telemetryDisconnected();
    void telemetryUpdated(double txRate, double rxRate);

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    bool connected;

    double minValue;
    double maxValue;

    Qt::AspectRatioMode aspectRatioMode;

    QGraphicsSvgItem *graph;

    QPointer<QGraphicsTextItem> txSpeed;
    QPointer<QGraphicsTextItem> rxSpeed;

    QList<QGraphicsSvgItem*> txNodes;
    QList<QGraphicsSvgItem*> rxNodes;
};

#endif // MONITORWIDGET_H
