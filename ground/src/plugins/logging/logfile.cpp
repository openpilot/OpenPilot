#include "logfile.h"
#include <QDebug>

LogFile::LogFile(QObject *parent) :
    QIODevice(parent)
{
}

bool LogFile::open(OpenMode mode) {

    // start a timer for playback
    myTime.restart();

    // TODO: support openning for read or write to determine playback or not
    Q_ASSERT(mode == QIODevice::WriteOnly);
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

    //qDebug() << "Wrote " << dataSize << " bytes at " << timeStamp << " ms";
    return dataSize;
}
