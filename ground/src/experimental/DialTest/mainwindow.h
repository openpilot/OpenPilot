#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QDebug>

#include "basicdial.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    BasicDial      *dial;

private slots:
    void on_buttonTest_clicked();
    void on_buttonQuit_clicked();
};

#endif // MAINWINDOW_H
