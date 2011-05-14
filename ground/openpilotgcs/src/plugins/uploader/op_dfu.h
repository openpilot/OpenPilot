#ifndef OP_DFU_H
#define OP_DFU_H

#include <QByteArray>
#include <rawhid/pjrc_rawhid.h>
#include <rawhid/usbmonitor.h>
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QMetaType>
#include <QCryptographicHash>
#include <QList>
#include <QVariant>
#include <iostream>
#include "delay.h"
#include <qextserialport/src/qextserialport.h>
#include <QTime>
#include "SSP/qssp.h"
#include "SSP/port.h"
#include "SSP/qsspt.h"

using namespace std;
#define BUF_LEN 64

#define MAX_PACKET_DATA_LEN	255
#define MAX_PACKET_BUF_SIZE	(1+1+MAX_PACKET_DATA_LEN+2)

namespace OP_DFU {

    enum TransferTypes
    {
        FW,
        Descript
    };

    enum CompareType
    {
        crccompare,
        bytetobytecompare
    };

    Q_ENUMS(Status)
    enum Status
    {
        DFUidle,//0
        uploading,//1
        wrong_packet_received,//2
        too_many_packets,//3
        too_few_packets,//4
        Last_operation_Success,//5
        downloading,//6
        idle,//7
        Last_operation_failed,//8
        uploadingStarting,//9
        outsideDevCapabilities,//10
        CRC_Fail,//11
        failed_jump,//12
        abort//13
    };

    enum Actions
    {
        actionProgram,
        actionProgramAndVerify,
        actionDownload,
        actionCompareAll,
        actionCompareCrc,
        actionListDevs,
        actionStatusReq,
        actionReset,
        actionJump
    };

    enum Commands
    {
        Reserved,//0
        Req_Capabilities,//1
        Rep_Capabilities,//2
        EnterDFU,//3
        JumpFW,//4
        Reset,//5
        Abort_Operation,//6
        Upload,//7
        Op_END,//8
        Download_Req,//9
        Download,//10
        Status_Request,//11
        Status_Rep,//12

    };

    struct device
    {
            int ID;
            quint32 FW_CRC;
            int BL_Version;
            int SizeOfDesc;
            quint32 SizeOfCode;
            bool Readable;
            bool Writable;
    };


    class DFUObject : public QThread
    {
        Q_OBJECT;

        public:

        //DFUObject(bool debug);
        DFUObject(bool debug,bool use_serial,QString port);

        ~DFUObject();

        // Service commands:
        bool enterDFU(int const &devNumber);
        bool findDevices();
        int JumpToApp();
        int ResetDevice(void);
        OP_DFU::Status StatusRequest();
        bool EndOperation();
        int AbortOperation(void);
        bool ready() { return mready; }

        // Upload (send to device) commands
        OP_DFU::Status UploadDescription(QVariant description);
        bool UploadFirmware(const QString &sfile, const bool &verify,int device);

        // Download (get from device) commands:
        // DownloadDescription is synchronous
        QString DownloadDescription(int const & numberOfChars);
        QByteArray DownloadDescriptionAsBA(int const & numberOfChars);
        // Asynchronous firmware download: initiates fw download,
        // and a downloadFinished signal is emitted when download
        // if finished:
        bool DownloadFirmware(QByteArray *byteArray, int device);

        // Comparison functions (is this needed?)
        OP_DFU::Status CompareFirmware(const QString &sfile, const CompareType &type,int device);

        bool SaveByteArrayToFile(QString const & file,QByteArray const &array);



        // Variables:
        QList<device> devices;
        int numberOfDevices;
        int send_delay;
        bool use_delay;

        // Helper functions:
        QString StatusToString(OP_DFU::Status  const & status);
        quint32 CRC32WideFast(quint32 Crc, quint32 Size, quint32 *Buffer);



    signals:
       void progressUpdated(int);
       void downloadFinished();
       void uploadFinished(OP_DFU::Status);
       void operationProgress(QString status);

    private:
       // Generic variables:
        bool debug;
        bool use_serial;
        bool mready;
        int RWFlags;
        qsspt * serialhandle;
        int sendData(void*,int);
        int receiveData(void * data,int size);
        uint8_t	sspTxBuf[MAX_PACKET_BUF_SIZE];
        uint8_t	sspRxBuf[MAX_PACKET_BUF_SIZE];
        port * info;


        // USB Bootloader:
        pjrc_rawhid hidHandle;
        int setStartBit(int command){ return command|0x20; }
        quint32 CRCFromQBArray(QByteArray array, quint32 Size);
        void CopyWords(char * source, char* destination, int count);
        void printProgBar( int const & percent,QString const& label);
        bool StartUpload(qint32  const &numberOfBytes, TransferTypes const & type,quint32 crc);
        bool UploadData(qint32 const & numberOfPackets,QByteArray  & data);

        // Thread management:
        // Same as startDownload except that we store in an external array:
        bool StartDownloadT(QByteArray *fw, qint32 const & numberOfBytes, TransferTypes const & type);
        OP_DFU::Status UploadFirmwareT(const QString &sfile, const bool &verify,int device);
        QMutex mutex;
        OP_DFU::Commands requestedOperation;
        qint32 requestSize;
        OP_DFU::TransferTypes requestTransferType;
        QByteArray *requestStorage;
        QString requestFilename;
        bool requestVerify;
        int requestDevice;

    protected:
       void run();// Executes the upload or download operations

    };

}

Q_DECLARE_METATYPE(OP_DFU::Status)


#endif // OP_DFU_H
