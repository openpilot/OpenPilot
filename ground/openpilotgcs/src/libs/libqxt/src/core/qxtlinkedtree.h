/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
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
#ifndef QXTLINKEDTREE_H
#define QXTLINKEDTREE_H

#include <qxtglobal.h>
#include <qxtsharedprivate.h>

template<class T>
class QxtLinkedTree;

template<class T>
class QxtLinkedTreeIterator;

template<class T>
class QXT_CORE_EXPORT QxtLinkedTreeItem
{
public:
    ~QxtLinkedTreeItem()
    {
        clear();
    }

private:
    QxtLinkedTreeItem(T tt)
    {
        t = tt;
        next = 0;
        previous = 0;
        parent = 0;
        child = 0;
        childcount = 0;
    }

    void clear()
    {
        if (child)
        {
            QxtLinkedTreeItem * c = child;
            while (c)
            {
                QxtLinkedTreeItem * e = c;
                c = c->next;
                delete e;
            }
            child = 0;
        }
    }

    friend class QxtLinkedTree<T>;
    friend class QxtLinkedTreeIterator<T>;
    QxtLinkedTreeItem * next;
    QxtLinkedTreeItem * previous;
    QxtLinkedTreeItem * parent;
    QxtLinkedTreeItem * child;
    int childcount;

    T t;
    ///TODO: somehow notify all iterators when one deletes this. so they can be made invalid instead of undefined.
};

///FIXME: nested would be cooler but c++ has no typdefs with templates and doxygen doesn't undertsand nested templates
template<class T>
class QXT_CORE_EXPORT QxtLinkedTreeIterator
{
public:
    QxtLinkedTreeIterator();
    QxtLinkedTreeIterator(const QxtLinkedTreeIterator<T> & other);
    QxtLinkedTreeIterator & operator= (const QxtLinkedTreeIterator<T> & other);

    QxtLinkedTreeIterator    parent() const;
    QxtLinkedTreeIterator    next() const;
    QxtLinkedTreeIterator    previous() const;
    QxtLinkedTreeIterator    child() const;

    bool isValid() const;
    int children() const;

    T & operator*() const;
    T * operator-> () const;
    operator T() const;

    QxtLinkedTreeIterator    operator + (int j) const;
    QxtLinkedTreeIterator &  operator ++ ();
    QxtLinkedTreeIterator    operator ++ (int);
    QxtLinkedTreeIterator &  operator += (int j);

    QxtLinkedTreeIterator    operator - (int j) const;
    QxtLinkedTreeIterator &  operator -- ();
    QxtLinkedTreeIterator    operator -- (int);
    QxtLinkedTreeIterator &  operator -= (int j);

    bool operator== (const QxtLinkedTreeIterator<T> & other) const;
    bool operator!= (const QxtLinkedTreeIterator<T> & other) const;

    QxtLinkedTreeIterator erase() ;
    QxtLinkedTreeIterator append(const T & value);
    QxtLinkedTreeIterator insert(int i, const T & value);

private:
    friend class QxtLinkedTree<T>;

    QxtLinkedTreeIterator(QxtLinkedTreeItem<T> * t);
    QxtLinkedTreeItem<T> *item;
};

template<class T>
class QXT_CORE_EXPORT QxtLinkedTree
{
public:

    QxtLinkedTree();
    QxtLinkedTree(T t);
    ~QxtLinkedTree();
    void clear();
    QxtLinkedTreeIterator<T> root();
    static QxtLinkedTreeIterator<T> fromVoid(void *) ;
    static void * toVoid(QxtLinkedTreeIterator<T>) ;

#if 0
    QxtLinkedTreeIterator insert(iterator before, const T & value);
#endif

private:
    QxtSharedPrivate< QxtLinkedTreeItem<T> >  qxt_d;
};

template<class T>
QxtLinkedTreeIterator<T>::QxtLinkedTreeIterator()
{
    item = 0;
}

template<class T>
QxtLinkedTreeIterator<T>::QxtLinkedTreeIterator(const QxtLinkedTreeIterator<T> & other)
{
    item = other.item;
}

template<class T>
QxtLinkedTreeIterator<T> & QxtLinkedTreeIterator<T>::operator= (const QxtLinkedTreeIterator<T> & other)
{
    item = other.item;
    return *this;
}

