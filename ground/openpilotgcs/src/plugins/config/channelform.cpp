#include "channelform.h"

#include <QGridLayout>


ChannelForm::ChannelForm(const int index, QWidget *parent) : ConfigTaskWidget(parent), m_index(index)
{}

ChannelForm::~ChannelForm()
{}

int ChannelForm::index() const
{
    return m_index;
}

void ChannelForm::moveTo(QGridLayout &dstLayout)
{
    QGridLayout *srcLayout = dynamic_cast<QGridLayout *>(layout());

    Q_ASSERT(srcLayout);

    // if we are the first row to be inserted then show the legend
    bool showLegend = (dstLayout.rowCount() == 1);

    if (showLegend) {
        // move legend row to target grid layout
        moveRow(0, *srcLayout, dstLayout);
    } else {
        removeRow(0, *srcLayout);
    }

    // move field row to target grid layout
    moveRow(1, *srcLayout, dstLayout);

    // this form is now empty so hide it
    setVisible(false);
}

void ChannelForm::moveRow(int row, QGridLayout &srcLayout, QGridLayout &dstLayout)
{
    int dstRow = dstLayout.rowCount();

    for (int col = 0; col < srcLayout.columnCount(); col++) {
        QLayoutItem *item = srcLayout.itemAtPosition(row, col);
        if (!item) {
            continue;
        }
        QWidget *widget = item->widget();
        if (widget) {
            dstLayout.addWidget(widget, dstRow, col);
            continue;
        }
        // TODO handle item of type QLayout
    }
}

void ChannelForm::removeRow(int row, QGridLayout &layout)
{
    for (int col = 0; col < layout.columnCount(); col++) {
        QLayoutItem *item = layout.itemAtPosition(row, col);
        if (!item) {
            continue;
        }
        QWidget *widget = item->widget();
        if (widget) {
            layout.removeWidget(widget);
            delete widget;
            continue;
        }
        // TODO handle item of type QLayout
    }
}
