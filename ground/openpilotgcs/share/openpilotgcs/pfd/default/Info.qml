import Qt 4.7

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

    Text {
        text: ["No GPS", "No Fix", "Fix2D", "Fix3D"][GPSPositionSensor.Status]

        // TODO: get coords from svg file, as soon as "gps-mode-text" text is converted to path
        x: info.sceneSize.width * 0.05
        y: info.sceneSize.height * 0.006

        font.pixelSize: info.sceneSize.height * 0.02
        color: "white"
    }
}
