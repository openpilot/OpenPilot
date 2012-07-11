#ifndef INPUTCHANNELFORM_H
#define INPUTCHANNELFORM_H

#include <QWidget>
#include "configinputwidget.h"
namespace Ui {
    class inputChannelForm;
}

class inputChannelForm : public ConfigTaskWidget
{
    Q_OBJECT

public:
    explicit inputChannelForm(QWidget *parent = 0,bool showlegend=false);
    ~inputChannelForm();
    friend class ConfigInputWidget;
    void setName(QString &name);
private slots:
    void minMaxUpdated();
    void neutralUpdated(int);
    void groupUpdated();
    void channelDropdownUpdated(int);
    void channelNumberUpdated(int);

private:
    Ui::inputChannelForm *ui;
};

#endif // INPUTCHANNELFORM_H
