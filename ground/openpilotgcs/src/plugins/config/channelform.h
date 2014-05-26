#ifndef CHANNELFORM_H
#define CHANNELFORM_H

#include "configtaskwidget.h"

#include <QWidget>

namespace Ui {
class ChannelForm;
}

class QGridLayout;

class ChannelForm : public ConfigTaskWidget {
    Q_OBJECT

public:
    explicit ChannelForm(const int index, QWidget *parent = 0);
    ~ChannelForm();

    int index() const;

    virtual QString name() = 0;
    virtual void setName(const QString &name) = 0;

    void moveTo(QGridLayout &dstLayout);

private:
    // Channel index
    int m_index;

    static void moveRow(int row, QGridLayout &srcLayout, QGridLayout &dstLayout);
    static void removeRow(int row, QGridLayout &layout);
};

#endif // CHANNELFORM_H
