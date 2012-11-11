/**
 ******************************************************************************
 *
 * @file       treeitem.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectBrowserPlugin UAVObject Browser Plugin
 * @{
 * @brief The UAVObject Browser gadget plugin
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

#include "treeitem.h"

TreeItem::TreeItem(const QList<QVariant> &data, TreeItem *parent) :
        QObject(0),
        m_data(data),
        m_parent(parent),
        m_highlight(false),
        m_changed(false)
{
}

TreeItem::TreeItem(const QVariant &data, TreeItem *parent) :
        QObject(0),
        m_parent(parent),
        m_highlight(false),
        m_changed(false)
{
    m_data << data << "" << "";
}

TreeItem::~TreeItem()
{
    qDeleteAll(m_children);
}

void TreeItem::appendChild(TreeItem *child)
{
    m_children.append(child);
    child->setParentTree(this);
}

void TreeItem::insert(int index, TreeItem *child)
{
    m_children.insert(index, child);
    child->setParentTree(this);
}

TreeItem *TreeItem::child(int row)
{
    return m_children.value(row);
}

int TreeItem::childCount() const
{
    return m_children.count();
}
int TreeItem::row() const
{
    if (m_parent)
        return m_parent->m_children.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

int TreeItem::columnCount() const
{
    return m_data.count();
}

QVariant TreeItem::data(int column) const
{
    return m_data.value(column);
}

void TreeItem::setData(QVariant value, int column)
{
    m_data.replace(column, value);
}

void TreeItem::update() {
    foreach(TreeItem *child, treeChildren())
        child->update();
}

void TreeItem::apply() {
    foreach(TreeItem *child, treeChildren())
        child->apply();
}

void TreeItem::setHighlight(bool highlight) {
    //m_highlight = highlight;
    m_changed = false;
}

void TreeItem::setActive(bool highlight) {
    m_highlight = highlight;
    foreach(TreeItem *child, treeChildren())
        child->setActive(highlight);
}


void TreeItem::removeHighlight() {
    m_highlight = false;
    update();
}
