#include <QAbstractListModel>
#include <QVariant>
#include <QIcon>
#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QtCore/QDebug>
#include "modestackmodel.h"
#include "modestack.h"


namespace Core{

ModeStack::ModeStack(QWidget *parent) :
    m_view(new QDeclarativeView(parent)),
    m_model(new ModeStackModel()),
    m_qmlObject(0)
{
    QDeclarativeContext *cntxt = m_view->rootContext();
    cntxt->setContextProperty("items", m_model);
    m_view->setSource(QUrl("qrc:/core/qml/ModeStack.qml"));
    m_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    m_qmlObject = m_view->rootObject();
    QObject::connect(m_qmlObject, SIGNAL(indexChanged(int)), this, SLOT(indexChanged(int)));
    qDebug() << "DSW: ModeStack created";
}

ModeStack::~ModeStack()
{
    delete m_view;
    delete m_model;
}

void ModeStack::setCurrentIndex(int index)
{
    m_qmlObject->setProperty("currentIndex", index);
    emit indexChanged(index);
}

void ModeStack::setItemIcon(int index, QIcon icon)
{
    m_model->setData(m_model->index(index), icon, ModeStackModel::IconRole);
}

void ModeStack::setItemText(int index, QString text)
{

}

void ModeStack::setItemToolTip(int index, QString tooltip)
{

}

void ModeStack::setIconSize(QSize size)
{

}

void ModeStack::insertItem(int index, Core::IMode *mode)
{
    m_model->insertMode(index, mode);
    qDebug() << "DSW: Inserted IMode name " << mode->name() << " qml " << mode->qmlPath();
}

void ModeStack::moveItem(int curIndex, int newIndex)
{
    m_model->moveMode(curIndex, newIndex);
}

void ModeStack::removeItem(int index)
{
    m_model->removeModeAt(index);
}

}
