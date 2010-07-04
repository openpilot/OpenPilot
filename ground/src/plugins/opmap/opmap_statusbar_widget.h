#ifndef OPMAP_STATUSBAR_WIDGET_H
#define OPMAP_STATUSBAR_WIDGET_H

#include <QWidget>

namespace Ui {
    class opmap_statusbar_widget;
}

class opmap_statusbar_widget : public QWidget
{
    Q_OBJECT

public:
    explicit opmap_statusbar_widget(QWidget *parent = 0);
    ~opmap_statusbar_widget();

private:
    Ui::opmap_statusbar_widget *ui;
};

#endif // OPMAP_STATUSBAR_WIDGET_H
