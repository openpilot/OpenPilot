#include "qxtfilterdialog.h"
#include "qxtfilterdialog_p.h"

#include <QVBoxLayout>
#include <QTreeView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QAbstractItemModel>
#include <QLineEdit>
#include <QHeaderView>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QApplication>
#include <QKeyEvent>
#include "qxtgroupbox.h"


QxtFilterDialogPrivate::QxtFilterDialogPrivate()    :   QObject(0),
                                                        matchCaseOption(0),
                                                        filterModeOption(0),
                                                        filterMode(0),
                                                        listingTreeView(0),
                                                        lineEditFilter(0),
                                                        proxyModel(0),
                                                        lookupColumn(0),
                                                        lookupRole(Qt::DisplayRole),
                                                        syntax(QRegExp::FixedString),
                                                        caseSensitivity(Qt::CaseInsensitive)
{
}

void QxtFilterDialogPrivate::createRegExpPattern(const QString &rawText)
{

    QRegExp regExp(rawText,caseSensitivity,syntax);
    if(regExp.isValid())
        proxyModel->setFilterRegExp(regExp);
    else
        proxyModel->setFilterRegExp(QString());
}

void QxtFilterDialogPrivate::updateFilterPattern()
{
    createRegExpPattern(lineEditFilter->text());
}

void QxtFilterDialogPrivate::filterModeOptionStateChanged(const int state)
{
    if(state == Qt::Checked)
        this->filterMode->setEnabled(true);
    else
        this->filterMode->setEnabled(false);

}

void QxtFilterDialogPrivate::filterModeChoosen(int index)
{
    if(index < 0)
        return;

    if( filterMode->itemData(index).isValid())
    {
        QRegExp::PatternSyntax mode = static_cast<QRegExp::PatternSyntax>(filterMode->itemData(index).toInt());
        qxt_p().setPatternSyntax(mode);
    }

}

void QxtFilterDialogPrivate::matchCaseOptionStateChanged(const int state)
{
    if(state == Qt::Checked)
        this->caseSensitivity = Qt::CaseSensitive;
    else
        this->caseSensitivity = Qt::CaseInsensitive;

    updateFilterPattern();
}

/*!
\class QxtFilterDialog QxtFilterDialog
\brief The QxtFilterDialog class provides a dialog to select data from a QAbstractItemModel

Provides a dialog to select data from a QAbstractItemModel, the user can filter the items depending on a
role and column to make the selection easier.

*/

QxtFilterDialog::QxtFilterDialog(QWidget *parent) : QDialog(parent)
{
    QXT_INIT_PRIVATE(QxtFilterDialog);

    qxt_d().proxyModel = new QSortFilterProxyModel(this);

    QVBoxLayout * dlgLayout  = new QVBoxLayout(this);

    /*the line edit to type in the filter*/
    qxt_d().lineEditFilter = new QLineEdit();
    dlgLayout->addWidget(qxt_d().lineEditFilter);
    connect(qxt_d().lineEditFilter,SIGNAL(textChanged(const QString &)),&qxt_d(),SLOT(createRegExpPattern(const QString &)));


    /*the treeview that shows the filtered choices */
    qxt_d().listingTreeView = new QTreeView(this);
    qxt_d().listingTreeView->setSortingEnabled(true);
    //not editable , even if the model supports it
    qxt_d().listingTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dlgLayout->addWidget(qxt_d().listingTreeView);
    connect(qxt_d().listingTreeView,SIGNAL(activated ( const QModelIndex & )),this,SLOT(accept()));

#if 1
    /*Group Box for changing search behaviour*/
    QxtGroupBox *group = new QxtGroupBox(this);
   // group->setCollapsive(true);
    group->setTitle(tr("Filter options"));

    qxt_d().matchCaseOption = new QCheckBox(tr("Match case"));
    connect(qxt_d().matchCaseOption,SIGNAL(stateChanged ( int )),&qxt_d(),SLOT(matchCaseOptionStateChanged(int)));
    
    qxt_d().filterModeOption = new QCheckBox(tr("Filter mode:"));
    connect(qxt_d().filterModeOption,SIGNAL(stateChanged ( int )),&qxt_d(),SLOT(filterModeOptionStateChanged(int)));
    
    qxt_d().filterMode = new QComboBox();
    qxt_d().filterMode->addItem(tr("Fixed String (Default)"),QRegExp::FixedString);
    qxt_d().filterMode->addItem(tr("Wildcard"),QRegExp::Wildcard);
    qxt_d().filterMode->addItem(tr("Regular Expression"),QRegExp::RegExp);
    qxt_d().filterMode->setEnabled(false);
    connect(qxt_d().filterMode,SIGNAL(activated (int)),&qxt_d(),SLOT(filterModeChoosen(int)));
    
    QVBoxLayout * groupBoxLayout  = new QVBoxLayout(group);
    groupBoxLayout->addWidget(qxt_d().matchCaseOption);
    groupBoxLayout->addWidget(qxt_d().filterModeOption);
    groupBoxLayout->addWidget(qxt_d().filterMode);


    group->setLayout(groupBoxLayout);
    dlgLayout->addWidget(group);
    //group->setCollapsed();
    group->setChecked(false);
    /*end group box*/
#endif
    /*a button to cancel the selection*/
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    dlgLayout->addWidget(cancelButton);
    connect(cancelButton,SIGNAL(released()),this,SLOT(reject()));

    QWidget::setTabOrder(qxt_d().lineEditFilter,qxt_d().listingTreeView);
    //QWidget::setTabOrder(qxt_d().listingTreeView,group);
    //QWidget::setTabOrder(group,qxt_d().matchCaseOption);
    QWidget::setTabOrder(qxt_d().listingTreeView,qxt_d().matchCaseOption);
    QWidget::setTabOrder(qxt_d().matchCaseOption,qxt_d().filterModeOption);
    QWidget::setTabOrder(qxt_d().filterModeOption,qxt_d().filterMode);
    QWidget::setTabOrder(qxt_d().filterMode,cancelButton);


    /*set the layout and title*/
    setLayout(dlgLayout);
    setWindowTitle(tr("Filter"));
}

