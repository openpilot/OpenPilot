#include <QtCore/QCoreApplication>
#include <QThread>
#include <../../plugins/rawhid/pjrc_rawhid.h>
#include "op_dfu.h"
#include <QStringList>

#define PRIVATE false

int main(int argc, char *argv[])
{
    ///SSP/////////////////////////////////



    /////////////////////////////////////////////
    QCoreApplication a(argc, argv);
//    argc=4;
//    argv[1]="-ls";
//    argv[2]="-t";
//    argv[3]="COM3";
    if(argc>1||!PRIVATE)
    {
        bool use_serial=false;
        bool verify;
        bool debug=false;
        bool umodereset=false;
        OP_DFU::Actions action;
        QString file;
        QString serialport;
        QString description;
        int device=-1;
        QStringList args;
        for(int i=0;i<argc;++i)
        {
            args<<argv[i];
        }
        if(args.contains("-debug"))
            debug=true;
        if(args.contains("-ur"))
            umodereset=true;
        if(args.contains("-?")||(!PRIVATE && argc==1))
        {
            cout<<"_________________________________________________________________________\n";
            cout<<"| Commands                                                               |\n";
            cout<<"|                                                                        |\n";
            cout<<"| -ls                  : lists available devices                         |\n";
            cout<<"| -p <file>            : program hw (requires:-d - optionals:-v,-w)      |\n";
            cout<<"| -v                   : verify     (requires:-d)                        |\n";
            cout<<"| -dn <file>           : download firmware to file (requires:-d)         |\n";
            //   cout<<"| -dd <file>           : download discription (requires:-d)         |\n";
            cout<<"| -d <number Of Device : (requires: -p or -dn)                           |\n";
            cout<<"| -w <description>     : (requires: -p)                                  |\n";
            cout<<"| -ca <file>           : compares byte by byte current firmware with file|\n";
            cout<<"| -cc <file>           : compares CRC  of current firmware with file     |\n";
            cout<<"| -s                   : requests status of device                       |\n";
            cout<<"| -r                   : resets the device                               |\n";
            cout<<"| -j                   : exits bootloader and jumps to user FW           |\n";
            cout<<"| -debug               : prints debug information                        |\n";
            cout<<"| -t <port>            : uses serial port(requires:-ur)                  |\n";
            cout<<"| -ur <port>           : user mode reset*                                |\n";
            cout<<"|                                                                        |\n";
            cout<<"| examples:                                                              |\n";
            cout<<"|                                                                        |\n";
            cout<<"| program and verify device #0                                           |\n";
            cout<<"| OPUploadTool -p c:/OpenPilot.bin -w \"Openpilot Firmware\" -v -d 0       |\n";
            cout<<"|                                                                        |\n";
            cout<<"| Perform a quick compare of FW in file with FW in device #1             |\n";
            cout<<"| OPUploadTool -ch c:/OpenPilot2.bin -d 2                                |\n";
            cout<<"|                                                                        |\n";
            cout<<"| *requires valid user space firmwares already running                   |\n";
            cout<<"|________________________________________________________________________|\n";
            return 0;


        }

        else if(args.contains(PROGRAMFW))
        {
            if(args.contains(VERIFY))
            {
                verify=true;
            }
            else
                verify=false;
            if(args.contains(PROGRAMDESC))
            {
                if(args.indexOf(PROGRAMDESC)+1<args.length())
                {
                    description=(args[args.indexOf(PROGRAMDESC)+1]);
                }

            }
            action=OP_DFU::actionProgram;
            if(args.contains(DEVICE))
            {
                if(args.indexOf(DEVICE)+1<args.length())
                {
                    device=(args[args.indexOf(DEVICE)+1]).toInt();
                }
                if(args.indexOf(PROGRAMFW)+1<args.length())
                {
                    file=args[args.indexOf(PROGRAMFW)+1];
                }
            }
            else
            {
                cout<<("Device not specified\n");
                return -1;
            }

        }
        else if(args.contains(COMPARECRC) || args.contains(COMPAREALL))
        {
            int index;
            if(args.contains(COMPARECRC))
            {
                index=args.indexOf(COMPARECRC);
                action=OP_DFU::actionCompareCrc;
            }
            else
            {
                index=args.indexOf(COMPAREALL);
                action=OP_DFU::actionCompareAll;
            }
            if(args.contains(DEVICE))
            {
                if(args.indexOf(DEVICE)+1<args.length())
                {
                    device=(args[args.indexOf(DEVICE)+1]).toInt();
                }
                if(index+1<args.length())
                {
                    file=args[index+1];
                }
            }
            else
            {
                cout<<"Device not specified";
                return -1;
            }
        }
        else if(args.contains(DOWNLOAD))
        {
            int index;
            index=args.indexOf(DOWNLOAD);
            action=OP_DFU::actionDownload;

            if(args.contains(DEVICE))
            {
                if(args.indexOf(DEVICE)+1<args.length())
                {
                    device=(args[args.indexOf(DEVICE)+1]).toInt();
                }
                if(index+1<args.length())
                {
                    file=args[index+1];
                }
            }
            else
            {
                cout<<"Device not specified";
                return -1;
            }
        }
        else if(args.contains(STATUSREQUEST))
        {
            action=OP_DFU::actionStatusReq;
        }
        else if(args.contains(RESET))
        {
            action=OP_DFU::actionReset;
        }
        else if(args.contains(JUMP))
        {
            action=OP_DFU::actionJump;
        }

        else if(args.contains(LISTDEVICES))
        {
            action=OP_DFU::actionListDevs;
        }
        if((file.isEmpty()|| device==-1)  &&  action!=OP_DFU::actionReset && action!=OP_DFU::actionStatusReq && action!=OP_DFU::actionListDevs&& action!=OP_DFU::actionJump)
        {
            cout<<"wrong parameters";
            return -1;
        }
        if(args.contains(USE_SERIAL))
        {
            if(args.indexOf(USE_SERIAL)+1<args.length())
            {
                serialport=(args[args.indexOf(USE_SERIAL)+1]);
                use_serial=true;
            }
        }
        if(debug)
        {
            qDebug()<<"Action="<<(int)action;
            qDebug()<<"File="<<file;
            qDebug()<<"Device="<<device;
            qDebug()<<"Action="<<action;
            qDebug()<<"Desctription"<<description;
            qDebug()<<"Use Serial port"<<use_serial;
            if(use_serial)
                qDebug()<<"Port Name"<<serialport;
        }

        ///////////////////////////////////ACTIONS START///////////////////////////////////////////////////
        OP_DFU dfu(debug,use_serial,serialport,umodereset);
        if(!dfu.ready())
            return -1;

        dfu.AbortOperation();
        if(!dfu.enterDFU(0))
        {
            cout<<"Could not enter DFU mode\n";
            return -1;
        }
        if (debug)
        {
            OP_DFU::Status ret=dfu.StatusRequest();
            qDebug()<<dfu.StatusToString(ret);
        }
        if(!(action==OP_DFU::actionStatusReq || action==OP_DFU::actionReset || action== OP_DFU::actionJump))
        {
            dfu.findDevices();
            if(action==OP_DFU::actionListDevs)
            {
                cout<<"Found "<<dfu.numberOfDevices<<"\n";
                for(int x=0;x<dfu.numberOfDevices;++x)
                {
                    cout<<"Device #"<<x<<"\n";
                    cout<<"Device ID="<<dfu.devices[x].ID<<"\n";
                    cout<<"Device Readable="<<dfu.devices[x].Readable<<"\n";
                    cout<<"Device Writable="<<dfu.devices[x].Writable<<"\n";
                    cout<<"Device SizeOfCode="<<dfu.devices[x].SizeOfCode<<"\n";
                    cout<<"BL Version="<<dfu.devices[x].BL_Version<<"\n";
                    cout<<"Device SizeOfDesc="<<dfu.devices[x].SizeOfDesc<<"\n";
                    cout<<"FW CRC="<<dfu.devices[x].FW_CRC<<"\n";

                    int size=((OP_DFU::device)dfu.devices[x]).SizeOfDesc;
                    dfu.enterDFU(x);
                    cout<<"Description:"<<dfu.DownloadDescription(size).toLatin1().data()<<"\n";
                    cout<<"\n";
                }
                return 0;
            }
            if(device>dfu.numberOfDevices-1)
            {
                cout<<"Error:Invalid Device";
                return -1;
            }
//            if(dfu.numberOfDevices==1)
//                dfu.use_delay=false;
            if(!dfu.enterDFU(device))
            {
                cout<<"Error:Could not enter DFU mode\n";
                return -1;
            }
            if (action==OP_DFU::actionProgram)
            {
                if(((OP_DFU::device)dfu.devices[device]).Writable==false)
                {
                    cout<<"ERROR device not Writable\n";
                    return false;
                }

                OP_DFU::Status retstatus=dfu.UploadFirmware(file.toAscii(),verify, device);
                if(retstatus!=OP_DFU::Last_operation_Success)
                {
                    cout<<"Upload failed with code:"<<dfu.StatusToString(retstatus).toLatin1().data();
                    return -1;
                }
                if(!description.isEmpty())
                {
                    retstatus=dfu.UploadDescription(description);
                    if(retstatus!=OP_DFU::Last_operation_Success)
                    {
                        cout<<"Upload failed with code:"<<dfu.StatusToString(retstatus).toLatin1().data();
                        return -1;
                    }
                }
                cout<<"Uploading Succeded!\n";
            }
            else if (action==OP_DFU::actionDownload)
            {
                if(((OP_DFU::device)dfu.devices[device]).Readable==false)
                {
                    cout<<"ERROR device not readable\n";
                    return false;
                }
                qint32 size=((OP_DFU::device)dfu.devices[device]).SizeOfCode;
                bool ret=dfu.SaveByteArrayToFile(file.toAscii(),dfu.StartDownload(size,OP_DFU::FW));
                return ret;
            }
            else if(action==OP_DFU::actionCompareCrc)
            {
                dfu.CompareFirmware(file.toAscii(),OP_DFU::crccompare,device);
                return 1;
            }
            else if(action==OP_DFU::actionCompareAll)
            {
                if(((OP_DFU::device)dfu.devices[device]).Readable==false)
                {
                    cout<<"ERROR device not readable\n";
                    return false;
                }
                dfu.CompareFirmware(file.toAscii(),OP_DFU::bytetobytecompare,device);
                return 1;
            }

        }
        else if(action==OP_DFU::actionStatusReq)
        {
            cout<<"Current device status="<<dfu.StatusToString(dfu.StatusRequest()).toLatin1().data()<<"\n";
        }
        else if(action==OP_DFU::actionReset)
        {
            dfu.ResetDevice();
        }
        else if(action== OP_DFU::actionJump)
        {
            dfu.JumpToApp();
        }

        return 0;
    }
    return a.exec();
}
