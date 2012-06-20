#ifndef FLIGHTDATAMODEL_H
#define FLIGHTDATAMODEL_H
#include <QAbstractTableModel>
#include "opmapcontrol/opmapcontrol.h"

struct pathPlanData
{
    QString wpDescritption;
    double latPosition;
    double lngPosition;
    double disRelative;
    double beaRelative;
    double altitudeRelative;
    bool isRelative;
    double altitude;
    float velocity;
    int mode;
    float mode_params[4];
    int condition;
    float condition_params[4];
    int command;
    int jumpdestination;
    int errordestination;
    bool locked;
};

class flightDataModel:public QAbstractTableModel
{
    Q_OBJECT
public:
    enum pathPlanDataEnum
    {
        WPDESCRITPTION,LATPOSITION,LNGPOSITION,DISRELATIVE,BEARELATIVE,ALTITUDERELATIVE,ISRELATIVE,ALTITUDE,
            VELOCITY,MODE,MODE_PARAMS0,MODE_PARAMS1,MODE_PARAMS2,MODE_PARAMS3,
            CONDITION,CONDITION_PARAMS0,CONDITION_PARAMS1,CONDITION_PARAMS2,CONDITION_PARAMS3,
            COMMAND,JUMPDESTINATION,ERRORDESTINATION,LOCKED
    };
    flightDataModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;
    bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    bool writeToFile(QString filename);
    void readFromFile(QString fileName);
private:
    QList<pathPlanData *> dataStorage;
    QVariant getColumnByIndex(const pathPlanData *row, const int index) const;
    bool setColumnByIndex(pathPlanData *row, const int index, const QVariant value);
signals:
     //void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );



};

#endif // FLIGHTDATAMODEL_H
