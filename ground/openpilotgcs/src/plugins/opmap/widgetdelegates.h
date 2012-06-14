#ifndef WIDGETDELEGATES_H
#define WIDGETDELEGATES_H
#include <QItemDelegate>
#include <QComboBox>
#include "flightdatamodel.h"


class ComboBoxDelegate : public QItemDelegate
 {
        Q_OBJECT

 public:
    typedef enum { MODE_FLYENDPOINT=0, MODE_FLYVECTOR=1, MODE_FLYCIRCLERIGHT=2, MODE_FLYCIRCLELEFT=3,
                   MODE_DRIVEENDPOINT=4, MODE_DRIVEVECTOR=5, MODE_DRIVECIRCLELEFT=6, MODE_DRIVECIRCLERIGHT=7,
                   MODE_FIXEDATTITUDE=8, MODE_SETACCESSORY=9, MODE_DISARMALARM=10 } ModeOptions;
    typedef enum { ENDCONDITION_NONE=0, ENDCONDITION_TIMEOUT=1, ENDCONDITION_DISTANCETOTARGET=2,
                   ENDCONDITION_LEGREMAINING=3, ENDCONDITION_ABOVEALTITUDE=4, ENDCONDITION_POINTINGTOWARDSNEXT=5,
                   ENDCONDITION_PYTHONSCRIPT=6, ENDCONDITION_IMMEDIATE=7 } EndConditionOptions;
    typedef enum { COMMAND_ONCONDITIONNEXTWAYPOINT=0, COMMAND_ONNOTCONDITIONNEXTWAYPOINT=1,
                   COMMAND_ONCONDITIONJUMPWAYPOINT=2, COMMAND_ONNOTCONDITIONJUMPWAYPOINT=3,
                   COMMAND_IFCONDITIONJUMPWAYPOINTELSENEXTWAYPOINT=4 } CommandOptions;

     ComboBoxDelegate(QObject *parent = 0);

     QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

     void setEditorData(QWidget *editor, const QModelIndex &index) const;
     void setModelData(QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index) const;

     void updateEditorGeometry(QWidget *editor,
         const QStyleOptionViewItem &option, const QModelIndex &index) const;
     static void loadComboBox(QComboBox * combo,flightDataModel::pathPlanDataEnum type);
     //void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
 };

#endif // WIDGETDELEGATES_H
