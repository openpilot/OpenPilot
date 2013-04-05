#include "gpssnrwidget.h"

GpsSnrWidget::GpsSnrWidget(QWidget *parent) :
        QGraphicsView(parent) {

    scene = new QGraphicsScene(this);
    setScene(scene);

    // Now create 'maxSatellites' satellite icons which we will move around on the map:
    for (int i=0; i < MAX_SATTELITES;i++) {
        satellites[i][0] = 0;
        satellites[i][1] = 0;
        satellites[i][2] = 0;
        satellites[i][3] = 0;

        boxes[i] = new QGraphicsRectItem();
        boxes[i]->setBrush(QColor("Green"));
        scene->addItem(boxes[i]);
        boxes[i]->hide();

        satTexts[i] = new QGraphicsSimpleTextItem("##",boxes[i]);
        satTexts[i]->setBrush(QColor("Black"));
        satTexts[i]->setFont(QFont("Courier"));

        satSNRs[i] = new QGraphicsSimpleTextItem("##",boxes[i]);
        satSNRs[i]->setBrush(QColor("Black"));
        satSNRs[i]->setFont(QFont("Courier"));

    }

}

GpsSnrWidget::~GpsSnrWidget() {
    delete scene;
    scene = 0;
}

void GpsSnrWidget::showEvent(QShowEvent *event) {
    Q_UNUSED(event)
    scene->setSceneRect(0,0, this->viewport()->width(), this->viewport()->height());
    for(int index = 0 ;index < MAX_SATTELITES ; index++) {
        drawSat(index);
    }
}

void GpsSnrWidget::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
    scene->setSceneRect(0,0, this->viewport()->width(), this->viewport()->height());
    for(int index = 0 ;index < MAX_SATTELITES ; index++) {
        drawSat(index);
    }
}

void GpsSnrWidget::updateSat(int index, int prn, int elevation, int azimuth, int snr) {
    if (index >= MAX_SATTELITES) {
        // A bit of error checking never hurts.
        return;
    }

    // TODO: add range checking
    satellites[index][0] = prn;
    satellites[index][1] = elevation;
    satellites[index][2] = azimuth;
    satellites[index][3] = snr;

    drawSat(index);
}

void GpsSnrWidget::drawSat(int index) {
    if (index >= MAX_SATTELITES) {
        // A bit of error checking never hurts.
        return;
    }

    const int prn = satellites[index][0];
    const int snr = satellites[index][3];
    if (prn && snr) {
        boxes[index]->show();

        // When using integer values, width and height are the
        // box width and height, but the left and bottom borders are drawn on the box,
        // and the top and right borders are drawn just next to the box.
        // So the box seems one pixel wider and higher with a border.
        // I'm sure there's a good explanation for that :)

        // Casting to int rounds down, which is what I want.
        // Minus 2 to allow a pixel of white left and right.
        int availableWidth = (int)((scene->width()-2) / MAX_SATTELITES);

        // 2 pixels, one on each side.
        qreal width = availableWidth - 2;
        // SNR = 1-99 (0 is special)..
        qreal height = int((scene->height() / 99) * snr + 0.5);
        // 1 for showing a pixel of white to the left.
        qreal x = availableWidth * index + 1;
        // Rember, 0 is at the top.
        qreal y = scene->height() - height;
        // Compensate for the extra pixel for the border.
        boxes[index]->setRect(0,0,width-1,height-1);
        boxes[index]->setPos(x,y);

        QRectF boxRect = boxes[index]->boundingRect();
        QString prnString = QString().number(prn);
        if(prnString.length() == 1) {
            prnString = "0" + prnString;
        }
        satTexts[index]->setText(prnString);
        QRectF textRect = satTexts[index]->boundingRect();

        QTransform matrix;
        qreal scale = 0.85 * (boxRect.width() / textRect.width());
        matrix.translate( boxRect.width()/2, boxRect.height());
        matrix.scale(scale,scale);
        matrix.translate(-textRect.width()/2,-textRect.height());
        satTexts[index]->setTransform(matrix,false);

        QString snrString = QString().number(snr);
        if (snrString.length() ==1) { // Will probably never happen!
            snrString = "0" + snrString;
        }
        satSNRs[index]->setText(snrString);
        textRect = satSNRs[index]->boundingRect();

        matrix.reset();
        scale = 0.85 * (boxRect.width() / textRect.width());
        matrix.translate( boxRect.width()/2,0);
        matrix.scale(scale,scale);
        matrix.translate(-textRect.width()/2,-textRect.height());
        satSNRs[index]->setTransform(matrix,false);

    } else {
        boxes[index]->hide();
    }
}
