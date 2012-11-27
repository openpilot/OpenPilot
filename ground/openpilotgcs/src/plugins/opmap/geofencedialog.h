#ifndef GEOFENCEDIALOG_H
#define GEOFENCEDIALOG_H

#include <QDialog>
#include <QItemSelectionModel>

#include "geofencedatamodel.h"

namespace Ui {
class GeofenceDialog;
}

class GeofenceDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit GeofenceDialog(QWidget *parent = 0);
    ~GeofenceDialog();

    void setModel(GeofenceDataModel *model,QItemSelectionModel *selection);

private slots:
    void rowsInserted ( const QModelIndex & parent, int start, int end );
    
private:
    Ui::GeofenceDialog *ui;

    GeofenceDataModel *dataModel;
};

#endif // GEOFENCEDIALOG_H
