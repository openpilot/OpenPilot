#ifndef LOGFILE_H
#define LOGFILE_H

#include <QIODevice>
#include <QTime>
#include <QTimer>
#include <QMutexLocker>
#include <QDebug>
#include <QBuffer>
#include <QFile>
#include "utils_global.h"

class QTCREATOR_UTILS_EXPORT LogFile : public QIODevice {
    Q_OBJECT
public:
    explicit LogFile(QObject *parent = 0);
    qint64 bytesAvailable() const;
    qint64 bytesToWrite()
    {
        return m_file.bytesToWrite();
    };
    bool open(OpenMode mode);
    void setFileName(QString name)
    {
        m_file.setFileName(name);
    };
    void close();
    qint64 writeData(const char *data, qint64 dataSize);
    qint64 readData(char *data, qint64 maxlen);

    bool startReplay();
    bool stopReplay();
    void useProvidedTimeStamp(bool useProvidedTimeStamp)
    {
        m_useProvidedTimeStamp = useProvidedTimeStamp;
    }

    void setNextTimeStamp(quint32 nextTimestamp)
    {
        m_nextTimeStamp = nextTimestamp;
    }

public slots:
    void setReplaySpeed(double val)
    {
        m_playbackSpeed = val;
        qDebug() << "Playback speed is now" << m_playbackSpeed;
    };
    void pauseReplay();
    void resumeReplay();

protected slots:
    void timerFired();

signals:
    void readReady();
    void replayStarted();
    void replayFinished();

protected:
    QByteArray m_dataBuffer;
    QTimer m_timer;
    QTime m_myTime;
    QFile m_file;
    qint32 m_lastTimeStamp;
    qint32 m_lastPlayed;
    QMutex m_mutex;


    int m_timeOffset;
    double m_playbackSpeed;

private:
    quint32 m_nextTimeStamp;
    bool m_useProvidedTimeStamp;
};

#endif // LOGFILE_H
