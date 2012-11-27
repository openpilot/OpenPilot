#include "geofencedialog.h"
#include "ui_geofencedialog.h"

GeofenceDialog::GeofenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeofenceDialog)
{
    ui->setupUi(this);
}

GeofenceDialog::~GeofenceDialog()
{
    delete ui;
}

void GeofenceDialog::setModel(GeofenceDataModel *model, QItemSelectionModel *selection)
{
    dataModel=model;
    ui->tableView->setModel(model);
    ui->tableView->setSelectionModel(selection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(dataModel,SIGNAL(rowsInserted(const QModelIndex&,int,int)),this,SLOT(rowsInserted(const QModelIndex&,int,int)));
    ui->tableView->resizeColumnsToContents();
}

void GeofenceDialog::rowsInserted(const QModelIndex &parent, int start, int end)
{
}
