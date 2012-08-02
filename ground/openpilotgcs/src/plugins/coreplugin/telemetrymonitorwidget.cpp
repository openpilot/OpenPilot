#include "telemetrymonitorwidget.h"

#include <QObject>
#include <QtGui>
#include <QDebug>

TelemetryMonitorWidget::TelemetryMonitorWidget(QWidget *parent) : QGraphicsView(parent)
{
    setMinimumSize(160,60);
    setMaximumSize(160,60);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setBackgroundBrush(Qt::transparent);

    QGraphicsScene *scene = new QGraphicsScene(0,0,160,60, this);
    scene->setBackgroundBrush(Qt::transparent);

    QSvgRenderer *renderer = new QSvgRenderer();
    if (renderer->load(QString(":/core/images/tx-rx.svg"))) {
        graph = new QGraphicsSvgItem();
        graph->setSharedRenderer(renderer);
        graph->setElementId("txrxBackground");

        QString name;
        QGraphicsSvgItem* pt;

        for (int i=0; i<NODE_NUMELEM; i++) {
            name = QString("tx%0").arg(i);
            if (renderer->elementExists(name)) {
                pt = new QGraphicsSvgItem();
                pt->setSharedRenderer(renderer);
                pt->setElementId(name);
                pt->setParentItem(graph);
                txNodes.append(pt);
            }

            name = QString("rx%0").arg(i);
            if (renderer->elementExists(name)) {
                pt = new QGraphicsSvgItem();
                pt->setSharedRenderer(renderer);
                pt->setElementId(name);
                pt->setParentItem(graph);
                rxNodes.append(pt);
            }
        }

        scene->addItem(graph);
        scene->setSceneRect(graph->boundingRect());
        setScene(scene);
    }

    connected = false;
    txValue = 0.0;
    rxValue = 0.0;

    setMin(0.0);
    setMax(1200.0);

    showTelemetry();
}

TelemetryMonitorWidget::~TelemetryMonitorWidget()
{
    while (!txNodes.isEmpty())
        delete txNodes.takeFirst();

    while (!rxNodes.isEmpty())
        delete rxNodes.takeFirst();
}

void TelemetryMonitorWidget::connect()
{
    connected = true;

    //flash the lights
    updateTelemetry(maxValue, maxValue);
}

void TelemetryMonitorWidget::disconnect()
{
    //flash the lights
    updateTelemetry(maxValue, maxValue);

    connected = false;
    updateTelemetry(0.0,0.0);
}
/*!
  \brief Called by the UAVObject which got updated

  Updates the numeric value and/or the icon if the dial wants this.
  */
void TelemetryMonitorWidget::updateTelemetry(double txRate, double rxRate)
{
    txValue = txRate;
    rxValue = rxRate;

    showTelemetry();
}

// Converts the value into an percentage:
// this enables smooth movement in moveIndex below
void TelemetryMonitorWidget::showTelemetry()
{
    txIndex = (txValue-minValue)/(maxValue-minValue) * NODE_NUMELEM;
    rxIndex = (rxValue-minValue)/(maxValue-minValue) * NODE_NUMELEM;

    if (connected)
        this->setToolTip(QString("Tx: %0 bytes/sec\nRx: %1 bytes/sec").arg(txValue).arg(rxValue));
    else
        this->setToolTip(QString("Disconnected"));

    QGraphicsItem* txNode;
    QGraphicsItem* rxNode;

    for (int i=0; i < NODE_NUMELEM; i++) {
        txNode = txNodes.at(i);
        txNode->setPos((i*(txNode->boundingRect().width() + 8)) + 8, (txNode->boundingRect().height()/2) - 2);
        txNode->setVisible(connected && i < txIndex);
        txNode->update();

        rxNode = rxNodes.at(i);
        rxNode->setPos((i*(rxNode->boundingRect().width() + 8)) + 8, (rxNode->boundingRect().height()*2) - 2);
        rxNode->setVisible(connected && i < rxIndex);
        rxNode->update();
    }
    update();
}

void TelemetryMonitorWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    fitInView(graph, Qt::KeepAspectRatio);
}

void TelemetryMonitorWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);

    graph->setPos(0,-90);
    fitInView(graph, Qt::KeepAspectRatio);
}

