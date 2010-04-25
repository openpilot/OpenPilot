/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "qxtconfigwidget.h"
#include "qxtconfigwidget_p.h"
#include <QStackedWidget>
#include <QApplication>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPainter>

QxtConfigTableWidget::QxtConfigTableWidget(QWidget* parent) : QTableWidget(parent)
{
    int pm = style()->pixelMetric(QStyle::PM_LargeIconSize);
    setIconSize(QSize(pm, pm));
    setItemDelegate(new QxtConfigDelegate(this));
    viewport()->setAttribute(Qt::WA_Hover, true);
}

QStyleOptionViewItem QxtConfigTableWidget::viewOptions() const
{
    QStyleOptionViewItem option = QTableWidget::viewOptions();
    option.displayAlignment = Qt::AlignHCenter | Qt::AlignTop;
    option.decorationAlignment = Qt::AlignHCenter | Qt::AlignTop;
    option.decorationPosition = QStyleOptionViewItem::Top;
    option.showDecorationSelected = false;
    return option;
}

QSize QxtConfigTableWidget::sizeHint() const
{
    return QSize(sizeHintForColumn(0), sizeHintForRow(0));
}

bool QxtConfigTableWidget::hasHoverEffect() const
{
    return static_cast<QxtConfigDelegate*>(itemDelegate())->hover;
}

void QxtConfigTableWidget::setHoverEffect(bool enabled)
{
    static_cast<QxtConfigDelegate*>(itemDelegate())->hover = enabled;
}

QxtConfigDelegate::QxtConfigDelegate(QObject* parent)
        : QItemDelegate(parent), hover(true)
{
}

void QxtConfigDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    if (hover)
    {
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        if (option.state & QStyle::State_Selected)
            painter->fillRect(opt.rect, option.palette.brush(cg, QPalette::Highlight));
        else if ((option.state & QStyle::State_MouseOver) && (option.state & QStyle::State_Enabled))
        {
            QColor color = option.palette.color(cg, QPalette::Highlight).light();
            if (color == option.palette.color(cg, QPalette::Base))
                color = option.palette.color(cg, QPalette::AlternateBase);
            painter->fillRect(opt.rect, color);
        }
        else
            painter->fillRect(opt.rect, option.palette.brush(cg, QPalette::Base));

        opt.showDecorationSelected = false;
        opt.state &= ~QStyle::State_HasFocus;
        opt.state &= ~QStyle::State_Selected;
    }
    QItemDelegate::paint(painter, opt, index);
}

QSize QxtConfigDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int margin = qApp->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    int textWidth = option.fontMetrics.width(index.data().toString());
    int width = qMax(textWidth, option.decorationSize.width()) + 2 * margin;
    int height = option.fontMetrics.height() + option.decorationSize.height() + margin;
    return QSize(width, height);
}

void QxtConfigWidgetPrivate::init(QxtConfigWidget::IconPosition position)
{
    QxtConfigWidget* p = &qxt_p();
    splitter = new QSplitter(p);
    stack = new QStackedWidget(p);
    table = new QxtConfigTableWidget(p);
    pos = position;
    QObject::connect(table, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(setCurrentIndex(int, int)));
    QObject::connect(stack, SIGNAL(currentChanged(int)), p, SIGNAL(currentIndexChanged(int)));
    QVBoxLayout* layout = new QVBoxLayout(p);
    layout->addWidget(splitter);
    initTable();
    relayout();
}

void QxtConfigWidgetPrivate::initTable()
{
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setTabKeyNavigation(true);
    table->setAcceptDrops(false);
    table->setDragEnabled(false);
    table->setShowGrid(false);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
}

