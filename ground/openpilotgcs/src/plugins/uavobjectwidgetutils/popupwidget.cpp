#include "popupwidget.h"

#include <QtGui>

PopupWidget::PopupWidget(QWidget *parent) :
    QDialog(parent)
{    
    m_widget = 0;

    QVBoxLayout* mainLayout = new QVBoxLayout();

    m_layout = new QHBoxLayout();
    mainLayout->addLayout(m_layout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_closeButton = new QPushButton(tr("Close"));
    buttonLayout->addWidget(m_closeButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    connect(m_closeButton,SIGNAL(clicked()), this, SLOT(close()));
    connect(this, SIGNAL(accepted()),this,SLOT(close()));
    connect(this,SIGNAL(rejected()), this, SLOT(close()));
}

void PopupWidget::popUp(QWidget* widget)
{
    setWidget(widget);
    exec();
}

void PopupWidget::setWidget(QWidget* widget)
{
    m_widget = widget;
    m_widgetParent = widget->parentWidget();

    // save the current width,height so we can restore when closed
    m_widgetWidth = m_widget->width();
    m_widgetHeight = m_widget->height();

    // double the size of the widget for the dialog
    m_widget->resize(m_widgetWidth * 2, m_widgetHeight * 2);
    m_layout->addWidget(m_widget);
}

bool PopupWidget::close()
{
    closePopup();

    return QDialog::close();
}

void PopupWidget::done(int result)
{
    closePopup();

    QDialog::done(result);
}

void PopupWidget::closePopup()
{
    if (m_widget && m_widgetParent) {
        if(QGroupBox * w =qobject_cast<QGroupBox *>(m_widgetParent))
        {
            m_widget->resize(m_widgetWidth, m_widgetHeight);
            w->layout()->addWidget(m_widget);
        }
    }
}
