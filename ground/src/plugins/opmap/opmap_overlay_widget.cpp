#include "opmap_overlay_widget.h"
#include "ui_opmap_overlay_widget.h"

opmap_overlay_widget::opmap_overlay_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::opmap_overlay_widget)
{
    ui->setupUi(this);
}

opmap_overlay_widget::~opmap_overlay_widget()
{
    delete ui;
}
