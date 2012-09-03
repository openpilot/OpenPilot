import Qt 4.7
import "."

Rectangle {
    color: "#666666"

    Image {
        id: background
        source: "image://svg/pfd.svg!background"

        fillMode: Image.PreserveAspectFit
        anchors.fill: parent

        sourceSize.width: width
        sourceSize.height: height

        Item {
            id: sceneItem
            width: parent.paintedWidth
            height: parent.paintedHeight
            anchors.centerIn: parent
            clip: true

            Loader {
                id: worldLoader
                anchors.fill: parent
                source: qmlWidget.terrainEnabled ? "PfdTerrainView.qml" : "PfdWorldView.qml"
            }

            Image {
                id: rollscale
                source: "image://svg/pfd.svg!rollscale"
                sourceSize: background.sourceSize
                smooth: true
                anchors.centerIn: parent
                //rotate it around the center of scene
                transform: Rotation {
                    angle: -AttitudeActual.Roll
                    origin.x : sceneItem.width/2 - x
                    origin.y : sceneItem.height/2 - y
                }
            }

            Image {
                id: foreground
                source: "image://svg/pfd.svg!foreground"
                sourceSize: background.sourceSize
                anchors.centerIn: parent
            }

            Image {
                id: compass
                source: "image://svg/pfd.svg!compass"
                sourceSize: background.sourceSize
                clip: true

                y: 12
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    id: compass_band
                    source: "image://svg/pfd.svg!compass-band"
                    sourceSize: background.sourceSize

                    anchors.centerIn: parent
                    //the band is 540 degrees wide, AttitudeActual.Yaw is converted to -180..180 range
                    anchors.horizontalCenterOffset: -1*((AttitudeActual.Yaw+180+720) % 360 - 180)/540*width
                }
            }

            SpeedScale {
                anchors.fill: parent
                sourceSize: background.sourceSize
            }

            AltitudeScale {
                anchors.fill: parent
                sourceSize: background.sourceSize
            }

            VsiScale {
                anchors.fill: parent
                sourceSize: background.sourceSize
            }

            PfdIndicators {
                anchors.fill: parent
                sourceSize: background.sourceSize
            }
        }
    }
}
