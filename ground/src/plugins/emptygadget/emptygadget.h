/*
 * emptygadget.h
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */

#ifndef EMPTYGADGET_H_
#define EMPTYGADGET_H_

#include <coreplugin/iuavgadget.h>

class IUAVGadget;
//class QList<int>;
class QWidget;
class QString;
class EmptyGadgetWidget;

using namespace Core;

class EmptyGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    EmptyGadget(EmptyGadgetWidget *widget = 0);
    ~EmptyGadget();

    QList<int> context() const { return m_context; }
    QWidget *widget() { return m_widget; }
    QString contextHelpId() const { return QString(); }

    QWidget *toolBar() { return m_toolbar; }
private:
        QWidget *m_widget;
        QWidget *m_toolbar;
	QList<int> m_context;
};


#endif // EMPTYGADGET_H_
