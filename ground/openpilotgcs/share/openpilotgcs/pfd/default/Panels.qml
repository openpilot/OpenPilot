import QtQuick 2.0

Item {
    id: panels
    property variant sceneSize

    property real est_flight_time: Math.round(FlightBatteryState.EstimatedFlightTime)
    property real est_time_h: (est_flight_time > 0 ? Math.floor(est_flight_time / 3600) : 0 )
    property real est_time_m: (est_flight_time > 0 ? Math.floor((est_flight_time - est_time_h*3600)/60) : 0) 
    property real est_time_s: (est_flight_time > 0 ? Math.floor(est_flight_time - est_time_h*3600 - est_time_m*60) : 0)

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

    property bool show_panels: false
    property bool hide_display_rc: false
    property bool hide_display_bat: false
    property bool hide_display_oplm: false

    function close_panels(){
        if (show_panels == true)
            show_panels = false;
    }

    function hide_display_rcinput(){
        show_panels = true;
        rc_input_bg.z = 10
        battery_bg.z = -1
        oplm_bg.z = -1
        system_bg.z = -1         
    }

    function hide_display_battery(){
        show_panels = true;
        rc_input_bg.z = 10
        battery_bg.z = 20
        oplm_bg.z = -1
        system_bg.z = -1
    }

    function hide_display_oplink(){
        show_panels = true;
        rc_input_bg.z = 10
        battery_bg.z = 20
        oplm_bg.z = 30
        system_bg.z = -1
    }

    function hide_display_system(){
        show_panels = true;
        rc_input_bg.z = 10
        battery_bg.z = 20
        oplm_bg.z = 30
        system_bg.z = 40
    }

                             // Uninitialised, Ok,   Warning, Critical, Error                      
    property variant batColors : ["#2c2929", "green", "orange", "red", "red"]

    property real smeter_angle

    property real memory_free : SystemStats.HeapRemaining > 1024 ? SystemStats.HeapRemaining / 1024 : SystemStats.HeapRemaining 

    // Needed to get correctly int8 value, reset value (-127) on disconnect
    property int oplm0_db: telemetry_link == 1 ? OPLinkStatus.PairSignalStrengths_0 : -127
    property int oplm1_db: telemetry_link == 1 ? OPLinkStatus.PairSignalStrengths_1 : -127
    property int oplm2_db: telemetry_link == 1 ? OPLinkStatus.PairSignalStrengths_2 : -127
    property int oplm3_db: telemetry_link == 1 ? OPLinkStatus.PairSignalStrengths_3 : -127

    property real telemetry_sum
    property real telemetry_sum_old
    property bool telemetry_link

    // Hack : check if telemetry is active. Works with real link and log replay

    function telemetry_check(){
       telemetry_sum = OPLinkStatus.RXRate + OPLinkStatus.RXRate
       
       if (telemetry_sum != telemetry_sum_old || OPLinkStatus.LinkState == 4){
           telemetry_link = 1
       } else {
           telemetry_link = 0
       }
       telemetry_sum_old = telemetry_sum
    } 

    Timer {
         id: telemetry_activity
         interval: 1200; running: true; repeat: true
         onTriggered: telemetry_check()
    }

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

    //
    // Close panel
    //

    SvgElementImage {
        id: close_bg
        elementName: "close-bg"
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: close_bg; x: Math.floor(scaledBounds.x * sceneItem.width) - (close_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              id: close_anim
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementImage {
        id: close_mousearea
        elementName: "close-panel-mousearea"
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)

        MouseArea { 
             id: hidedisp_close; 
             anchors.fill: parent; 
             cursorShape: show_panels == true ? Qt.WhatsThisCursor : Qt.ArrowCursor  
             onClicked: close_panels()
        }

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: close_mousearea; x: Math.floor(scaledBounds.x * sceneItem.width) - (close_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    //
    // Rc-Input panel
    //

    SvgElementImage {
        id: rc_input_bg
        elementName: "rc-input-bg"
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 10

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: rc_input_bg.z+1

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: rc_input_bg.z+1

        MouseArea { 
             id: hidedisp_rcinput; 
             anchors.fill: parent; 
             cursorShape: hide_display_bat == false && hide_display_oplm == false ? Qt.WhatsThisCursor : Qt.ArrowCursor  
             onClicked: hide_display_bat == false && hide_display_oplm == false ? hide_display_rcinput() : 0
        }

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        z: rc_input_bg.z+2
        
        width: scaledBounds.width * sceneItem.width
        height: (scaledBounds.height * sceneItem.height) * (ManualControlCommand.Throttle)

        x: scaledBounds.x * sceneItem.width
        y: (scaledBounds.y * sceneItem.height) - rc_throttle.height + (scaledBounds.height * sceneItem.height)

        smooth: true
        
        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        z: rc_input_bg.z+3
        
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
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 20

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        elementName: "battery-volt-text"
        z: battery_bg.z+1
        
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: battery_volt; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Rectangle {
            anchors.fill: parent
            color: panels.batColors[SystemAlarms.Alarm_Battery]
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
        sceneSize: panels.sceneSize
        elementName: "battery-amp-text"
        z: battery_bg.z+2

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: battery_amp; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Rectangle {
            anchors.fill: parent
            color: panels.batColors[SystemAlarms.Alarm_Battery]
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
        sceneSize: panels.sceneSize
        elementName: "battery-milliamp-text"
        z: battery_bg.z+3

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: show_panels !== true
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
                   (FlightBatteryState.EstimatedFlightTime <= 60 ? "red": panels.batColors[SystemAlarms.Alarm_Battery]))

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
        sceneSize: panels.sceneSize
        elementName: "battery-estimated-flight-time"
        z: battery_bg.z+4

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: battery_estimated_flight_time; x: Math.floor(scaledBounds.x * sceneItem.width) - (battery_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Rectangle {
            anchors.fill: parent
            //color: panels.batColors[SystemAlarms.Alarm_Battery]

            // Alarm based on FlightBatteryState.EstimatedFlightTime < 120s orange, < 60s red
            color: (FlightBatteryState.EstimatedFlightTime <= 120 && FlightBatteryState.EstimatedFlightTime > 60 ? "orange" :
                   (FlightBatteryState.EstimatedFlightTime <= 60 ? "red": panels.batColors[SystemAlarms.Alarm_Battery]))

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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: battery_bg.z+5

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: battery_bg.z+6

        MouseArea { 
             id: hidedisp_battery; 
             anchors.fill: parent; 
             cursorShape: Qt.WhatsThisCursor
             onClicked: hide_display_battery()
        }

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 30

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: oplm_bg.z+1

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: oplm_bg.z+2

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: oplm_bg.z+3

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: oplm_bg.z+4

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: oplm_bg.z+5

        states: State  {
             name: "fading"
             when: show_panels !== true
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
            z: oplm_bg.z+5
            property variant idButton_oplm: "oplm_button_" + index
            property variant idButton_oplm_mousearea: "oplm_button_mousearea" + index
            property variant button_color: "button"+index+"_color"

            id: idButton_oplm
            
            elementName: "oplm-button-" + index
            sceneSize: panels.sceneSize

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
                 when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: oplm_bg.z+6
        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        elementName: "oplm-id-text"
        z: oplm_bg.z+7

        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        y: scaledBounds.y * sceneItem.height

        states: State  {
             name: "fading"
             when: show_panels !== true
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
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: oplm_bg.z

        MouseArea { 
             id: hidedisp_oplm; 
             anchors.fill: parent; 
             cursorShape: Qt.WhatsThisCursor
             onClicked: hide_display_oplink()
        }

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: oplm_mousearea; x: Math.floor(scaledBounds.x * sceneItem.width) - (oplm_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    //
    // System panel
    //

    SvgElementImage {
        id: system_bg
        elementName: "system-bg"
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: 40

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: system_bg; x: Math.floor(scaledBounds.x * sceneItem.width) - (system_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              id: system_anim
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }

    SvgElementPositionItem {
        id: system_frametype
        elementName: "system-frame-type"
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: system_bg.z+1

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: system_frametype; x: Math.floor(scaledBounds.x * sceneItem.width) - (system_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        }

        Text {
             text: ["FixedWing", "FixedWingElevon", "FixedWingVtail", "VTOL", "HeliCP", "QuadX", "QuadP",
                    "Hexa+", "Octo+", "Custom", "HexaX", "HexaH", "OctoV", "OctoCoaxP", "OctoCoaxX", "OctoX", "HexaCoax",
                    "Tricopter", "GroundVehicleCar", "GroundVehicleDiff", "GroundVehicleMoto"][SystemSettings.AirframeType]
             anchors.centerIn: parent
             color: "white"
             font {
                 family: "Arial"
                 pixelSize: Math.floor(parent.height * 1.4)
                 weight: Font.DemiBold
             }
        } 
    }

    SvgElementPositionItem {
        id: system_cpuloadtemp
        elementName: "system-cpu-load-temp"
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: system_bg.z+1

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: system_cpuloadtemp; x: Math.floor(scaledBounds.x * sceneItem.width) - (system_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Text {
             // CC3D hack, Cputemp not working
             text: SystemStats.CPULoad+"% - "+ 
                   [String(SystemStats.CPUTemp).charCodeAt(0) == "64" ? "??" : String(SystemStats.CPUTemp).charCodeAt(0)] +"°C"
             anchors.centerIn: parent
             color: "white"
             font {
                 family: "Arial"
                 pixelSize: Math.floor(parent.height * 1.4)
                 weight: Font.DemiBold
             }
        } 
    }

    SvgElementPositionItem {
        id: system_memfree
        elementName: "system-mem-free"
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: system_bg.z+1

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: system_memfree; x: Math.floor(scaledBounds.x * sceneItem.width) - (system_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 

        Text {
             text: SystemStats.HeapRemaining > 1024 ? memory_free.toFixed(2) +"Kb" : memory_free +"bytes"
             anchors.centerIn: parent
             color: "white"
             font {
                 family: "Arial"
                 pixelSize: Math.floor(parent.height * 1.4)
                 weight: Font.DemiBold
             }
        } 
    }

    SvgElementImage {
        id: system_mousearea
        elementName: "system-panel-mousearea"
        sceneSize: panels.sceneSize
        y: Math.floor(scaledBounds.y * sceneItem.height)
        z: system_bg.z+1

        MouseArea { 
             id: hidedisp_system; 
             anchors.fill: parent; 
             cursorShape: Qt.WhatsThisCursor 
             onClicked: hide_display_system()
        }

        states: State  {
             name: "fading"
             when: show_panels !== true
             PropertyChanges  { target: system_mousearea; x: Math.floor(scaledBounds.x * sceneItem.width) - (system_bg.width * 0.85); }
        }
 
        transitions: Transition  {
        SequentialAnimation  {
              PropertyAnimation  { property: "x"; duration: 800 }
              }
        } 
    }
}
