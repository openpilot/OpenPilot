#include <QtCore/QCoreApplication>
#include <QThread>
#include <../../plugins/rawhid/pjrc_rawhid.h>
#include "op_dfu.h"
#include <QStringList>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if(argc>1)
    {
    bool debug=false;
    OP_DFU::Actions action;
    QString file;
    QString description;
    int device=-1;
    QStringList args;
    for(int i=0;i<argc;++i)
    {
        args<<argv[i];
    }

    if(args.contains("-?"))
    {
        cout<<"| Commands\n";
        cout<<"| \n";
        cout<<"| -ls                  : lists available devices\n";
        cout<<"| -p <file>            : program hw (requires:-d - optionals:-v,-w)\n";
        cout<<"| -dn <file>           : download firmware to file (requires:-d)\n";
        cout<<"| -dd <file>           : download discription (requires:-d)\n";
        cout<<"| -d <number Of Device : (requires: -p or -dn)\n";
        cout<<"| -w <description>     : (requires: -p)\n";
        cout<<"| -ca <file>           : compares byte by byte current firmware with file\n";
        cout<<"| -ch <file>           : compares hash of current firmware with file\n";
        cout<<"| -s                   : requests status of device\n";
        return 0;


    }
    else if(args.contains("-dd"))
    {
        action=OP_DFU::downdesc;
        if(args.contains(DEVICE))
        {
            if(args.indexOf(DEVICE)+1<args.length())
            {
                device=(args[args.indexOf(DEVICE)+1]).toInt();
            }
        }
    }
    else if(args.contains("-p"))
    {
        if(args.contains("-w"))
        {
            if(args.indexOf("-w")+1<args.length())
            {
                description=(args[args.indexOf("-w")+1]);
            }

        }
        action=OP_DFU::program;
        if(args.contains(DEVICE))
        {
            if(args.indexOf(DEVICE)+1<args.length())
            {
                device=(args[args.indexOf(DEVICE)+1]).toInt();
            }
            if(args.indexOf("-p")+1<args.length())
            {
                file=args[args.indexOf("-p")+1];
                           }
        }
        else
        {
            cout<<("Device not specified\n");
            return -1;
        }
        if(args.contains("-v"))
            action=OP_DFU::programandverify;
    }
    else if(args.contains("-ch") || args.contains("-ca"))
    {
        int index;
        if(args.contains("-ch"))
        {
            index=args.indexOf("-ch");
            action=OP_DFU::comparehash;
        }
        else
        {
            index=args.indexOf("-ca");
            action=OP_DFU::compareall;
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
            action=OP_DFU::download;

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
    else if(args.contains("-s"))
    {
        action=OP_DFU::statusreq;
    }
    else if(args.contains("-ls"))
    {
        action=OP_DFU::listdevs;
    }
    if((file.isEmpty()|| device==-1)  &&  action!=OP_DFU::downdesc && action!=OP_DFU::statusreq && action!=OP_DFU::listdevs)
    {
        cout<<"wrong parameters";
        return -1;
    }

//    qDebug()<<"Action="<<(int)action;
  //  qDebug()<<"File="<<file;
  //  qDebug()<<"Device="<<device;
  //  qDebug()<<"Action="<<action;
    OP_DFU dfu(debug);
    if(!dfu.enterDFU(0))
    {
        cout<<"Could not enter DFU mode\n";
        return -1;
    }
    if(action!=OP_DFU::statusreq)
    {
        dfu.findDevices();
        if(action==OP_DFU::listdevs)
        {
            cout<<"Found "<<dfu.numberOfDevices<<"\n";
            for(int x=0;x<dfu.numberOfDevices;++x)
            {
                cout<<"Device #"<<x+1<<"\n";
                cout<<"Device ID="<<dfu.devices[x].ID<<"\n";
                cout<<"Device Readable="<<dfu.devices[x].Readable<<"\n";
                cout<<"Device Writable="<<dfu.devices[x].Writable<<"\n";
                cout<<"Device SizeOfCode="<<dfu.devices[x].SizeOfCode<<"\n";
                cout<<"Device SizeOfHash="<<dfu.devices[x].SizeOfHash<<"\n";
                cout<<"Device SizeOfDesc="<<dfu.devices[x].SizeOfDesc<<"\n";
                cout<<"\n";
            }
            return 0;
        }
        else if (action==OP_DFU::program)
        {
            if(device>dfu.numberOfDevices)
            {
                cout<<"Error:Invalid Device";
                return -1;
            }
            if(!dfu.enterDFU(device))
            {
                cout<<"Error:Could not enter DFU mode\n";
                return -1;
            }
            OP_DFU::Status retstatus=dfu.UploadFirmware(file.toAscii());
            if(retstatus!=OP_DFU::Last_operation_Success)
            {
                cout<<"Upload failed with code:"<<dfu.StatusToString(retstatus).data();
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
        else if (action==OP_DFU::download)
        {
            qint32 size=((OP_DFU::device)dfu.devices[device]).SizeOfCode;

        }
        else if(action==OP_DFU::downdesc)
        {
            int size=((OP_DFU::device)dfu.devices[device]).SizeOfDesc;
            cout<<"Description:"<<dfu.DownloadDescription(size).toLatin1().data();
        }
    }
    return 0;
}

OP_DFU dfu(true);
    dfu.enterDFU(0);
    //    dfu.enterDFU(1);
    //    dfu.StartUpload(4,OP_DFU::Descript);
    //    QByteArray array;
    //    array[0]=11;
    //    array[1]=2;
    //    array[2]=3;
    //    array[3]=4;
    //    array[4]=5;
    //    array[5]=6;
    //    array[6]=7;
    //    array[7]=8;
    //    dfu.UploadData(8,array);
    //   dfu.UploadDescription(1,"jose manuel");
    // QString str=dfu.DownloadDescription(1,12);
    // dfu.JumpToApp();
  //  dfu.findDevices();

  //  dfu.UploadFirmware(filename.toAscii());
    // dfu.UploadDescription("josemanuel");
     QString str=dfu.DownloadDescription(12);
    // dfu.JumpToApp();
      qDebug()<<"Description="<<str;
    return a.exec();
}
