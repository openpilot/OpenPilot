#ifndef OPMAPCONTROL_H
#define OPMAPCONTROL_H

#include "../internals/core.h"
#include <QtGui>
#include <QTransform>
#include <QWidget>
#include <QBrush>
#include <QFont>
class OPMapControl:public QWidget
{
    Q_OBJECT

public:
    OPMapControl(QWidget *parent=0);
    QBrush EmptytileBrush;
    QString EmptyTileText;
    QPen EmptyTileBorders;
    bool ShowTileGridLines;
protected:
    void paintEvent ( QPaintEvent* evnt );
    void mousePressEvent ( QMouseEvent* evnt );
    void mouseReleaseEvent ( QMouseEvent* evnt );
    void mouseMoveEvent ( QMouseEvent* evnt );
    void resizeEvent ( QResizeEvent * event );
private:
    Core core;
    qreal MapRenderTransform;
    void DrawMap2D(QPainter &painter);
    QFont MissingDataFont;
    void resize();
private slots:
    void Core_OnNeedInvalidation();
};

#endif // OPMAPCONTROL_H
