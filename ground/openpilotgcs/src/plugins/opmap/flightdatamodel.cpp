/**
 ******************************************************************************
 *
 * @file       flightdatamodel.cpp
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

#include "flightdatamodel.h"
#include <QMessageBox>
#include <QDomDocument>

flightDataModel::flightDataModel(QObject *parent) : QAbstractTableModel(parent)
{
}

int flightDataModel::rowCount(const QModelIndex & /*parent*/) const
{
    return dataStorage.length();
}

int flightDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 23;
}

QVariant flightDataModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        int rowNumber    = index.row();
        int columnNumber = index.column();
        if (rowNumber > dataStorage.length() - 1 || rowNumber < 0) {
            return QVariant::Invalid;
        }
        pathPlanData *myRow = dataStorage.at(rowNumber);
        QVariant ret = getColumnByIndex(myRow, columnNumber);
        return ret;
    }
    /*
       else if (role == Qt::BackgroundRole) {
       //  WaypointActive::DataFields waypointActive = waypointActiveObj->getData();

        if(index.row() == waypointActive.Index) {
            return QBrush(Qt::lightGray);
        } else
            return QVariant::Invalid;
       }*/
    else {
        return QVariant::Invalid;
    }
}

bool flightDataModel::setColumnByIndex(pathPlanData *row, const int index, const QVariant value)
{
    bool b;
    switch (index) {
    case WPDESCRITPTION:
        row->wpDescritption = value.toString();
        b = true;
        break;
    case LATPOSITION:
        row->latPosition = value.toDouble();
        b = true;
        break;
    case LNGPOSITION:
        row->lngPosition = value.toDouble();
        b = true;
        break;
    case DISRELATIVE:
        row->disRelative = value.toDouble();
        b = true;
        break;
    case BEARELATIVE:
        row->beaRelative = value.toDouble();
        b = true;
        break;
    case ALTITUDERELATIVE:
        row->altitudeRelative = value.toFloat();
        b = true;
        break;
    case ISRELATIVE:
        row->isRelative = value.toBool();
        b = true;
        break;
    case ALTITUDE:
        row->altitude = value.toDouble();
        b = true;
        break;
    case VELOCITY:
        row->velocity = value.toFloat();
        b = true;
        break;
    case MODE:
        row->mode = value.toInt();
        b = true;
        break;
    case MODE_PARAMS0:
        row->mode_params[0] = value.toFloat();
        b = true;
        break;
    case MODE_PARAMS1:
        row->mode_params[1] = value.toFloat();
        b = true;
        break;
    case MODE_PARAMS2:
        row->mode_params[2] = value.toFloat();
        b = true;
        break;
    case MODE_PARAMS3:
        row->mode_params[3] = value.toFloat();
        b = true;
        break;
    case CONDITION:
        row->condition = value.toInt();
        b = true;
        break;
    case CONDITION_PARAMS0:
        row->condition_params[0] = value.toFloat();
        b = true;
        break;
    case CONDITION_PARAMS1:
        row->condition_params[1] = value.toFloat();
        b = true;
        break;
    case CONDITION_PARAMS2:
        row->condition_params[2] = value.toFloat();
        b = true;
        break;
    case CONDITION_PARAMS3:
        row->condition_params[3] = value.toFloat();
        b = true;
        break;
    case COMMAND:
        row->command = value.toInt();
        b = true;
        break;
    case JUMPDESTINATION:
        row->jumpdestination = value.toInt();
        b = true;
        break;
    case ERRORDESTINATION:
        row->errordestination = value.toInt();
        b = true;
        break;
    case LOCKED:
        row->locked = value.toBool();
        b = true;
        break;
    default:
        b = false;
        break;
    }
    return b;
}

