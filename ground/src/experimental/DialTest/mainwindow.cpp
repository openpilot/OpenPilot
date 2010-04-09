#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPen>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dial = new BasicDial(this);
    ui->dialLayout->addWidget(dial);

    ui->spinBottomAngle->setValue(30);
    ui->spinTopAngle->setValue(330);
    ui->spinBottomValue->setValue(0);
    ui->spinTopValue->setValue(100);
    ui->lineFile->setText(QString("bg.svg"));
    on_buttonUpdate_clicked();

    dial->setValue(0);
    ui->statusBar->showMessage(QString("Configure dial and hit Update! Set value with slider."));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonQuit_clicked()
{
    close();
}

void MainWindow::on_buttonUpdate_clicked()
{
    dial->setAngles(ui->spinBottomAngle->value(), ui->spinTopAngle->value());
    dial->setRange(ui->spinBottomValue->value(), ui->spinTopValue->value());
    dial->setBackgroundFile(ui->lineFile->text());
    dial->setValue(dial->getValue());

    QPen pen;
    if( ui->comboColor->currentText() == "red" ) pen.setColor(Qt::red);
    if( ui->comboColor->currentText() == "yellow" ) pen.setColor(Qt::yellow);
    if( ui->comboColor->currentText() == "black" ) pen.setColor(Qt::black);
    if( ui->comboColor->currentText() == "green" ) pen.setColor(Qt::green);
    if( ui->comboColor->currentText() == "blue" ) pen.setColor(Qt::blue);
    dial->setPen(pen);

    ui->sliderValue->setRange(ui->spinBottomValue->value(), ui->spinTopValue->value());
    ui->statusBar->showMessage(QString("Dial settings updated!"));

}

void MainWindow::on_sliderValue_valueChanged(int value)
{
    dial->setValue((qreal)value);
    ui->statusBar->showMessage(QString().sprintf("Value: %f", dial->getValue()));
}

void MainWindow::on_buttonFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "~", tr("SVG Files (*.svg)"));
    ui->lineFile->setText(fileName);
}
