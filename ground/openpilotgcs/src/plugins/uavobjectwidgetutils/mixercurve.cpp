#include "mixercurve.h"
#include "ui_mixercurve.h"

MixerCurve::MixerCurve(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MixerCurve)
{
    ui->setupUi(this);
}

MixerCurve::~MixerCurve()
{
    delete ui;
}
