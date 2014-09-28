/**
 ******************************************************************************
 *
 * @file       airframestabfixedwingpage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup AirframeStabFixedwingPage
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

#ifndef AIRFRAMEINITIALTUNINGPAGE_H
#define AIRFRAMEINITIALTUNINGPAGE_H

#include "abstractwizardpage.h"
#include <QJsonObject>

namespace Ui {
class AirframeInitialTuningPage;
}

class AirframeInitialTuningPage : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit AirframeInitialTuningPage(SetupWizard *wizard, QWidget *parent = 0);
    ~AirframeInitialTuningPage();
    void initializePage();
    bool validatePage();
    bool isComplete() const;

public slots:
    void templateSelectionChanged();

protected:
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);

private:
    Ui::AirframeInitialTuningPage *ui;
    const char *m_dir;
    QMap<QString, QJsonObject *> m_templates;
    QGraphicsPixmapItem *m_photoItem;

    void loadValidFiles();
    void setupTemplateList();
    QString getTemplateKey(QJsonObject *templ);
    void updatePhoto(QJsonObject *templ);
    void updateDescription(QJsonObject *templ);
};

Q_DECLARE_METATYPE(QJsonObject *)

#endif // AIRFRAMEINITIALTUNINGPAGE_H
