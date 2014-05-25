import QtQuick 2.0

Item {
    id: info
    property variant sceneSize

    SvgElementImage {
        id: info_bg
        elementName: "info-bg"
        sceneSize: info.sceneSize
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
            font.pixelSize: parent.height*1.2
            color: "white"
        }
    }

    SvgElementPositionItem {
        sceneSize: info.sceneSize
        elementName: "telemetry-status"

        Text {
            text: ["Disconnected","HandshakeReq","HandshakeAck","Connected"][GCSTelemetryStats.Status]

            anchors.centerIn: parent
            font.pixelSize: parent.height*1.2
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
            visible: txNumberBar.txRateNumber >= minTxRateNumber
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
            visible: rxNumberBar.rxRateNumber >= minRxRateNumber
        }
    }
    Item {
        id: battery_voltage

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "battery-volt-text")

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        //visible: FlightBatteryState.Voltage > 0

        Text {
            id: battery_volt
            text:  FlightBatteryState.Voltage.toFixed(2)
            color: "white"
            font {
                family: "Arial"
                pixelSize: parent.height * 1.2
            }
            anchors.centerIn: parent
        }
    }

   Item {
        id: battery_current

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "battery-amp-text")

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        //visible: FlightBatteryState.Current > 0

        Text {
            id: battery_cur
            text:  FlightBatteryState.Current.toFixed(2)
            color: "white"
            font {
                family: "Arial"
                pixelSize: parent.height * 1.2
            }
            anchors.centerIn: parent
        }
    }

   Item {
        id: battery_consumed_energy

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "battery-milliamp-text")

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
        //visible: FlightBatteryState.ConsumedEnergy > 0

        Text {
            id: battery_conso
            text:  FlightBatteryState.ConsumedEnergy.toFixed()
            color: "white"
            font {
                family: "Arial"
                pixelSize: parent.height * 1.2
            }
            anchors.centerIn: parent
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
}
