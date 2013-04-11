#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "../mapwidget/opmapwidget.h"
#include "../mapwidget/waypointitem.h"
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    mapcontrol::OPMapWidget *map;
    mapcontrol::WayPointItem* wp;
    QImage ima;
    QLabel l;

private slots:
    void on_pushButton_2_clicked();
    void on_comboBox_currentIndexChanged(QString );
    void on_checkBox_2_clicked(bool checked);
    void on_pushButtonGO_clicked();
    void on_pushButton_clicked();
    void on_pushButtonRR_clicked();
    void on_pushButtonRC_clicked();
    void on_pushButtonRL_clicked();
    void on_checkBox_clicked(bool checked);
    void on_pushButtonZoomM_clicked();
    void on_pushButtonZoomP_clicked();
    void zoomChanged(double zoomt,double zoom,double zoomdigi);
    void ttl(int value);
    void time();
};

#endif // MAINWINDOW_H
