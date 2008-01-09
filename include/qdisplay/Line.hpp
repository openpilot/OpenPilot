#ifndef _LINE_HPP_
#define _LINE_HPP_

#include <QGraphicsLineItem>

namespace jafar {
namespace qdisplay {

class Line : public QGraphicsLineItem {
  public:
    /**
      * Create an Line to use with a Viewer to display a basic shape.
      */
    Line(double x1, double y1, double x2, double y2);
    /**
     * Set the color of the line of the shape
     * @param r red [0 to 255]
     * @param g green [0 to 255]
     * @param b blue [0 to 255]
     */
    void setColor(int r, int g, int b);
    /**
     * Set the position of the shape
     * @param x
     * @param y
     */
    inline void setPos(double x, double y) { QGraphicsLineItem::setPos(x,y); }
};

}

}

#endif