void QxtConfigWidgetPrivate::relayout()
{
    if (pos == QxtConfigWidget::North)
    {
        splitter->setOrientation(Qt::Vertical);
        table->setRowCount(1);
        table->setColumnCount(0);
        table->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
        table->verticalHeader()->setResizeMode(QHeaderView::Stretch);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    else
    {
        splitter->setOrientation(Qt::Horizontal);
        table->setRowCount(0);
        table->setColumnCount(1);
        table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

    // clear
    for (int i = splitter->count() - 1; i >= 0; --i)
    {
        splitter->widget(i)->setParent(0);
    }

    // relayout
    switch (pos)
    {
    case QxtConfigWidget::North:
        // +-----------+
        // |   Icons   |
        // +-----------|
        // |   Stack   |
        // +-----------|
        splitter->addWidget(table);
        splitter->addWidget(stack);
        break;

    case QxtConfigWidget::West:
        // +---+-------+
        // | I |       |
        // | c |       |
        // | o | Stack |
        // | n |       |
        // | s |       |
        // +---+-------+
        splitter->addWidget(table);
        splitter->addWidget(stack);
        break;

    case QxtConfigWidget::East:
        // +-------+---+
        // |       | I |
        // |       | c |
        // | Stack | o |
        // |       | n |
        // |       | s |
        // +-------+---+
        splitter->addWidget(stack);
        splitter->addWidget(table);
        break;

    default:
        qWarning("QxtConfigWidgetPrivate::relayout(): unknown position");
        break;
    }

    if (pos == QxtConfigWidget::East)
    {
        splitter->setStretchFactor(0, 10);
        splitter->setStretchFactor(1, 1);
    }
    else
    {
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 10);
    }
}

QTableWidgetItem* QxtConfigWidgetPrivate::item(int index) const
{
    return pos == QxtConfigWidget::North ? table->item(0, index) : table->item(index, 0);
}

void QxtConfigWidgetPrivate::setCurrentIndex(int row, int column)
{
    if (pos == QxtConfigWidget::North)
        setCurrentIndex(column);
    else
        setCurrentIndex(row);
}

void QxtConfigWidgetPrivate::setCurrentIndex(int index)
{
    int previousIndex = stack->currentIndex();
    if (previousIndex != -1 && previousIndex != index)
        qxt_p().cleanupPage(previousIndex);

    stack->setCurrentIndex(index);
    table->setCurrentItem(item(index));

    if (index != -1)
        qxt_p().initializePage(index);
}

/*!
    \class QxtConfigWidget
    \inmodule QxtGui
    \brief The QxtConfigWidget class provides a configuration widget.

    QxtConfigWidget provides a convenient interface for building
    common configuration views. QxtConfigWidget consists of a
    list of icons and a stack of pages.

    \sa QxtConfigDialog
 */

/*!
    \enum QxtConfigWidget::IconPosition

    This enum describes the page icon position.

    \value North The icons are located above the pages.
    \value West The icons are located to the left of the pages.
    \value East The icons are located to the right of the pages.
 */

/*!
    \fn QxtConfigWidget::currentIndexChanged(int index)

    This signal is emitted whenever the current page \a index changes.

    \sa currentIndex()
 */

/*!
    Constructs a new QxtConfigWidget with \a parent and \a flags.
 */
QxtConfigWidget::QxtConfigWidget(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags)
{
    QXT_INIT_PRIVATE(QxtConfigWidget);
    qxt_d().init();
}

/*!
    Constructs a new QxtConfigWidget with icon \a position, \a parent and \a flags.
 */
QxtConfigWidget::QxtConfigWidget(QxtConfigWidget::IconPosition position, QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags)
{
    QXT_INIT_PRIVATE(QxtConfigWidget);
    qxt_d().init(position);
}

/*!
    Destructs the config widget.
 */
QxtConfigWidget::~QxtConfigWidget()
{
}

/*!
    \property QxtConfigWidget::hoverEffect
    \brief whether a hover effect is shown for page icons

    The default value is \c true.

    \bold {Note:} Hovered (but not selected) icons are highlighted with lightened QPalette::Highlight
    (whereas selected icons are highlighted with QPalette::Highlight). In case lightened
    QPalette::Highlight ends up same as QPalette::Base, QPalette::AlternateBase is used
    as a fallback color for the hover effect. This usually happens when QPalette::Highlight
    already is a light color (eg. light gray).
 */
bool QxtConfigWidget::hasHoverEffect() const
{
    return qxt_d().table->hasHoverEffect();
}

void QxtConfigWidget::setHoverEffect(bool enabled)
{
    qxt_d().table->setHoverEffect(enabled);
}

/*!
    \property QxtConfigWidget::iconPosition
    \brief the position of page icons
 */
QxtConfigWidget::IconPosition QxtConfigWidget::iconPosition() const
{
    return qxt_d().pos;
}

void QxtConfigWidget::setIconPosition(QxtConfigWidget::IconPosition position)
{
    if (qxt_d().pos != position)
    {
        qxt_d().pos = position;
        qxt_d().relayout();
    }
}

/*!
    \property QxtConfigWidget::iconSize
    \brief the size of page icons
 */
QSize QxtConfigWidget::iconSize() const
{
    return qxt_d().table->iconSize();
}

void QxtConfigWidget::setIconSize(const QSize& size)
{
    qxt_d().table->setIconSize(size);
}

/*!
    Adds a \a page with \a icon and \a title.

    In case \a title is an empty string, QWidget::windowTitle of \a page is used.

    Returns the index of added page.

    \warning Adding and removing pages dynamically at run time might cause flicker.

    \sa insertPage()
*/
int QxtConfigWidget::addPage(QWidget* page, const QIcon& icon, const QString& title)
{
    return insertPage(-1, page, icon, title);
}

/*!
    Inserts a \a page with \a icon and \a title at \a index.

    In case \a title is an empty string, QWidget::windowTitle of \a page is used.

    Returns the index of inserted page.

    \warning Inserting and removing pages dynamically at run time might cause flicker.

    \sa addPage()
*/
int QxtConfigWidget::insertPage(int index, QWidget* page, const QIcon& icon, const QString& title)
{
    if (!page)
    {
        qWarning("QxtConfigWidget::insertPage(): Attempt to insert null page");
        return -1;
    }

    index = qxt_d().stack->insertWidget(index, page);
    const QString label = !title.isEmpty() ? title : page->windowTitle();
    if (label.isEmpty())
        qWarning("QxtConfigWidget::insertPage(): Inserting a page with an empty title");
    QTableWidgetItem* item = new QTableWidgetItem(icon, label);
    item->setToolTip(label);
    if (qxt_d().pos == QxtConfigWidget::North)
    {
        qxt_d().table->model()->insertColumn(index);
        qxt_d().table->setItem(0, index, item);
        qxt_d().table->resizeRowToContents(0);
    }
    else
    {
        qxt_d().table->model()->insertRow(index);
        qxt_d().table->setItem(index, 0, item);
        qxt_d().table->resizeColumnToContents(0);
    }
    qxt_d().table->updateGeometry();
    return index;
}

/*!
   Removes the page at \a index and returns it.

   \bold {Note:} Does not delete the page widget.
*/
QWidget* QxtConfigWidget::takePage(int index)
{
    if (QWidget* page = qxt_d().stack->widget(index))
    {
        qxt_d().stack->removeWidget(page);
        delete qxt_d().item(index);
        return page;
    }
    else
    {
        qWarning("QxtConfigWidget::removePage(): Unknown index");
        return 0;
    }
}

/*!
    \property QxtConfigWidget::count
    \brief the number of pages
*/
int QxtConfigWidget::count() const
{
    return qxt_d().stack->count();
}

/*!
    \property QxtConfigWidget::currentIndex
    \brief the index of current page
*/
int QxtConfigWidget::currentIndex() const
{
    return qxt_d().stack->currentIndex();
}

void QxtConfigWidget::setCurrentIndex(int index)
{
    qxt_d().setCurrentIndex(index);
}

/*!
    Returns the current page.

    \sa currentIndex(), setCurrentPage()
*/
QWidget* QxtConfigWidget::currentPage() const
{
    return qxt_d().stack->currentWidget();
}

/*!
    Sets the current \a page.

    \sa currentPage(), currentIndex()
*/
void QxtConfigWidget::setCurrentPage(QWidget* page)
{
    qxt_d().setCurrentIndex(qxt_d().stack->indexOf(page));
}

/*!
    Returns the index of \a page or \c -1 if the page is unknown.
*/
int QxtConfigWidget::indexOf(QWidget* page) const
{
    return qxt_d().stack->indexOf(page);
}

/*!
    Returns the page at \a index or \c 0 if the \a index is out of range.
*/
QWidget* QxtConfigWidget::page(int index) const
{
    return qxt_d().stack->widget(index);
}

/*!
    Returns \c true if the page at \a index is enabled; otherwise \c false.

    \sa setPageEnabled()
*/
bool QxtConfigWidget::isPageEnabled(int index) const
{
    const QWidget* widget = page(index);
    return widget && widget->isEnabled();
}

/*!
    Sets the page at \a index \a enabled. The corresponding
    page icon is also \a enabled.

    \sa isPageEnabled()
*/
void QxtConfigWidget::setPageEnabled(int index, bool enabled)
{
    QWidget* page = qxt_d().stack->widget(index);
    QTableWidgetItem* item = qxt_d().item(index);
    if (page && item)
    {
        page->setEnabled(enabled);
        if (enabled)
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
        else
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }
    else
    {
        qWarning("QxtConfigWidget::setPageEnabled(): Unknown index");
    }
}

/*!
    Returns \c true if the page at \a index is hidden; otherwise \c false.

    \sa setPageHidden()
*/
bool QxtConfigWidget::isPageHidden(int index) const
{
    if (qxt_d().pos == QxtConfigWidget::North)
        return qxt_d().table->isColumnHidden(index);
    return qxt_d().table->isRowHidden(index);
}

/*!
    Sets the page at \a index \a hidden. The corresponding
    page icon is also \a hidden.

    \sa isPageHidden()
*/
void QxtConfigWidget::setPageHidden(int index, bool hidden)
{
    if (qxt_d().pos == QxtConfigWidget::North)
        qxt_d().table->setColumnHidden(index, hidden);
    else
        qxt_d().table->setRowHidden(index, hidden);
}

/*!
    Returns the icon of page at \a index.

    \sa setPageIcon()
*/
QIcon QxtConfigWidget::pageIcon(int index) const
{
    const QTableWidgetItem* item = qxt_d().item(index);
    return (item ? item->icon() : QIcon());
}

/*!
    Sets the \a icon of page at \a index.

    \sa pageIcon()
*/
void QxtConfigWidget::setPageIcon(int index, const QIcon& icon)
{
    QTableWidgetItem* item = qxt_d().item(index);
    if (item)
    {
        item->setIcon(icon);
    }
    else
    {
        qWarning("QxtConfigWidget::setPageIcon(): Unknown index");
    }
}

/*!
    Returns the title of page at \a index.

    \sa setPageTitle()
*/
QString QxtConfigWidget::pageTitle(int index) const
{
    const QTableWidgetItem* item = qxt_d().item(index);
    return (item ? item->text() : QString());
}

/*!
    Sets the \a title of page at \a index.

    \sa pageTitle()
*/
void QxtConfigWidget::setPageTitle(int index, const QString& title)
{
    QTableWidgetItem* item = qxt_d().item(index);
    if (item)
    {
        item->setText(title);
    }
    else
    {
        qWarning("QxtConfigWidget::setPageTitle(): Unknown index");
    }
}

/*!
    Returns the tooltip of page at \a index.

    \sa setPageToolTip()
*/
QString QxtConfigWidget::pageToolTip(int index) const
{
    const QTableWidgetItem* item = qxt_d().item(index);
    return (item ? item->toolTip() : QString());
}

/*!
    Sets the \a tooltip of page at \a index.

    \sa pageToolTip()
*/
void QxtConfigWidget::setPageToolTip(int index, const QString& tooltip)
{
    QTableWidgetItem* item = qxt_d().item(index);
    if (item)
    {
        item->setToolTip(tooltip);
    }
    else
    {
        qWarning("QxtConfigWidget::setPageToolTip(): Unknown index");
    }
}

/*!
    Returns the what's this of page at \a index.

    \sa setPageWhatsThis()
*/
QString QxtConfigWidget::pageWhatsThis(int index) const
{
    const QTableWidgetItem* item = qxt_d().item(index);
    return (item ? item->whatsThis() : QString());
}

/*!
    Sets the \a whatsthis of page at \a index.

    \sa pageWhatsThis()
*/
void QxtConfigWidget::setPageWhatsThis(int index, const QString& whatsthis)
{
    QTableWidgetItem* item = qxt_d().item(index);
    if (item)
    {
        item->setWhatsThis(whatsthis);
    }
    else
    {
        qWarning("QxtConfigWidget::setPageWhatsThis(): Unknown index");
    }
}

/*!
    \bold {Note:} The default implementation calls SLOT(accept()) of
    each page page provided that such slot exists.

    \sa reject()
 */
void QxtConfigWidget::accept()
{
    Q_ASSERT(qxt_d().stack);
    for (int i = 0; i < qxt_d().stack->count(); ++i)
    {
        QMetaObject::invokeMethod(qxt_d().stack->widget(i), "accept");
    }
}

/*!

    \bold {Note:} The default implementation calls SLOT(reject()) of
    each page provided that such slot exists.

    \sa accept()
 */
void QxtConfigWidget::reject()
{
    Q_ASSERT(qxt_d().stack);
    for (int i = 0; i < qxt_d().stack->count(); ++i)
    {
        QMetaObject::invokeMethod(qxt_d().stack->widget(i), "reject");
    }
}

/*!
    Returns the internal table widget used for showing page icons.

    \sa stackedWidget()
*/
QTableWidget* QxtConfigWidget::tableWidget() const
{
    return qxt_d().table;
}

/*!
    Returns the internal stacked widget used for stacking pages.

    \sa tableWidget()
*/
QStackedWidget* QxtConfigWidget::stackedWidget() const
{
    return qxt_d().stack;
}

/*!
    This virtual function is called to clean up previous
    page at \a index before switching to a new page.

    \bold {Note:} The default implementation calls SLOT(cleanup()) of
    the corresponding page provided that such slot exists.

    \sa initializePage()
*/
void QxtConfigWidget::cleanupPage(int index)
{
    Q_ASSERT(qxt_d().stack);
    QMetaObject::invokeMethod(qxt_d().stack->widget(index), "cleanup");
}

/*!
    This virtual function is called to initialize page at
    \a index before switching to it.

    \bold {Note:} The default implementation calls SLOT(initialize()) of
    the corresponding page provided that such slot exists.

    \sa cleanupPage()
*/
void QxtConfigWidget::initializePage(int index)
{
    Q_ASSERT(qxt_d().stack);
    QMetaObject::invokeMethod(qxt_d().stack->widget(index), "initialize");
}
