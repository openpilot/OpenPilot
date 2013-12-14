#ifndef FLIGHTLOGDIALOG_H
#define FLIGHTLOGDIALOG_H

#include <QDialog>
#include "flightlogmanager.h"

class FlightLogDialog : public QDialog {
    Q_OBJECT

public:
    explicit FlightLogDialog(QWidget *parent, FlightLogManager *flightLogManager);
    ~FlightLogDialog();
};

#endif // FLIGHTLOGDIALOG_H
