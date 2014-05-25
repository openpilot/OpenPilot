import QtQuick 2.0

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
            property variant viewportSize : Qt.size(width, height)

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
                sceneSize: sceneItem.viewportSize
                anchors.fill: parent
            }

            RollScale {
                id: rollscale
                sceneSize: sceneItem.viewportSize
                horizontCenter: horizontCenterItem.horizontCenter
                anchors.fill: parent
            }

            SvgElementImage {
                id: foreground
                elementName: "foreground"
                sceneSize: sceneItem.viewportSize

                anchors.centerIn: parent
            }

            SvgElementImage {
                id: side_slip
                elementName: "sideslip"
                sceneSize: sceneItem.viewportSize
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
                sceneSize: sceneItem.viewportSize
            }

            SpeedScale {
                anchors.fill: parent
                sceneSize: sceneItem.viewportSize
            }

            AltitudeScale {
                anchors.fill: parent
                sceneSize: sceneItem.viewportSize
            }

            VsiScale {
                anchors.fill: parent
                sceneSize: sceneItem.viewportSize
            }

//            PfdIndicators {
//                anchors.fill: parent
//                sceneSize: sceneItem.viewportSize
//            }

            Info {
                anchors.fill: parent
                sceneSize: sceneItem.viewportSize
            }

            Warnings {
                anchors.fill: parent
                sceneSize: sceneItem.viewportSize
            }
        }
    }
}
