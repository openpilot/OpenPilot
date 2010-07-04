#include "opmap_statusbar_widget.h"
#include "ui_opmap_statusbar_widget.h"

opmap_statusbar_widget::opmap_statusbar_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::opmap_statusbar_widget)
{
    ui->setupUi(this);
}

opmap_statusbar_widget::~opmap_statusbar_widget()
{
    delete ui;
}
