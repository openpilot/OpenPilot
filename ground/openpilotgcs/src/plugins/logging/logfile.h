#ifndef LOGFILE_H
#define LOGFILE_H

#include <QIODevice>
#include <QTime>
#include <QTimer>
#include <QMutexLocker>
#include <QDebug>
#include <QBuffer>
#include "uavobjectmanager.h"
#include <math.h>

class LogFile : public QIODevice
{
    Q_OBJECT
public:
    explicit LogFile(QObject *parent = 0);
    qint64 bytesAvailable() const;
    qint64 bytesToWrite() { return file.bytesToWrite(); };
    bool open(OpenMode mode);
    void setFileName(QString name) { file.setFileName(name); };
    void close();
    qint64 writeData(const char * data, qint64 dataSize);
    qint64 readData(char * data, qint64 maxlen);

    bool startReplay();
    bool stopReplay();

public slots:
    void setReplaySpeed(double val) { playbackSpeed = pow(10,(val)/100); qDebug() << playbackSpeed; };
    void pauseReplay();
    void resumeReplay();

protected slots:
    void timerFired();

signals:
    void readReady();
    void replayFinished();

protected:
    QByteArray dataBuffer;
    QTimer timer;
    QTime myTime;
    QFile file;
    qint32 lastTimeStamp;
    QMutex mutex;


    int timeOffset;
    int pausedTime;
    double playbackSpeed;
};

#endif // LOGFILE_H
