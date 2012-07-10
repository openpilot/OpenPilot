#include "endpage.h"
#include "ui_endpage.h"

EndPage::EndPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::EndPage)
{
    setFinalPage(true);
    ui->setupUi(this);
}

EndPage::~EndPage()
{
    delete ui;
}
