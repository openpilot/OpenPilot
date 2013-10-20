import Qt 4.7
import "."

Rectangle {
    color: "#666666"

    SvgElementImage {
        id: background
        elementName: "pfd-window"
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

            HorizontCenter {
                id: horizontCenterItem
                sceneSize: background.sceneSize
                anchors.fill: parent
            }

            RollScale {
                id: rollscale
                sceneSize: background.sceneSize
                horizontCenter: horizontCenterItem.horizontCenter
                anchors.fill: parent
            }

            SvgElementImage {
                id: foreground
                elementName: "foreground"
                sceneSize: background.sceneSize

                anchors.centerIn: parent
            }

            SvgElementImage {
                id: side_slip
                elementName: "sideslip"
                sceneSize: background.sceneSize
                smooth: true

                property real sideSlip: AccelState.y
                //smooth side slip changes, a low pass filter replacement
                //accels are updated once per second
                Behavior on sideSlip {
                    SmoothedAnimation {
                        duration: 1000
                        velocity: -1
                    }
                }

                anchors.horizontalCenter: foreground.horizontalCenter
                //0.5 coefficient is empirical to limit indicator movement
                anchors.horizontalCenterOffset: -sideSlip*width*0.5
                y: scaledBounds.y * sceneItem.height
            }

            Compass {
                anchors.fill: parent
                sceneSize: background.sceneSize
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
