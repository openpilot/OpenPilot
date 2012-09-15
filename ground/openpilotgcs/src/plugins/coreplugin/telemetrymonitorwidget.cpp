#include "telemetrymonitorwidget.h"

#include <QObject>
#include <QtGui>
#include <QtGui/QFont>
#include <QDebug>

TelemetryMonitorWidget::TelemetryMonitorWidget(QWidget *parent) : QGraphicsView(parent)
{
    setMinimumSize(180,100);
    setMaximumSize(180,100);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignCenter);
    setFrameStyle(QFrame::NoFrame);
    setStyleSheet("background:transparent;");
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);

    QGraphicsScene *scene = new QGraphicsScene(0,0,180,100, this);

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

        txSpeed = new QGraphicsTextItem();
        txSpeed->setDefaultTextColor(Qt::white);
        txSpeed->setFont(QFont("Helvetica",22,2));
        txSpeed->setParentItem(graph);
        scene->addItem(txSpeed);

        rxSpeed = new QGraphicsTextItem();
        rxSpeed->setDefaultTextColor(Qt::white);
        rxSpeed->setFont(QFont("Helvetica",22,2));
        rxSpeed->setParentItem(graph);
        scene->addItem(rxSpeed);

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

    int i;
    int nodeMargin = 8;
    int leftMargin = 60;
    QGraphicsItem* node;

    for (i=0; i < txNodes.count(); i++) {
        node = txNodes.at(i);
        node->setPos((i*(node->boundingRect().width() + nodeMargin)) + leftMargin, (node->boundingRect().height()/2) - 2);
        node->setVisible(connected && i < txIndex);
        node->update();
    }

    for (i=0; i < rxNodes.count(); i++) {
        node = rxNodes.at(i);
        node->setPos((i*(node->boundingRect().width() + nodeMargin)) + leftMargin, (node->boundingRect().height()*2) - 2);
        node->setVisible(connected && i < rxIndex);
        node->update();
    }

    QRectF rect = graph->boundingRect();
    txSpeed->setPos(rect.right() - 110, rect.top());
    txSpeed->setPlainText(QString("%0").arg(txValue));
    txSpeed->setVisible(connected);

    rxSpeed->setPos(rect.right() - 110, rect.top() + (rect.height() / 2));
    rxSpeed->setPlainText(QString("%0").arg(rxValue));
    rxSpeed->setVisible(connected);

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

    graph->setPos(0,-130);
    fitInView(graph, Qt::KeepAspectRatio);
}

