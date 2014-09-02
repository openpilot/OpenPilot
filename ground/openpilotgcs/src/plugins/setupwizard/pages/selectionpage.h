/**
 ******************************************************************************
 *
 * @file       subvehiclepage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup SubVehiclePage
 * @{
 * @brief
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

#ifndef SUBVEHICLEPAGEPAGE_H
#define SUBVEHICLEPAGEPAGE_H

#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QList>

#include "abstractwizardpage.h"

namespace Ui {
class SelectionPage;
}

class SelectionItem {
public:
    SelectionItem(QString name, QString description, QString shapeId, int id);
    ~SelectionItem();

    QString name() { return m_name; }
    QString description() { return m_description; }
    QString shapeId() { return m_shapeId; }
    int id() { return m_id; }

private:
    QString m_name;
    QString m_description;
    QString m_shapeId;
    int m_id;
};

class Selection {
public:
    Selection() {}
    virtual void addItem(QString name, QString description, QString shapeId, int id) = 0;
    virtual void setTitle(QString title) = 0;
    virtual void setText(QString text) = 0;
};

class SelectionPage : public AbstractWizardPage, public Selection {
    Q_OBJECT

public:
    explicit SelectionPage(SetupWizard *wizard, QString shapeFile, QWidget *parent = 0);
    ~SelectionPage();

    void initializePage();
    bool validatePage();
    void addItem(QString name, QString description, QString shapeId, int id);
    void setTitle(QString title);
    void setText(QString text);

    virtual void setupSelection(Selection *selection) = 0;
    virtual bool validatePage(SelectionItem *selectedItem) = 0;

protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent * event);

private:
    Ui::SelectionPage *ui;
    QGraphicsSvgItem *m_shape;
    QList<SelectionItem*> m_selectionItems;

private slots:
    void selectionChanged(int index);
    void fitImage();
};

#endif // SUBVEHICLEPAGEPAGE_H
