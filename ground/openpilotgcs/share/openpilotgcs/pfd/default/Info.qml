import QtQuick 2.0

Item {
    id: info
    property variant sceneSize

    //
    // Waypoint functions
    //

    property real posEast_old
    property real posNorth_old
    property real total_distance
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

    property real est_flight_time: Math.round(FlightBatteryState.EstimatedFlightTime)
    property real est_time_h: (est_flight_time > 0 ? Math.floor(est_flight_time / 3600) : 0 )
    property real est_time_m: (est_flight_time > 0 ? Math.floor((est_flight_time - est_time_h*3600)/60) : 0) 
    property real est_time_s: (est_flight_time > 0 ? Math.floor(est_flight_time - est_time_h*3600 - est_time_m*60) : 0)

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

    //
    // Panel functions
    //

    property bool hide_display_rc: false
    property bool hide_display_bat: false
    property bool hide_display_oplm: false

    function hide_display_rcinput(){
        if (hide_display_rc == false && hide_display_bat == false && hide_display_oplm == false)
            hide_display_rc = true;
        else
            hide_display_rc = false;
            battery_bg.z = -1
            oplm_bg.z = -1
    }

    function hide_display_battery(){
        if (hide_display_bat == false && hide_display_rc == false && hide_display_oplm == false)
            hide_display_bat = true;
        else
            hide_display_bat = false;
            battery_bg.z = 10
            oplm_bg.z = -1
    }

    function hide_display_oplink(){
        if (hide_display_oplm == false && hide_display_rc == false && hide_display_bat == false)
            hide_display_oplm = true;
        else
            hide_display_oplm = false;
            oplm_bg.z = 20
    }

                             // Uninitialised, Ok,   Warning, Critical, Error                      
    property variant batColors : ["#2c2929", "green", "orange", "red", "red"]

    property real smeter_angle

    // Needed to get correctly int8 value, reset value (-127) on disconnect
    property int oplm0_db: OPLinkStatus.LinkState == 4 ? OPLinkStatus.PairSignalStrengths_0 : -127
    property int oplm1_db: OPLinkStatus.LinkState == 4 ? OPLinkStatus.PairSignalStrengths_1 : -127
    property int oplm2_db: OPLinkStatus.LinkState == 4 ? OPLinkStatus.PairSignalStrengths_2 : -127
    property int oplm3_db: OPLinkStatus.LinkState == 4 ? OPLinkStatus.PairSignalStrengths_3 : -127
 
    // Filtering for S-meter. Smeter range -127dB <--> -13dB = S9+60dB

    Timer {
         id: smeter_filter0
         interval: 100; running: true; repeat: true
         onTriggered: smeter_angle = (0.90 * smeter_angle) + (0.1 * (oplm0_db + 13))
    }

    Timer {
         id: smeter_filter1
         interval: 100; repeat: true
         onTriggered: smeter_angle = (0.90 * smeter_angle) + (0.1 * (oplm1_db + 13))
    }

    Timer {
         id: smeter_filter2
         interval: 100; repeat: true
         onTriggered: smeter_angle = (0.90 * smeter_angle) + (0.1 * (oplm2_db + 13))
     }

    Timer {
         id: smeter_filter3
         interval: 100; repeat: true
         onTriggered: smeter_angle = (0.90 * smeter_angle) + (0.1 * (oplm3_db + 13))
    }

    property int smeter_filter
    property variant oplm_pair_id : OPLinkStatus.PairIDs_0

    function select_oplm(index){
         smeter_filter0.running = false;
         smeter_filter1.running = false;
         smeter_filter2.running = false;
         smeter_filter3.running = false;

         switch(index) {
            case 0:
                smeter_filter0.running = true;
                smeter_filter = 0;
                oplm_pair_id = OPLinkStatus.PairIDs_0
                break;
            case 1:
                smeter_filter1.running = true;
                smeter_filter = 1;
                oplm_pair_id = OPLinkStatus.PairIDs_1
                break;
            case 2:
                smeter_filter2.running = true;
                smeter_filter = 2;
                oplm_pair_id = OPLinkStatus.PairIDs_2
                break;
            case 3:
                smeter_filter3.running = true;
                smeter_filter = 3;
                oplm_pair_id = OPLinkStatus.PairIDs_3
                break;
         }
     }

    // End Functions
    // 
    // Start Drawing

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
             cursorShape: hide_display_bat == false && hide_display_oplm == false ? Qt.WhatsThisCursor : Qt.ArrowCursor  
             onClicked: hide_display_bat == false && hide_display_oplm == false ? hide_display_rcinput() : 0
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
             cursorShape: hide_display_rc == false && hide_display_oplm == false ? Qt.WhatsThisCursor : Qt.ArrowCursor
             onClicked: hide_display_rc == false && hide_display_oplm == false ? hide_display_battery() : 0
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

    //
    // OPLM panel
    //

    SvgElementImage {
        id: oplm_bg
        elementName: "oplm-bg"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 20

        states: State  {
             name: "fading"
             when: hide_display_oplm !== true
             PropertyChanges  { target: oplm_bg; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementImage {
        id: smeter_bg
        elementName: "smeter-bg"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 21

        states: State  {
             name: "fading"
             when: hide_display_oplm !== true
             PropertyChanges  { target: smeter_bg; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementImage {
        id: smeter_scale
        elementName: "smeter-scale"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 22

        states: State  {
             name: "fading"
             when: hide_display_oplm !== true
             PropertyChanges  { target: smeter_scale; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementImage {
        id: smeter_needle
        elementName: "smeter-needle"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 23

        states: State  {
             name: "fading"
             when: hide_display_oplm !== true
             PropertyChanges  { target: smeter_needle; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        }

        transform: Rotation {
            angle: smeter_angle.toFixed(1)
             origin.y : smeter_needle.height 
        } 
    }

    SvgElementImage {
        id: smeter_mask
        elementName: "smeter-mask"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 24

        states: State  {
             name: "fading"
             when: hide_display_oplm !== true
             PropertyChanges  { target: smeter_mask; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        }
    }

    SvgElementImage {
        id: oplm_button_bg
        elementName: "oplm-button-bg"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 25

        states: State  {
             name: "fading"
             when: hide_display_oplm !== true
             PropertyChanges  { target: oplm_button_bg; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        }
    }

    Repeater {
        model: 4

        SvgElementImage {
            z: 25
            property variant idButton_oplm: "oplm_button_" + index
            property variant idButton_oplm_mousearea: "oplm_button_mousearea" + index
            property variant button_color: "button"+index+"_color"

            id: idButton_oplm
            
            elementName: "oplm-button-" + index
            sceneSize: info.sceneSize

            Rectangle {
                anchors.fill: parent
                border.color: "red"
                border.width: parent.width * 0.04
                radius: border.width*3
                color: "transparent"
                opacity: smeter_filter == index ? 0.5 : 0
            }

            MouseArea { 
                 id: idButton_oplm_mousearea; 
                 anchors.fill: parent; 
                 cursorShape: Qt.PointingHandCursor
                 onClicked: select_oplm(index)
            }

            states: State  {
                 name: "fading"
                 when: hide_display_oplm !== true
                 PropertyChanges  { target: idButton_oplm; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
            }
 
            transitions: Transition  {
            SequentialAnimation  {
                 PropertyAnimation  { property: "x"; duration: 800 }
                 }
            }
        }
    }

    SvgElementImage {
        id: oplm_id_label
        elementName: "oplm-id-label"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 26
        states: State  {
             name: "fading"
             when: hide_display_oplm !== true
             PropertyChanges  { target: oplm_id_label; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementPositionItem {
        id: oplm_id_text
        sceneSize: info.sceneSize
        elementName: "oplm-id-text"
        z: 27

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: hide_display_oplm !== true
             PropertyChanges  { target: oplm_id_text; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Text {
             text: oplm_pair_id > 0 ? oplm_pair_id.toString(16) : "--  --  --  --"
             anchors.centerIn: parent
             color: "white"
             font {
                 family: "Arial"
                 pixelSize: Math.floor(parent.height * 1.4)
                 weight: Font.DemiBold
                 capitalization: Font.AllUppercase
             }
        }
    }

    SvgElementImage {
        id: oplm_mousearea
        elementName: "oplm-panel-mousearea"
        sceneSize: info.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 26

        MouseArea { 
             id: hidedisp_oplm; 
             anchors.fill: parent; 
             cursorShape: hide_display_rc == false && hide_display_bat == false ? Qt.WhatsThisCursor : Qt.ArrowCursor
             onClicked: hide_display_rc == false && hide_display_bat == false ? hide_display_oplink() : 0
        }

        states: State  {
             name: "fading"
             when: hide_display_oplm !== true
             PropertyChanges  { target: oplm_mousearea; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
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