QRegExp::PatternSyntax QxtFilterDialog::patternSyntax ()
{
    return qxt_d().syntax;
}

void QxtFilterDialog::setPatternSyntax (QRegExp::PatternSyntax syntax)
{
    qxt_d().syntax = syntax;
    qxt_d().updateFilterPattern();
}

void QxtFilterDialog::setSourceModel(QAbstractItemModel *model)
{
    qxt_d().listingTreeView->setModel(0);
    qxt_d().proxyModel->setSourceModel(model);
    qxt_d().model = model;
    qxt_d().listingTreeView->setModel(qxt_d().proxyModel);
    if(model)
        qxt_d().listingTreeView->setCurrentIndex(model->index(0,0));
}

void QxtFilterDialog::setLookupColumn (int column)
{
    qxt_d().proxyModel->setHeaderData(qxt_d().lookupColumn,Qt::Horizontal,QVariant(),Qt::ForegroundRole);
    qxt_d().lookupColumn = column;
    qxt_d().proxyModel->setFilterKeyColumn(qxt_d().lookupColumn);
    qxt_d().proxyModel->setHeaderData(qxt_d().lookupColumn,Qt::Horizontal,QColor(Qt::red),Qt::ForegroundRole);

}

void QxtFilterDialog::setLookupRole (int role)
{
    qxt_d().lookupRole = role;
    qxt_d().proxyModel->setFilterRole(qxt_d().lookupRole);
}

QAbstractItemModel * QxtFilterDialog::sourceModel( ) const
{
    return qxt_d().model;
}

int QxtFilterDialog::lookupColumn ( ) const
{
    return qxt_d().lookupColumn;
}

int QxtFilterDialog::lookupRole ( ) const
{
    return qxt_d().lookupRole;
}

QString QxtFilterDialog::filterText ( ) const
{
    return qxt_d().lineEditFilter->text();
}

void QxtFilterDialog::setFilterText (QString text)
{
    qxt_d().lineEditFilter->setText(text);
}

QModelIndex QxtFilterDialog::selectedIndex() const
{
    return qxt_d().selectedIndex;
}

void QxtFilterDialog::setCaseSensitivity (Qt::CaseSensitivity caseSensitivity)
{
    qxt_d().caseSensitivity = caseSensitivity;
    qxt_d().updateFilterPattern();
}

QModelIndex QxtFilterDialog::getIndex(QWidget* parent, QAbstractItemModel *model, int column, int role, QString filterText)
{
    QxtFilterDialog dialog(parent);

    dialog.setSourceModel(model);
    dialog.setLookupColumn(column);
    dialog.setLookupRole(role);
    dialog.setFilterText(filterText);

    if(dialog.qxt_d().proxyModel->rowCount() == 1){
        QModelIndex source;
        QModelIndex filtered = dialog.qxt_d().proxyModel->index(0,column);
        if(filtered.isValid()){
            source = dialog.qxt_d().proxyModel->mapToSource(filtered);
        }
        return source;
    }

    int ret = dialog.exec();

    if(ret == QDialog::Accepted)
        return dialog.selectedIndex();
    return QModelIndex();
}

void QxtFilterDialog::done ( int r )
{
    if(r==QDialog::Accepted){
        qxt_d().selectedIndex = QModelIndex();
        QModelIndex proxyIndex = qxt_d().listingTreeView->currentIndex();
        proxyIndex = qxt_d().proxyModel->index(proxyIndex.row(),lookupColumn());
        if(proxyIndex.isValid())
        {
            qxt_d().selectedIndex = qxt_d().proxyModel->mapToSource(proxyIndex);
        }
    }
    else
        qxt_d().selectedIndex = QModelIndex();
    return QDialog::done(r);
}

void QxtFilterDialog::accept ()
{

    return QDialog::accept();
}

void QxtFilterDialog::reject ()
{
    return QDialog::reject();
}


void QxtFilterDialog::keyPressEvent(QKeyEvent *e)
{
    //reimplemented to disable the default button !
    if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) {
        switch (e->key()) 
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                accept();
                return;
        }
    }
    QDialog::keyPressEvent(e);
}

