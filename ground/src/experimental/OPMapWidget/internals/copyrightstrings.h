#ifndef COPYRIGHTSTRINGS_H
#define COPYRIGHTSTRINGS_H

#include <QString>
#include <QDateTime>
static const QString googleCopyright = QString("©%1 Google - Map data ©%1 Tele Atlas, Imagery ©%1 TerraMetrics").arg(QDate::currentDate().year());
static const QString openStreetMapCopyright = QString("© OpenStreetMap - Map data ©%1 OpenStreetMap").arg(QDate::currentDate().year());
static const QString yahooMapCopyright = QString("© Yahoo! Inc. - Map data & Imagery ©%1 NAVTEQ").arg(QDate::currentDate().year());
static const QString virtualEarthCopyright = QString("©%1 Microsoft Corporation, ©%1 NAVTEQ, ©%1 Image courtesy of NASA").arg(QDate::currentDate().year());
static const QString arcGisCopyright = QString("©%1 ESRI - Map data ©%1 ArcGIS").arg(QDate::currentDate().year());

#endif // COPYRIGHTSTRINGS_H
