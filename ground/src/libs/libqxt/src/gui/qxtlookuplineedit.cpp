#include "qxtlookuplineedit.h"
#include "qxtlookuplineedit_p.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QTreeView>
#include <QDialog>
#include <QKeyEvent>
#include <QRegExp>
#include <QPushButton>
#include <QModelIndex>

/*!
  \class QxtLookupLineEdit QxtLookupLineEdit
  \brief The QxtLookupLineEdit class is a QLineEdit to select from data provided by a QAbstractItemModel.
  QxtLookupLineEdit uses QxtFilterDialog to make it easy for the user,
  to select data from a large dataset.
  \code
    QSqlQueryModel *model = new QSqlQueryModel(parent);
    model->setQuery("Select employeeNr,name,forename,hometown,phone from employees;",dbConnection);
    QxtLookupLineEdit* edit = new QxtLookupLineEdit(parent);
    edit->setSourceModel(model);
    //we want to search by name not by nr so use the name column
    edit->setLookupColumn(1);
    //we need the employeeNr in the lineEdit , so choose col 0 as dataColumn
    edit->setDataColumn(0);
    //we want to seek the Display Role
    edit->setLookupRole(Qt::DisplayRole);
    //if the user presses * in the lineEdit open the popup
    edit->setPopupTrigger(QKeySequence("*"));
  \endcode
  */


QxtLookupLineEditPrivate::QxtLookupLineEditPrivate()
{
}

QxtLookupLineEdit::QxtLookupLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    QXT_INIT_PRIVATE(QxtLookupLineEdit);
    qxt_d().m_dataColumn = qxt_d().m_lookupColumn = 0;
    qxt_d().m_lookupRole = Qt::DisplayRole;
    qxt_d().m_sourceModel = 0;
}

QxtLookupLineEdit::~QxtLookupLineEdit()
{
    
}

/*!
  \brief returns the sourceModel
  \sa setSourceModel(QAbstractItemModel *model)
 */
QAbstractItemModel *QxtLookupLineEdit::sourceModel() const
{
    return qxt_d().m_sourceModel;
}
/*!
  \brief returns the column the lineEdit is looking
  \sa setLookupColumn(const int column)
 */
int QxtLookupLineEdit::lookupColumn () const
{
    return qxt_d().m_lookupColumn;
}
/*!
  \brief returns the role the lineEdit is looking
  \sa setLookupRole(const int role)
 */
int QxtLookupLineEdit::lookupRole () const
{
    return qxt_d().m_lookupRole;
}
/*!
  \brief returns the trigger that opens the popup
  \sa setPopupTrigger (const QKeySequence &trigger)
 */
QKeySequence QxtLookupLineEdit::popupTrigger () const
{
    return qxt_d().m_trigger;
}

/*!
  \brief sets the sourceModel used in the popup dialog
  */
void QxtLookupLineEdit::setSourceModel(QAbstractItemModel *model)
{
    qxt_d().m_sourceModel = model;
}
/*!
  \brief sets the column wherethe popup dialog searches
  */
void QxtLookupLineEdit::setLookupColumn (const int column)
{
    qxt_d().m_lookupColumn = (column);
}
/*!
  \brief sets the model role the popup should use
  */
void QxtLookupLineEdit::setLookupRole (const int role)
{
    qxt_d().m_lookupRole = (role);
}
/*!
  \brief Sets the popup trigger that makes QxtLookupLineEdit open the popup dialog
  */
void QxtLookupLineEdit::setPopupTrigger (const QKeySequence &trigger)
{
    qxt_d().m_trigger = trigger;
}

void QxtLookupLineEdit::showPopup ()
{
    /*open wildcard dialog*/
    if(!sourceModel())
        return;

    QString tempText = text();
    if(hasSelectedText()){
        QString selected = selectedText();
        tempText.replace(selected,"");
    }

    QModelIndex currIndex = QxtFilterDialog::getIndex(this,qxt_d().m_sourceModel,qxt_d().m_lookupColumn,qxt_d().m_lookupRole,tempText);
    if(currIndex.isValid()){
        QModelIndex dataIndex = sourceModel()->index(currIndex.row(),dataColumn());
        if(dataIndex.isValid())
        {
            setText(dataIndex.data(lookupRole()).toString());
            emit selected();
            this->nextInFocusChain()->setFocus();
        }
    }

#if 0

    QPoint pos = this->geometry().topLeft();
    if(qobject_cast<QWidget*>(this->parent()))
        pos = qobject_cast<QWidget*>(this->parent())->mapToGlobal(pos);

    qxt_d().filterView->setGeometry(QRect(pos,QSize(400,300)));
    qxt_d().filterView->setFilterText(text());
    int ret = qxt_d().filterView->exec();

    if(ret == QDialog::Accepted)
    {
        QModelIndex currIndex = qxt_d().filterView->selectedIndex();
        QModelIndex dataIndex = sourceModel()->index(currIndex.row(),dataColumn());
        if(dataIndex.isValid())
        {
            setText(dataIndex.data(lookupRole()).toString());
            this->nextInFocusChain()->setFocus();
        }
    }
#endif
}

void QxtLookupLineEdit::keyPressEvent ( QKeyEvent * event )
{
    QKeySequence currSeq = QKeySequence(event->key() | event->modifiers());
    if(currSeq.matches(qxt_d().m_trigger))
        showPopup ();
    else
        QLineEdit::keyPressEvent(event);
}

/*!
  \brief set the model column index QxtLookupLineEdit will use to get data from the model
         after the user has chosen the dataset in the popup
  */
void QxtLookupLineEdit::setDataColumn (const int column)
{
    qxt_d().m_dataColumn = column;
}
/*!
  \brief returns the model column index QxtLookupLineEdit will use to get data from the model
         after the user has chosen the dataset in the popup
  */
int QxtLookupLineEdit::dataColumn   ( ) const
{
    return qxt_d().m_dataColumn;
}
