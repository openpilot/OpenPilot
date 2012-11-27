/**
 ******************************************************************************
 *
 * @file       geofencedatamodel.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin OpenPilot Map Plugin
 * @{
 * @brief The OpenPilot Map plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <QFile>
#include <QMessageBox>
#include <QDomDocument>
#include "geofencedatamodel.h"

GeofenceDataModel::GeofenceDataModel(QObject *parent) :
    QAbstractTableModel(parent),
    nextIndex(0)
{
}

int GeofenceDataModel::rowCount(const QModelIndex &/*parent*/) const
{
    return dataStorage.length();
}

int GeofenceDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 6;
}

QVariant GeofenceDataModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole||role==Qt::EditRole)
    {
        int rowNumber=index.row();
        int columnNumber=index.column();
        if(rowNumber>dataStorage.length()-1 || rowNumber<0)
            return QVariant::Invalid;
        GeofenceData * myRow=dataStorage.at(rowNumber);
        QVariant ret=getColumnByIndex(myRow,columnNumber);
        return ret;
    }
    else {
        return QVariant::Invalid;
    }
}

QVariant GeofenceDataModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role == Qt::DisplayRole)
     {
         if(orientation==Qt::Vertical)
         {
             return QString::number(section+1);
         }
         else if (orientation == Qt::Horizontal) {
             switch (section)
             {
             case LATITUDE:
                 return QString(tr("Latitude"));
                 break;
             case LONGITUDE:
                 return QString(tr("Longitude"));
                 break;
             case ALTITUDE:
                 return QString(tr("Altitude"));
                 break;
             case VERTEX_ID:
                 return QString(tr("Vertex ID"));
                 break;
             case VERTEX_PAIR_ID:
                 return QString(tr("Vertex Pair ID"));
                 break;
             case POLYGON_ID:
                 return QString(tr("Polygon ID"));
                 break;
             default:
                 return QString();
                 break;
             }
         }
     }
     else
       return QAbstractTableModel::headerData(section, orientation, role);
}

bool GeofenceDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole)
    {
        int columnIndex=index.column();
        int rowIndex=index.row();
        if(rowIndex>dataStorage.length()-1)
            return false;
        GeofenceData * myRow=dataStorage.at(rowIndex);
        setColumnByIndex(myRow,columnIndex,value);
        emit dataChanged(index,index);
    }
    return true;
}

