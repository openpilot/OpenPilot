#include <QGraphicsItem>

namespace jafar {
namespace qdisplay {
class Viewer;
class ShapeItem : public QGraphicsPixmapItem {
  public:
    enum ShapeType {
      ShapeRectangle,
      ShapeCross
    };
    /**
      * Create an ShapeItem to use with a Viewer to display a basic shape.
      * @param shapeType the type of the shape to display
      */
    ShapeItem(ShapeType shapeType, double x, double y, double w, double h);
    virtual QRectF boundingRect () const { return m_boundingRect; } 
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    /**
     * Set the color of the line of the shape
     * @param r red [0 to 255]
     * @param g green [0 to 255]
     * @param b blue [0 to 255]
     */
    inline void setColor(int r, int g, int b) { m_color.setRgb(r,g,b); }
    /**
     * Set the position of the shape
     * @param x
     * @param y
     */
    inline void setPos(double x, double y) { QGraphicsPixmapItem::setPos(x,y); }
  private:
    QRectF m_boundingRect;
    ShapeType m_shapeType;
    QColor m_color;
};

}

}

