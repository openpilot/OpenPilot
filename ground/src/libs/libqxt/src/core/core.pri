INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS  += qxtabstractconnectionmanager.h
HEADERS  += qxtabstractsignalserializer.h
HEADERS  += qxtalgorithms.h
HEADERS  += qxtboundcfunction.h
HEADERS  += qxtboundfunction.h
HEADERS  += qxtboundfunctionbase.h
HEADERS  += qxtcore.h
HEADERS  += qxtcommandoptions.h
HEADERS  += qxtcsvmodel.h
HEADERS  += qxtdaemon.h
HEADERS  += qxtdatastreamsignalserializer.h
HEADERS  += qxtdeplex.h
HEADERS  += qxtdeplex_p.h
HEADERS  += qxterror.h
HEADERS  += qxtfifo.h
HEADERS  += qxtglobal.h
HEADERS  += qxthmac.h
HEADERS  += qxtjson.h
HEADERS  += qxtjob.h
HEADERS  += qxtjob_p.h
HEADERS  += qxtlinesocket.h
HEADERS  += qxtlinesocket_p.h
HEADERS  += qxtlinkedtree.h
HEADERS  += qxtlocale.h
HEADERS  += qxtlocale_data_p.h
HEADERS  += qxtlogger.h
HEADERS  += qxtlogger_p.h
HEADERS  += qxtloggerengine.h
HEADERS  += qxtlogstream.h
HEADERS  += qxtlogstream_p.h
HEADERS  += qxtmetaobject.h
HEADERS  += qxtmetatype.h
HEADERS  += qxtmodelserializer.h
HEADERS  += qxtmultisignalwaiter.h
HEADERS  += qxtnamespace.h
HEADERS  += qxtnull.h
HEADERS  += qxtnullable.h
HEADERS  += qxtpairlist.h
HEADERS  += qxtpimpl.h
HEADERS  += qxtpipe.h
HEADERS  += qxtpipe_p.h
HEADERS  += qxtpointerlist.h
HEADERS  += qxtsharedprivate.h
HEADERS  += qxtsignalgroup.h
HEADERS  += qxtsignalwaiter.h
HEADERS  += qxtslotjob.h
HEADERS  += qxtslotjob_p.h
HEADERS  += qxtslotmapper.h
HEADERS  += qxtstdio.h
HEADERS  += qxtstdio_p.h
HEADERS  += qxtstdstreambufdevice.h
HEADERS  += qxttimer.h
HEADERS  += qxttypelist.h
HEADERS  += qxtrpcservice.h
HEADERS  += qxtrpcservice_p.h

SOURCES  += qxtabstractconnectionmanager.cpp
SOURCES  += qxtcommandoptions.cpp
SOURCES  += qxtcsvmodel.cpp
SOURCES  += qxtdaemon.cpp
SOURCES  += qxtdatastreamsignalserializer.cpp
SOURCES  += qxtdeplex.cpp
SOURCES  += qxterror.cpp
SOURCES  += qxtfifo.cpp
SOURCES  += qxtglobal.cpp
SOURCES  += qxthmac.cpp
SOURCES  += qxtlocale.cpp
SOURCES  += qxtjson.cpp
SOURCES  += qxtjob.cpp
SOURCES  += qxtlinesocket.cpp
SOURCES  += qxtlinkedtree.cpp
SOURCES  += qxtlogger.cpp
SOURCES  += qxtloggerengine.cpp
SOURCES  += qxtlogstream.cpp
SOURCES  += qxtmetaobject.cpp
SOURCES  += qxtmodelserializer.cpp
SOURCES  += qxtmultisignalwaiter.cpp
SOURCES  += qxtnull.cpp
SOURCES  += qxtpipe.cpp
SOURCES  += qxtpointerlist.cpp
SOURCES  += qxtsignalgroup.cpp
SOURCES  += qxtsignalwaiter.cpp
SOURCES  += qxtslotjob.cpp
SOURCES  += qxtslotmapper.cpp
SOURCES  += qxtstdio.cpp
SOURCES  += qxtstdstreambufdevice.cpp
SOURCES  += qxttimer.cpp
SOURCES  += qxtrpcservice.cpp

!symbian {
    # QxtSerialDevice and QxtFileLock
    # are disabled for Symbian pending implementation

    HEADERS  += qxtfilelock.h
    HEADERS  += qxtfilelock_p.h

    SOURCES  += qxtfilelock.cpp

    unix {
        HEADERS  += qxtserialdevice.h
        HEADERS  += qxtserialdevice_p.h

        SOURCES  += qxtfilelock_unix.cpp
        SOURCES  += qxtserialdevice.cpp
        SOURCES  += qxtserialdevice_unix.cpp
    }
    
    win32 {
        SOURCES  += qxtfilelock_win.cpp
    }
}
