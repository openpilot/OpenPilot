/**
 ******************************************************************************
 *
 * @file       mixercurvewidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectWidgetUtils Plugin
 * @{
 * @brief Utility plugin for UAVObject to Widget relation management
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "mixercurvewidget.h"
#include "mixercurveline.h"
#include "mixercurvepoint.h"

#include <QObject>
#include <QtGui>
#include <QDebug>

/*
 * Initialize the widget
 */
MixerCurveWidget::MixerCurveWidget(QWidget *parent) : QGraphicsView(parent)
{

    // Create a layout, add a QGraphicsView and put the SVG inside.
    // The Mixer Curve widget looks like this:
    // |--------------------|
    // |                    |
    // |                    |
    // |     Graph  |
    // |                    |
    // |                    |
    // |                    |
    // |--------------------|


    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);

    curveMin=0.0;
    curveMax=1.0;

    setFrameStyle(QFrame::NoFrame);
    setStyleSheet("background:transparent");

    QGraphicsScene *scene = new QGraphicsScene(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    plot = new QGraphicsSvgItem();
    renderer->load(QString(":/configgadget/images/curve-bg.svg"));
    plot->setSharedRenderer(renderer);
    //plot->setElementId("map");
    scene->addItem(plot);
    plot->setZValue(-1);

    scene->setSceneRect(plot->boundingRect());
    setScene(scene);

    posColor0 = "#1c870b";  //greenish?
    posColor1 = "#116703";  //greenish?
    negColor0 = "#ff0000";  //red
    negColor1 = "#ff0000";  //red

    // commmand nodes
    // reset
    MixerNode* node = getCommandNode(0);
    node->setName("Reset");
    node->setToolTip("Reset Curve to Defaults");
    node->setToggle(false);
    node->setPositiveColor("#ffffff", "#ffffff");  //white
    node->setNegativeColor("#ffffff", "#ffffff");
    scene->addItem(node);

    // linear
    node = getCommandNode(1);
    node->setName("Linear");
    node->setToolTip("Generate a Linear Curve");
    QImage img = QImage(":/core/images/curve_linear.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("/");

    scene->addItem(node);

    // log
    node = getCommandNode(2);
    node->setName("Log");
    node->setToolTip("Generate a Logarithmic Curve");
    img = QImage(":/core/images/curve_log.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("(");
    scene->addItem(node);

    // exp
    node = getCommandNode(3);
    node->setName("Exp");
    node->setToolTip("Generate an Exponential Curve");
    img = QImage(":/core/images/curve_exp.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText(")");
    scene->addItem(node);

    // flat
    node = getCommandNode(4);
    node->setName("Flat");
    node->setToolTip("Generate a Flat Curve");
    img = QImage(":/core/images/curve_flat.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("--");
    scene->addItem(node);

    // step
    node = getCommandNode(5);
    node->setName("Step");
    node->setToolTip("Generate a Stepped Curve");
    img = QImage(":/core/images/curve_step.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("z");
    scene->addItem(node);


    // curve min/max nodes
    node = getCommandNode(6);
    node->setName("MinPlus");
    node->setToolTip("Increase Curve Minimum");
    img = QImage(":/core/images/curve_plus.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("+");
    node->setToggle(false);
    node->setPositiveColor("#00aa00", "#00aa00");   //green
    node->setNegativeColor("#00aa00", "#00aa00");
    scene->addItem(node);

    node = getCommandNode(7);
    node->setName("MinMinus");
    node->setToolTip("Decrease Curve Minimum");
    img = QImage(":/core/images/curve_minus.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("-");

    node->setToggle(false);
    node->setPositiveColor("#aa0000", "#aa0000");   //red
    node->setNegativeColor("#aa0000", "#aa0000");
    scene->addItem(node);

    node = getCommandNode(8);
    node->setName("MaxPlus");
    node->setToolTip("Increase Curve Maximum");
    img = QImage(":/core/images/curve_plus.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("+");

    node->setToggle(false);
    node->setPositiveColor("#00aa00", "#00aa00");   //green
    node->setNegativeColor("#00aa00", "#00aa00");
    scene->addItem(node);

    node = getCommandNode(9);
    node->setName("MaxMinus");
    node->setToolTip("Decrease Curve Maximum");
    img = QImage(":/core/images/curve_plus.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("-");

    node->setToggle(false);
    node->setPositiveColor("#aa0000", "#aa0000");   //red
    node->setNegativeColor("#aa0000", "#aa0000");
    scene->addItem(node);

    node = getCommandNode(10);
    node->setName("StepPlus");
    node->setToolTip("Increase Step/Power Value");
    img = QImage(":/core/images/curve_plus.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("+");
    node->setToggle(false);
    node->setPositiveColor("#00aa00", "#00aa00");   //green
    node->setNegativeColor("#00aa00", "#00aa00");
    scene->addItem(node);

    node = getCommandNode(11);
    node->setName("StepMinus");
    node->setToolTip("Decrease Step/Power Value");
    img = QImage(":/core/images/curve_minus.png");
    if (!img.isNull())
        node->setImage(img);
    else
        node->commandText("-");

    node->setToggle(false);
    node->setPositiveColor("#aa0000", "#aa0000");   //red
    node->setNegativeColor("#aa0000", "#aa0000");
    scene->addItem(node);

    node = getCommandNode(12);
    node->setName("StepValue");
    node->setDrawNode(false);
    node->setToolTip("Current Step/Power Value");
    node->setToggle(false);
    node->setPositiveColor("#0000aa", "#0000aa");  //blue
    node->setNegativeColor("#0000aa", "#0000aa");
    scene->addItem(node);

    // commands toggle
    node = getCommandNode(13);
    node->setName("Commands");
    node->setToolTip("Toggle Command Buttons On/Off");
    node->setToggle(true);
    node->setPositiveColor("#00aa00", "#00aa00");  //greenish
    node->setNegativeColor("#000000", "#000000");
    scene->addItem(node);

    // popup
    node = getCommandNode(14);
    node->setName("Popup");
    node->setToolTip("Advanced Mode...");
    node->commandText("");
    node->setToggle(false);
    node->setPositiveColor("#ff0000", "#ff0000");  //red
    node->setNegativeColor("#ff0000", "#ff0000");
    scene->addItem(node);

    resizeCommands();

    initNodes(MixerCurveWidget::NODE_NUMELEM);

}

MixerCurveWidget::~MixerCurveWidget()
{
    while (!nodePool.isEmpty())
        delete nodePool.takeFirst();

    while (!edgePool.isEmpty())
        delete edgePool.takeFirst();

    while (!cmdNodePool.isEmpty())
        delete cmdNodePool.takeFirst();
}

MixerNode* MixerCurveWidget::getCommandNode(int index)
{
    MixerNode* node;

    if (index >= 0 && index < cmdNodePool.count())
    {
        node = cmdNodePool.at(index);
    }
    else {
        node = new MixerNode(this);
        node->commandNode(true);
        node->commandText("");
        node->setCommandIndex(index);
        node->setActive(false);        
        node->setPositiveColor("#aaaa00", "#aaaa00");
        node->setNegativeColor("#1c870b", "#116703");
        cmdNodePool.append(node);
    }
    return node;

}

MixerNode* MixerCurveWidget::getNode(int index)
{
    MixerNode* node;

    if (index >= 0 && index < nodePool.count())
    {
        node = nodePool.at(index);
    }
    else {
        node = new MixerNode(this);
        nodePool.append(node);
    }
    return node;
}
Edge* MixerCurveWidget::getEdge(int index, MixerNode* sourceNode, MixerNode* destNode)
{
    Edge* edge;

    if (index >= 0 && index < edgePool.count())
    {
        edge = edgePool.at(index);
        edge->setSourceNode(sourceNode);
        edge->setDestNode(destNode);
    }
    else {
        edge = new Edge(sourceNode,destNode);
        edgePool.append(edge);
    }
    return edge;
}

void MixerCurveWidget::setPositiveColor(QString color0, QString color1)
{
    posColor0 = color0;
    posColor1 = color1;
    for (int i=0; i<nodePool.count(); i++) {
        MixerNode* node = nodePool.at(i);
        node->setPositiveColor(color0, color1);
    }
}
void MixerCurveWidget::setNegativeColor(QString color0, QString color1)
{
    negColor0 = color0;
    negColor1 = color1;
    for (int i=0; i<nodePool.count(); i++) {
        MixerNode* node = nodePool.at(i);
        node->setNegativeColor(color0, color1);
    }
}


/**
  Init curve: create a (flat) curve with a specified number of points.

  If a curve exists already, resets it.
  Points should be between 0 and 1.
  */
void MixerCurveWidget::initCurve(const QList<double>* points)
{
    if (points->length() < 2)
        return; // We need at least 2 points on a curve!

    // finally, set node positions
    setCurve(points);
}

void MixerCurveWidget::initNodes(int numPoints)
{
    // First of all, clear any existing list
    if (nodeList.count()) {
        foreach (MixerNode *node, nodeList ) {
            foreach(Edge *edge, node->edges()) {
                if (edge->sourceNode() == node) {
                    scene()->removeItem(edge);
                }
            }
            scene()->removeItem(node);
        }

        nodeList.clear();
    }

    // Create the nodes and edges
    MixerNode* prevNode = 0;
    for (int i=0; i<numPoints; i++) {

        MixerNode *node = getNode(i);

        nodeList.append(node);
        scene()->addItem(node);

        node->setPos(0,0);

        if (prevNode) {
            scene()->addItem(getEdge(i, prevNode, node));
        }

        prevNode = node;
    }
}

/**
  Returns the current curve settings
  */
QList<double> MixerCurveWidget::getCurve() {

    QList<double> list;

    foreach(MixerNode *node, nodeList) {
        list.append(node->value());
    }

    return list;
}
/**
  Sets a linear graph
  */
void MixerCurveWidget::initLinearCurve(int numPoints, double maxValue, double minValue)
{
    double range = maxValue - minValue; // setRange(minValue, maxValue);

    QList<double> points;
    for (double i=0; i < (double)numPoints; i++) {
        double val = (range * ( i / (double)(numPoints-1) ) ) + minValue;
        points.append(val);
    }
    initCurve(&points);
}
/**
  Setd the current curve settings
  */
void MixerCurveWidget::setCurve(const QList<double>* points)
{
    curveUpdating = true;

    int ptCnt = points->count();
    if (nodeList.count() != ptCnt)
        initNodes(ptCnt);

    double range = curveMax - curveMin;

    qreal w = plot->boundingRect().width()/(ptCnt-1);
    qreal h = plot->boundingRect().height();
    for (int i=0; i<ptCnt; i++) {

        double val = (points->at(i) < curveMin) ? curveMin : (points->at(i) > curveMax) ? curveMax : points->at(i);

        val += range;
        val -= (curveMin + range);
        val /= range;

        MixerNode* node = nodeList.at(i);
        node->setPos(w*i, h - (val*h));
        node->verticalMove(true);

        node->update();
    }
    curveUpdating = false;

    update();

    emit curveUpdated();
}


void MixerCurveWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.

    QRectF rect = plot->boundingRect();
    resizeCommands();
    fitInView(rect.adjusted(-15,-15,15,15), Qt::KeepAspectRatio);
}

void MixerCurveWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);

    QRectF rect = plot->boundingRect();
    resizeCommands();
    fitInView(rect.adjusted(-15,-15,15,15), Qt::KeepAspectRatio);
}

void MixerCurveWidget::resizeCommands()
{
    QRectF rect = plot->boundingRect();

    MixerNode* node;
    //popup
    node = getCommandNode(14);
    node->setPos((rect.left() + rect.width() / 2) - 20, rect.height() + 10);

    //reset
    node = getCommandNode(0);
    node->setPos((rect.left() + rect.width() / 2) + 20, rect.height() + 10);

    //commands on/off
    node = getCommandNode(13);
    node->setPos(rect.right() - 15, rect.bottomRight().x() - 14);

    for (int i = 1; i<6; i++) {
        node = getCommandNode(i);

        //bottom right of widget
        node->setPos(rect.right() - 130 + (i * 18), rect.bottomRight().x() - 14);
    }

    //curveminplus
    node = getCommandNode(6);
    node->setPos(rect.bottomLeft().x() + 15, rect.bottomLeft().y() - 10);

    //curveminminus
    node = getCommandNode(7);
    node->setPos(rect.bottomLeft().x() + 15, rect.bottomLeft().y() + 5);

    //curvemaxplus
    node = getCommandNode(8);
    node->setPos(rect.topRight().x() - 20, rect.topRight().y() - 7);

    //curvemaxminus
    node = getCommandNode(9);
    node->setPos(rect.topRight().x() - 20, rect.topRight().y() + 8);

    //stepplus
    node = getCommandNode(10);
    node->setPos(rect.bottomRight().x() - 40, rect.bottomRight().y() + 5);

    //stepminus
    node = getCommandNode(11);
    node->setPos(rect.bottomRight().x() - 40, rect.bottomRight().y() + 15);

    //step
    node = getCommandNode(12);
    node->setPos(rect.bottomRight().x() - 22, rect.bottomRight().y() + 9);
}

void MixerCurveWidget::itemMoved(double itemValue)
{
    Q_UNUSED(itemValue);

    if (!curveUpdating) {
        emit curveUpdated();
    }
}

void MixerCurveWidget::setMin(double value)
{
    if (curveMin != value)
        emit curveMinChanged(value);

    curveMin = value;
}

void MixerCurveWidget::setMax(double value)
{
    if (curveMax != value)
        emit curveMaxChanged(value);

    curveMax = value;
}

double MixerCurveWidget::getMin()
{
    return curveMin;
}
double MixerCurveWidget::getMax()
{
    return curveMax;
}
double MixerCurveWidget::setRange(double min, double max)
{
    curveMin = min;
    curveMax = max;
    return curveMax - curveMin;
}

MixerNode* MixerCurveWidget::getCmdNode(const QString& name)
{
    MixerNode* node = 0;
    for (int i=0; i<cmdNodePool.count(); i++) {
        MixerNode* n = cmdNodePool.at(i);
        if (n->getName() == name)
            node = n;
    }
    return node;
}

void MixerCurveWidget::setCommandText(const QString& name, const QString& text)
{
    for (int i=0; i<cmdNodePool.count(); i++) {
        MixerNode* n = cmdNodePool.at(i);
        if (n->getName() == name) {
            n->commandText(text);
            n->update();
        }
    }
}
void MixerCurveWidget::activateCommand(const QString& name)
{
    for (int i=1; i<cmdNodePool.count()-2; i++) {
        MixerNode* node = cmdNodePool.at(i);
        node->setCommandActive(node->getName() == name);
        node->update();
    }
}

void MixerCurveWidget::showCommand(const QString& name, bool show)
{
    MixerNode* node = getCmdNode(name);
    if (node) {
        if (show)
            node->show();
        else
            node->hide();
    }
}
void MixerCurveWidget::showCommands(bool show)
{
    for (int i=1; i<cmdNodePool.count()-2; i++) {
        MixerNode* node = cmdNodePool.at(i);
        if (show)
            node->show();
        else
            node->hide();

        node->update();
    }
}
bool MixerCurveWidget::isCommandActive(const QString& name)
{
    bool active = false;
    MixerNode* node = getCmdNode(name);
    if (node) {
        active = node->getCommandActive();
    }
    return active;
}

void MixerCurveWidget::cmdActivated(MixerNode* node)
{
    if (node->getToggle()) {
        if (node->getName() == "Commands") {
            node->setCommandActive(!node->getCommandActive());
            showCommands(node->getCommandActive());
        }
        else {
            for (int i=1; i<cmdNodePool.count()-2; i++) {
                MixerNode* n = cmdNodePool.at(i);
                n->setCommandActive(false);
                n->update();
            }

            node->setCommandActive(true);
        }

    }
    node->update();
    emit commandActivated(node);
}

