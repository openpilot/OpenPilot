#ifndef WIZARDMODEL_H
#define WIZARDMODEL_H

#include <QStateMachine>
#include <QQmlListProperty>
#include "wizardstate.h"

class WizardModel : public QStateMachine
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> steps READ steps CONSTANT)
    Q_PROPERTY(QString instructions READ instructions NOTIFY instructionChanged)
    Q_PROPERTY(WizardState* currentState READ currentState NOTIFY currentStateChanged)
public:
    explicit WizardModel(QObject *parent = 0);

    QQmlListProperty<QObject> steps() {
        return QQmlListProperty<QObject>(this, m_steps);
    }

    QString instructions(){
        return m_instructions;
    }

    void setInstructions(QString instructions){
        m_instructions = instructions;
        emit instructionChanged();
    }
    WizardState *currentState();
protected:
    QList<QObject*> m_steps;
private:
    QString m_instructions;
signals:
    void instructionChanged();
    void currentStateChanged();
public slots:

};

#endif // WIZARDMODEL_H
