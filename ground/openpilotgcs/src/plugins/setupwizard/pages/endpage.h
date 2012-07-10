#ifndef ENDPAGE_H
#define ENDPAGE_H

#include <QWizardPage>

namespace Ui {
class EndPage;
}

class EndPage : public QWizardPage
{
    Q_OBJECT
    
public:
    explicit EndPage(QWidget *parent = 0);
    ~EndPage();
    
private:
    Ui::EndPage *ui;
};

#endif // ENDPAGE_H
