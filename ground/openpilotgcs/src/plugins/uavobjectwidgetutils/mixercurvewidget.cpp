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
    Node* node = getCommandNode(0);    
    node->setName("Reset");
    node->setToggle(false);
    node->setPositiveColor("#ffffff", "#ffffff");  //white
    node->setNegativeColor("#ffffff", "#ffffff");
    scene->addItem(node);

    // linear
    node = getCommandNode(1);
    node->setName("Linear");
    node->commandText("/");
    scene->addItem(node);

    // log
    node = getCommandNode(2);
    node->setName("Log");
    node->commandText("(");
    scene->addItem(node);

    // exp
    node = getCommandNode(3);
    node->setName("Exp");
    node->commandText(")");
    scene->addItem(node);

    // flat
    node = getCommandNode(4);
    node->setName("Flat");
    node->commandText("--");
    scene->addItem(node);

    // step
    node = getCommandNode(5);
    node->setName("Step");
    node->commandText("z");
    scene->addItem(node);


    // curve min/max nodes
    node = getCommandNode(6);
    node->setName("MinPlus");
    node->setToggle(false);
    node->setPositiveColor("#00ff00", "#00ff00");   //green
    node->setNegativeColor("#00ff00", "#00ff00");
    node->commandText("+");
    scene->addItem(node);

    node = getCommandNode(7);
    node->setName("MinMinus");
    node->setToggle(false);
    node->setPositiveColor("#ff0000", "#ff0000");   //red
    node->setNegativeColor("#ff0000", "#ff0000");
    node->commandText("-");
    scene->addItem(node);

    node = getCommandNode(8);
    node->setName("MaxPlus");
    node->setToggle(false);
    node->setPositiveColor("#00ff00", "#00ff00");   //green
    node->setNegativeColor("#00ff00", "#00ff00");
    node->commandText("+");
    scene->addItem(node);

    node = getCommandNode(9);
    node->setName("MaxMinus");
    node->setToggle(false);
    node->setPositiveColor("#ff0000", "#ff0000");   //red
    node->setNegativeColor("#ff0000", "#ff0000");
    node->commandText("-");
    scene->addItem(node);

    node = getCommandNode(10);
    node->setName("StepPlus");
    node->setToggle(false);
    node->setPositiveColor("#00ff00", "#00ff00");   //green
    node->setNegativeColor("#00ff00", "#00ff00");
    node->commandText("+");
    scene->addItem(node);

    node = getCommandNode(11);
    node->setName("StepMinus");
    node->setToggle(false);
    node->setPositiveColor("#ff0000", "#ff0000");   //red
    node->setNegativeColor("#ff0000", "#ff0000");
    node->commandText("-");
    scene->addItem(node);

    node = getCommandNode(12);
    node->setName("StepValue");
    node->setToggle(false);
    node->setPositiveColor("#0000ff", "#0000ff");  //blue
    node->setNegativeColor("#0000ff", "#0000ff");
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

Node* MixerCurveWidget::getCommandNode(int index)
{
    Node* node;

    if (index >= 0 && index < cmdNodePool.count())
    {
        node = cmdNodePool.at(index);
    }
    else {
        node = new Node(this);        
        node->commandNode(true);
        node->commandText("");
        node->setCommandIndex(index);
        node->setActive(false);        
        node->setPositiveColor("#ff0000", "#ff0000");
        node->setNegativeColor("#0000cc", "#0000cc");
        cmdNodePool.append(node);
    }
    return node;
}

Node* MixerCurveWidget::getNode(int index)
{
    Node* node;

    if (index >= 0 && index < nodePool.count())
    {
        node = nodePool.at(index);
    }
    else {
        node = new Node(this);
        nodePool.append(node);
    }
    return node;
}
Edge* MixerCurveWidget::getEdge(int index, Node* sourceNode, Node* destNode)
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
        Node* node = nodePool.at(i);
        node->setPositiveColor(color0, color1);
    }
}
void MixerCurveWidget::setNegativeColor(QString color0, QString color1)
{
    negColor0 = color0;
    negColor1 = color1;
    for (int i=0; i<nodePool.count(); i++) {
        Node* node = nodePool.at(i);
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
        foreach (Node *node, nodeList ) {
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
    Node* prevNode = 0;
    for (int i=0; i<numPoints; i++) {

        Node *node = getNode(i);

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

    foreach(Node *node, nodeList) {
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

        Node* node = nodeList.at(i);
        node->setPos(w*i, h - (val*h));
        node->verticalMove(true);

        node->setPositiveColor(posColor0, posColor1);
        node->setNegativeColor(negColor0, negColor1);

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

    Node* node;
    for (int i = 0; i<6; i++) {
        node = getCommandNode(i);

        node->setPos((rect.left() + rect.width() / 2) - 60 + (i * 20), rect.height() + 10);
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
    node->setPos(rect.bottomRight().x() - 60, rect.bottomRight().y() + 8);

    //stepminus
    node = getCommandNode(11);
    node->setPos(rect.bottomRight().x() - 40, rect.bottomRight().y() + 8);

    //step
    node = getCommandNode(12);
    node->setPos(rect.bottomRight().x() - 20, rect.bottomRight().y() + 8);
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

Node* MixerCurveWidget::getCmdNode(const QString& name)
{
    Node* node = 0;
    for (int i=0; i<cmdNodePool.count(); i++) {
        Node* n = cmdNodePool.at(i);
        if (n->getName() == name)
            node = n;
    }
    return node;
}

void MixerCurveWidget::setCommandText(const QString& name, const QString& text)
{
    for (int i=0; i<cmdNodePool.count(); i++) {
        Node* n = cmdNodePool.at(i);
        if (n->getName() == name) {
            n->commandText(text);
            n->update();
        }
    }
}
void MixerCurveWidget::activateCommand(const QString& name)
{
    for (int i=0; i<cmdNodePool.count(); i++) {
        Node* node = cmdNodePool.at(i);
        node->setActive(node->getName() == name);
        node->update();
    }
}

void MixerCurveWidget::cmdActivated(Node* node)
{
    if (node->getToggle()) {
        for (int i=0; i<cmdNodePool.count(); i++) {
            Node* n = cmdNodePool.at(i);
            n->setActive(false);
            n->update();
        }

        node->setActive(true);
    }
    node->update();
    emit commandActivated(node);
}

