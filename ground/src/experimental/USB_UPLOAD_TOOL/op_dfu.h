#ifndef OP_DFU_H
#define OP_DFU_H

#include <QByteArray>
#include <../../plugins/rawhid/pjrc_rawhid.h>
#include <QDebug>
#include <QFile>
 #include <QCryptographicHash>
#include <QList>
#include <iostream>
using namespace std;
#define BUF_LEN 64

//Command Line Options
#define DOWNLOAD            "-dn"    //done
#define DEVICE              "-d"     //done
//#define DOWNDESCRIPTION     "-dd"    //done
#define PROGRAMFW           "-p"     //done
#define PROGRAMDESC         "-w"     //done
#define VERIFY              "-v"     //done
#define COMPAREHASH         "-ch"
#define COMPAREALL          "-ca"
#define STATUSREQUEST       "-s"    //done
#define LISTDEVICES         "-ls"   //done
#define RESET               "-r"
#define JUMP                "-j"

class OP_DFU
{
public:
    enum TransferTypes
    {
        FW,
        Hash,
        Descript
    };
    enum CompareType
    {
        hashcompare,
        bytetobytecompare
    };

    enum Status
    {
        DFUidle,
        uploading,
        wrong_packet_received,
        too_many_packets,
        too_few_packets,
        Last_operation_Success,
        downloading,
        idle,
        Last_operation_failed,
        outsideDevCapabilities,
        abort

    };
    enum Actions
    {
        actionProgram,
        actionProgramAndVerify,
        actionDownload,
        actionCompareAll,
        actionCompareHash,
        actionListDevs,
        actionStatusReq,
        actionReset,
        actionJump
    };

    enum Commands
    {
        Reserved,
        Req_Capabilities,
        Rep_Capabilities,
        EnterDFU,
        JumpFW,
        Reset,
        Abort_Operation,
        Upload,
        Op_END,
        Download_Req,
        Download,
        Status_Request,
        Status_Rep,

    };
    struct device
    {
            int ID;
            int SizeOfHash;
            int SizeOfDesc;
            quint32 SizeOfCode;
            bool Readable;
            bool Writable;
    };

    void JumpToApp();
    void ResetDevice(void);
    bool enterDFU(int devNumber);
    bool StartUpload(qint32 numberOfBytes, TransferTypes type);
    bool UploadData(qint32 numberOfPackets,QByteArray data);
    Status UploadDescription(QString description);
    Status UploadFirmware(const QString &sfile, const bool &verify);
    Status StatusRequest();
    bool EndOperation();
    void printProgBar( int percent,QString const& label);
    QString DownloadDescription(int numberOfChars);
    QByteArray StartDownload(qint32 numberOfBytes, TransferTypes type);
    bool SaveByteArrayToFile(QString file,QByteArray const &array);
    void CopyWords(char * source, char* destination, int count);
   // QByteArray DownloadData(int devNumber,int numberOfPackets);
    OP_DFU(bool debug);
    bool findDevices();
    QList<device> devices;
    int numberOfDevices;
    QString StatusToString(OP_DFU::Status);
    OP_DFU::Status CompareFirmware(const QString &sfile, const CompareType &type);
private:
    bool debug;
    int RWFlags;

    pjrc_rawhid hidHandle;
    int setStartBit(int command){return command|0x20;}
};

#endif // OP_DFU_H
