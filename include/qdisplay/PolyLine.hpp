
#ifndef _POLYLINE_H_
#define _POLYLINE_H_

#include <QAbstractGraphicsShapeItem>

namespace jafar {
namespace qdisplay {

/**
 * @ingroup qdisplay
 * Allows to display a poly line on a viewer overlay. For instance to display the
 * path of a robot.
 */
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
        
        /**
         * Add a point at the end of the polyline
         */
        void addPoint(double x, double y);
        
        QRectF boundingRect() const;
        QPainterPath shape() const;
        
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        
        /**
         * @return the scale applied to the point of the polyline
         */
        double scale() const;
    private:
        QVector<QPointF> m_points;
        mutable QRectF m_boundingRect;
        double m_scale;
};

}
}

#endif
