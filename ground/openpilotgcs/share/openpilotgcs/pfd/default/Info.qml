import QtQuick 2.0

Item {
    id: info
    property variant sceneSize

                             // Uninitialised, Ok,   Warning, Critical, Error                      
    property variant batColors : ["black", "green", "orange", "red", "red"]

    //
    // Waypoint functions
    //

    property real posEast_old
    property real posNorth_old
    property real total_distance
    property real total_distance_km
   
    property bool init_dist: false
    
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

    function reset_distance(){
        total_distance = 0;
    }

    function compute_distance(posEast,posNorth) {
        if (total_distance == 0 && !init_dist){init_dist = "true"; posEast_old = posEast; posNorth_old = posNorth;}
        if (posEast > posEast_old+3 || posEast < posEast_old-3 || posNorth > posNorth_old+3 || posNorth < posNorth_old-3) {
        total_distance += Math.sqrt(Math.pow((posEast - posEast_old ),2) + Math.pow((posNorth - posNorth_old),2)); 
        total_distance_km = total_distance / 1000;   

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

    // End Functions
    // 
    // Start Drawing
    // Foreground (gradient)

    SvgElementImage {
        id: foreground
        elementName: "foreground"
        sceneSize: info.sceneSize

        x: Math.floor(scaledBounds.x * info.width)
        y: Math.floor(scaledBounds.y * info.height)

    }

    SvgElementImage {
        id: info_bg
        sceneSize: info.sceneSize
        elementName: "info-bg"
        width: parent.width
    }

    // 
    // GPS Info (Top)
    //  

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
            text: ["NO GPS", "NO FIX", "FIX 2D", "FIX 3D"][GPSPositionSensor.Status]
            anchors.centerIn: parent
            font.pixelSize: Math.floor(parent.height*1.3)
            font.family: "Arial"
            font.weight: Font.DemiBold
            color: "white"
        }
    }

    // 
    // Waypoint Info (Top)
    // Only visible when PathPlan is active (WP loaded)

    SvgElementImage  {
        sceneSize: info.sceneSize
        elementName: "waypoint-labels"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: SystemAlarms.Alarm_PathPlan == 1
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
            color: "cyan"

            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1.5)
                weight: Font.DemiBold
            }
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
            color: "cyan"

            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1.5)
                weight: Font.DemiBold
            }
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "waypoint-total-distance-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: SystemAlarms.Alarm_PathPlan == 1

        MouseArea { id: total_dist_mouseArea; anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: reset_distance()}

        Text {
            text: "  "+total_distance.toFixed(0)+" m"
            anchors.centerIn: parent
            color: "cyan"

            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1.5)
                weight: Font.DemiBold
            }
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
            color: "cyan"

            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1.5)
                weight: Font.DemiBold
            }
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
            color: "cyan"

            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1.5)
                weight: Font.DemiBold
            }
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
            text: ["FLY END POINT","FLY VECTOR","FLY CIRCLE RIGHT","FLY CIRCLE LEFT","DRIVE END POINT","DRIVE VECTOR","DRIVE CIRCLE LEFT",
                   "DRIVE CIRCLE RIGHT","FIXED ATTITUDE","SET ACCESSORY","LAND","DISARM ALARM"][PathDesired.Mode]
            anchors.centerIn: parent
            color: "cyan"

            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1.5)
                weight: Font.DemiBold
            }
        }
    }

    // 
    // Battery Info (Top)
    // Only visible when PathPlan not active and Battery module enabled

    SvgElementPositionItem {
        id: topbattery_voltamp_bg
        sceneSize: info.sceneSize
        elementName: "topbattery-label-voltamp-bg"
        
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height
        visible: (SystemAlarms.Alarm_PathPlan != 1) && (HwSettings.OptionalModules_Battery == 1)

        Rectangle {
            anchors.fill: parent
            color:  FlightBatterySettings.NbCells > 0 ? info.batColors[SystemAlarms.Alarm_Battery] : "black"

        }
    }

    SvgElementImage  {
        sceneSize: info.sceneSize
        elementName: "topbattery-labels"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: (SystemAlarms.Alarm_PathPlan != 1) && (HwSettings.OptionalModules_Battery == 1)
    }

    SvgElementPositionItem {
        id: topbattery_volt
        sceneSize: info.sceneSize
        elementName: "topbattery-volt-text"
        
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height
        visible: (SystemAlarms.Alarm_PathPlan != 1) && (HwSettings.OptionalModules_Battery == 1)

        Rectangle {
            anchors.fill: parent
            color: "transparent"

            Text {
               text: FlightBatteryState.Voltage.toFixed(2)
               anchors.centerIn: parent
               color: "white"
               font {
                   family: "Arial"
                   pixelSize: Math.floor(parent.height * 0.6)
                   weight: Font.DemiBold
               }
            }
        }
    }

    SvgElementPositionItem {
        id: topbattery_amp
        sceneSize: info.sceneSize
        elementName: "topbattery-amp-text"

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height
        visible: (SystemAlarms.Alarm_PathPlan != 1) && (HwSettings.OptionalModules_Battery == 1)

        Rectangle {
            anchors.fill: parent
            color: "transparent"

            Text {
               text: FlightBatteryState.Current.toFixed(2)
               anchors.centerIn: parent
               color: "white"
               font {
                   family: "Arial"
                   pixelSize: Math.floor(parent.height * 0.6)
                   weight: Font.DemiBold
               }
            }
        }
    }

    SvgElementPositionItem {
        id: topbattery_milliamp
        sceneSize: info.sceneSize
        elementName: "topbattery-milliamp-text"

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height
        visible: (SystemAlarms.Alarm_PathPlan != 1) && (HwSettings.OptionalModules_Battery == 1)

        Rectangle {
            anchors.fill: parent

            // Alarm based on FlightBatteryState.EstimatedFlightTime < 120s orange, < 60s red
            color: (FlightBatteryState.EstimatedFlightTime <= 120 && FlightBatteryState.EstimatedFlightTime > 60 ? "orange" :
                   (FlightBatteryState.EstimatedFlightTime <= 60 ? "red": info.batColors[SystemAlarms.Alarm_Battery]))

            border.color: "white"
            border.width: topbattery_volt.width * 0.01
            radius: border.width * 4

            Text {
               text: FlightBatteryState.ConsumedEnergy.toFixed(0)
               anchors.centerIn: parent
               color: "white"
               font {
                   family: "Arial"
                   pixelSize: Math.floor(parent.height * 0.6)
                   weight: Font.DemiBold
               }
            }
        }
    }

    // 
    // Default counter
    // Only visible when PathPlan not active

    SvgElementImage  {
        sceneSize: info.sceneSize
        elementName: "topbattery-total-distance-label"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: SystemAlarms.Alarm_PathPlan != 1
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "topbattery-total-distance-text"
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: Math.floor(scaledBounds.y * sceneItem.height)
        visible: SystemAlarms.Alarm_PathPlan != 1

        MouseArea { id: total_dist_mouseArea2; anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: reset_distance()}

        Text {
            text:  total_distance > 1000 ? total_distance_km.toFixed(2) +" Km" : total_distance.toFixed(0)+" m"
            anchors.right: parent.right
            color: "cyan"

            font {
                family: "Arial"
                pixelSize: Math.floor(parent.height * 1)
                weight: Font.DemiBold
            }
        }

        Timer {
            interval: 1000; running: true; repeat: true;
            onTriggered: {if (GPSPositionSensor.Status == 3) compute_distance(PositionState.East,PositionState.North)}
        }
    }


    SvgElementImage {
        id: mask_SatBar
        elementName: "satbar-mask"
        sceneSize: info.sceneSize
    }

    //
    // Home info (visible after arming)
    //

    SvgElementImage {
        id: home_bg
        elementName: "home-bg"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)

        states: State  {
             name: "fading"
             when: TakeOffLocation.Status !== 0
             PropertyChanges  { target: home_bg; x: Math.floor(scaledBounds.x * sceneItem.width) + home_bg.width; }
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
             PropertyChanges  { target: home_heading_text; x: Math.floor(scaledBounds.x * sceneItem.width) + home_bg.width; }
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
             PropertyChanges  { target: home_distance_text; x: Math.floor(scaledBounds.x * sceneItem.width) + home_bg.width; }
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
             PropertyChanges  { target: home_eta_text; x: Math.floor(scaledBounds.x * sceneItem.width) + home_bg.width; }
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
