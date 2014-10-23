import QtQuick 2.0

Item {
    id: warnings
    property variant sceneSize
                          //  Uninitialised, OK,    Warning, Error, Critical
    property variant statusColors : ["gray", "green", "red", "red", "red"]

                              //  DisArmed , Arming, Armed
    property variant armColors : ["gray", "orange", "green"]
                      
                      // All 'manual modes' are green, 'assisted' modes in cyan
                      // "MANUAL","STAB 1","STAB 2", "STAB 3", "STAB 4", "STAB 5", "STAB 6", "AUTOTUNE",
                      // "POS HOLD", "POS VFPV", "POS VLOS", "POS VNSEW", "RTB", "LAND", "PATHPLANNER", "POI", "AUTOCRUISE"

    property variant flightmodeColors : ["gray", "green", "green", "green", "green", "green", "green", "red", 
                                         "cyan", "cyan", "cyan", "cyan", "cyan", "cyan", "cyan", "cyan", "cyan"]

                      // Manual,Rate,Attitude,AxisLock,WeakLeveling,VirtualBar,Acro+,Rattitude,RelayRate,RelayAttitude,
                      // AltitudeHold,AltitudeVario,CruiseControl + Auto mode (VTOL/Wing pathfollower)
                      // grey : 'disabled' modes

    property variant thrustmodeColors : ["green", "grey", "grey", "grey", "grey", "grey", "grey", "grey", "grey", "grey",  
                                         "green", "green", "green", "cyan"]

                      // SystemSettings.AirframeType 3 - 17 : VtolPathFollower, check ThrustControl
 
    property var thrust_mode: FlightStatus.FlightMode < 7 ? StabilizationDesired.StabilizationMode_Thrust : 
                              FlightStatus.FlightMode > 7 && SystemSettings.AirframeType > 2 && SystemSettings.AirframeType < 18
                              && VtolPathFollowerSettings.ThrustControl == 1 ? 12 : 
                              FlightStatus.FlightMode > 7 && SystemSettings.AirframeType < 3 ? 12: 0 


    property real flight_time: Math.round(SystemStats.FlightTime / 1000)
    property real time_h: (flight_time > 0 ? Math.floor(flight_time / 3600) : 0 )
    property real time_m: (flight_time > 0 ? Math.floor((flight_time - time_h*3600)/60) : 0) 
    property real time_s: (flight_time > 0 ? Math.floor(flight_time - time_h*3600 - time_m*60) : 0)

    function formatTime(time) {
        if (time === 0)
            return "00"
        if (time < 10)
            return "0" + time;
        else
            return time.toString();
    }

    SvgElementImage {
        id: warning_bg
        elementName: "warnings-bg"
        sceneSize: warnings.sceneSize
        width: background.width
        anchors.bottom: parent.bottom
    }

    SvgElementPositionItem {
        id: warning_time
        sceneSize: parent.sceneSize
        elementName: "warning-time"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        Rectangle {
            anchors.fill: parent
            color: (SystemStats.FlightTime > 0 ? "green" : "grey")

            Text {
                anchors.centerIn: parent
                text: formatTime(time_h) + ":" + formatTime(time_m) + ":" + formatTime(time_s)
                font {
                    family: "Arial"
                    pixelSize: Math.floor(parent.height * 0.8)
                    weight: Font.DemiBold
                }
            }
        }
    }

    SvgElementPositionItem {
        id: warning_arm
        sceneSize: parent.sceneSize
        elementName: "warning-arm"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        Rectangle {
            anchors.fill: parent
            color: warnings.armColors[FlightStatus.Armed]

            Text {
                anchors.centerIn: parent
                text: ["DISARMED","ARMING","ARMED"][FlightStatus.Armed]
                font {
                    family: "Arial"
                    pixelSize: Math.floor(parent.height * 0.74)
                    weight: Font.DemiBold
                }
            }
        }
    }

    SvgElementPositionItem {
        id: warning_rc_input
        sceneSize: parent.sceneSize
        elementName: "warning-rc-input"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        Rectangle {
            anchors.fill: parent
            color: warnings.statusColors[SystemAlarms.Alarm_ManualControl]

            Text {
                anchors.centerIn: parent
                text: "RC INPUT"
                font {
                    family: "Arial"
                    pixelSize: Math.floor(parent.height * 0.74)
                    weight: Font.DemiBold
                }
            }
        }
    }

    SvgElementPositionItem {
        id: warning_master_caution
        sceneSize: parent.sceneSize
        elementName: "warning-master-caution"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        property bool warningActive: (SystemAlarms.Alarm_BootFault > 1 ||
                                      SystemAlarms.Alarm_OutOfMemory > 1 ||
                                      SystemAlarms.Alarm_StackOverflow > 1 ||
                                      SystemAlarms.Alarm_CPUOverload > 1 ||
                                      SystemAlarms.Alarm_EventSystem > 1)
        Rectangle {
            anchors.fill: parent
            color: parent.warningActive ? "red" : "red"
            opacity: parent.warningActive ? 1.0 : 0.4

            Text {
                anchors.centerIn: parent
                text: "MASTER CAUTION"
                color: "white"
                font {
                    family: "Arial"
                    pixelSize: Math.floor(parent.height * 0.74)
                    weight: Font.DemiBold
                }
            }
        }
    }

    SvgElementPositionItem {
        id: warning_autopilot
        sceneSize: parent.sceneSize
        elementName: "warning-autopilot"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        Rectangle {
            anchors.fill: parent
            color: warnings.statusColors[SystemAlarms.Alarm_Guidance]

            Text {
                anchors.centerIn: parent
                text: "AUTOPILOT"
                font {
                    family: "Arial"
                    pixelSize: Math.floor(parent.height * 0.74)
                    weight: Font.DemiBold
                }
            }
        }
    }

    SvgElementPositionItem {
        id: warning_flightmode
        sceneSize: parent.sceneSize
        elementName: "warning-flightmode"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        Rectangle {
            anchors.fill: parent
            color: warnings.flightmodeColors[FlightStatus.FlightMode]

            Text {
                anchors.centerIn: parent
                text: ["MANUAL","STAB 1","STAB 2", "STAB 3", "STAB 4", "STAB 5", "STAB 6", "AUTOTUNE", "POS HOLD", "POS VFPV",
                       "POS VLOS", "POS VNSEW", "RTB", "LAND", "PATHPLAN", "POI", "AUTOCRUISE"][FlightStatus.FlightMode]
                font {
                    family: "Arial"
                    pixelSize: Math.floor(parent.height * 0.74)
                    weight: Font.DemiBold
                }
            }
        }
    }

    SvgElementPositionItem {
        id: warning_thrustmode
        sceneSize: parent.sceneSize
        elementName: "warning-thrustmode"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height

        Rectangle {
            anchors.fill: parent
            color: FlightStatus.FlightMode < 1 ? "grey" : warnings.thrustmodeColors[thrust_mode.toString()]

                      // Manual,Rate,Attitude,AxisLock,WeakLeveling,VirtualBar,Acro+,Rattitude,RelayRate,RelayAttitude,
                      // AltitudeHold,AltitudeVario,CruiseControl
                      // grey : 'disabled' modes
            Text {
                anchors.centerIn: parent
                text: ["MANUAL"," "," ", " ", " ", " ", " ", " ", " ", " ",
                       "ALT HOLD", "ALT VARIO", "CRUISECTRL", "AUTO"][thrust_mode.toString()]
                font {
                    family: "Arial"
                    pixelSize: Math.floor(parent.height * 0.74)
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
        id: warning_attitude
        elementName: "warning-attitude"
        sceneSize: warnings.sceneSize
        anchors.centerIn: background.centerIn
        visible: SystemAlarms.Alarm_Attitude > 1
    }
}
