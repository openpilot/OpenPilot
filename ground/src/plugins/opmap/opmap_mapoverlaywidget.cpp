#include "opmap_mapoverlaywidget.h"
#include "ui_opmap_mapoverlaywidget.h"

OPMap_MapOverlayWidget::OPMap_MapOverlayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OPMap_MapOverlayWidget)
{
    ui->setupUi(this);
}

OPMap_MapOverlayWidget::~OPMap_MapOverlayWidget()
{
    delete ui;
}

void OPMap_MapOverlayWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void OPMap_MapOverlayWidget::on_verticalSlider_sliderMoved(int position)
{

}

void OPMap_MapOverlayWidget::on_dial_sliderMoved(int position)
{

}
