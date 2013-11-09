import Qt 4.7

Item {
    id: sceneItem
    property variant sceneSize

    SvgElementImage {
        id: altitude_window
        elementName: "altitude-window"
        sceneSize: sceneItem.sceneSize
        clip: true

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "altitude-window")

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        SvgElementImage {
            id: altitude_scale

            elementName: "altitude-scale"
            sceneSize: sceneItem.sceneSize

            anchors.verticalCenter: parent.verticalCenter
            // The altitude scale represents 30 meters,
            // move it in 0..5m range
            anchors.verticalCenterOffset: -height/30 * (PositionState.Down-Math.floor(PositionState.Down/5*qmlWidget.altitudeFactor)*5)
            anchors.left: parent.left

            property int topNumber: 15-Math.floor(PositionState.Down/5*qmlWidget.altitudeFactor)*5

            // Altitude numbers
            Column {
                Repeater {
                    model: 7
                    Item {
                        height: altitude_scale.height / 6
                        width: altitude_window.width

                        Text {
                            text: altitude_scale.topNumber - index*5
                            color: "white"
                            font.pixelSize: parent.height / 4
                            font.family: "Arial"

                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.top
                        }
                    }
                }
            }
        }

        SvgElementImage {
            id: altitude_waypoint
            elementName: "altitude-waypoint"
            sceneSize: sceneItem.sceneSize
            visible: PathDesired.End_Down !== 0.0

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            anchors.verticalCenterOffset: -altitude_scale.height/30 * (PositionState.Down - PathDesired.End_Down)
        }
    }

    SvgElementImage {
        id: altitude_box
        clip: true

        elementName: "altitude-box"
        sceneSize: sceneItem.sceneSize

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "altitude-box")

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        Text {
            id: altitude_text
            text: Math.floor(-PositionState.Down * qmlWidget.altitudeFactor).toFixed()
            color: "white"
            font {
                family: "Arial"
                pixelSize: parent.height * 0.4
            }
            anchors.centerIn: parent
        }
    }

    Text {
        id: altitude_unit_text
        text: qmlWidget.altitudeUnit
        color: "white"
        font {
            family: "Arial"
            pixelSize: sceneSize.height * 0.025
        }
        anchors.top: altitude_window.bottom
        anchors.right: altitude_window.right
        anchors.margins: font.pixelSize * 0.3
    }
}
