#include "logfile.h"
#include <QDebug>
#include <QtGlobal>

LogFile::LogFile(QObject *parent) :
    QIODevice(parent),
    m_lastTimeStamp(0),
    m_lastPlayed(0),
    m_timeOffset(0),
    m_playbackSpeed(1.0),
    m_nextTimeStamp(0),
    m_useProvidedTimeStamp(false)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timerFired()));
}

/**
 * Opens the logfile QIODevice and the underlying logfile. In case
 * we want to save the logfile, we open in WriteOnly. In case we
 * want to read the logfile, we open in ReadOnly.
 */
bool LogFile::open(OpenMode mode)
{
    // start a timer for playback
    m_myTime.restart();
    if (m_file.isOpen()) {
        // We end up here when doing a replay, because the connection
        // manager will also try to open the QIODevice, even though we just
        // opened it after selecting the file, which happens before the
        // connection manager call...
        return true;
    }

    if (m_file.open(mode) == false) {
        qDebug() << "Unable to open " << m_file.fileName() << " for logging";
        return false;
    }

    // TODO: Write a header at the beginng describing objects so that in future
    // they can be read back if ID's change

    // Must call parent function for QIODevice to pass calls to writeData
    // We always open ReadWrite, because otherwise we will get tons of warnings
    // during a logfile replay. Read nature is checked upon write ops below.
    QIODevice::open(QIODevice::ReadWrite);

    return true;
}

void LogFile::close()
{
    emit aboutToClose();

    if (m_timer.isActive()) {
        m_timer.stop();
    }
    m_file.close();
    QIODevice::close();
}

qint64 LogFile::writeData(const char *data, qint64 dataSize)
{
    if (!m_file.isWritable()) {
        return dataSize;
    }

    // If m_nextTimeStamp != -1 then use this timestamp instead of the timer
    // This is used when saving logs from on-board logging
    quint32 timeStamp = m_useProvidedTimeStamp ? m_nextTimeStamp : m_myTime.elapsed();

    m_file.write((char *)&timeStamp, sizeof(timeStamp));
    m_file.write((char *)&dataSize, sizeof(dataSize));

    qint64 written = m_file.write(data, dataSize);
    if (written != -1) {
        emit bytesWritten(written);
    }

    return dataSize;
}

qint64 LogFile::readData(char *data, qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);
    qint64 toRead = qMin(maxSize, (qint64)m_dataBuffer.size());

    memcpy(data, m_dataBuffer.data(), toRead);
    m_dataBuffer.remove(0, toRead);
    return toRead;
}

qint64 LogFile::bytesAvailable() const
{
    return m_dataBuffer.size();
}

void LogFile::timerFired()
{
    qint64 dataSize;

    if (m_file.bytesAvailable() > 4) {
        int time;
        time = m_myTime.elapsed();

        // TODO: going back in time will be a problem
        while ((m_lastPlayed + ((time - m_timeOffset) * m_playbackSpeed) > m_lastTimeStamp)) {
            m_lastPlayed += ((time - m_timeOffset) * m_playbackSpeed);
            if (m_file.bytesAvailable() < (qint64)sizeof(dataSize)) {
                stopReplay();
                return;
            }

            m_file.read((char *)&dataSize, sizeof(dataSize));

            if (dataSize < 1 || dataSize > (1024 * 1024)) {
                qDebug() << "Error: Logfile corrupted! Unlikely packet size: " << dataSize << "\n";
                stopReplay();
                return;
            }

            if (m_file.bytesAvailable() < dataSize) {
                stopReplay();
                return;
            }

            m_mutex.lock();
            m_dataBuffer.append(m_file.read(dataSize));
            m_mutex.unlock();

            emit readyRead();

            if (m_file.bytesAvailable() < (qint64)sizeof(m_lastTimeStamp)) {
                stopReplay();
                return;
            }

            int save = m_lastTimeStamp;
            m_file.read((char *)&m_lastTimeStamp, sizeof(m_lastTimeStamp));
            // some validity checks
            if (m_lastTimeStamp < save // logfile goes back in time
                || (m_lastTimeStamp - save) > (60 * 60 * 1000)) { // gap of more than 60 minutes)
                qDebug() << "Error: Logfile corrupted! Unlikely timestamp " << m_lastTimeStamp << " after " << save << "\n";
                stopReplay();
                return;
            }

            m_timeOffset = time;
            time = m_myTime.elapsed();
        }
    } else {
        stopReplay();
    }
}

bool LogFile::startReplay()
{
    m_dataBuffer.clear();
    m_myTime.restart();
    m_timeOffset = 0;
    m_lastPlayed = 0;
    m_file.read((char *)&m_lastTimeStamp, sizeof(m_lastTimeStamp));
    m_timer.setInterval(10);
    m_timer.start();
    emit replayStarted();
    return true;
}

bool LogFile::stopReplay()
{
    close();
    emit replayFinished();
    return true;
}

void LogFile::pauseReplay()
{
    m_timer.stop();
}

void LogFile::resumeReplay()
{
    m_timeOffset = m_myTime.elapsed();
    m_timer.start();
}
