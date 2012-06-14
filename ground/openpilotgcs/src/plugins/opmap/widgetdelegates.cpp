#include "widgetdelegates.h"
#include <QComboBox>
#include <QRadioButton>
#include <QDebug>
QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem & option,
                                        const QModelIndex & index) const
{
    int column=index.column();
    QComboBox * box;
    switch(column)
    {
    case flightDataModel::MODE:
        box=new QComboBox(parent);
        ComboBoxDelegate::loadComboBox(box,flightDataModel::MODE);
        return box;
        break;
    case flightDataModel::CONDITION:
        box=new QComboBox(parent);
        ComboBoxDelegate::loadComboBox(box,flightDataModel::CONDITION);
        return box;
        break;

    case flightDataModel::COMMAND:
        box=new QComboBox(parent);
        ComboBoxDelegate::loadComboBox(box,flightDataModel::COMMAND);
        return box;
        break;
    default:
        return QItemDelegate::createEditor(parent,option,index);
        break;
    }

    QComboBox *editor = new QComboBox(parent);
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    QString className=editor->metaObject()->className();
    if (className.contains("QComboBox")) {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int x=comboBox->findData(value);
        qDebug()<<"VALUE="<<x;
        comboBox->setCurrentIndex(x);
    }
  /*  else if (className.contains("QRadioButton")) {
        bool value = index.model()->data(index, Qt::EditRole).toBool();
        QRadioButton *radioButton = static_cast<QRadioButton*>(editor);
        radioButton->setChecked(value);
    }*/
    else
        QItemDelegate::setEditorData(editor, index);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QString className=editor->metaObject()->className();
    if (className.contains("QComboBox")) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int value = comboBox->itemData(comboBox->currentIndex()).toInt();
        model->setData(index, value, Qt::EditRole);
    }/*
    else if (className.contains("QRadioButton")) {
        QRadioButton *radioButton = static_cast<QRadioButton*>(editor);
        bool value = radioButton->isChecked();
        model->setData(index, value, Qt::EditRole);
    }*/
    else
        QItemDelegate::setModelData(editor,model,index);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::loadComboBox(QComboBox *combo, flightDataModel::pathPlanDataEnum type)
{
    switch(type)
    {
    case flightDataModel::MODE:
        combo->addItem("Fly Direct",MODE_FLYENDPOINT);
        combo->addItem("Fly Vector",MODE_FLYVECTOR);
        combo->addItem("Fly Circle Right",MODE_FLYCIRCLERIGHT);
        combo->addItem("Fly Circle Left",MODE_FLYCIRCLELEFT);

        combo->addItem("Drive Direct",MODE_DRIVEENDPOINT);
        combo->addItem("Drive Vector",MODE_DRIVEVECTOR);
        combo->addItem("Drive Circle Right",MODE_DRIVECIRCLELEFT);
        combo->addItem("Drive Circle Left",MODE_DRIVECIRCLERIGHT);

        combo->addItem("Fixed Attitude",MODE_FIXEDATTITUDE);
        combo->addItem("Set Accessory",MODE_SETACCESSORY);
        combo->addItem("Disarm Alarm",MODE_DISARMALARM);
        break;
    case flightDataModel::CONDITION:
        combo->addItem("None",ENDCONDITION_NONE);
        combo->addItem("Timeout",ENDCONDITION_TIMEOUT);
        combo->addItem("Distance to tgt",ENDCONDITION_DISTANCETOTARGET);
        combo->addItem("Leg remaining",ENDCONDITION_LEGREMAINING);
        combo->addItem("Above Altitude",ENDCONDITION_ABOVEALTITUDE);
        combo->addItem("Pointing towards next",ENDCONDITION_POINTINGTOWARDSNEXT);
        combo->addItem("Python script",ENDCONDITION_PYTHONSCRIPT);
        combo->addItem("Immediate",ENDCONDITION_IMMEDIATE);
        break;
    case flightDataModel::COMMAND:
        combo->addItem("On conditon next wp",COMMAND_ONCONDITIONNEXTWAYPOINT);
        combo->addItem("On NOT conditon next wp",COMMAND_ONNOTCONDITIONNEXTWAYPOINT);
        combo->addItem("On conditon jump wp",COMMAND_ONCONDITIONJUMPWAYPOINT);
        combo->addItem("On NOT conditon jump wp",COMMAND_ONNOTCONDITIONJUMPWAYPOINT);
        combo->addItem("On conditon jump wp else next wp",COMMAND_IFCONDITIONJUMPWAYPOINTELSENEXTWAYPOINT);
        break;
    default:
        break;
    }
}
/*
void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionComboBox opt;
    opt.rect=option.rect;
    QApplication::style()->drawComplexControl(QStyle::CC_ComboBox, &opt,
    painter,createEditor(this,option,index));
}*/

ComboBoxDelegate::ComboBoxDelegate(QObject *parent):QItemDelegate(parent)
{
}
