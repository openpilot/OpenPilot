#ifndef GCSDIRS_H
#define GCSDIRS_H

#include "utils_global.h"

class QString;

class QTCREATOR_UTILS_EXPORT GCSDirs {
public:
    GCSDirs();

    static QString rootDir();

    static QString libraryPath(QString provider);

    static QString pluginPath(QString provider);

    static QString sharePath(QString provider);

    static QString gcsSharePath();

    static QString gcsPluginPath();

    static void debug();

private:
    Q_DISABLE_COPY(GCSDirs)
    class Private;
    Private *const d;
};

#endif // GCSDIRS_H
