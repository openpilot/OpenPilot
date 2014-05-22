#ifndef INPUTCHANNELFORM_H
#define INPUTCHANNELFORM_H

#include "channelform.h"
#include "configinputwidget.h"

#include <QWidget>

namespace Ui {
class InputChannelForm;
}

class InputChannelForm : public ChannelForm {
    Q_OBJECT

public:
    explicit InputChannelForm(const int index, QWidget *parent = NULL);
    ~InputChannelForm();

    friend class ConfigInputWidget;

    virtual QString name();
    virtual void setName(const QString &name);

private slots:
    void minMaxUpdated();
    void neutralUpdated();
    void reversedUpdated();
    void groupUpdated();

private:
    Ui::InputChannelForm *ui;
};

#endif // INPUTCHANNELFORM_H
