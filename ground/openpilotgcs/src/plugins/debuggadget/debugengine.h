#ifndef DEBUGENGINE_H
#define DEBUGENGINE_H
#include "qxtbasicstdloggerengine.h"
#include <QObject>
class debugengine:public QObject,public QxtBasicSTDLoggerEngine
{
    Q_OBJECT
public:
    debugengine();
protected:
    void writeToStdErr ( const QString & level, const QList<QVariant> & msgs );
    void writeToStdOut ( const QString & level, const QList<QVariant> & msgs );
signals:
    void dbgMsgError( const QString & level, const QList<QVariant> & msgs );
    void dbgMsg( const QString & level, const QList<QVariant> & msgs );
};

#endif // DEBUGENGINE_H
