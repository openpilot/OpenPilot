/**
 ******************************************************************************
 *
 * @file       fancytabwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef FANCYTABWIDGET_H
#define FANCYTABWIDGET_H

#include <QtGui/QPushButton>
#include <QtGui/QTabBar>
#include <QtGui/QStyleOptionTabV2>
#include <QtCore/QTimeLine>

QT_BEGIN_NAMESPACE
class QPainter;
class QStackedLayout;
class QStatusBar;
QT_END_NAMESPACE

    struct FancyTab {
        QIcon icon;
        QString text;
        QString toolTip;
    };

class FancyTabBar : public QWidget
{
    Q_OBJECT

public:
    FancyTabBar(QWidget *parent = 0, bool isVertical=false);
    ~FancyTabBar();

    bool event(QEvent *event);

    void paintEvent(QPaintEvent *event);
    void paintTab(QPainter *painter, int tabIndex) const;
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void insertTab(int index, const QIcon &icon, const QString &label) {
        FancyTab tab;
        tab.icon = icon;
        tab.text = label;
        m_tabs.insert(index, tab);
    }
    void removeTab(int index) {
        m_tabs.removeAt(index);
    }
    void updateTabNameIcon(int index, const QIcon &icon, const QString &label);
    void setCurrentIndex(int index);
    int currentIndex() const { return m_currentIndex; }

    void setTabToolTip(int index, QString toolTip) { m_tabs[index].toolTip = toolTip; }
    QString tabToolTip(int index) const { return m_tabs.at(index).toolTip; }

    void setIconSize(int s) { iconSize = s; }
    QIcon tabIcon(int index) const {return m_tabs.at(index).icon; }
    QString tabText(int index) const { return m_tabs.at(index).text; }
    int count() const {return m_tabs.count(); }
    QRect tabRect(int index) const;


signals:
    void currentChanged(int);
    void aboutToChange(bool *);

public slots:
    void updateHover();

private:
    static const int m_rounding;
    static const int m_textPadding;
    QTimeLine m_hoverControl;
    QRect m_hoverRect;
    int m_hoverIndex;
    int m_currentIndex;
    int iconSize;
    QList<FancyTab> m_tabs;
    bool verticalTabs;

    QSize tabSizeHint(bool minimum = false) const;

};

class FancyTabWidget : public QWidget
{
    Q_OBJECT

public:
    FancyTabWidget(QWidget *parent = 0, bool isVertical = false);
    FancyTabBar *m_tabBar;
    void insertTab(int index, QWidget *tab, const QIcon &icon, const QString &label);
    void removeTab(int index);
    void setBackgroundBrush(const QBrush &brush);
    void addCornerWidget(QWidget *widget);
    void insertCornerWidget(int pos, QWidget *widget);
    int cornerWidgetCount() const;
    void setTabToolTip(int index, const QString &toolTip);
    void updateTabNameIcon(int index, const QIcon &icon, const QString &label);
    void setIconSize(int s) { m_tabBar->setIconSize(s); }

    void paintEvent(QPaintEvent *event);

    int currentIndex() const;
    QStatusBar *statusBar() const;

    QWidget * currentWidget();
signals:
    void currentAboutToShow(int index);
    void currentChanged(int index);

public slots:
    void setCurrentIndex(int index);

private slots:
    void showWidget(int index);

private:
    QWidget *m_cornerWidgetContainer;
    QStackedLayout *m_modesStack;
    QWidget *m_selectionWidget;
    QStatusBar *m_statusBar;
};


#endif // FANCYTABWIDGET_H
