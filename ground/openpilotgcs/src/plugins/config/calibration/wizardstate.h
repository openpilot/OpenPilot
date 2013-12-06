#ifndef WIZARDSTATE_H
#define WIZARDSTATE_H

#include <QState>

class WizardState : public QState
{
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isDone READ isDone NOTIFY isDoneChanged)
    Q_PROPERTY(qint8 completion READ completion NOTIFY completionChanged)
    Q_PROPERTY(QString stepName READ stepName NOTIFY stepNameChanged)
public:
    explicit WizardState(QString name, QState *parent = 0);
    bool isActive(){
        return m_active;
    }

    bool isDone(){
        return m_done;
    }

    qint8 completion(){
        return m_completion;
    }

    QString stepName(){
        return m_stepName;
    }

    void setStepName(QString name);
    void setCompletion(qint8 completion);
    virtual void onEntry(QEvent *event) Q_DECL_OVERRIDE;
    virtual void onExit(QEvent *event) Q_DECL_OVERRIDE;
signals:
    void isActiveChanged();
    void isDoneChanged();
    void stepNameChanged();
    void completionChanged();
public slots:
    void clean();
private:
    void setIsDone(bool done);
    bool m_done;
    bool m_active;
    qint8 m_completion;
    QString m_stepName;
};

#endif // WIZARDSTATE_H
