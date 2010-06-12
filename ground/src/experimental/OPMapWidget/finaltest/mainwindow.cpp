#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    map=new mapcontrol::OPMapWidget();
    ui->setupUi(this);
    ui->comboBox->addItems(mapcontrol::Helper::MapTypes());
    ui->comboBox->setCurrentIndex(mapcontrol::Helper::MapTypes().indexOf("GoogleHybrid"));
    QHBoxLayout *layout=new QHBoxLayout(parent);
    layout->addWidget(map);
    layout->addWidget(ui->widget);
    ui->centralWidget->setLayout(layout);
    connect(map,SIGNAL(zoomChanged(double)),this,SLOT(zoomChanged(double)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete map;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButtonZoomP_clicked()
{
    double x,y;
    x=map->Zoom();
    y=ui->doubleSpinBox->value();
    map->SetZoom(map->Zoom()+ui->doubleSpinBox->value());
}

void MainWindow::on_pushButtonZoomM_clicked()
{
    map->SetZoom(map->Zoom()-ui->doubleSpinBox->value());
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    map->SetShowTileGridLines(checked);
}
void MainWindow::zoomChanged(double zoom)
{
    ui->label_5->setText("CurrentZoom="+QString::number(zoom));
}

void MainWindow::on_pushButtonRL_clicked()
{
    map->SetRotate(map->Rotate()-1);
}

void MainWindow::on_pushButtonRC_clicked()
{
     map->SetRotate(0);
}

void MainWindow::on_pushButtonRR_clicked()
{
     map->SetRotate(map->Rotate()+1);
}

void MainWindow::on_pushButton_clicked()
{
    map->ReloadMap();
}

void MainWindow::on_pushButtonGO_clicked()
{
    core::GeoCoderStatusCode::Types x=map->SetCurrentPositionByKeywords(ui->lineEdit->text());
    ui->label->setText( mapcontrol::Helper::StrFromGeoCoderStatusCode(x));

}

void MainWindow::on_checkBox_2_clicked(bool checked)
{
    map->SetUseOpenGL(checked);
}

void MainWindow::on_comboBox_currentIndexChanged(QString value)
{
    if (map->isStarted())
        map->SetMapType(mapcontrol::Helper::MapTypeFromString(value));
}

void MainWindow::on_pushButton_2_clicked()
{
    map->X();
}
