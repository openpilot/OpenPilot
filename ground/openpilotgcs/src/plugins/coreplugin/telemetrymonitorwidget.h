#ifndef TELEMETRYMONITORWIDGET_H
#define TELEMETRYMONITORWIDGET_H

#include <QWidget>
#include <QObject>
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>
#include <QtCore/QPointer>

class TelemetryMonitorWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit TelemetryMonitorWidget(QWidget *parent = 0);
    ~TelemetryMonitorWidget();

    void setMin(double min) { minValue = min;}
    double getMin() { return minValue; }
    void setMax(double max) { maxValue = max;}
    double getMax() { return maxValue; }

    //number of tx/rx nodes in the graph
    static const int NODE_NUMELEM = 7;

signals:
    
public slots:
    void connect();
    void disconnect();

    void updateTelemetry(double txRate, double rxRate);
    void showTelemetry();

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
   QGraphicsSvgItem *graph;
   QPointer<QGraphicsTextItem> txSpeed;
   QPointer<QGraphicsTextItem> rxSpeed;
   QList<QGraphicsSvgItem*> txNodes;
   QList<QGraphicsSvgItem*> rxNodes;

   bool   connected;
   double txIndex;
   double txValue;
   double rxIndex;
   double rxValue;
   double minValue;
   double maxValue;
};

#endif // TELEMETRYMONITORWIDGET_H
