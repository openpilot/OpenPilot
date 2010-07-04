#ifndef OPMAP_OVERLAY_WIDGET_H
#define OPMAP_OVERLAY_WIDGET_H

#include <QWidget>

namespace Ui {
    class opmap_overlay_widget;
}

class opmap_overlay_widget : public QWidget
{
    Q_OBJECT

public:
    explicit opmap_overlay_widget(QWidget *parent = 0);
    ~opmap_overlay_widget();

private:
    Ui::opmap_overlay_widget *ui;
};

#endif // OPMAP_OVERLAY_WIDGET_H
