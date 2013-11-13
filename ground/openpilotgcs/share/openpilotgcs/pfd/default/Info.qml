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
}