QVariant flightDataModel::getColumnByIndex(const pathPlanData *row, const int index) const
{
    QVariant value;
    switch (index) {
    case WPDESCRITPTION:
        value = row->wpDescritption;
        break;
    case LATPOSITION:
        value = row->latPosition;
        break;
    case LNGPOSITION:
        value = row->lngPosition;
        break;
    case DISRELATIVE:
        value = row->disRelative;
        break;
    case BEARELATIVE:
        value = row->beaRelative;
        break;
    case ALTITUDERELATIVE:
        value = row->altitudeRelative;
        break;
    case ISRELATIVE:
        value = row->isRelative;
        break;
    case ALTITUDE:
        value = row->altitude;
        break;
    case VELOCITY:
        value = row->velocity;
        break;
    case MODE:
        value = row->mode;
        break;
    case MODE_PARAMS0:
        value = row->mode_params[0];
        break;
    case MODE_PARAMS1:
        value = row->mode_params[1];
        break;
    case MODE_PARAMS2:
        value = row->mode_params[2];
        break;
    case MODE_PARAMS3:
        value = row->mode_params[3];
        break;
    case CONDITION:
        value = row->condition;
        break;
    case CONDITION_PARAMS0:
        value = row->condition_params[0];
        break;
    case CONDITION_PARAMS1:
        value = row->condition_params[1];
        break;
    case CONDITION_PARAMS2:
        value = row->condition_params[2];
        break;
    case CONDITION_PARAMS3:
        value = row->condition_params[3];
        break;
    case COMMAND:
        value = row->command;
        break;
    case JUMPDESTINATION:
        value = row->jumpdestination;
        break;
    case ERRORDESTINATION:
        value = row->errordestination;
        break;
    case LOCKED:
        value = row->locked;
        break;
    }
    return value;
}

QVariant flightDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical) {
            value = QString::number(section + 1);
        } else if (orientation == Qt::Horizontal) {
            switch (section) {
            case WPDESCRITPTION:
                value = QString("Description");
                break;
            case LATPOSITION:
                value = QString("Latitude");
                break;
            case LNGPOSITION:
                value = QString("Longitude");
                break;
            case DISRELATIVE:
                value = QString("Distance to home");
                break;
            case BEARELATIVE:
                value = QString("Bearing from home");
                break;
            case ALTITUDERELATIVE:
                value = QString("Altitude above home");
                break;
            case ISRELATIVE:
                value = QString("Relative to home");
                break;
            case ALTITUDE:
                value = QString("Altitude");
                break;
            case VELOCITY:
                value = QString("Velocity");
                break;
            case MODE:
                value = QString("Mode");
                break;
            case MODE_PARAMS0:
                value = QString("Mode parameter 0");
                break;
            case MODE_PARAMS1:
                value = QString("Mode parameter 1");
                break;
            case MODE_PARAMS2:
                value = QString("Mode parameter 2");
                break;
            case MODE_PARAMS3:
                value = QString("Mode parameter 3");
                break;
            case CONDITION:
                value = QString("Condition");
                break;
            case CONDITION_PARAMS0:
                value = QString("Condition parameter 0");
                break;
            case CONDITION_PARAMS1:
                value = QString("Condition parameter 1");
                break;
            case CONDITION_PARAMS2:
                value = QString("Condition parameter 2");
                break;
            case CONDITION_PARAMS3:
                value = QString("Condition parameter 3");
                break;
            case COMMAND:
                value = QString("Command");
                break;
            case JUMPDESTINATION:
                value = QString("Jump Destination");
                break;
            case ERRORDESTINATION:
                value = QString("Error Destination");
                break;
            case LOCKED:
                value = QString("Locked");
                break;
            default:
                value = QString();
                break;
            }
        }
    } else {
        value = QAbstractTableModel::headerData(section, orientation, role);
    }
    return value;
}

bool flightDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        int columnIndex = index.column();
        int rowIndex    = index.row();
        if (rowIndex > dataStorage.length() - 1) {
            return false;
        }
        pathPlanData *myRow = dataStorage.at(rowIndex);
        setColumnByIndex(myRow, columnIndex, value);
        emit dataChanged(index, index);
    }
    return true;
}

Qt::ItemFlags flightDataModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

