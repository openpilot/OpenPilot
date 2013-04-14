#ifndef QXTFILTERDIALOG_P_H_INCLUDED
#define QXTFILTERDIALOG_P_H_INCLUDED

#include <QObject>
#include <QModelIndex>
#include <QRegExp>
#include <QPointer>
#include "qxtpimpl.h"

class QxtFilterDialog;
class QAbstractItemModel;
class QTreeView;
class QSortFilterProxyModel;
class QLineEdit;
class QCheckBox;
class QComboBox;

class QxtFilterDialogPrivate : public QObject, public QxtPrivate<QxtFilterDialog>
{
    Q_OBJECT
    public:
        QxtFilterDialogPrivate();
        QXT_DECLARE_PUBLIC(QxtFilterDialog);

        /*widgets*/
        QCheckBox * matchCaseOption;
        QCheckBox * filterModeOption;
        QComboBox * filterMode;
        QTreeView * listingTreeView;
        QLineEdit * lineEditFilter;

        /*models*/
        QPointer<QAbstractItemModel>    model;
        QSortFilterProxyModel*          proxyModel;
        
        /*properties*/
        int lookupColumn;
        int lookupRole;
        QRegExp::PatternSyntax syntax;
        Qt::CaseSensitivity caseSensitivity;

        /*result*/
        QModelIndex selectedIndex;

        /*member functions*/
        void updateFilterPattern();

    public slots:
        void createRegExpPattern(const QString &rawText);
        void filterModeOptionStateChanged(const int state);
        void matchCaseOptionStateChanged(const int state);
        void filterModeChoosen(int index);
};


#endif