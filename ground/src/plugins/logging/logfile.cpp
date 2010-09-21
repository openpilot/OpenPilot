#include "logfile.h"
#include <QDebug>
#include <QtGlobal>

LogFile::LogFile(QObject *parent) :
    QIODevice(parent)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerFired()));
}

bool LogFile::open(OpenMode mode) {

    // start a timer for playback
    myTime.restart();

    if(file.open(mode) == FALSE)
    {
        qDebug() << "Unable to open " << file.fileName() << " for logging";
        return false;
    }

    // TODO: Write a header at the beginng describing objects so that in future
    // they can be read back if ID's change

    // Must call parent function for QIODevice to pass calls to writeData
    QIODevice::open(mode);

    return true;
}

void LogFile::close()
{
    file.close();
    QIODevice::close();
}

qint64 LogFile::writeData(const char * data, qint64 dataSize) {
    quint32 timeStamp = myTime.elapsed();

    file.write((char *) &timeStamp,sizeof(timeStamp));
    file.write((char *) &dataSize, sizeof(dataSize));

    qint64 written = file.write(data, dataSize);
    if(written != -1)
        emit bytesWritten(written);

    return dataSize;
}

qint64 LogFile::readData(char * data, qint64 maxSize) {
    qint64 toRead = qMin(maxSize,(qint64)dataBuffer.size());
    memcpy(data,dataBuffer.data(),toRead);
    dataBuffer.remove(0,toRead);
    return toRead;
}

qint64 LogFile::bytesAvailable() const
{
    return dataBuffer.size();
}

void LogFile::timerFired()
{
    qint64 dataSize;

    // TODO: support time rescaling and seeking
    while (myTime.elapsed() > lastTimeStamp) {
        file.read((char *) &dataSize, sizeof(dataSize));
        dataBuffer.append(file.read(dataSize));
        emit readyRead();

        file.read((char *) &lastTimeStamp,sizeof(lastTimeStamp));
    }
}

bool LogFile::startReplay() {
    dataBuffer.clear();
    myTime.restart();

    file.read((char *) &lastTimeStamp,sizeof(lastTimeStamp));

    timer.setInterval(10);
    timer.start();
    return true;
}

bool LogFile::stopReplay() {
    timer.stop();
    return true;
}