bool flightDataModel::insertRows(int row, int count, const QModelIndex & /*parent*/)
{
    pathPlanData *data;

    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int x = 0; x < count; ++x) {
        data = new pathPlanData;
        data->latPosition         = 0;
        data->lngPosition         = 0;
        data->disRelative         = 0;
        data->beaRelative         = 0;
        data->altitudeRelative    = 0;
        data->isRelative          = true;
        data->altitude            = 0;
        data->velocity            = 0;
        data->mode = 1;
        data->mode_params[0]      = 0;
        data->mode_params[1]      = 0;
        data->mode_params[2]      = 0;
        data->mode_params[3]      = 0;
        data->condition           = 3;
        data->condition_params[0] = 0;
        data->condition_params[1] = 0;
        data->condition_params[2] = 0;
        data->condition_params[3] = 0;
        data->command = 0;
        data->jumpdestination     = 0;
        data->errordestination    = 0;
        data->locked = false;
        if (rowCount() > 0) {
            data->altitude            = this->data(this->index(rowCount() - 1, ALTITUDE)).toDouble();
            data->altitudeRelative    = this->data(this->index(rowCount() - 1, ALTITUDERELATIVE)).toDouble();
            data->isRelative          = this->data(this->index(rowCount() - 1, ISRELATIVE)).toBool();
            data->velocity            = this->data(this->index(rowCount() - 1, VELOCITY)).toFloat();
            data->mode = this->data(this->index(rowCount() - 1, MODE)).toInt();
            data->mode_params[0]      = this->data(this->index(rowCount() - 1, MODE_PARAMS0)).toFloat();
            data->mode_params[1]      = this->data(this->index(rowCount() - 1, MODE_PARAMS1)).toFloat();
            data->mode_params[2]      = this->data(this->index(rowCount() - 1, MODE_PARAMS2)).toFloat();
            data->mode_params[3]      = this->data(this->index(rowCount() - 1, MODE_PARAMS3)).toFloat();
            data->condition           = this->data(this->index(rowCount() - 1, CONDITION)).toInt();
            data->condition_params[0] = this->data(this->index(rowCount() - 1, CONDITION_PARAMS0)).toFloat();
            data->condition_params[1] = this->data(this->index(rowCount() - 1, CONDITION_PARAMS1)).toFloat();
            data->condition_params[2] = this->data(this->index(rowCount() - 1, CONDITION_PARAMS2)).toFloat();
            data->condition_params[3] = this->data(this->index(rowCount() - 1, CONDITION_PARAMS3)).toFloat();
            data->command = this->data(this->index(rowCount() - 1, COMMAND)).toInt();
            data->errordestination    = this->data(this->index(rowCount() - 1, ERRORDESTINATION)).toInt();
        }
        dataStorage.insert(row, data);
    }
    endInsertRows();
    return true;
}

bool flightDataModel::removeRows(int row, int count, const QModelIndex & /*parent*/)
{
    if (row < 0 || count <= 0) {
        return false;
    }
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int x = 0; x < count; ++x) {
        delete dataStorage.at(row);
        dataStorage.removeAt(row);
    }
    endRemoveRows();
    return true;
}

bool flightDataModel::writeToFile(QString fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(NULL, tr("Unable to open file"), file.errorString());
        return false;
    }
    QDataStream out(&file);
    QDomDocument doc("PathPlan");
    QDomElement root = doc.createElement("waypoints");
    doc.appendChild(root);

    foreach(pathPlanData * obj, dataStorage) {
        QDomElement waypoint = doc.createElement("waypoint");

        waypoint.setAttribute("number", dataStorage.indexOf(obj));
        root.appendChild(waypoint);
        QDomElement field = doc.createElement("field");
        field.setAttribute("value", obj->wpDescritption);
        field.setAttribute("name", "description");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->latPosition);
        field.setAttribute("name", "latitude");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->lngPosition);
        field.setAttribute("name", "longitude");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->disRelative);
        field.setAttribute("name", "distance_to_home");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->beaRelative);
        field.setAttribute("name", "bearing_from_home");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->altitudeRelative);
        field.setAttribute("name", "altitude_above_home");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->isRelative);
        field.setAttribute("name", "is_relative_to_home");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->altitude);
        field.setAttribute("name", "altitude");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->velocity);
        field.setAttribute("name", "velocity");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->mode);
        field.setAttribute("name", "mode");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->mode_params[0]);
        field.setAttribute("name", "mode_param0");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->mode_params[1]);
        field.setAttribute("name", "mode_param1");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->mode_params[2]);
        field.setAttribute("name", "mode_param2");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->mode_params[3]);
        field.setAttribute("name", "mode_param3");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->condition);
        field.setAttribute("name", "condition");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->condition_params[0]);
        field.setAttribute("name", "condition_param0");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->condition_params[1]);
        field.setAttribute("name", "condition_param1");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->condition_params[2]);
        field.setAttribute("name", "condition_param2");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->condition_params[3]);
        field.setAttribute("name", "condition_param3");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->command);
        field.setAttribute("name", "command");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->jumpdestination);
        field.setAttribute("name", "jumpdestination");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->errordestination);
        field.setAttribute("name", "errordestination");
        waypoint.appendChild(field);

        field = doc.createElement("field");
        field.setAttribute("value", obj->locked);
        field.setAttribute("name", "is_locked");
        waypoint.appendChild(field);
    }
    file.write(doc.toString().toLatin1());
    file.close();
    return true;
}

