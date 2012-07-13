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

    connect(m_closeButton,SIGNAL(clicked()), this, SLOT(closePopup()));
}

void PopupWidget::setWidget(QWidget* widget)
{
    m_widget = widget;
    m_widgetParent = widget->parentWidget();

    m_layout->addWidget(m_widget);
}


void PopupWidget::closePopup()
{
    if (m_widget && m_widgetParent) {
        if(QGroupBox * w =qobject_cast<QGroupBox *>(m_widgetParent))
        {
            w->layout()->addWidget(m_widget);
        }
    }

    close();
}
