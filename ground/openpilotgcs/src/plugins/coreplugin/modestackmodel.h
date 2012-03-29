#ifndef MODESTACKMODEL_H
#define MODESTACKMODEL_H

#include <QAbstractListModel>
#include <coreplugin/imode.h>

namespace Core{

class ModeStackModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ModeRoles{
        QmlRole = Qt::UserRole + 1,
        NameRole,
        IconRole
    };
    explicit ModeStackModel(QObject *parent = 0);

    void addMode(Core::IMode* mode);
    void insertMode(int index, Core::IMode* mode);
    void moveMode(int indexFrom, int indexTo);
    void removeModeAt(int index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = ModeStackModel::QmlRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

signals:

public slots:
private:
    QList<Core::IMode*> m_modes;

};
}

#endif // MODESTACKMODEL_H
