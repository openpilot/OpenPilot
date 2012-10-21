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
    qint64 bytesToWrite() { return file.bytesToWrite(); }
    bool open(OpenMode mode);
    void setFileName(QString name) { file.setFileName(name); }
    void close();
    qint64 writeData(const char * data, qint64 dataSize);
    qint64 readData(char * data, qint64 maxlen);

    bool startReplay();
    bool stopReplay();

public slots:
    void setReplaySpeed(double val) { playbackSpeed = val; qDebug() << "New playback speed: " << playbackSpeed; }
    void setReplayTime(double val);
    void pauseReplay();
    void resumeReplay();

protected slots:
    void timerFired();

signals:
    void readReady();
    void replayStarted();
    void replayFinished();

protected:
    QByteArray dataBuffer;
    QTimer timer;
    QTime myTime;
    QFile file;
    qint32 lastTimeStamp;
    qint32 lastPlayTime;
    QMutex mutex;


    int lastPlayTimeOffset;
    double playbackSpeed;

private:
    QList<qint32> timestampBuffer;
    QList<qint32> timestampPos;
    quint32 timestampBufferIdx;
    qint32 lastTimeStampPos;
};

#endif // LOGFILE_H
