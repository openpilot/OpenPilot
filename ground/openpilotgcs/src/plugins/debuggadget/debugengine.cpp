#include "debugengine.h"

debugengine::debugengine()
{
    mut_lock = new QMutex(QMutex::Recursive);
}

debugengine *debugengine::getInstance()
{
    static debugengine objectInstance;

    return &objectInstance;
}

debugengine::~debugengine()
{
    delete mut_lock;
    mut_lock = NULL;
}

void debugengine::setTextEdit(QTextBrowser *textEdit)
{
    QMutexLocker lock(mut_lock);

    _textEdit = textEdit;
}

void debugengine::writeMessage(const QString &message)
{
    QMutexLocker lock(mut_lock);

    if (_textEdit) {
        _textEdit->append(message);
    }
}

void debugengine::setColor(const QColor &c)
{
    QMutexLocker lock(mut_lock);

    if (_textEdit) {
        _textEdit->setTextColor(c);
    }
}
