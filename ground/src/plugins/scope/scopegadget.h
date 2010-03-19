/*
 * scopegadget.h
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */

#ifndef SCOPEGADGET_H_
#define SCOPEGADGET_H_

#include <coreplugin/iuavgadget.h>

class IUAVGadget;
//class QList<int>;
class QWidget;
class QString;
class ScopeGadgetWidget;

using namespace Core;

class ScopeGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    ScopeGadget(ScopeGadgetWidget *widget = 0);
    ~ScopeGadget();

    QList<int> context() const { return m_context; }
    QWidget *widget() { return m_widget; }
    QString contextHelpId() const { return QString(); }

    QWidget *toolBar() { return m_toolbar; }
private:
        QWidget *m_widget;
        QWidget *m_toolbar;
	QList<int> m_context;
};


#endif // SCOPEGADGET_H_
