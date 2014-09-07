#include <QtCore/QCoreApplication>
#include <QThread>
#include "op_dfu.h"
#include <QStringList>

void showProgress(QString status);
void progressUpdated(int percent);
void usage(QTextStream *standardOutput);
QString label;
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream standardOutput(stdout);
    int argumentCount = QCoreApplication::arguments().size();
    bool use_serial   = false;
    bool verify;
    bool debug = false;
    bool umodereset   = false;
    OP_DFU::Actions action;
    QString file;
    QString serialport;
    QString description;
    int device = -1;
    QStringList args = QCoreApplication::arguments();

    if (args.contains("-debug")) {
        debug = true;
    }
    if (args.contains("-ur")) {
        umodereset = true;
    }
    standardOutput << "OpenPilot serial firmware uploader tool." << endl;
    if (args.indexOf(PROGRAMFW) + 1 < args.length()) {
        file = args[args.indexOf(PROGRAMFW) + 1];
    }
    if (args.contains(DEVICE)) {
        if (args.indexOf(DEVICE) + 1 < args.length()) {
            device = (args[args.indexOf(DEVICE) + 1]).toInt();
        }
    } else {
        device = 0;
    }

    if (argumentCount == 0 || args.contains("-?") || !args.contains(USE_SERIAL)) {
        usage(&standardOutput);
        return -1;
    } else if (args.contains(PROGRAMFW)) {
        if (args.contains(VERIFY)) {
            verify = true;
        } else {
            verify = false;
        }
        if (args.contains(PROGRAMDESC)) {
            if (args.indexOf(PROGRAMDESC) + 1 < args.length()) {
                description = (args[args.indexOf(PROGRAMDESC) + 1]);
            }
        }
        action = OP_DFU::actionProgram;
    } else if (args.contains(COMPARECRC) || args.contains(COMPAREALL)) {
        int index;
        if (args.contains(COMPARECRC)) {
            index  = args.indexOf(COMPARECRC);
            action = OP_DFU::actionCompareCrc;
        } else {
            index  = args.indexOf(COMPAREALL);
            action = OP_DFU::actionCompareAll;
        }
    } else if (args.contains(DOWNLOAD)) {
        int index;
        index  = args.indexOf(DOWNLOAD);
        action = OP_DFU::actionDownload;
    } else if (args.contains(STATUSREQUEST)) {
        action = OP_DFU::actionStatusReq;
    } else if (args.contains(RESET)) {
        action = OP_DFU::actionReset;
    } else if (args.contains(JUMP)) {
        action = OP_DFU::actionJump;
    } else if (args.contains(LISTDEVICES)) {
        action = OP_DFU::actionListDevs;
    }
    if ((file.isEmpty() || device == -1) && action != OP_DFU::actionReset && action != OP_DFU::actionStatusReq && action != OP_DFU::actionListDevs && action != OP_DFU::actionJump) {
        usage(&standardOutput);
        return -1;
    }
    if (args.contains(USE_SERIAL)) {
        if (args.indexOf(USE_SERIAL) + 1 < args.length()) {
            serialport = (args[args.indexOf(USE_SERIAL) + 1]);
            use_serial = true;
        }
    }
    if (debug) {
        qDebug() << "Action=" << (int)action << endl;
        qDebug() << "File=" << file << endl;
        qDebug() << "Device=" << device << endl;
        qDebug() << "Action=" << action << endl;
        qDebug() << "Desctription" << description << endl;
        qDebug() << "Use Serial port" << use_serial << endl;
        if (use_serial) {
            qDebug() << "Port Name" << serialport << endl;
        }
    }
    if (use_serial) {
        if (args.contains(NO_COUNTDOWN)) {} else {
            standardOutput << "Connect the board" << endl;
            label = "";
            for (int i = 0; i < 6; i++) {
                progressUpdated(i * 100 / 5);
                QThread::msleep(500);
            }
        }
        standardOutput << endl << "Connect the board NOW" << endl;
        QThread::msleep(1000);
    }
    ///////////////////////////////////ACTIONS START///////////////////////////////////////////////////
    OP_DFU::DFUObject dfu(debug, use_serial, serialport);

    QObject::connect(&dfu, &OP_DFU::DFUObject::operationProgress, showProgress);
    QObject::connect(&dfu, &OP_DFU::DFUObject::progressUpdated, progressUpdated);

    if (!dfu.ready()) {
        return -1;
    }
    dfu.AbortOperation();
    if (!dfu.enterDFU(0)) {
        standardOutput << "Could not enter DFU mode\n" << endl;
        return -1;
    }
    if (debug) {
        OP_DFU::Status ret = dfu.StatusRequest();
        qDebug() << dfu.StatusToString(ret);
    }
    if (!(action == OP_DFU::actionStatusReq || action == OP_DFU::actionReset || action == OP_DFU::actionJump)) {
        dfu.findDevices();
        if (action == OP_DFU::actionListDevs) {
            standardOutput << "Found " << dfu.numberOfDevices << "\n" << endl;
            for (int x = 0; x < dfu.numberOfDevices; ++x) {
                standardOutput << "Device #" << x << "\n" << endl;
                standardOutput << "Device ID=" << dfu.devices[x].ID << "\n" << endl;
                standardOutput << "Device Readable=" << dfu.devices[x].Readable << "\n" << endl;
                standardOutput << "Device Writable=" << dfu.devices[x].Writable << "\n" << endl;
                standardOutput << "Device SizeOfCode=" << dfu.devices[x].SizeOfCode << "\n" << endl;
                standardOutput << "BL Version=" << dfu.devices[x].BL_Version << "\n" << endl;
                standardOutput << "Device SizeOfDesc=" << dfu.devices[x].SizeOfDesc << "\n" << endl;
                standardOutput << "FW CRC=" << dfu.devices[x].FW_CRC << "\n";

                int size = ((OP_DFU::device)dfu.devices[x]).SizeOfDesc;
                dfu.enterDFU(x);
                standardOutput << "Description:" << dfu.DownloadDescription(size).toLatin1().data() << "\n" << endl;
                standardOutput << "\n";
            }
            return 0;
        }
        if (device > dfu.numberOfDevices - 1) {
            standardOutput << "Error:Invalid Device" << endl;
            return -1;
        }
        if (dfu.numberOfDevices == 1) {
            dfu.use_delay = false;
        }
        if (!dfu.enterDFU(device)) {
            standardOutput << "Error:Could not enter DFU mode\n" << endl;
            return -1;
        }
        if (action == OP_DFU::actionProgram) {
            if (((OP_DFU::device)dfu.devices[device]).Writable == false) {
                standardOutput << "ERROR device not Writable\n" << endl;
                return false;
            }
            standardOutput << "Uploading..." << endl;
            bool retstatus = dfu.UploadFirmware(file.toLatin1(), verify, device);
            if (!retstatus) {
                standardOutput << "Upload failed with code:" << retstatus << endl;
                return -1;
            }
            if (!description.isEmpty()) {
                retstatus = dfu.UploadDescription(description);
                if (retstatus != OP_DFU::Last_operation_Success) {
                    standardOutput << "Upload failed with code:" << retstatus << endl;
                    return -1;
                }
            }
            while (!dfu.isFinished()) {
                QThread::msleep(500);
            }
            standardOutput << "Uploading Succeded!\n" << endl;
        } else if (action == OP_DFU::actionDownload) {
            if (((OP_DFU::device)dfu.devices[device]).Readable == false) {
                standardOutput << "ERROR device not readable\n" << endl;
                return false;
            }
            qint32 size = ((OP_DFU::device)dfu.devices[device]).SizeOfCode;
            QByteArray fw;
            dfu.DownloadFirmware(&fw, 0);
            bool ret    = dfu.SaveByteArrayToFile(file.toLatin1(), fw);
            return ret;
        } else if (action == OP_DFU::actionCompareCrc) {
            dfu.CompareFirmware(file.toLatin1(), OP_DFU::crccompare, device);
            return 1;
        } else if (action == OP_DFU::actionCompareAll) {
            if (((OP_DFU::device)dfu.devices[device]).Readable == false) {
                standardOutput << "ERROR device not readable\n" << endl;
                return false;
            }
            dfu.CompareFirmware(file.toLatin1(), OP_DFU::bytetobytecompare, device);
            return 1;
        }
    } else if (action == OP_DFU::actionStatusReq) {
        standardOutput << "Current device status=" << dfu.StatusToString(dfu.StatusRequest()).toLatin1().data() << "\n" << endl;
    } else if (action == OP_DFU::actionReset) {
        dfu.ResetDevice();
    } else if (action == OP_DFU::actionJump) {
        dfu.JumpToApp(false, false);
    }
    return 0;

    // return a.exec();
}

