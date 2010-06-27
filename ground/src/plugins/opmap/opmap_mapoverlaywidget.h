#ifndef OPMAP_MAPOVERLAYWIDGET_H
#define OPMAP_MAPOVERLAYWIDGET_H

#include <QWidget>

namespace Ui {
    class OPMap_MapOverlayWidget;
}

class OPMap_MapOverlayWidget : public QWidget {
    Q_OBJECT
public:
    OPMap_MapOverlayWidget(QWidget *parent = 0);
    ~OPMap_MapOverlayWidget();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::OPMap_MapOverlayWidget *ui;

private slots:
    void on_dial_sliderMoved(int position);
    void on_verticalSlider_sliderMoved(int position);
};

#endif // OPMAP_MAPOVERLAYWIDGET_H
