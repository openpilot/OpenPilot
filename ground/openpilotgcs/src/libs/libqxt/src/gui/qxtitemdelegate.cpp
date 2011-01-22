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
#include "qxtitemdelegate.h"
#include "qxtitemdelegate_p.h"
#include <QTextDocument>
#include <QPixmapCache>
#include <QApplication>
#include <QTreeView>
#include <QPainter>
#include <QTimer>

static const int TOP_LEVEL_EXTENT = 2;

QxtItemDelegatePrivate::QxtItemDelegatePrivate() :
        textVisible(true),
        progressFormat("%1%"),
        elide(Qt::ElideMiddle),
        style(Qxt::NoDecoration),
        document(0)
{
}

void QxtItemDelegatePrivate::paintButton(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, const QTreeView* view) const
{
    // draw the button
    QStyleOptionButton buttonOption;
    buttonOption.state = option.state;
#ifdef Q_WS_MAC
    buttonOption.state |= QStyle::State_Raised;
#endif
    buttonOption.state &= ~QStyle::State_HasFocus;
    if (view->isExpanded(index))
        buttonOption.state |= QStyle::State_Sunken;
    buttonOption.rect = option.rect;
    buttonOption.palette = option.palette;
    buttonOption.features = QStyleOptionButton::None;
    view->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, view);

    // draw the branch indicator
    static const int i = 9;
    const QRect& r = option.rect;
    if (index.model()->hasChildren(index))
    {
        QStyleOption branchOption;
        branchOption.initFrom(view);
        if (branchOption.direction == Qt::LeftToRight)
            branchOption.rect = QRect(r.left() + i / 2, r.top() + (r.height() - i) / 2, i, i);
        else
            branchOption.rect = QRect(r.right() - i / 2 - i, r.top() + (r.height() - i) / 2, i, i);
        branchOption.palette = option.palette;
        branchOption.state = QStyle::State_Children;
        if (view->isExpanded(index))
            branchOption.state |= QStyle::State_Open;
        view->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, view);
    }

    // draw the text
    QRect textrect = QRect(r.left() + i * 2, r.top(), r.width() - ((5 * i) / 2), r.height());
    QString text = option.fontMetrics.elidedText(index.data().toString(), elide, textrect.width());
    view->style()->drawItemText(painter, textrect, Qt::AlignCenter, option.palette, view->isEnabled(), text);
}

void QxtItemDelegatePrivate::paintMenu(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, const QTreeView* view) const
{
    // draw the menu bar item
    QStyleOptionMenuItem menuOption;
    menuOption.palette = view->palette();
    menuOption.fontMetrics = view->fontMetrics();
    menuOption.state = QStyle::State_None;
    // QModelIndex::flags() was introduced in 4.2
    // => therefore "index.model()->flags(index)"
    if (view->isEnabled() && index.model()->flags(index) & Qt::ItemIsEnabled)
        menuOption.state |= QStyle::State_Enabled;
    else
        menuOption.palette.setCurrentColorGroup(QPalette::Disabled);
    menuOption.state |= QStyle::State_Selected;
    menuOption.state |= QStyle::State_Sunken;
    menuOption.state |= QStyle::State_HasFocus;
    menuOption.rect = option.rect;
    menuOption.text = index.data().toString();
    menuOption.icon = QIcon(index.data(Qt::DecorationRole).value<QPixmap>());
    view->style()->drawControl(QStyle::CE_MenuBarItem, &menuOption, painter, view);

    // draw the an arrow as a branch indicator
    if (index.model()->hasChildren(index))
    {
        QStyle::PrimitiveElement arrow;
        if (view->isExpanded(index))
            arrow = QStyle::PE_IndicatorArrowUp;
        else
            arrow = QStyle::PE_IndicatorArrowDown;
        static const int i = 9;
        const QRect& r = option.rect;
        menuOption.rect = QRect(r.left() + i / 2, r.top() + (r.height() - i) / 2, i, i);
        view->style()->drawPrimitive(arrow, &menuOption, painter, view);
    }
}

void QxtItemDelegatePrivate::paintProgress(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QVariant min = index.data(QxtItemDelegate::ProgressMinimumRole);
    QVariant max = index.data(QxtItemDelegate::ProgressMaximumRole);

    QStyleOptionProgressBar opt;
    opt.minimum = (min.isValid() && min.canConvert(QVariant::Int)) ? min.toInt() : 0;
    opt.maximum = (max.isValid() && max.canConvert(QVariant::Int)) ? max.toInt() : 100;
    opt.progress = index.data(QxtItemDelegate::ProgressValueRole).toInt();
    opt.rect = option.rect;
    opt.textVisible = textVisible;
    opt.text = progressFormat.arg(opt.progress);
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &opt, painter, 0);

    QWidget* viewport = dynamic_cast<QWidget*>(painter->device());
    if (viewport)
    {
        if (opt.minimum == 0 && opt.maximum == 0)
        {
            if (!updatedItems.contains(viewport))
                connect(viewport, SIGNAL(destroyed()), this, SLOT(viewDestroyed()));
            updatedItems.replace(viewport, index);
        }
        else
        {
            updatedItems.remove(viewport, index);
            if (!updatedItems.contains(viewport))
                disconnect(viewport, SIGNAL(destroyed()), this, SLOT(viewDestroyed()));
        }
    }

    if (updatedItems.isEmpty())
    {
        if (updateTimer.isActive())
            updateTimer.stop();
    }
    else
    {
        if (!updateTimer.isActive())
            updateTimer.start(1000 / 25, const_cast<QxtItemDelegatePrivate*>(this));
    }
}

