#include "pathplanner.h"
#include "ui_pathplanner.h"
#include "widgetdelegates.h"
#include <QAbstractItemModel>
#include <QFileDialog>

pathPlanner::pathPlanner(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::testTable),wid(NULL),myModel(NULL)
{
    ui->setupUi(this);
}

pathPlanner::~pathPlanner()
{
    delete ui;
    if(wid)
        delete wid;
}
void pathPlanner::setModel(flightDataModel *model,QItemSelectionModel *selection)
{
    myModel=model;
    ui->tableView->setModel(model);
    ui->tableView->setSelectionModel(selection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setItemDelegate(new ComboBoxDelegate(this));
    connect(model,SIGNAL(rowsInserted(const QModelIndex&,int,int)),this,SLOT(on_rowsInserted(const QModelIndex&,int,int)));
    wid=new opmap_edit_waypoint_dialog(NULL,model,selection);
    ui->tableView->resizeColumnsToContents();
}

void pathPlanner::on_rowsInserted ( const QModelIndex & parent, int start, int end )
{
    Q_UNUSED(parent);
    for(int x=start;x<end+1;x++)
    {
        QModelIndex index=ui->tableView->model()->index(x,flightDataModel::MODE);
        ui->tableView->openPersistentEditor(index);
        index=ui->tableView->model()->index(x,flightDataModel::CONDITION);
        ui->tableView->openPersistentEditor(index);
        index=ui->tableView->model()->index(x,flightDataModel::COMMAND);
        ui->tableView->openPersistentEditor(index);
        ui->tableView->size().setHeight(10);
    }
}

void pathPlanner::on_tbAdd_clicked()
{
    ui->tableView->model()->insertRow(ui->tableView->model()->rowCount());
}

void pathPlanner::on_tbDelete_clicked()
{

}

void pathPlanner::on_tbInsert_clicked()
{
    ui->tableView->model()->insertRow(ui->tableView->selectionModel()->currentIndex().row());
}

void pathPlanner::on_tbReadFromFile_clicked()
{
    if(!myModel)
        return;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    myModel->readFromFile(fileName);
}

void pathPlanner::on_tbSaveToFile_clicked()
{
    if(!myModel)
        return;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));
    myModel->writeToFile(fileName);
}

void pathPlanner::on_groupBox_clicked()
{

}

void pathPlanner::on_groupBox_toggled(bool arg1)
{
   // wid->close();
    wid->setVisible(arg1);
}

void pathPlanner::on_tbDetails_clicked()
{
    if(wid)
       wid->show();
}

void pathPlanner::on_tbSendToUAV_clicked()
{
    emit sendPathPlanToUAV();
}

void pathPlanner::on_tbFetchFromUAV_clicked()
{
    emit receivePathPlanFromUAV();
}
