#ifndef LOGFILE_H
#define LOGFILE_H

#include <QIODevice>
#include <QTime>
#include <QDebug>
#include <uavobjects/uavobjectmanager.h>

class LogFile : public QIODevice
{
    Q_OBJECT
public:
    explicit LogFile(QObject *parent = 0);
    qint64 bytesAvailable() { return 0; };
    qint64 bytesToWrite() { return file.bytesToWrite(); };
    bool open(OpenMode mode);
    void setFileName(QString name) { file.setFileName(name); };
    void close();
    qint64 writeData(const char * data, qint64 dataSize);
    qint64 readData(char * /*data*/, qint64 /*maxlen*/) {return 0; }

signals:
public slots:
private:
protected:
    QTime myTime;
    QFile file;
};

#endif // LOGFILE_H