void QxtItemDelegatePrivate::setCurrentEditor(QWidget* editor, const QModelIndex& index) const
{
    currentEditor = editor;
    currentEdited = index;
}

void QxtItemDelegatePrivate::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == updateTimer.timerId())
    {
        QMutableHashIterator<QWidget*, QPersistentModelIndex> it(updatedItems);
        while (it.hasNext())
        {
            it.next();
            if (!it.key())
            {
                it.remove();
                continue;
            }

            // try to update the specific view item instead of the whole view if possible
            if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(it.key()->parentWidget()))
                view->update(it.value());
            else
                it.key()->update();
        }
    }
}

void QxtItemDelegatePrivate::viewDestroyed()
{
    QWidget* viewport = qobject_cast<QWidget*>(sender());
    if (viewport)
    {
        updatedItems.remove(viewport);
    }
}

void QxtItemDelegatePrivate::closeEditor(QWidget* editor)
{
    if (currentEdited.isValid() && editor == currentEditor)
    {
        setCurrentEditor(0, QModelIndex());
        emit qxt_p().editingFinished(currentEdited);
    }
}

/*!
    \class QxtItemDelegate
    \inmodule QxtGui
    \brief The QxtItemDelegate class is an extended QItemDelegate

    QxtItemDelegate provides signals for starting and finishing of
    editing and an optional decoration of top level items in a QTreeView.

    QxtItemDelegate can also draw a progress bar for items providing
    appropriate progress data. Just like QProgressBar, QxtItemDelegate
    can show a busy indicator. If minimum and maximum both are set to
    \c 0, a busy indicator is shown instead of a percentage of steps.
 */

/*!
    \fn QxtItemDelegate::editingStarted(const QModelIndex& index)

    This signal is emitted after the editing of \a index has been started.

    \sa editingFinished()
 */

/*!
    \fn QxtItemDelegate::editingFinished(const QModelIndex& index)

    This signal is emitted after the editing of \a index has been finished.

    \sa editingStarted()
 */

/*!
    \enum QxtItemDelegate::Role

    This enum defines custom roles used by QxtItemDelegate.

    \value ProgressValueRole The progress value.
    \value ProgressMinimumRole The minimum progress value. The default value \c 0 is used if no value is provided for ProgressMinimumRole.
    \value ProgressMaximumRole The maximum progress value. The default value \c 100 is used if no value is provided for ProgressMaximumRole.
 */

/*!
    Constructs a new QxtItemDelegate with \a parent.
 */
QxtItemDelegate::QxtItemDelegate(QObject* parent) : QItemDelegate(parent)
{
    QXT_INIT_PRIVATE(QxtItemDelegate);
    connect(this, SIGNAL(closeEditor(QWidget*)), &qxt_d(), SLOT(closeEditor(QWidget*)));
}

/*!
    Destructs the item delegate.
 */
QxtItemDelegate::~QxtItemDelegate()
{}

/*!
    \property QxtItemDelegate::decorationStyle
    \brief the top level index decoration style

    Top level indices are decorated according to this property.
    The default value is Qxt::NoDecoration.

    \bold {Note:} The property has effect only in case the delegate is installed
    on a QTreeView. The view must be the parent of the delegate.

    \bold {Note:} Set QTreeView::rootIsDecorated to \c false to avoid
    multiple branch indicators.

    \sa Qxt::DecorationStyle, QTreeView::rootIsDecorated
 */
Qxt::DecorationStyle QxtItemDelegate::decorationStyle() const
{
    return qxt_d().style;
}

void QxtItemDelegate::setDecorationStyle(Qxt::DecorationStyle style)
{
    qxt_d().style = style;
}

/*!
    \property QxtItemDelegate::elideMode
    \brief the text elide mode

    The text of a decorated top level index is elided according to this property.
    The default value is Qt::ElideMiddle.

    \bold {Note:} The property has effect only for decorated top level indices.

    \sa decorationStyle, Qt::TextElideMode
 */
Qt::TextElideMode QxtItemDelegate::elideMode() const
{
    return qxt_d().elide;
}

