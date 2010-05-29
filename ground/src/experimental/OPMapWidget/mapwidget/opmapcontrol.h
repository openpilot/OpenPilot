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

    Q_PROPERTY(int MaxZoom READ MaxZoom WRITE SetMaxZoom)
    Q_PROPERTY(int MinZoom READ MinZoom WRITE SetMinZoom)
    Q_PROPERTY(MouseWheelZoomType::Types MouseWheelZoom READ GetMouseWheelZoomType WRITE SetMouseWheelZoomType)
    Q_PROPERTY(QString MouseWheelZoomStr READ GetMouseWheelZoomTypeStr WRITE SetMouseWheelZoomTypeByStr)

public:
    OPMapControl(QWidget *parent=0);
    QBrush EmptytileBrush;
    QString EmptyTileText;
    QPen EmptyTileBorders;
    bool ShowTileGridLines;
    int MaxZoom()const{return maxZoom;}
    void SetMaxZoom(int const& value){maxZoom = value;}
    int MinZoom()const{return minZoom;}
    void SetMinZoom(int const& value){minZoom = value;}
    MouseWheelZoomType::Types GetMouseWheelZoomType(){return core.GetMouseWheelZoomType();}
    void SetMouseWheelZoomType(MouseWheelZoomType::Types const& value){core.SetMouseWheelZoomType(value);}
    void SetMouseWheelZoomTypeByStr(const QString &value){core.SetMouseWheelZoomType(MouseWheelZoomType::TypeByStr(value));}
    QString GetMouseWheelZoomTypeStr(){return MouseWheelZoomType::TypesStrList().at((int)core.GetMouseWheelZoomType());}
protected:
    void paintEvent ( QPaintEvent* evnt );
    void mousePressEvent ( QMouseEvent* evnt );
    void mouseReleaseEvent ( QMouseEvent* evnt );
    void mouseMoveEvent ( QMouseEvent* evnt );
    void resizeEvent ( QResizeEvent * event );
    void showEvent ( QShowEvent * event );
private:
    Core core;
    qreal MapRenderTransform;
    void DrawMap2D(QPainter &painter);
    QFont MissingDataFont;
    void resize();
    int maxZoom;
    int minZoom;
private slots:
    void Core_OnNeedInvalidation();
};

#endif // OPMAPCONTROL_H
