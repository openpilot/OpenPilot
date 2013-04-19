#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "../mapwidget/mapgraphicitem.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    map=new mapcontrol::OPMapWidget();
    map->SetShowUAV(true);
    map->SetShowHome(true);
    ui->setupUi(this);
    ui->comboBox->addItems(mapcontrol::Helper::MapTypes());
    ui->comboBox->setCurrentIndex(mapcontrol::Helper::MapTypes().indexOf("GoogleHybrid"));
    QHBoxLayout *layout=new QHBoxLayout(parent);
    layout->addWidget(map);
    layout->addWidget(ui->widget);
    ui->centralWidget->setLayout(layout);
    QTimer * timer=new QTimer;
    timer->setInterval(500);
    timer->start();
    connect(map,SIGNAL(zoomChanged(double,double,double)),this,SLOT(zoomChanged(double,double,double)));
    connect(map,SIGNAL(OnTilesStillToLoad(int)),this,SLOT(ttl(int)));
    connect(timer,SIGNAL(timeout()),this,SLOT(time()));
    map->WPCreate(internals::PointLatLng(38.42,-9.5),0,"lisbon");
    map->SetMouseWheelZoomType(internals::MouseWheelZoomType::MousePositionAndCenter);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete map;
}

void MainWindow::ttl(int value)
{
    ui->label_2->setText(QString::number(value));
}

void MainWindow::time()
{
    internals::PointLatLng ll;
    ll.SetLat(map->currentMousePosition().Lat());
    ll.SetLng(map->currentMousePosition().Lng());
   // map->UAV->SetUAVPos(ll,10);
    ui->label_2->setText(map->currentMousePosition().ToString());
    QGraphicsItem* itemm=map->itemAt(map->mapFromGlobal(QCursor::pos()));
    mapcontrol::WayPointItem* w=qgraphicsitem_cast<mapcontrol::WayPointItem*>(itemm);
    //if(w)
      //  qDebug()<<itemm->Type;
    if (itemm->Type==mapcontrol::WayPointItem::Type)
    {
        int x=itemm->Type;
        int xx= x;
        QLabel* l=new QLabel(this);
        l->show();
    }
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
  double x=map->ZoomTotal();
  double y=ui->doubleSpinBox->value();
  double z=x+y;
    map->SetZoom(map->ZoomTotal()+ui->doubleSpinBox->value());
}

void MainWindow::on_pushButtonZoomM_clicked()
{
    map->SetZoom(map->ZoomTotal()-ui->doubleSpinBox->value());
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    map->SetShowTileGridLines(checked);
}
void MainWindow::zoomChanged(double zoomt,double zoom,double zoomdigi)
{
    ui->label_5->setText("CurrentZoom="+QString::number(zoomt)+QString::number(zoom)+QString::number(zoomdigi));
}

void MainWindow::on_pushButtonRL_clicked()
{
    map->UAV->DeleteTrail();
   // map->UAV->SetUAVHeading(10);
    //map->SetRotate(map->Rotate()-1);
    //map->WPCreate();
  //  map->WPCreate(internals::PointLatLng(20,20),100);
}

void MainWindow::on_pushButtonRC_clicked()
{
     map->SetRotate(0);
//    wp=map->WPInsert(1);
}

void MainWindow::on_pushButtonRR_clicked()
{
     map->SetRotate(map->Rotate()+1);
   // map->WPDeleteAll();
   // map->WPDelete(wp);
//    QList<mapcontrol::WayPointItem*> list= map->WPSelected();
//    int x=list.at(0)->Number();
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
    map->RipMap();
//    QLabel *x=new QLabel();
//   ima=map->X();
//   x->setPixmap(QPixmap::fromImage(ima));
//   x->show();
}
