#include "monitorwidget.h"

#include <utils/stylehelper.h>

#include <QObject>
#include <QDebug>
#include <QtGui/QFont>

namespace {
/**
 * Create an SVG item and connect it to an element of the SVG file previously loaded into the parent item.
 * This then allows to show, hide, move, scale and rotate the element.
 * Opacity can also be changed.
 * Other characteristics (color, ...) of the element cannot be modified.
 */
// TODO move to some utility class that can be reused by other SVG manipulating code
QGraphicsSvgItem *createSvgItem(QGraphicsSvgItem *parent, QString elementId)
{
    QGraphicsSvgItem *item = new QGraphicsSvgItem(parent);

    QSvgRenderer *renderer = parent->renderer();

    // connect item to its corresponding element
    item->setSharedRenderer(renderer);
    item->setElementId(elementId);

    // move item to its location
    QMatrix elementMatrix = renderer->matrixForElement(elementId);
    QRectF elementRect    = elementMatrix.mapRect(renderer->boundsOnElement(elementId));
    item->setPos(elementRect.x(), elementRect.y());

    return item;
}

/**
 * Create a text item based on a svg rectangle.
 * The rectangle must be in the correct location (hint: use a "text" layer on top of the "background layer")
 * The font size will be set to match as well as possible the rectangle height but it is not guaranteed.
 *
 * It is possible to show the text rectangle to help understand layout issues.
 *
 */
// TODO move to some utility class that can be reused by other SVG manipulating code
QGraphicsTextItem *createTextItem(QGraphicsSvgItem *parent, QString elementId, QString fontName,
                                  bool showRect = false)
{
    if (showRect) {
        // create and display the text rectangle
        // needs to be done first otherwise the rectangle will blank out the text.
        createSvgItem(parent, elementId);
    }

    QGraphicsTextItem *item = new QGraphicsTextItem();

    QSvgRenderer *renderer  = parent->renderer();

    // move new text item to location of rectangle element
    QMatrix elementMatrix   = renderer->matrixForElement(elementId);
    QRectF elementRect = elementMatrix.mapRect(renderer->boundsOnElement(elementId));

    qreal fontPointSizeF    = elementRect.height();

    QTransform matrix;
    matrix.translate(elementRect.x(), elementRect.y() - (fontPointSizeF / 2.0));

    item->setParentItem(parent);
    item->setTransform(matrix, false);
    // to right align or center text we must provide a text width
    // item->setTextWidth(elementRect.width());

    // create font to match the rectangle height
    // there is not guaranteed that all fonts will play well...
    QFont font(fontName);
    // not sure if PreferMatch helps to get the correct font size (i.e. that fits the text rectangle nicely)
    font.setStyleStrategy(QFont::PreferMatch);
    font.setPointSizeF(fontPointSizeF);

    item->setFont(font);

#ifdef DEBUG_FONT
    // just in case
    qDebug() << "Font point size: " << fontPointSizeF;
    qDebug() << "Font pixel size: " << font.pixelSize();
    qDebug() << "Font point size: " << font.pointSize();
    qDebug() << "Font point size F: " << font.pointSizeF();
    qDebug() << "Font exact match: " << font.exactMatch();

    QFontInfo fontInfo(font);
    qDebug() << "Font info pixel size: " << fontInfo.pixelSize();
    qDebug() << "Font info point size: " << fontInfo.pointSize();
    qDebug() << "Font info point size F: " << fontInfo.pointSizeF();
    qDebug() << "Font info exact match: " << fontInfo.exactMatch();
#endif
    return item;
}
} // anonymous namespace

