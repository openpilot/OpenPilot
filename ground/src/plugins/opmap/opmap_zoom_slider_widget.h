#ifndef OPMAP_ZOOM_SLIDER_WIDGET_H
#define OPMAP_ZOOM_SLIDER_WIDGET_H

#include <QWidget>

namespace Ui {
    class opmap_zoom_slider_widget;
}

class opmap_zoom_slider_widget : public QWidget
{
    Q_OBJECT

public:
    explicit opmap_zoom_slider_widget(QWidget *parent = 0);
    ~opmap_zoom_slider_widget();

private:
    Ui::opmap_zoom_slider_widget *ui;
};

#endif // OPMAP_ZOOM_SLIDER_WIDGET_H