void QxtItemDelegate::setElideMode(Qt::TextElideMode mode)
{
    qxt_d().elide = mode;
}

/*!
    \property QxtItemDelegate::progressTextFormat
    \brief the format of optional progress text

    The progress text is formatted according to this property.
    The default value is \bold "%1%".

    \bold {Note:} Progress bar is rendered for indices providing valid
    numerical data for ProgressValueRole.

 \bold {Note:} \bold \%1 is replaced by the progress percent.

    \sa progressTextVisible, Role
 */
QString QxtItemDelegate::progressTextFormat() const
{
    return qxt_d().progressFormat;
}

void QxtItemDelegate::setProgressTextFormat(const QString& format)
{
    qxt_d().progressFormat = format;
}

/*!
    \property QxtItemDelegate::progressTextVisible
    \brief whether progress text is visible

    The default value is \c true.

    \bold {Note:} Progress bar is rendered for indices providing valid
    numerical data for ProgressValueRole.

    \sa progressTextFormat, QxtItemDelegate::Role
 */
bool QxtItemDelegate::isProgressTextVisible() const
{
    return qxt_d().textVisible;
}

void QxtItemDelegate::setProgressTextVisible(bool visible)
{
    qxt_d().textVisible = visible;
}

/*!
    \reimp
 */
QWidget* QxtItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QWidget* editor = QItemDelegate::createEditor(parent, option, index);
    qxt_d().setCurrentEditor(editor, index);
    emit const_cast<QxtItemDelegate*>(this)->editingStarted(index);
    return editor;
}

/*!
    \reimp
 */
void QxtItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QItemDelegate::setModelData(editor, model, index);
    qxt_d().setCurrentEditor(0, QModelIndex());
    emit const_cast<QxtItemDelegate*>(this)->editingFinished(index);
}

/*!
    \reimp
 */
void QxtItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QAbstractItemModel* model = index.model();
    const QTreeView* tree = qobject_cast<QTreeView*>(parent());
    const bool topLevel = !index.parent().isValid();

    if (tree && model && topLevel && qxt_d().style != Qxt::NoDecoration)
    {
        QStyleOptionViewItem opt;
        opt.QStyleOption::operator=(option);
        opt.showDecorationSelected = false;

        QModelIndex valid = model->index(index.row(), 0);
        QModelIndex sibling = valid;
        while (sibling.isValid())
        {
            opt.rect |= tree->visualRect(sibling);
            sibling = sibling.sibling(sibling.row(), sibling.column() + 1);
        }

        switch (qxt_d().style)
        {
        case Qxt::Buttonlike:
            qxt_d().paintButton(painter, opt, valid, tree);
            break;
        case Qxt::Menulike:
            qxt_d().paintMenu(painter, opt, valid, tree);
            break;
        default:
            qWarning("QxtItemDelegate::paint() unknown decoration style");
            QItemDelegate::paint(painter, opt, valid);
            break;
        }
    }
    else
    {
        QItemDelegate::paint(painter, option, index);

        const QVariant data = index.data(ProgressValueRole);
        if (data.isValid() && data.canConvert(QVariant::Int))
            qxt_d().paintProgress(painter, option, index);
    }
}

/*!
    \reimp
 */
void QxtItemDelegate::drawDisplay(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text) const
{
    if (!Qt::mightBeRichText(text))
    {
        QItemDelegate::drawDisplay(painter, option, rect, text);
        return;
    }

    QString key = QString(QLatin1String("QxtItemDelegate:%1")).arg(text);
    QPixmap pixmap;
    if (!QPixmapCache::find(key, pixmap))
    {
        if (!qxt_d().document)
            qxt_d().document = new QTextDocument(const_cast<QxtItemDelegate*>(this));
        qxt_d().document->setHtml(text);
        qxt_d().document->adjustSize();

        pixmap = QPixmap(qxt_d().document->size().toSize());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        qxt_d().document->drawContents(&painter);
        painter.end();
        QPixmapCache::insert(key, pixmap);
    }
    painter->drawPixmap(option.rect.topLeft(), pixmap);
}

/*!
    \reimp
 */
QSize QxtItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // something slightly bigger for top level indices if decorated
    QSize size = QItemDelegate::sizeHint(option, index);
    if (!index.parent().isValid() && qxt_d().style != Qxt::NoDecoration)
        size += QSize(TOP_LEVEL_EXTENT, TOP_LEVEL_EXTENT);

    const QString text = index.data(Qt::DisplayRole).toString();
    if (Qt::mightBeRichText(text))
    {
        if (!qxt_d().document)
            qxt_d().document = new QTextDocument(const_cast<QxtItemDelegate*>(this));
        qxt_d().document->setHtml(text);
        qxt_d().document->adjustSize();
        size = size.expandedTo(qxt_d().document->size().toSize()); // TODO: checkbox, icon, etc.
    }
    return  size;
}
