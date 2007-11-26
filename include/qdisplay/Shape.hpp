#include <QGraphicsItem>

namespace jafar {
namespace qdisplay {
class Viewer;
class Shape : public QGraphicsItemGroup {
  public:
    enum ShapeType {
      ShapeRectangle,
      ShapeCross,
      ShapeEllipse
    };
    /**
      * Create an Shape to use with a Viewer to display a basic shape.
      * @param shapeType the type of the shape to display
      */
    Shape(ShapeType shapeType, double x, double y, double w, double h);
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
    inline void setPos(double x, double y) { QGraphicsItem::setPos(x,y); }
    void setLabel(char * text);
    void setFontSize(int s) { m_fontSize = s; }
  private:
    ShapeType m_shapeType;
    QRectF m_boundingRect;
    QColor m_color;
    QGraphicsTextItem* m_label;
    int m_fontSize;
};

}

}

