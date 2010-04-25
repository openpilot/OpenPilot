#ifndef QXTLOOKUPLINEEDIT_H_INCLUDED
#define QXTLOOKUPLINEEDIT_H_INCLUDED

#include <QLineEdit>
#include <QKeySequence>
#include <QxtPimpl>
#include "qxtglobal.h"

class QxtLookupLineEditPrivate;
class QAbstractItemModel;
class QKeyEvent;

class QXT_GUI_EXPORT QxtLookupLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    QxtLookupLineEdit(QWidget *parent = 0);
    ~QxtLookupLineEdit();

    void setSourceModel  (QAbstractItemModel *model);
    void setLookupColumn (const int column);
    void setDataColumn   (const int column); 
    void setLookupRole   (const int role);
    void setPopupTrigger (const QKeySequence &trigger); 

    QAbstractItemModel *sourceModel() const;
    int lookupColumn ( ) const;
    int dataColumn   ( ) const;
    int lookupRole   ( ) const;
    QKeySequence popupTrigger () const; 

protected:
    virtual void keyPressEvent ( QKeyEvent * event );

protected slots:
    virtual void showPopup();

signals:
    void selected ();


private:
    QXT_DECLARE_PRIVATE(QxtLookupLineEdit);
    
};

#endif // QXTLOOKUPLINEEDIT_H_INCLUDED
