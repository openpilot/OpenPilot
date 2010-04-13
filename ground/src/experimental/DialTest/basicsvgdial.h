#ifndef BASICSVGDIAL_H
#define BASICSVGDIAL_H

#include <QGraphicsView>

class BasicSvgDial : public QGraphicsView
{
    Q_OBJECT

public:
    enum RendererType { Native, OpenGL, Image};
    BasicSvgDial(QWidget *parent = 0);
    void setRenderer(RendererType type = Native);

    void setAngles(qreal bottom, qreal top);
    void setRange(qreal bottom, qreal top);
    qreal getValue(void);
    void setBackgroundFile(QString file);
    void setForegroundFile(QString file);
    void setNeedleFile(QString file);

public slots:
    void  setAngle(int i);
    void  setValue(qreal value);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void  renderBackground(void);
    void  renderForeground(void);
    void  renderNeedle(qreal angle);
    qreal value2angle(qreal value);

    RendererType m_renderer;
    QGraphicsItem *m_backgroundItem;
    QGraphicsItem *m_foregroundItem;
    QGraphicsItem *m_needleItem;

    QRectF calculateCenteredFrame(void);

    qreal         angle;
    QString       backgroundFile, foregroundFile, needleFile;

    qreal         topValue, topAngle;
    qreal         bottomValue, bottomAngle;
    qreal         angleSpan;
    qreal         currentValue;
};

#endif // BASICDIAL_H
