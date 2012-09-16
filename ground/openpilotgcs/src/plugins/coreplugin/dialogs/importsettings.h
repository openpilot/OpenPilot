#ifndef IMPORTSETTINGS_H
#define IMPORTSETTINGS_H

#include <QDialog>
#include <QMap>
namespace Ui {
class importSettings;
}

class importSettings : public QDialog
{
    Q_OBJECT
    struct fileInfo
    {
        QString file;
        QString description;
        QString details;
    };

public:
    explicit importSettings(QWidget *parent = 0);
    ~importSettings();
    
    void loadFiles(QString path);
    QString choosenConfig();
private:
    Ui::importSettings *ui;
    QMap<int,fileInfo*> configList;
private slots:
    void updateDetails(int);
};

#endif // IMPORTSETTINGS_H
