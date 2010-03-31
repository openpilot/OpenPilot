/**
 ******************************************************************************
 *
 * @file       modelviewgadgetoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   modelview
 * @{
 * 
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

#ifndef MODELVIEWGADGETOPTIONSPAGE_H
#define MODELVIEWGADGETOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"
#include <QtGui/QLabel>
#include <QtGui/QFileDialog>
#include <QtGui/QPushButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

namespace Core {
class IUAVGadgetConfiguration;
}
class ModelViewGadgetConfiguration;
class QFileDialog;

using namespace Core;

class ModelViewGadgetOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit ModelViewGadgetOptionsPage(ModelViewGadgetConfiguration *config, QObject *parent = 0);
    QString id() const { return ""; }
    QString trName() const { return ""; }
    QString category() const { return ""; }
    QString trCategory() const { return ""; }

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();
private:

signals:

public slots:
private slots:
    void changeAC();
    void changeBG();

private:
    ModelViewGadgetConfiguration *m_config;
    QLabel *m_acFileLabel;
    QLabel *m_bgFileLabel;
};

#endif // MODELVIEWGADGETOPTIONSPAGE_H
