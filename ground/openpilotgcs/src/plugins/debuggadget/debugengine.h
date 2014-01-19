#ifndef DEBUGENGINE_H
#define DEBUGENGINE_H
#include <QTextBrowser>
#include <QPointer>
#include <QMutex>

class debugengine {
// Add all missing constructor etc... to have singleton
    debugengine();
    ~debugengine();
public:
    static debugengine *getInstance();
    void setTextEdit(QTextBrowser *textEdit);
    void writeMessage(const QString &message);
    void setColor(const QColor &c);
    QMutex *mut_lock;
private:
    QPointer<QTextBrowser> _textEdit;
};

#endif // DEBUGENGINE_H
