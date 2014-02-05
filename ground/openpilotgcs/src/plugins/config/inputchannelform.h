#ifndef INPUTCHANNELFORM_H
#define INPUTCHANNELFORM_H

#include <QWidget>
#include "configinputwidget.h"
namespace Ui {
class InputChannelForm;
}

class InputChannelForm : public ConfigTaskWidget {
    Q_OBJECT

public:
    explicit InputChannelForm(QWidget *parent = 0, bool showlegend = false);
    ~InputChannelForm();
    friend class ConfigInputWidget;
    void setName(QString &name);
private slots:
    void minMaxUpdated();
    void neutralUpdated();
    void reversedUpdated();
    void groupUpdated();
    void channelDropdownUpdated(int);
    void channelNumberUpdated(int);

private:
    Ui::InputChannelForm *ui;
};

#endif // INPUTCHANNELFORM_H
