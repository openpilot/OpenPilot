#include <QIcon>
#include <QtCore/QDebug>

#include "modestackmodel.h"

using namespace Core;

ModeStackModel::ModeStackModel(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[QmlRole] = "itemQml";
    roles[NameRole] = "itemName";
    roles[IconRole] = "itemIcon";
    setRoleNames(roles);
}

void ModeStackModel::addMode(Core::IMode* mode)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    qDebug() << "DSW: Begin add IMode name " << mode->name() << " qml " << mode->qmlPath();
    m_modes << mode;
    qDebug() << "DSW: End add IMode name " << mode->name() << " qml " << mode->qmlPath();
    endInsertRows();
}

void ModeStackModel::moveMode(int indexFrom, int indexTo)
{
    Core::IMode* foundMode = m_modes.at(indexFrom);

    if(foundMode != NULL){
        removeModeAt(indexFrom);
        insertMode(indexTo, foundMode);
    }
}

void ModeStackModel::insertMode(int index, Core::IMode *mode)
{
    if(index >= rowCount()){
        index = rowCount();
    }
    qDebug() << "DSW: Begin insert IMode name " << mode->name() << " qml " << mode->qmlPath();
    beginInsertRows(QModelIndex(), index, index);
    m_modes.insert(index, mode);
    endInsertRows();
    qDebug() << "DSW: End insert IMode name " << mode->name() << " qml " << mode->qmlPath();
}

void ModeStackModel::removeModeAt(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_modes.removeAt(index);
    endRemoveRows();
}

int ModeStackModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_modes.count();
}

QVariant ModeStackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
             return QVariant();

    QVariant retVal;
    Core::IMode* mode = m_modes[index.row()];
    switch(role){
    case QmlRole:
        retVal = mode->qmlPath();
        break;
    case NameRole:
        qDebug() << "DSW: Getting mode name from model " << mode->name();
        retVal = mode->name();
        break;
    case IconRole:
        retVal = mode->icon();
        break;
    default:
        break;
    }

    return retVal;
}

bool ModeStackModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
             return false;

    Core::IMode* mode = m_modes[index.row()];
    switch(role){
    case QmlRole:
        mode->setQmlPath(value.toString());
        emit dataChanged(index, index);
        break;
    case NameRole:
        mode->setName(value.toString());
        emit dataChanged(index, index);
        break;
    case IconRole:
        mode->setIcon(value.value<QIcon>());
        emit dataChanged(index, index);
        break;
    default:
        break;
    }
}

Qt::ItemFlags ModeStackModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}
