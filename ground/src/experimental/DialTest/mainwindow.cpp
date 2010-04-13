#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dial = new BasicSvgDial(this);
    ui->dialLayout->addWidget(dial);

    ui->spinBottomAngle->setValue(30);
    ui->spinTopAngle->setValue(330);
    ui->spinBottomValue->setValue(0);
    ui->spinTopValue->setValue(100);
    ui->lineFile->setText(QString("bg.svg"));
    ui->lineFileForeground->setText(QString("fg.svg"));
    ui->lineFileNeedle->setText(QString("need.svg"));
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
    dial->setForegroundFile(ui->lineFileForeground->text());
    dial->setNeedleFile(ui->lineFileNeedle->text());
    dial->setValue(dial->getValue());


    ui->sliderValue->setRange(ui->spinBottomValue->value(), ui->spinTopValue->value());
    ui->statusBar->showMessage(QString("Dial settings updated!"));

}

void MainWindow::on_sliderValue_valueChanged(int value)
{
    dial->setValue((qreal)value);
    ui->statusBar->showMessage(QString().sprintf("Value: %f", dial->getValue()));
}


void MainWindow::on_buttonBackground_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "~", tr("SVG Files (*.svg)"));
    ui->lineFile->setText(fileName);
}

void MainWindow::on_buttonNeedle_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "~", tr("SVG Files (*.svg)"));
    ui->lineFileNeedle->setText(fileName);
}

void MainWindow::on_buttonForeground_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "~", tr("SVG Files (*.svg)"));
    ui->lineFileForeground->setText(fileName);
}

