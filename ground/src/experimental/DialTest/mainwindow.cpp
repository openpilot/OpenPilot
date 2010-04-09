#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dial = new BasicDial(this);
    ui->dialLayout->addWidget(dial);

    connect(ui->sliderAngle, SIGNAL(valueChanged(int)), dial, SLOT(setAngle(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonQuit_clicked()
{
    close();
}

void MainWindow::on_buttonTest_clicked()
{
    qDebug() << "Test!";
}
