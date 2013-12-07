import QtQuick 2.0

Item {
    id: warnings
    property variant sceneSize
                          //  Uninitialised, OK,    Warning, Error, Critical
    property variant statusColors : ["gray", "green", "red", "red", "red"]

    SvgElementImage {
        id: warning_bg
        elementName: "warnings-bg"
        sceneSize: warnings.sceneSize
    }

    SvgElementPositionItem {
        id: warning_rc_input
        sceneSize: parent.sceneSize
        elementName: "warning-rc-input"

        Rectangle {
            anchors.fill: parent
            color: warnings.statusColors[SystemAlarms.Alarm_ManualControl]

            Text {
                anchors.centerIn: parent
                text: "RC INPUT"
                font {
                    family: "Arial"
                    pixelSize: parent.height * 0.8
                    weight: Font.DemiBold
                }
            }
        }
    }

    SvgElementPositionItem {
        id: warning_master_caution
        sceneSize: parent.sceneSize
        elementName: "warning-master-caution"

        property bool warningActive: (SystemAlarms.Alarm_BootFault > 1 ||
                                      SystemAlarms.Alarm_OutOfMemory > 1 ||
                                      SystemAlarms.Alarm_StackOverflow > 1 ||
                                      SystemAlarms.Alarm_CPUOverload > 1 ||
                                      SystemAlarms.Alarm_EventSystem > 1)
        Rectangle {
            anchors.fill: parent
            color: parent.warningActive ? "red" : "red"
            opacity: parent.warningActive ? 1.0 : 0.15

            Text {
                anchors.centerIn: parent
                text: "MASTER CAUTION"
                font {
                    family: "Arial"
                    pixelSize: parent.height * 0.8
                    weight: Font.DemiBold
                }
            }
        }
    }

    SvgElementPositionItem {
        id: warning_autopilot
        sceneSize: parent.sceneSize
        elementName: "warning-autopilot"

        Rectangle {
            anchors.fill: parent
            color: warnings.statusColors[SystemAlarms.Alarm_Guidance]

            Text {
                anchors.centerIn: parent
                text: "AUTOPILOT"
                font {
                    family: "Arial"
                    pixelSize: parent.height * 0.8
                    weight: Font.DemiBold
                }
            }
        }
    }

    SvgElementImage {
        id: warning_gps
        elementName: "warning-gps"
        sceneSize: warnings.sceneSize

        visible: SystemAlarms.Alarm_GPS > 1
    }

    SvgElementImage {
        id: warning_telemetry
        elementName: "warning-telemetry"
        sceneSize: warnings.sceneSize

        visible: SystemAlarms.Alarm_Telemetry > 1
    }

    SvgElementImage {
        id: warning_battery
        elementName: "warning-battery"
        sceneSize: warnings.sceneSize

        visible: SystemAlarms.Alarm_Battery > 1
    }

    SvgElementImage {
        id: warning_attitude
        elementName: "warning-attitude"
        sceneSize: warnings.sceneSize

        visible: SystemAlarms.Alarm_Attitude > 1
    }
}
