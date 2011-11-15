#include "debugengine.h"
debugengine::debugengine()
{
}

void debugengine::writeToStdErr(const QString &level, const QList<QVariant> &msgs)
{
    emit dbgMsgError(level,msgs);
}
void debugengine::writeToStdOut(const QString &level, const QList<QVariant> &msgs)
{
    emit dbgMsg(level,msgs);
}
