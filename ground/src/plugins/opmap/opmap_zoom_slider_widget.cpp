#include "opmap_zoom_slider_widget.h"
#include "ui_opmap_zoom_slider_widget.h"

opmap_zoom_slider_widget::opmap_zoom_slider_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::opmap_zoom_slider_widget)
{
    ui->setupUi(this);
}

opmap_zoom_slider_widget::~opmap_zoom_slider_widget()
{
    delete ui;
}
