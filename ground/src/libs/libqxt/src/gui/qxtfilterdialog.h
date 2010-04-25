#ifndef QXTFILTERDIALOG_H_INCLUDED
#define QXTFILTERDIALOG_H_INCLUDED


#include <QDialog>
#include <QModelIndex>
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QAbstractItemModel;
class QxtFilterDialogPrivate;
class QKeyEvent;

class QXT_GUI_EXPORT QxtFilterDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(int      lookupColumn    READ lookupColumn   WRITE setLookupColumn   )
    Q_PROPERTY(int      lookupRole      READ lookupRole     WRITE setLookupRole     )
    Q_PROPERTY(QString  filterText      READ filterText     WRITE setFilterText     )

    public:
        QxtFilterDialog(QWidget *parent = 0);

        void                    setSourceModel      (QAbstractItemModel *model);

        QRegExp::PatternSyntax  patternSyntax       ();
        

        QAbstractItemModel *    sourceModel         ( ) const;
        int                     lookupColumn        ( ) const;
        int                     lookupRole          ( ) const;
        QString                 filterText          ( ) const;
        QModelIndex             selectedIndex       ( ) const;

        static QModelIndex      getIndex            (QWidget* parent = 0, QAbstractItemModel *model = 0
                                                    , int column = 0, int role = Qt::DisplayRole
                                                    , QString filterText = QString());

    protected:
        virtual void            keyPressEvent       (QKeyEvent *e);

    public slots:
        virtual void            done                ( int r ); 
        virtual void            accept              ();
        virtual void            reject              ();
        void                    setPatternSyntax    (QRegExp::PatternSyntax syntax);
        void                    setCaseSensitivity  (Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);
        void                    setFilterText       (QString text);
        void                    setLookupColumn     (int column);
        void                    setLookupRole       (int role);

    private:
        QXT_DECLARE_PRIVATE(QxtFilterDialog);
};

#endif
