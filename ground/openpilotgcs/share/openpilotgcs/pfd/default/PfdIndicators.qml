import Qt 4.7

Item {
    id: sceneItem
    property variant sceneSize

    //telemetry status arrow
    SvgElementImage {
        id: telemetry_status
        elementName: "gcstelemetry-"+statusName
        sceneSize: sceneItem.sceneSize

        property string statusName : ["Disconnected","HandshakeReq","HandshakeAck","Connected"][GCSTelemetryStats.Status]

        scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "gcstelemetry-Disconnected")
        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }

    //telemetry rate text
    Text {
        id: telemetry_rate
        text: GCSTelemetryStats.TxDataRate.toFixed()+"/"+GCSTelemetryStats.RxDataRate.toFixed()
        color: "white"
        font.family: "Arial"
        font.pixelSize: telemetry_status.height * 0.75

        anchors.top: telemetry_status.bottom
        anchors.horizontalCenter: telemetry_status.horizontalCenter
    }

    Text {
        id: gps_text
        text: "GPS: " + GPSPosition.Satellites + "\nPDP: " + GPSPosition.PDOP
        color: "white"
        font.family: "Arial"
        font.pixelSize: telemetry_status.height * 0.75

        visible: GPSPosition.Satellites > 0

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "gps-txt")
        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }

    Text {
        id: battery_text

        text: FlightBatteryState.Voltage.toFixed(2)+" V\n" +
              FlightBatteryState.Current.toFixed(2)+" A\n" +
              FlightBatteryState.ConsumedEnergy.toFixed()+" mAh"


        color: "white"
        font.family: "Arial"

        //I think it should be pixel size,
        //but making it more consistent with C++ version instead
        font.pointSize: scaledBounds.height * sceneItem.height

        visible: FlightBatteryState.Voltage > 0 || FlightBatteryState.Current > 0

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "battery-txt")
        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }
}
