#ifndef TESTTABLE_H
#define TESTTABLE_H

#include <QWidget>
#include "flightdatamodel.h"
#include "opmap_edit_waypoint_dialog.h"
namespace Ui {
class testTable;
}

class pathPlanner : public QWidget
{
    Q_OBJECT
    
public:
    explicit pathPlanner(QWidget *parent = 0);
    ~pathPlanner();
    
    void setModel(flightDataModel *model,QItemSelectionModel *selection);
private slots:
        void on_rowsInserted ( const QModelIndex & parent, int start, int end );

        void on_tbAdd_clicked();

        void on_tbDelete_clicked();

        void on_tbInsert_clicked();

        void on_tbReadFromFile_clicked();

        void on_tbSaveToFile_clicked();

        void on_groupBox_clicked();

        void on_groupBox_toggled(bool arg1);

        void on_tbDetails_clicked();

        void on_tbSendToUAV_clicked();

        void on_tbFetchFromUAV_clicked();

private:
    Ui::testTable *ui;
    opmap_edit_waypoint_dialog * wid;
    flightDataModel * myModel;
signals:
    void sendPathPlanToUAV();
    void receivePathPlanFromUAV();
};

#endif // TESTTABLE_H