Qt::ItemFlags GeofenceDataModel::flags(const QModelIndex & /*index*/) const
 {
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

bool GeofenceDataModel::insertRows(int row, int count, const QModelIndex &/*parent*/)
{
    GeofenceData * data;
    beginInsertRows(QModelIndex(),row,row+count-1);
    for(int x=0; x<count;++x)
    {
        data=new GeofenceData;
        data->latitude=0;
        data->longitude=0;
        data->altitude=0;
        data->vertexId = 0;
        data->vertexPairId = 0;
        data->polygonId=0;
        if(rowCount()>0)
        {
            data->latitude=this->data(this->index(rowCount()-1,LATITUDE)).toDouble();
            data->longitude=this->data(this->index(rowCount()-1,LONGITUDE)).toDouble();
            data->altitude=this->data(this->index(rowCount()-1,ALTITUDE)).toDouble();
            data->vertexId = nextIndex++;
            data->vertexPairId = this->data(this->index(rowCount()-1, VERTEX_PAIR_ID)).toInt();
            data->polygonId=this->data(this->index(rowCount()-1,POLYGON_ID)).toInt();
        }
        dataStorage.insert(row,data);
    }
    endInsertRows();
}

bool GeofenceDataModel::removeRows(int row, int count, const QModelIndex &/*parent*/)
{
    if(row<0)
        return false;
    beginRemoveRows(QModelIndex(),row,row+count-1);
    for(int x=0; x<count;++x)
    {
        delete dataStorage.at(row);
        dataStorage.removeAt(row);
    }
    endRemoveRows();
}

bool GeofenceDataModel::writeToFile(QString fileName)
{

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(NULL, tr("Unable to open file"), file.errorString());
        return false;
    }
    QDataStream out(&file);
    QDomDocument doc("Geofence");
    QDomElement root = doc.createElement("dataModel");
    doc.appendChild(root);
    QDomElement vertices = doc.createElement("vertices");
    root.appendChild(vertices);

    foreach(GeofenceData * obj,dataStorage)
    {

        QDomElement waypoint = doc.createElement("vertex");
        waypoint.setAttribute("number",dataStorage.indexOf(obj));
        vertices.appendChild(waypoint);
        QDomElement field=doc.createElement("field");
        field.setAttribute("value",obj->latitude);
        field.setAttribute("name","latitude");
        waypoint.appendChild(field);

        field=doc.createElement("field");
        field.setAttribute("value",obj->longitude);
        field.setAttribute("name","longitude");
        waypoint.appendChild(field);

        field=doc.createElement("field");
        field.setAttribute("value",obj->altitude);
        field.setAttribute("name","altitude");
        waypoint.appendChild(field);

        field=doc.createElement("field");
        field.setAttribute("value",obj->vertexId);
        field.setAttribute("name","vertexId");
        waypoint.appendChild(field);

        field=doc.createElement("field");
        field.setAttribute("value",obj->vertexPairId);
        field.setAttribute("name","vertexPairId");
        waypoint.appendChild(field);

        field=doc.createElement("field");
        field.setAttribute("value",obj->polygonId);
        field.setAttribute("name","polygonId");
        waypoint.appendChild(field);

    }

    QDomElement metaData=doc.createElement("metaData");
    root.appendChild(metaData);
    QDomElement item = doc.createElement("item");
    metaData.appendChild(item);
    item.setAttribute("value",nextIndex);
    item.setAttribute("name","nextIndex");
    file.write(doc.toString().toAscii());
    file.close();

    return true;
}
void GeofenceDataModel::readFromFile(QString fileName)
{
    //TODO warning message
    removeRows(0,rowCount());
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QDomDocument doc("Geofence");
    QByteArray array=file.readAll();
    QString error;
    if (!doc.setContent(array,&error)) {
        QMessageBox msgBox;
        msgBox.setText(tr("File Parsing Failed."));
        msgBox.setInformativeText(QString(tr("This file is not a correct XML file:%0")).arg(error));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();

    if (root.isNull() || (root.tagName() != "dataModel")) {
        QMessageBox msgBox;
        msgBox.setText(tr("Wrong file contents"));
        msgBox.setInformativeText(tr("This file does not contain correct UAVSettings"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    GeofenceData * data=NULL;
    QDomNode node = root.firstChild();
    while (!node.isNull()) {
        QDomElement modelSection = node.toElement();
        if(modelSection.tagName() == "vertices"){
            QDomElement e = modelSection.toElement();
            if (e.tagName() == "vertex") {
                QDomNode fieldNode=e.firstChild();
                data=new GeofenceData;
                while (!fieldNode.isNull()) {
                    QDomElement field = fieldNode.toElement();
                    if (field.tagName() == "field") {
                        if(field.attribute("name")=="latitude")
                            data->latitude=field.attribute("value").toDouble();
                        else if(field.attribute("name")=="longitude")
                            data->longitude=field.attribute("value").toDouble();
                        else if(field.attribute("name")=="altitude")
                            data->altitude=field.attribute("value").toDouble();
                        else if(field.attribute("name")=="vertexId")
                            data->vertexId=field.attribute("value").toInt();
                        else if(field.attribute("name")=="vertexPairId")
                            data->vertexPairId=field.attribute("value").toInt();
                        else if(field.attribute("name")=="polygonId")
                            data->polygonId=field.attribute("value").toInt();

                    }
                    fieldNode=fieldNode.nextSibling();
                }
            beginInsertRows(QModelIndex(),dataStorage.length(),dataStorage.length());
            dataStorage.append(data);
            endInsertRows();
            }
        }
        else if(modelSection.tagName() == "metaData"){
            QDomElement e = modelSection.toElement();
            if (e.tagName() == "item") {
                QDomNode itemNode=e.firstChild();
                while (!itemNode.isNull()) {
                    QDomElement item = itemNode.toElement();
                    if(item.attribute("name")=="nextIndex")
                        nextIndex=item.attribute("value").toLong();
                    itemNode=itemNode.nextSibling();
                }
            }
        }
        node=node.nextSibling();
    }
}

bool GeofenceDataModel::setColumnByIndex(GeofenceData  *row,const int index,const QVariant value)
{
    bool retVal = false;
    switch(index)
    {
    case LATITUDE:
        row->latitude = value.toDouble();
        retVal = true;
        break;
    case LONGITUDE:
        row->longitude = value.toDouble();
        retVal = true;
        break;
    case ALTITUDE:
        row->altitude = value.toDouble();
        retVal = true;
        break;
    case VERTEX_ID:
        row->vertexId = value.toInt();
        retVal = true;
        break;
    case VERTEX_PAIR_ID:
        row->vertexPairId = value.toInt();
        retVal = true;
        break;
    case POLYGON_ID:
        row->polygonId = value.toInt();
        retVal = true;
        break;
    }
    return retVal;
}
QVariant GeofenceDataModel::getColumnByIndex(const GeofenceData *row,const int index) const
{
    switch(index)
    {
    case LATITUDE:
        return row->latitude;
        break;
    case LONGITUDE:
        return row->longitude;
        break;
    case ALTITUDE:
        return row->altitude;
        break;
    case VERTEX_ID:
        return row->vertexId;
        break;
    case VERTEX_PAIR_ID:
        return row->vertexPairId;
        break;
    case POLYGON_ID:
        return row->polygonId;
        break;
    }
}
