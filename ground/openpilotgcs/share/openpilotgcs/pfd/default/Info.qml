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

    property real est_flight_time: Math.round(FlightBatteryState.EstimatedFlightTime)
    property real est_time_h: (est_flight_time > 0 ? Math.floor(est_flight_time / 3600) : 0 )
    property real est_time_m: (est_flight_time > 0 ? Math.floor((est_flight_time - est_time_h*3600)/60) : 0) 
    property real est_time_s: (est_flight_time > 0 ? Math.floor(est_flight_time - est_time_h*3600 - est_time_m*60) : 0)

    property real posEast_old
    property real posNorth_old
    property real total_distance
    property bool init_dist: false

    property bool hide_display_rc: false
    property bool hide_display_bat: false

                             // Uninitialised, Ok,   Warning, Critical, Error                      
    property variant batColors : ["#2c2929", "green", "orange", "red", "red"]

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

    function hide_display_rcinput(){
        if (hide_display_rc == false && hide_display_bat == false)
            hide_display_rc = true;
        else
            hide_display_rc = false;
            battery_bg.z = -1
    }

    function hide_display_battery(){
        if (hide_display_bat == false && hide_display_rc == false)
            hide_display_bat = true;
        else
            hide_display_bat = false;
            battery_bg.z = 10
    }
    
    SvgElementImage {
        id: info_bg
        sceneSize: info.sceneSize
        elementName: "info-bg"
        width: parent.width
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
            text: ["NO GPS", "NO FIX", "FIX 2D", "FIX 3D"][GPSPositionSensor.Status]
            anchors.centerIn: parent
            font.pixelSize: Math.floor(parent.height*1.3)
            font.family: "Arial"
            font.weight: Font.DemiBold
            color: "white"
        }
    }

    // 
    // Waypoint Info
    //

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
        visible: OPLinkStatus.LinkState == 4 //OPLink Connected

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

    SvgElementImage {
        id: mask_SatBar
        elementName: "satbar-mask"
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

    //
    // Rc-Input panel
    //

    SvgElementImage {
        id: rc_input_bg
        elementName: "rc-input-bg"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)

        states: State  {
             name: "fading"
             when: hide_display_rc !== true
             PropertyChanges  { target: rc_input_bg; x: Math.floor(scaledBounds.x * sceneItem.width) - (rc_input_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              id: rc_input_anim
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementImage {
        id: rc_input_labels
        elementName: "rc-input-labels"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)

        states: State  {
             name: "fading"
             when: hide_display_rc !== true
             PropertyChanges  { target: rc_input_labels; x: Math.floor(scaledBounds.x * sceneItem.width) - (rc_input_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementImage {
        id: rc_input_mousearea
        elementName: "rc-input-panel-mousearea"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)

        MouseArea { 
             id: hidedisp_rcinput; 
             anchors.fill: parent; 
             cursorShape: hide_display_bat == false  ? Qt.WhatsThisCursor : Qt.ArrowCursor  
             onClicked: hide_display_bat == false ? hide_display_rcinput() : 0
        }

        states: State  {
             name: "fading"
             when: hide_display_rc !== true
             PropertyChanges  { target: rc_input_mousearea; x: Math.floor(scaledBounds.x * sceneItem.width) - (rc_input_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementImage {
        id: rc_throttle
        elementName: "rc-throttle"
        sceneSize: info.sceneSize
        
        width: scaledBounds.width * sceneItem.width
        height: (scaledBounds.height * sceneItem.height) * (ManualControlCommand.Throttle)

        x: scaledBounds.x * sceneItem.width
        y: (scaledBounds.y * sceneItem.height) - rc_throttle.height + (scaledBounds.height * sceneItem.height)

        smooth: true
        
        states: State  {
             name: "fading"
             when: hide_display_rc !== true
             PropertyChanges  { target: rc_throttle; x: Math.floor(scaledBounds.x * sceneItem.width) - (rc_input_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementImage {
        id: rc_stick
        elementName: "rc-stick"
        sceneSize: info.sceneSize
        
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        x: (scaledBounds.x * sceneItem.width) + (ManualControlCommand.Roll * rc_stick.width * 2.5)
        y: (scaledBounds.y * sceneItem.height) + (ManualControlCommand.Pitch * rc_stick.width * 2.5)

        smooth: true
        
        //rotate it around his center
        transform: Rotation {
            angle: ManualControlCommand.Yaw * 90
            origin.y : rc_stick.height / 2
            origin.x : rc_stick.width / 2
        }

        states: State  {
             name: "fading"
             when: hide_display_rc !== true
             PropertyChanges  { target: rc_stick; x: Math.floor(scaledBounds.x * sceneItem.width) - (rc_input_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    //
    // Battery panel
    //

    SvgElementImage {
        id: battery_bg
        elementName: "battery-bg"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 10

        //visible: !hide_display_rc
        states: State  {
             name: "fading"
             when: hide_display_bat !== true
             PropertyChanges  { target: battery_bg; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementPositionItem {
        id: battery_volt
        sceneSize: info.sceneSize
        elementName: "battery-volt-text"
        z: 11
        
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: hide_display_bat !== true
             PropertyChanges  { target: battery_volt; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Rectangle {
            anchors.fill: parent
            color: info.batColors[SystemAlarms.Alarm_Battery]
            border.color: "white"
            border.width: battery_volt.width * 0.01
            radius: border.width * 4

            Text {
               text: FlightBatteryState.Voltage.toFixed(2)
               anchors.centerIn: parent
               color: "white"
               font {
                   family: "Arial"
                   pixelSize: Math.floor(parent.height * 0.6)
               }
            }
        }
    }

    SvgElementPositionItem {
        id: battery_amp
        sceneSize: info.sceneSize
        elementName: "battery-amp-text"
        z: 12

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: hide_display_bat !== true
             PropertyChanges  { target: battery_amp; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Rectangle {
            anchors.fill: parent
            color: info.batColors[SystemAlarms.Alarm_Battery]
            border.color: "white"
            border.width: battery_volt.width * 0.01
            radius: border.width * 4

            Text {
               text: FlightBatteryState.Current.toFixed(2)
               anchors.centerIn: parent
               color: "white"
               font {
                   family: "Arial"
                   pixelSize: Math.floor(parent.height * 0.6)
               }
            }
        }
    }

    SvgElementPositionItem {
        id: battery_milliamp
        sceneSize: info.sceneSize
        elementName: "battery-milliamp-text"
        z: 13

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: hide_display_bat !== true
             PropertyChanges  { target: battery_milliamp; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Rectangle {
            anchors.fill: parent

            // Alarm based on FlightBatteryState.EstimatedFlightTime < 120s orange, < 60s red
            color: (FlightBatteryState.EstimatedFlightTime <= 120 && FlightBatteryState.EstimatedFlightTime > 60 ? "orange" :
                   (FlightBatteryState.EstimatedFlightTime <= 60 ? "red": info.batColors[SystemAlarms.Alarm_Battery]))

            border.color: "white"
            border.width: battery_volt.width * 0.01
            radius: border.width * 4

            Text {
               text: FlightBatteryState.ConsumedEnergy.toFixed(0)
               anchors.centerIn: parent
               color: "white"
               font {
                   family: "Arial"
                   pixelSize: Math.floor(parent.height * 0.6)
               }
            }
        }
    }

    SvgElementPositionItem {
        id: battery_estimated_flight_time
        sceneSize: info.sceneSize
        elementName: "battery-estimated-flight-time"
        z: 14
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: hide_display_bat !== true
             PropertyChanges  { target: battery_estimated_flight_time; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Rectangle {
            anchors.fill: parent
            //color: info.batColors[SystemAlarms.Alarm_Battery]

            // Alarm based on FlightBatteryState.EstimatedFlightTime < 120s orange, < 60s red
            color: (FlightBatteryState.EstimatedFlightTime <= 120 && FlightBatteryState.EstimatedFlightTime > 60 ? "orange" :
                   (FlightBatteryState.EstimatedFlightTime <= 60 ? "red": info.batColors[SystemAlarms.Alarm_Battery]))

            border.color: "white"
            border.width: battery_volt.width * 0.01
            radius: border.width * 4

            Text {
               text: formatTime(est_time_h) + ":" + formatTime(est_time_m) + ":" + formatTime(est_time_s)
               anchors.centerIn: parent
               color: "white"
               font {
                   family: "Arial"
                   pixelSize: Math.floor(parent.height * 0.6)
               }
            }
        }
    }

    SvgElementImage {
        id: battery_labels
        elementName: "battery-labels"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 15
        states: State  {
             name: "fading"
             when: hide_display_bat !== true
             PropertyChanges  { target: battery_labels; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementImage {
        id: battery_mousearea
        elementName: "battery-panel-mousearea"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 16

        MouseArea { 
             id: hidedisp_battery; 
             anchors.fill: parent; 
             cursorShape: hide_display_rc == false  ? Qt.WhatsThisCursor : Qt.ArrowCursor
             onClicked: hide_display_rc == false ? hide_display_battery() : 0
        }

        states: State  {
             name: "fading"
             when: hide_display_bat !== true
             PropertyChanges  { target: battery_mousearea; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
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