void flightDataModel::readFromFile(QString fileName)
{
    // TODO warning message
    removeRows(0, rowCount());
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QDomDocument doc("PathPlan");
    QByteArray array = file.readAll();
    QString error;
    if (!doc.setContent(array, &error)) {
        QMessageBox msgBox;
        msgBox.setText(tr("File Parsing Failed."));
        msgBox.setInformativeText(QString(tr("This file is not a correct XML file:%0")).arg(error));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();

    if (root.isNull() || (root.tagName() != "waypoints")) {
        QMessageBox msgBox;
        msgBox.setText(tr("Wrong file contents"));
        msgBox.setInformativeText(tr("This file does not contain correct UAVSettings"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    pathPlanData *data = NULL;
    QDomNode node = root.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (e.tagName() == "waypoint") {
            QDomNode fieldNode = e.firstChild();
            data = new pathPlanData;
            while (!fieldNode.isNull()) {
                QDomElement field = fieldNode.toElement();
                if (field.tagName() == "field") {
                    QString name = field.attribute("name");
                    QString value = field.attribute("value");
                    if (name == "altitude") {
                        data->altitude = value.toDouble();
                    } else if (name == "description") {
                        data->wpDescritption = value;
                    } else if (name == "latitude") {
                        data->latPosition = value.toDouble();
                    } else if (name == "longitude") {
                        data->lngPosition = value.toDouble();
                    } else if (name == "distance_to_home") {
                        data->disRelative = value.toDouble();
                    } else if (name == "bearing_from_home") {
                        data->beaRelative = value.toDouble();
                    } else if (name == "altitude_above_home") {
                        data->altitudeRelative = value.toFloat();
                    } else if (name == "is_relative_to_home") {
                        data->isRelative = value.toInt();
                    } else if (name == "altitude") {
                        data->altitude = value.toDouble();
                    } else if (name == "velocity") {
                        data->velocity = value.toFloat();
                    } else if (name == "mode") {
                        data->mode = value.toInt();
                    } else if (name == "mode_param0") {
                        data->mode_params[0] = value.toFloat();
                    } else if (name == "mode_param1") {
                        data->mode_params[1] = value.toFloat();
                    } else if (name == "mode_param2") {
                        data->mode_params[2] = value.toFloat();
                    } else if (name == "mode_param3") {
                        data->mode_params[3] = value.toFloat();
                    } else if (name == "condition") {
                        data->condition = value.toDouble();
                    } else if (name == "condition_param0") {
                        data->condition_params[0] = value.toFloat();
                    } else if (name == "condition_param1") {
                        data->condition_params[1] = value.toFloat();
                    } else if (name == "condition_param2") {
                        data->condition_params[2] = value.toFloat();
                    } else if (name == "condition_param3") {
                        data->condition_params[3] = value.toFloat();
                    } else if (name == "command") {
                        data->command = value.toInt();
                    } else if (name == "jumpdestination") {
                        data->jumpdestination = value.toInt();
                    } else if (name == "errordestination") {
                        data->errordestination = value.toInt();
                    } else if (name == "is_locked") {
                        data->locked = value.toInt();
                    }
                }
                fieldNode = fieldNode.nextSibling();
            }
            beginInsertRows(QModelIndex(), dataStorage.length(), dataStorage.length());
            dataStorage.append(data);
            endInsertRows();
        }
        node = node.nextSibling();
    }
}
