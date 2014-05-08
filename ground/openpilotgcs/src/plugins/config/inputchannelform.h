#ifndef INPUTCHANNELFORM_H
#define INPUTCHANNELFORM_H

#include "configinputwidget.h"

#include <QWidget>

namespace Ui {
class InputChannelForm;
}

class InputChannelForm : public ConfigTaskWidget {
    Q_OBJECT

public:
    explicit InputChannelForm(QWidget *parent = 0, const bool showLegend = false);
    ~InputChannelForm();
    friend class ConfigInputWidget;
    void setName(QString &name);
private slots:
    void minMaxUpdated();
    void neutralUpdated();
    void reversedUpdated();
    void groupUpdated();

private:
    Ui::InputChannelForm *ui;
};

#endif // INPUTCHANNELFORM_H
