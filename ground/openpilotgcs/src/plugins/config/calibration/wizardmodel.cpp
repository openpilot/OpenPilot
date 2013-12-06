#include "wizardmodel.h"

WizardModel::WizardModel(QObject *parent) :
    QStateMachine(parent)
{
}

WizardState* WizardModel::currentState(){
    foreach (QAbstractState *value, this->configuration()){
        if(value->parent() != 0){\
            return static_cast<WizardState*>(value);
        }
    }
    return NULL;
}
