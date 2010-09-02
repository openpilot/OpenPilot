#ifndef OP_DFU_H
#define OP_DFU_H

#include <QByteArray>
#include <../../plugins/rawhid/pjrc_rawhid.h>
#include <QDebug>
#include <QFile>
 #include <QCryptographicHash>
#include <QList>
#define BUF_LEN 64
class OP_DFU
{
public:
    enum TransferTypes
    {
        FW,
        Hash,
        Descript
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
        Last_operation_failed

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
    void enterDFU(int devNumber);
    void StartUpload(qint32 numberOfBytes, TransferTypes type);
    void UploadData(qint32 numberOfPackets,QByteArray data);
    void UploadDescription(QString description);
    void UploadFirmware(const QString &sfile);
    int StatusRequest();
    void EndOperation();
    QString DownloadDescription(int devNumber,int numberOfChars);
    QByteArray StartDownload(qint32 numberOfPackets, TransferTypes type);
    void CopyWords(char * source, char* destination, int count);
   // QByteArray DownloadData(int devNumber,int numberOfPackets);
    OP_DFU();
    void findDevices();
private:
    int numberOfDevices;
    int RWFlags;
    QList<device> devices;
    pjrc_rawhid hidHandle;
    int setStartBit(int command){return command|0x20;}
};

#endif // OP_DFU_H