MonitorWidget::MonitorWidget(QWidget *parent) :
    QGraphicsView(parent), aspectRatioMode(Qt::KeepAspectRatio)
{
    // setMinimumWidth(180);

    QGraphicsScene *scene = new QGraphicsScene();

    setScene(scene);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    // no scroll bars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setBackgroundBrush(QBrush(Utils::StyleHelper::baseColor()));

    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QSvgRenderer *renderer = new QSvgRenderer();

    if (renderer->load(QString(":/telemetry/images/tx-rx.svg"))) {
        // create graph
        graph = new QGraphicsSvgItem();
        graph->setSharedRenderer(renderer);
        graph->setElementId("background");

        graph->setFlags(QGraphicsItem::ItemClipsChildrenToShape | QGraphicsItem::ItemClipsToShape);

        scene->addItem(graph);

        int i;

        // create tx nodes
        i = 0;
        while (true) {
            QString id   = QString("tx%0").arg(i);
            QString bgId = QString("tx_bg%0").arg(i);
            if (!renderer->elementExists(id) || !renderer->elementExists(bgId)) {
                break;
            }
            QGraphicsSvgItem *item = createSvgItem(graph, bgId);
            item->setElementId(id);
            txNodes.append(item);
            i++;
        }

        // create rx nodes
        i = 0;
        while (true) {
            QString id   = QString("rx%0").arg(i);
            QString bgId = QString("rx_bg%0").arg(i);
            if (!renderer->elementExists(id) || !renderer->elementExists(bgId)) {
                break;
            }
            QGraphicsSvgItem *item = createSvgItem(graph, bgId);
            item->setElementId(id);
            rxNodes.append(item);
            i++;
        }

        if (renderer->elementExists("txSpeed")) {
            txSpeed = createTextItem(graph, "txSpeed", "Helvetica");
            txSpeed->setDefaultTextColor(Qt::white);
        } else {
            txSpeed = NULL;
        }

        if (renderer->elementExists("rxSpeed")) {
            rxSpeed = createTextItem(graph, "rxSpeed", "Helvetica");
            rxSpeed->setDefaultTextColor(Qt::white);
        } else {
            rxSpeed = NULL;
        }
        // scene->setSceneRect(graph->boundingRect());
    }

    connected = false;

    setMin(0.0);
    setMax(1200.0);

    telemetryUpdated(0.0, 0.0);
}

MonitorWidget::~MonitorWidget()
{
    while (!txNodes.isEmpty()) {
        delete txNodes.takeFirst();
    }
    while (!rxNodes.isEmpty()) {
        delete rxNodes.takeFirst();
    }
    if (txSpeed) {
        delete txSpeed;
    }
    if (rxSpeed) {
        delete rxSpeed;
    }
}

/*!
   \brief Enables/Disables OpenGL
 */
// void LineardialGadgetWidget::enableOpenGL(bool flag)
// {
// if (flag) {
// setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
// } else {
// setViewport(new QWidget);
// }
// }

void MonitorWidget::telemetryConnected()
{
    qDebug() << "telemetry connected";
    if (!connected) {
        // flash the lights
        setToolTip(tr("Connected"));
        telemetryUpdated(maxValue, maxValue);
        connected = true;
    }
}

void MonitorWidget::telemetryDisconnected()
{
    qDebug() << "telemetry disconnected";
    if (connected) {
        connected = false;

        setToolTip(tr("Disconnected"));

        // flash the lights???
        telemetryUpdated(maxValue, maxValue);

        telemetryUpdated(0.0, 0.0);
    }
}

/*!
   \brief Called by the UAVObject which got updated

   Updates the numeric value and/or the icon if the dial wants this.
 */
void MonitorWidget::telemetryUpdated(double txRate, double rxRate)
{
    double txIndex = (txRate - minValue) / (maxValue - minValue) * txNodes.count();
    double rxIndex = (rxRate - minValue) / (maxValue - minValue) * rxNodes.count();

    if (connected) {
        this->setToolTip(QString("Tx: %0 bytes/s, Rx: %1 bytes/s").arg(txRate).arg(rxRate));
    }

    for (int i = 0; i < txNodes.count(); i++) {
        QGraphicsItem *node = txNodes.at(i);
        bool visible = ( /*connected &&*/ (i < txIndex));
        if (visible != node->isVisible()) {
            node->setVisible(visible);
            node->update();
        }
    }

    for (int i = 0; i < rxNodes.count(); i++) {
        QGraphicsItem *node = rxNodes.at(i);
        bool visible = ( /*connected &&*/ (i < rxIndex));
        if (visible != node->isVisible()) {
            node->setVisible(visible);
            node->update();
        }
    }

    if (txSpeed) {
        if (connected) {
            txSpeed->setPlainText(QString("%0").arg(txRate));
        }
        txSpeed->setVisible(connected);
        txSpeed->update();
    }

    if (rxSpeed) {
        if (connected) {
            rxSpeed->setPlainText(QString("%0").arg(rxRate));
        }
        rxSpeed->setVisible(connected);
        rxSpeed->update();
    }

    update();
}

void MonitorWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    fitInView(graph, aspectRatioMode);
}

void MonitorWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    fitInView(graph, aspectRatioMode);
}
