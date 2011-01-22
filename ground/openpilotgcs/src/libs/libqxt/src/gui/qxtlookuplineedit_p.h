#ifndef QXTLOOKUPLINEEDIT_P_H_INCLUDED
#define QXTLOOKUPLINEEDIT_P_H_INCLUDED

#include "qxtpimpl.h"
#include "qxtfilterdialog.h"
#include <QKeySequence>
#include <QObject>
#include <QRegExp>
#include <QAction>

class QxtLookupLineEdit;
class QAbstractItemModel;

class QxtLookupLineEditPrivate : public QxtPrivate <QxtLookupLineEdit>
{
    public:
        QxtLookupLineEditPrivate();
        QXT_DECLARE_PUBLIC(QxtLookupLineEdit);

        int                 m_dataColumn;
        int                 m_lookupColumn;
        int                 m_lookupRole;
        QAbstractItemModel* m_sourceModel;
        QKeySequence        m_trigger;

};

#endif //QXTLOOKUPLINEEDIT_P_H_INCLUDED
