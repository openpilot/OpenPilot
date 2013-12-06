#include "wizardstate.h"

WizardState::WizardState(QString name, QState *parent) :
    QState(parent)
{
    m_stepName = name;
    m_done = false;
    m_active = false;
}

void WizardState::setCompletion(qint8 completion){
    m_completion = completion;
    emit completionChanged();
}

void WizardState::setStepName(QString name)
{
    m_stepName = name;
    emit stepNameChanged();
}

void WizardState::onEntry(QEvent *event)
{
    Q_UNUSED(event);
    m_active = true;
    emit isActiveChanged();
}

void WizardState::onExit(QEvent *event)
{
    Q_UNUSED(event);
    m_active = false;
    setIsDone(true);
    emit isActiveChanged();
}

void WizardState::clean()
{
    setIsDone(false);
}

void WizardState::setIsDone(bool done)
{
    m_done = done;
    emit isDoneChanged();
}