void showProgress(QString status)
{
    QTextStream standardOutput(stdout);

    standardOutput << status << endl;
    label = status;
}

void progressUpdated(int percent)
{
    std::string bar;

    for (int i = 0; i < 50; i++) {
        if (i < (percent / 2)) {
            bar.replace(i, 1, "=");
        } else if (i == (percent / 2)) {
            bar.replace(i, 1, ">");
        } else {
            bar.replace(i, 1, " ");
        }
    }
    std::cout << "\r" << label.toLatin1().data() << "[" << bar << "] ";
    std::cout.width(3);
    std::cout << percent << "%     " << std::flush;
}
void usage(QTextStream *standardOutput)
{
    *standardOutput << "_________________________________________________________________________\n";
    *standardOutput << "| Commands                                                               |\n";
    *standardOutput << "|                                                                        |\n";
    *standardOutput << "| -ls                  : lists available devices                         |\n";
    *standardOutput << "| -p <file>            : program hw (requires:-d - optionals:-v,-w)      |\n";
    *standardOutput << "| -v                   : verify     (requires:-d)                        |\n";
    *standardOutput << "| -dn <file>           : download firmware to file                       |\n";
    // *standardOutput  << "| -dd <file>           : download discription (requires:-d)              |\n";
    *standardOutput << "| -d <Device Number>   : target device number (default 0, first device)  |\n";
    // *standardOutput  << "| -w <description>     : (requires: -p)                                  |\n";
    *standardOutput << "| -ca <file>           : compares byte by byte current firmware with file|\n";
    *standardOutput << "| -cc <file>           : compares CRC  of current firmware with file     |\n";
    *standardOutput << "| -s                   : requests status of device                       |\n";
    *standardOutput << "| -r                   : resets the device                               |\n";
    *standardOutput << "| -j                   : exits bootloader and jumps to user FW           |\n";
    *standardOutput << "| -debug               : prints debug information                        |\n";
    *standardOutput << "| -t <port>            : uses serial port                                |\n";
    *standardOutput << "| -i                   : immediate, doesn't show the connection countdown|\n";
    // *standardOutput  << "| -ur <port>           : user mode reset*                                |\n";
    *standardOutput << "|                                                                        |\n";
    *standardOutput << "| examples:                                                              |\n";
    *standardOutput << "|                                                                        |\n";
    *standardOutput << "| program and verify the fist device device connected to COM1            |\n";
    *standardOutput << "| OPUploadTool -p c:/gpsp.opfw -v -t COM1                                |\n";
    *standardOutput << "|                                                                        |\n";
    *standardOutput << "| program and verify the fist device device connected to COM1            |\n";
    *standardOutput << "| OPUploadTool -p c:/gpsp.opfw -v -t COM1                                |\n";
    *standardOutput << "|                                                                        |\n";
    *standardOutput << "| Perform a quick compare of FW in file with FW in device #1             |\n";
    *standardOutput << "| OPUploadTool -ch /home/user1/gpsp.opfw  -t ttyUSB0                     |\n";
    *standardOutput << "|                                                                        |\n";
    // *standardOutput  << "| *requires valid user space firmwares already running                   |\n";
    *standardOutput << "|________________________________________________________________________|\n";
    *standardOutput << endl;
}

void howToUsage(QTextStream *standardOutput)
{
    *standardOutput << "run the tool with -? for more informations" << endl;
}
