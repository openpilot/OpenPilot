import Qt 4.7
import "."

Rectangle {
    color: "#666666"

    SvgElementImage {
        id: background
        elementName: "background"
        fillMode: Image.PreserveAspectFit
        anchors.fill: parent

        sceneSize: Qt.size(width, height)

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

            SvgElementImage {
                id: rollscale
                elementName: "rollscale"
                sceneSize: background.sceneSize

                smooth: true
                anchors.centerIn: parent
                //rotate it around the center of scene
                transform: Rotation {
                    angle: -AttitudeActual.Roll
                    origin.x : sceneItem.width/2 - x
                    origin.y : sceneItem.height/2 - y
                }
            }

            SvgElementImage {
                id: foreground
                elementName: "foreground"
                sceneSize: background.sceneSize

                anchors.centerIn: parent
            }

            SvgElementImage {
                id: compass
                elementName: "compass"
                sceneSize: background.sceneSize

                clip: true

                y: 12
                anchors.horizontalCenter: parent.horizontalCenter

                SvgElementImage {
                    id: compass_band
                    elementName: "compass-band"
                    sceneSize: background.sceneSize

                    anchors.centerIn: parent
                    //the band is 540 degrees wide, AttitudeActual.Yaw is converted to -180..180 range
                    anchors.horizontalCenterOffset: -1*((AttitudeActual.Yaw+180+720) % 360 - 180)/540*width
                }
            }

            SpeedScale {
                anchors.fill: parent
                sceneSize: background.sceneSize
            }

            AltitudeScale {
                anchors.fill: parent
                sceneSize: background.sourceSize
            }

            VsiScale {
                anchors.fill: parent
                sceneSize: background.sourceSize
            }

            PfdIndicators {
                anchors.fill: parent
                sceneSize: background.sourceSize
            }
        }
    }
}
