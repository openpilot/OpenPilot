import QtQuick 2.0

Item {
    id: info
    property variant sceneSize
    
    property var tele_status : (OPLinkStatus.LinkState == 1 ? GCSTelemetryStats.Status : 
                                OPLinkStatus.LinkState == 4 ? GCSTelemetryStats.Status : 0 )

    property real home_heading: 180/3.1415 * Math.atan2(TakeOffLocation.East - PositionState.East, 
                                                        TakeOffLocation.North - PositionState.North)

    property real home_distance: Math.sqrt(Math.pow((TakeOffLocation.East - PositionState.East),2) +
                                           Math.pow((TakeOffLocation.North - PositionState.North),2))

    property real wp_heading: 180/3.1415 * Math.atan2(PathDesired.End_East - PositionState.East, 
                                                      PathDesired.End_North - PositionState.North)

    property real wp_distance: Math.sqrt(Math.pow((PathDesired.End_East - PositionState.East),2) +
                                           Math.pow(( PathDesired.End_North - PositionState.North),2))

    property real current_velocity: Math.sqrt(Math.pow(VelocityState.North,2)+Math.pow(VelocityState.East,2))

    property real home_eta: (home_distance > 0 && current_velocity > 0 ? Math.round(home_distance/current_velocity) : 0)
    property real home_eta_h: (home_eta > 0 ? Math.floor(home_eta / 3600) : 0 )
    property real home_eta_m: (home_eta > 0 ? Math.floor((home_eta - home_eta_h*3600)/60) : 0) 
    property real home_eta_s: (home_eta > 0 ? Math.floor(home_eta - home_eta_h*3600 - home_eta_m*60) : 0)

    property real wp_eta: (wp_distance > 0 && current_velocity > 0 ? Math.round(wp_distance/current_velocity) : 0)
    property real wp_eta_h: (wp_eta > 0 ? Math.floor(wp_eta / 3600) : 0 )
    property real wp_eta_m: (wp_eta > 0 ? Math.floor((wp_eta - wp_eta_h*3600)/60) : 0) 
    property real wp_eta_s: (wp_eta > 0 ? Math.floor(wp_eta - wp_eta_h*3600 - wp_eta_m*60) : 0)

    property real posEast_old
    property real posNorth_old
    property real total_distance
    property bool init_dist: false

    function reset_distance(){
        total_distance = 0;
    }

    function compute_distance(posEast,posNorth) {
        if (total_distance == 0 && !init_dist){init_dist = "true"; posEast_old = posEast; posNorth_old = posNorth;}
        if (posEast > posEast_old+3 || posEast < posEast_old-3 || posNorth > posNorth_old+3 || posNorth < posNorth_old-3) {
        total_distance += Math.sqrt(Math.pow((posEast - posEast_old ),2) + Math.pow((posNorth - posNorth_old),2));    

        posEast_old = posEast;
        posNorth_old = posNorth;
        return total_distance;
        } 
    }

    function formatTime(time) {
        if (time === 0)
            return "00"
        if (time < 10)
            return "0" + time;
        else
            return time.toString();
    }
    
    SvgElementImage {
        id: info_bg
        sceneSize: info.sceneSize
        elementName: "info-bg"
        width: parent.width
    }

    SvgElementImage {
        id: batinfo_energy
        elementName: "warning-low-energy"
        sceneSize: info.sceneSize
        Rectangle {
            anchors.fill: batinfo_energy
            border.width: 0
            // Alarm based on FlightBatteryState.EstimatedFlightTime < 120s orange, < 60s red
            color: (FlightBatteryState.EstimatedFlightTime <= 120 && FlightBatteryState.EstimatedFlightTime > 60 ? "orange" :
                   (FlightBatteryState.EstimatedFlightTime <= 60 ? "red": "black"))

            visible: FlightBatteryState.ConsumedEnergy > 0
        }
    }


    Repeater {
        id: satNumberBar

        // hack, qml/js treats qint8 as a char, necessary to convert it back to integer value
        property int satNumber : String(GPSPositionSensor.Satellites).charCodeAt(0)

        model: 10
        SvgElementImage {
            property int minSatNumber : index+1
            elementName: "gps" + minSatNumber
            sceneSize: info.sceneSize
            visible: satNumberBar.satNumber >= minSatNumber
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "gps-mode-text"

        Text {
            text: ["No GPS", "No Fix", "Fix2D", "Fix3D"][GPSPositionSensor.Status]
            anchors.centerIn: parent
            font.pixelSize: Math.floor(parent.height*1.2)
            color: "white"
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "telemetry-status"

        Text {
            text: ["Disconnected","HandshakeReq","HandshakeAck","Connected"][tele_status.toString()]

            anchors.centerIn: parent
            font.pixelSize: Math.floor(parent.height*1.2)
            color: "white"
        }
    }

    Repeater {
        id: txNumberBar

        property int txRateNumber : GCSTelemetryStats.TxDataRate.toFixed()/10 // 250 max

        model: 25
        SvgElementImage {
            property int minTxRateNumber : index+1
            elementName: "tx" + minTxRateNumber
            sceneSize: info.sceneSize
            visible: txNumberBar.txRateNumber >= minTxRateNumber && ((GCSTelemetryStats.Status ==3 && OPLinkStatus.LinkState == 4 )
                                                                  || (GCSTelemetryStats.Status ==3 && OPLinkStatus.LinkState == 1 ))
        }
    }

    Repeater {
        id: rxNumberBar

        property int rxRateNumber : GCSTelemetryStats.RxDataRate.toFixed()/100 // 2500 max

        model: 25
        SvgElementImage {
            property int minRxRateNumber : index+1
            elementName: "rx" + minRxRateNumber
            sceneSize: info.sceneSize
            visible: rxNumberBar.rxRateNumber >= minRxRateNumber && ((GCSTelemetryStats.Status ==3 && OPLinkStatus.LinkState == 4 )
                                                                  || (GCSTelemetryStats.Status ==3 && OPLinkStatus.LinkState == 1 ))
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "waypoint-heading-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: SystemAlarms.Alarm_PathPlan == 1

        Text {
            text: "   "+wp_heading.toFixed(1)+"°"

            anchors.centerIn: parent
            font.pixelSize: parent.height*1.1
            color: "magenta"
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "waypoint-distance-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: SystemAlarms.Alarm_PathPlan == 1

        Text {
            text: "  "+wp_distance.toFixed(0)+" m"

            anchors.centerIn: parent
            font.pixelSize: parent.height*1.1
            color: "magenta"
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "waypoint-total-distance-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: OPLinkStatus.LinkState == 4 //OPLink Connected

        MouseArea { id: total_dist_mouseArea; anchors.fill: parent; onClicked: reset_distance()}

        Text {
            text: "  "+total_distance.toFixed(0)+" m"

            anchors.centerIn: parent
            font.pixelSize: parent.height*1.1
            color: "magenta"
        }

        Timer {
            interval: 1000; running: true; repeat: true;
            onTriggered: {if (GPSPositionSensor.Status == 3) compute_distance(PositionState.East,PositionState.North)}
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "waypoint-eta-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: SystemAlarms.Alarm_PathPlan == 1

        Text {
            text: formatTime(wp_eta_h) + ":" + formatTime(wp_eta_m) + ":" + formatTime(wp_eta_s)

            anchors.centerIn: parent
            font.pixelSize: parent.height*1.1
            color: "magenta"
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "waypoint-number-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: SystemAlarms.Alarm_PathPlan == 1

        Text {
            text: (WaypointActive.Index+1)+" / "+PathPlan.WaypointCount

            anchors.centerIn: parent
            font.pixelSize: parent.height*1.1
            color: "magenta"
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "waypoint-mode-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: SystemAlarms.Alarm_PathPlan == 1

        Text {
            text: ["Fly End Point","Fly Vector","Fly Circle Right","Fly Circle Left","Drive End Point","Drive Vector","Drive Circle Right",
                   "Drive Circle Left","Fixed Attitude","Set Accessory","Land","Disarm Alarm"][PathDesired.Mode]

            anchors.centerIn: parent
            font.pixelSize: parent.height*1.1
            color: "magenta"
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "battery-volt-text"
        visible: FlightBatteryState.Voltage > 0

        Text {
            text: FlightBatteryState.Voltage.toFixed(2)
            anchors.centerIn: parent
            color: "white"
            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1.2)
            }
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "battery-amp-text"
        visible: FlightBatteryState.Current > 0

        Text {
            text: FlightBatteryState.Current.toFixed(2)
            anchors.centerIn: parent
            color: "white"
            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1.2)
            }
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "battery-milliamp-text"
        visible: FlightBatteryState.ConsumedEnergy > 0

        Text {
            text: FlightBatteryState.ConsumedEnergy.toFixed()
            anchors.centerIn: parent
            color: "white"
            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1.2)
            }
        }
    }

    Repeater {
        id: throttleNumberBar

        property int throttleNumber : ActuatorDesired.Thrust.toFixed(1)*10

        model: 10
        SvgElementImage {
            property int minThrottleNumber : index+1
            elementName: "eng" + minThrottleNumber
            sceneSize: info.sceneSize

            visible: throttleNumberBar.throttleNumber >= minThrottleNumber
        }
    }

    SvgElementImage {
        id: mask_ThrottleBar
        elementName: "throttle-mask"
        sceneSize: info.sceneSize
    }

    SvgElementImage {
        id: mask_SatBar
        elementName: "satbar-mask"
        sceneSize: info.sceneSize
    }

    SvgElementImage {
        id: mask_telemetryTx
        elementName: "tx-mask"
        sceneSize: info.sceneSize
    }

    SvgElementImage {
        id: mask_telemetryRx
        elementName: "rx-mask"
        sceneSize: info.sceneSize
    }

    SvgElementImage {
        id: home_bg
        elementName: "home-bg"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)

        states: State  {
             name: "fading"
             when: TakeOffLocation.Status !== 0
             PropertyChanges  { target: home_bg; x: Math.floor(scaledBounds.x * sceneItem.width) + home_bg.width;  }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        id: home_heading_text
        elementName: "home-heading-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)

        states: State  {
             name: "fading_heading"
             when: TakeOffLocation.Status !== 0
             PropertyChanges  { target: home_heading_text; x: Math.floor(scaledBounds.x * sceneItem.width) + home_bg.width  }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
        Text {
            text: "  "+home_heading.toFixed(1)+"°"
            anchors.centerIn: parent
            color: "cyan"
            font {
                family: "Arial"
                pixelSize: parent.height * 1.2
            }
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        id: home_distance_text
        elementName: "home-distance-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)

        states: State  {
             name: "fading_distance"
             when: TakeOffLocation.Status !== 0
             PropertyChanges  { target: home_distance_text; x: Math.floor(scaledBounds.x * sceneItem.width) + home_bg.width;  }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Text {
            text: home_distance.toFixed(0)+" m"
            anchors.centerIn: parent
            color: "cyan"
            font {
                family: "Arial"
                pixelSize: parent.height * 1.2
            }
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        id: home_eta_text
        elementName: "home-eta-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)

        states: State  {
             name: "fading_distance"
             when: TakeOffLocation.Status !== 0
             PropertyChanges  { target: home_eta_text; x: Math.floor(scaledBounds.x * sceneItem.width) + home_bg.width;  }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Text {
            text: formatTime(home_eta_h) + ":" + formatTime(home_eta_m) + ":" + formatTime(home_eta_s)
            anchors.centerIn: parent
            color: "cyan"
            font {
                family: "Arial"
                pixelSize: parent.height * 1.2
            }
        }
    }


    SvgElementImage {
        id: info_border
        elementName: "info-border"
        sceneSize: info.sceneSize
        width: Math.floor(parent.width * 1.009)
    }
}
