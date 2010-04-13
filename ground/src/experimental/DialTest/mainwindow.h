#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QDebug>
#include <QFileDialog>

#include "basicsvgdial.h"

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
    BasicSvgDial      *dial;

private slots:
    void on_buttonBackground_clicked();
    void on_buttonNeedle_clicked();
    void on_buttonForeground_clicked();
    void on_sliderValue_valueChanged(int value);
    void on_buttonUpdate_clicked();
    void on_buttonQuit_clicked();
};

#endif // MAINWINDOW_H
