#ifndef OP_DFU_H
#define OP_DFU_H

#include <QByteArray>
#include <../../plugins/rawhid/pjrc_rawhid.h>
#include <QDebug>
#include <QFile>
#include <QCryptographicHash>
#include <QList>
#include <iostream>
#include "delay.h"
#include "../../libs/qextserialport/src/qextserialport.h"
#include <QTime>
#include "SSP/qssp.h"
#include "SSP/port.h"
#include "SSP/qsspt.h"
using namespace std;
#define BUF_LEN 64

//Command Line Options
#define DOWNLOAD            "-dn"    //done
#define DEVICE              "-d"     //done
//#define DOWNDESCRIPTION     "-dd"    //done
#define PROGRAMFW           "-p"     //done
#define PROGRAMDESC         "-w"     //done
#define VERIFY              "-v"     //done
#define COMPARECRC         "-cc"
#define COMPAREALL          "-ca"
#define STATUSREQUEST       "-s"    //done
#define LISTDEVICES         "-ls"   //done
#define RESET               "-r"
#define JUMP                "-j"
#define USE_SERIAL          "-t"

#define MAX_PACKET_DATA_LEN	255
#define MAX_PACKET_BUF_SIZE	(1+1+MAX_PACKET_DATA_LEN+2)


class OP_DFU
{
public:
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
    void JumpToApp();
    void ResetDevice(void);
    bool enterDFU(int const &devNumber);
    bool StartUpload(qint32  const &numberOfBytes, TransferTypes const & type,quint32 crc);
    bool UploadData(qint32 const & numberOfPackets,QByteArray  & data);
    Status UploadDescription(QString  & description);
    Status UploadFirmware(const QString &sfile, const bool &verify,int device);
    Status StatusRequest();
    bool EndOperation();
    void printProgBar( int const & percent,QString const& label);
    QString DownloadDescription(int const & numberOfChars);
    QByteArray StartDownload(qint32 const & numberOfBytes, TransferTypes const & type);
    bool SaveByteArrayToFile(QString const & file,QByteArray const &array);
    void CopyWords(char * source, char* destination, int count);
    // QByteArray DownloadData(int devNumber,int numberOfPackets);
    OP_DFU(bool debug,bool use_serial,QString port,bool umodereset);
    void sendReset(void);
    bool findDevices();
    QList<device> devices;
    int numberOfDevices;
    QString StatusToString(OP_DFU::Status  const & status);
    OP_DFU::Status CompareFirmware(const QString &sfile, const CompareType &type,int device);
    quint32 CRC32WideFast(quint32 Crc, quint32 Size, quint32 *Buffer);
    quint32 CRCFromQBArray(QByteArray array, quint32 Size);
    void test();
    int send_delay;
    bool use_delay;
    bool ready(){return mready;}
    void AbortOperation(void);
private:
    bool mready;
    bool debug;
    int RWFlags;
    bool use_serial;
    qsspt * serialhandle;
    int sendData(void*,int);
    int receiveData(void * data,int size);
    pjrc_rawhid hidHandle;
    int setStartBit(int command){return command|0x20;}
    uint8_t	sspTxBuf[MAX_PACKET_BUF_SIZE];
    uint8_t	sspRxBuf[MAX_PACKET_BUF_SIZE];

    port * info;
};



#endif // OP_DFU_H
