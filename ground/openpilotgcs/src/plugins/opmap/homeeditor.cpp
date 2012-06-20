#include "homeeditor.h"
#include "ui_homeeditor.h"

homeEditor::homeEditor(HomeItem *home, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::homeEditor),
    myhome(home)
{
    if(!home)
    {
        deleteLater();
        return;
    }
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose,true);
    ui->altitude->setValue(home->Altitude());
    ui->latitude->setValue(home->Coord().Lat());
    ui->longitude->setValue(home->Coord().Lng());
}

homeEditor::~homeEditor()
{
    delete ui;
}

void homeEditor::on_buttonBox_accepted()
{
    myhome->SetCoord(internals::PointLatLng(ui->latitude->value(),ui->longitude->value()));
    myhome->SetAltitude(ui->altitude->value());
}

void homeEditor::on_buttonBox_rejected()
{
    this->close();
}
