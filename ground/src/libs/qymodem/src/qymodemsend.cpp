#include "qymodemsend.h"


QymodemSend::QymodemSend(QextSerialPort& port)
    : QymodemTx(port)
    {
        percent=0;
    }
int QymodemSend::PercentSend()
{
    return percent;
}

/**
Class for presenting a file as a input stream.
*/
class QymodemSend::InFile : public QymodemTx::InStream
{
public:
    InFile()
        : File(0)
    {
    }
    /**
        Open stream for reading a file.

        @param fileName		Name of the file to read.

        @return Zero if successful, or a negative error value if failed.
        */
    int Open(const char* fileName)
    {
        File = fopen(fileName,"rb");
        if(!File)
            return QymodemTx::ErrorInputStreamError;
        // find file size...
        if(fseek(File,0,SEEK_END))
        {
            fclose(File);
            return QymodemTx::ErrorInputStreamError;
        }
        TotalSize = ftell(File);
        if(fseek(File,0,SEEK_SET))
        {
            fclose(File);
            return QymodemTx::ErrorInputStreamError;
        }
        TransferredSize = 0;
        return 0;
    }

    /**
        Close the stream.
        */
    void Close()
    {
        if(File)
        {
            fclose(File);
            File = 0;
        }
    }

    /**
        Return the size of the file.

        @return File size.
        */
    inline size_t Size()
    {
        return TotalSize;
    }

    /**
        Read data from the stream.

        @param[out] data	Pointer to buffer to hold data read from stream.
        @param size			Maximum size of data to read.

        @return Zero if successful, or a negative error value if failed.
        */
    int In(char* data, size_t size, int * percent)
    {
        *percent = TotalSize ? ((uint64_t)TransferredSize*(uint64_t)100)/(uint64_t)TotalSize : 0;

        //printf("%8d bytes of %d - %3d%%\r",TransferredSize, TotalSize, percent);

        fflush(stdout);
        size=fread(data,sizeof(uint8_t),size,File);
        if(size)
        {
            TransferredSize += size;
            return size;
        }
        if(TransferredSize!=TotalSize)
            return QymodemTx::ErrorInputStreamError;
        return 0;
    }
private:
    FILE* File;
    size_t TotalSize;
    size_t TransferredSize;
};


/**
Class for presenting a file as a output stream.
*/



/**
Send the file.

@param port	The serial port to use.
*/
void QymodemSend::Send()
{

    InFile source;
    int error = source.Open(FileName);
    if(error)
        emit Error("Can't open file " + QString(FileName),error);


    unsigned Timeout=30000;
    error = SendY(FileName,source.Size(),source,Timeout);
    if(error)
    {
        emit Error("Error during file transfer, error "+QString(error),error);
    }
    else
    {
        emit Information("Sent OK",QymodemSend::InfoSent);
    }
    source.Close();
}

int QymodemSend::SendFile(QString filename)
{
    QFile file;
    if(!file.exists(filename))
    {
        emit Error("File not found",QymodemSend::ErrorFileNotFound);
        return QymodemSend::ErrorFileNotFound;
    }
    if(!Port.open(QIODevice::ReadWrite| QIODevice::Unbuffered))
    {
        emit Error("Could not open port",QymodemSend::ErrorCoulNotOpenPort);
        return QymodemSend::ErrorCoulNotOpenPort;
    }
        QByteArray a=filename.toLocal8Bit();
        FileName=a.constData();
        Send();
        Port.close();
        return 0;

}

int QymodemSend::SendFileT(QString filename)
{
    if(!isRunning())
    {
        FileNameT=filename;
        start();
    }
    else
    {
        return QymodemSend::ErrorFileTransmissionInProgress;
    }
    return 0;
}
void QymodemSend::run()
{
    QFile file;
    if(!file.exists(FileNameT))
    {
        emit Error("File not found",QymodemSend::ErrorFileNotFound);
        return;
    }
    if(!Port.open(QIODevice::ReadWrite| QIODevice::Unbuffered))
    {
        emit Error("Could not open port",QymodemSend::ErrorCoulNotOpenPort);
        return;
    }
        QByteArray a=FileNameT.toLocal8Bit();
        FileName=a.constData();
        Send();
        Port.close();
        return;

}
