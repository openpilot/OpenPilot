#ifndef HOMEEDITOR_H
#define HOMEEDITOR_H

#include <QDialog>
#include "opmapcontrol/opmapcontrol.h"

using namespace mapcontrol;

namespace Ui {
class homeEditor;
}

class homeEditor : public QDialog
{
    Q_OBJECT
    
public:
    explicit homeEditor(HomeItem * home,QWidget *parent = 0);
    ~homeEditor();
    
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::homeEditor *ui;
    HomeItem * myhome;
};

#endif // HOMEEDITOR_H