template<class T>
QxtLinkedTreeIterator<T> QxtLinkedTreeIterator<T>::parent() const
{
    Q_ASSERT_X(item, Q_FUNC_INFO, "iterator out of range");
    return QxtLinkedTreeIterator<T>(item->parent);
}

template<class T>
QxtLinkedTreeIterator<T> QxtLinkedTreeIterator<T>::next() const
{
    Q_ASSERT_X(item, Q_FUNC_INFO, "iterator out of range");
    return QxtLinkedTreeIterator<T>(item->next);
}

template<class T>
QxtLinkedTreeIterator<T> QxtLinkedTreeIterator<T>::previous() const
{
    Q_ASSERT_X(item, Q_FUNC_INFO, "iterator out of range");
    return QxtLinkedTreeIterator<T>(item->previous);
}

template<class T>
QxtLinkedTreeIterator<T> QxtLinkedTreeIterator<T>::child() const
{
    Q_ASSERT_X(item, Q_FUNC_INFO, "iterator out of range");
    return QxtLinkedTreeIterator<T>(item->child);
}

template<class T>
bool  QxtLinkedTreeIterator<T>::isValid() const
{
    return (item != 0);
}

template<class T>
int  QxtLinkedTreeIterator<T>::children() const
{
    Q_ASSERT_X(item, Q_FUNC_INFO, "iterator out of range");
    return item->childcount;
}

template<class T>
T &  QxtLinkedTreeIterator<T>::operator*() const
{
    Q_ASSERT_X(item, Q_FUNC_INFO, "iterator out of range");
    return item->t;
}

template<class T>
T *  QxtLinkedTreeIterator<T>::operator-> () const
{
    Q_ASSERT_X(item, Q_FUNC_INFO, "iterator out of range");
    return &item->t;
}

template<class T>
QxtLinkedTreeIterator<T>::operator T() const
{
    Q_ASSERT_X(item,Q_FUNC_INFO, "iterator out of range");
    return item->t;
}

template<class T>
QxtLinkedTreeIterator<T>::QxtLinkedTreeIterator(QxtLinkedTreeItem<T> * t)
{
    item = t;
}

template<class T>
QxtLinkedTreeIterator<T>   QxtLinkedTreeIterator<T>::operator + (int j) const
{
    QxtLinkedTreeItem<T>  * m = item;
    for (int i = 0;i < j;i++)
    {
        m = m->next;
        if (m == 0)
            return QxtLinkedTreeIterator<T>();
    }
    return QxtLinkedTreeIterator<T>(m);
}

template<class T>
QxtLinkedTreeIterator<T> &  QxtLinkedTreeIterator<T>::operator ++ () /*prefix*/
{
    *this = QxtLinkedTreeIterator<T>(item->next);
    return *this;
}

template<class T>
QxtLinkedTreeIterator<T>   QxtLinkedTreeIterator<T>::operator ++ (int) /*postfix*/
{
    QxtLinkedTreeIterator<T> d(*this);
    *this = QxtLinkedTreeIterator<T>(item->next);
    return d;
}

template<class T>
QxtLinkedTreeIterator<T> &  QxtLinkedTreeIterator<T>::operator += (int j)
{
    *this = *this + j;
    return *this;
}

template<class T>
QxtLinkedTreeIterator<T>   QxtLinkedTreeIterator<T>::operator - (int j) const
{
    QxtLinkedTreeItem<T>  * m = item;
    for (int i = 0;i < j;i++)
    {
        m = m->previous;
        if (m == 0)
            return QxtLinkedTreeIterator<T>();
    }
    return QxtLinkedTreeIterator<T>(m);
}

template<class T>
QxtLinkedTreeIterator<T> &  QxtLinkedTreeIterator<T>::operator -- () /*prefix*/
{
    *this = QxtLinkedTreeIterator<T>(item->previous);
    return *this;
}

template<class T>
QxtLinkedTreeIterator<T>   QxtLinkedTreeIterator<T>::operator -- (int) /*postfix*/
{
    QxtLinkedTreeIterator<T> d(*this);
    *this = QxtLinkedTreeIterator<T>(item->previous);
    return d;

}

template<class T>
QxtLinkedTreeIterator<T> &  QxtLinkedTreeIterator<T>::operator -= (int j)
{
    *this = *this - j;
    return *this;
}

template<class T>
bool QxtLinkedTreeIterator<T>::operator== (const QxtLinkedTreeIterator<T> & other) const
{
    return (other.item == item);
}

