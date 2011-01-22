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
 **
 ** This is a derived work of PictureFlow (http://pictureflow.googlecode.com)
 ** The original code was distributed under the following terms:
 **
 ** Copyright (C) 2008 Ariya Hidayat (ariya@kde.org)
 ** Copyright (C) 2007 Ariya Hidayat (ariya@kde.org)
 **
 ** Permission is hereby granted, free of charge, to any person obtaining a copy
 ** of this software and associated documentation files (the "Software"), to deal
 ** in the Software without restriction, including without limitation the rights
 ** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 ** copies of the Software, and to permit persons to whom the Software is
 ** furnished to do so, subject to the following conditions:
 **
 ** The above copyright notice and this permission notice shall be included in
 ** all copies or substantial portions of the Software.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 ** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 ** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 ** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 ** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 ** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 ** THE SOFTWARE.
 ****************************************************************************/

#ifndef QXT_FLOWVIEW_H
#define QXT_FLOWVIEW_H

#include <QWidget>
#include <QAbstractItemModel>
#include "qxtglobal.h"


class QxtFlowViewPrivate;
class QXT_GUI_EXPORT QxtFlowView : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QSize slideSize READ slideSize WRITE setSlideSize)
    Q_PROPERTY(QModelIndex currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(int pictureRole READ pictureRole WRITE setPictureRole)
    Q_PROPERTY(int pictureColumn READ pictureColumn WRITE setPictureColumn)
    Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex)
    Q_PROPERTY(ReflectionEffect reflectionEffect READ reflectionEffect WRITE setReflectionEffect)
    Q_ENUMS(ReflectionEffect)

#if 0
    Q_PROPERTY(int textRole READ textRole WRITE setTextRole)
    Q_PROPERTY(int textColumn READ textColumn WRITE setTextColumn)
#endif


public:

    enum ReflectionEffect
    {
        NoReflection,
        PlainReflection,
        BlurredReflection
    };

    QxtFlowView(QWidget* parent = 0);
    ~QxtFlowView();

    void setModel(QAbstractItemModel * model);
    QAbstractItemModel * model();

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& c);

    QSize slideSize() const;
    void setSlideSize(QSize size);

    QModelIndex currentIndex() const;

    QModelIndex rootIndex() const;
    void setRootIndex(QModelIndex index);

    ReflectionEffect reflectionEffect() const;
    void setReflectionEffect(ReflectionEffect effect);

    int pictureRole();
    void setPictureRole(int);

    int pictureColumn();
    void setPictureColumn(int);

#if 0
    int textRole();
    void setTextRole(int);
    int textColumn();
    void setTextColumn(int);
#endif

public Q_SLOTS:
    void setCurrentIndex(QModelIndex index);

    void showPrevious();
    void showNext();

    void showSlide(QModelIndex index);

    void render();
    void triggerRender();

Q_SIGNALS:
    void currentIndexChanged(QModelIndex index);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void wheelEvent(QWheelEvent * event);

private Q_SLOTS:
    void updateAnimation();

private:
    QxtFlowViewPrivate* d;
};

#endif // PICTUREFLOW_H
