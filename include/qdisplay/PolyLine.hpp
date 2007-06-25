
#ifndef _POLYLINE_H_
#define _POLYLINE_H_

#include <QAbstractGraphicsShapeItem>

namespace jafar {
namespace qdisplay {

class PolyLine : public QAbstractGraphicsShapeItem
{
    public:
        PolyLine(double scale = 1.0, QGraphicsItem *parent = 0);
        ~PolyLine();
    
        /**
        * Set the color of the line of the shape
        * @param r red [0 to 255]
        * @param g green [0 to 255]
        * @param b blue [0 to 255]
        */
        void setColor(int r, int g, int b);
        
        void addPoint(double x, double y);
        
        QRectF boundingRect() const;
        QPainterPath shape() const;
        
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        
    private:
        QVector<QPointF> m_points;
        mutable QRectF m_boundingRect;
        double m_scale;
};

}
}

#endif