template<class T>
bool QxtLinkedTreeIterator<T>::operator!= (const QxtLinkedTreeIterator<T> & other) const
{
    return (other.item != item);
}

template<class T>
QxtLinkedTreeIterator<T>  QxtLinkedTreeIterator<T>::erase()
{
    QxtLinkedTreeItem <T> *node = item;
    Q_ASSERT_X(item, Q_FUNC_INFO, "can't erase invalid node.");
    QxtLinkedTreeItem <T> *parent = item->parent;
    Q_ASSERT_X(parent, Q_FUNC_INFO, "erasing root node not supported.");
    QxtLinkedTreeItem <T> *next = node->next;

    ///delete children
    QxtLinkedTreeIterator<T> ci = child();
    while (ci.isValid())
    {
        ci = ci.erase();
    }

    ///realign chains
    if (parent->child == node)
    {
        parent->child = node->next;
    }
    else
    {
        QxtLinkedTreeItem <T> * n = parent->child;
        while (n->next != node)
        {
            Q_ASSERT_X(n->next != 0, Q_FUNC_INFO, "reached end of chain and didn't find the node requested for removal.");
            n = n->next;
        }
        n->next = node->next;
    }
    parent->childcount--;
    delete node;
    item = 0;
    return QxtLinkedTreeIterator<T>(next);
}

template<class T>
QxtLinkedTreeIterator<T>  QxtLinkedTreeIterator<T>::append(const T & value)
{
    QxtLinkedTreeItem <T> * parent = item;
    Q_ASSERT_X(parent, Q_FUNC_INFO, "invalid iterator");

    QxtLinkedTreeItem<T> *node = new QxtLinkedTreeItem<T>(value);

    if (parent->child == 0)
    {
        parent->child = node;
        node->parent = parent;
        node->previous = 0;
        parent->childcount = 1;
        return QxtLinkedTreeIterator<T>(node);
    }

    QxtLinkedTreeItem <T> * n = parent->child;
    while (n->next != 0)
        n = n->next;
    n->next = node;
    node->parent = parent;
    node->previous = n;
    parent->childcount++;
    return QxtLinkedTreeIterator<T>(node);
}

template<class T>
QxtLinkedTreeIterator<T>  QxtLinkedTreeIterator<T>::insert(int i, const T & value)
{
    QxtLinkedTreeItem <T> * parent = item;
    Q_ASSERT_X(parent, Q_FUNC_INFO, "invalid iterator");


    Q_ASSERT_X(i <= children(), Q_FUNC_INFO, "cannot insert out of range");


    if (parent->child == 0 ||  i == children())
    {
        return append(value);
    }

    QxtLinkedTreeItem<T> *node = new QxtLinkedTreeItem<T>(value);

    QxtLinkedTreeItem <T> * n = parent->child;

    while (i-- > 0)
    {
        n = n->next;
        Q_ASSERT_X(n, Q_FUNC_INFO, "out of range");
    }
    if (n->previous)
    {
        n->previous->next = node;
        node->previous = n->previous;
    }
    else
    {
        Q_ASSERT_X(parent->child == n, Q_FUNC_INFO, "corupted linked tree");
        parent->child = node;
        node->previous = 0;
    }
    node->next = n;
    n->previous = node;
    node->parent = parent;
    parent->childcount++;
    return QxtLinkedTreeIterator<T>(node);
}

template<class T>
QxtLinkedTree<T>::QxtLinkedTree(T t)
{
    qxt_d = new QxtLinkedTreeItem<T>(t);
}

template<class T>
QxtLinkedTree<T>::QxtLinkedTree()
{
    qxt_d = new QxtLinkedTreeItem<T>(T());
}

template<class T>
QxtLinkedTree<T>::~QxtLinkedTree()
{
}

template<class T>
void QxtLinkedTree<T>::clear()
{
    qxt_d().clear();
}

template<class T>
QxtLinkedTreeIterator<T>  QxtLinkedTree<T>::fromVoid(void * d)
{
    return QxtLinkedTreeIterator<T>(reinterpret_cast<QxtLinkedTreeItem<T> *>(d));
}

template<class T>
void *  QxtLinkedTree<T>::toVoid(QxtLinkedTreeIterator<T> n)
{
    return reinterpret_cast<void*>(n.item);
}

template<class T>
QxtLinkedTreeIterator<T> QxtLinkedTree<T>::root()
{
    return QxtLinkedTreeIterator<T>(&qxt_d());
}

#endif // QXTLINKEDTREE_H
