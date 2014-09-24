import QtQuick 2.0

Item {
    id: sceneItem
    property variant sceneSize

    property real altitude : -qmlWidget.altitudeFactor * PositionState.Down

    SvgElementImage {
        id: altitude_window
        elementName: "altitude-window"
        sceneSize: sceneItem.sceneSize
        clip: true

        visible: qmlWidget.altitudeUnit != 0

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "altitude-window")

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        SvgElementImage {
            id: altitude_scale

            elementName: "altitude-scale"
            sceneSize: sceneItem.sceneSize

            anchors.verticalCenter: parent.verticalCenter
            // The altitude scale represents 10 units (ft or meters),
            // move using decimal term from value to display
            anchors.verticalCenterOffset: height/10 * (altitude - Math.floor(altitude))
            anchors.left: parent.left

            property int topNumber: Math.floor(altitude)+5

            // Altitude numbers
            Column {
                Repeater {
                    model: 10
                    Item {
                        height: altitude_scale.height / 10
                        width: altitude_window.width

                        Text {
                            text: altitude_scale.topNumber - index
                            color: "white"
                            font.pixelSize: parent.height / 3
                            font.family: "Arial"

                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.top
                        }
                    }
                }
            }
        }

        SvgElementImage {
            id: altitude_vector
            elementName: "altitude-vector"
            sceneSize: sceneItem.sceneSize

            height: -NedAccel.Down * altitude_scale.height/10

            anchors.left: parent.left
            anchors.bottom: parent.verticalCenter
        }

        SvgElementImage {
            id: altitude_waypoint
            elementName: "altitude-waypoint"
            sceneSize: sceneItem.sceneSize
            visible: PathDesired.End_Down !== 0.0

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            anchors.verticalCenterOffset: -altitude_scale.height/10 * (PositionState.Down - PathDesired.End_Down) * qmlWidget.altitudeFactor
        }
    }

    SvgElementImage {
        id: altitude_box
        clip: true

        visible: qmlWidget.altitudeUnit != 0

        elementName: "altitude-box"
        sceneSize: sceneItem.sceneSize

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "altitude-box")

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        Text {
            id: altitude_text
            text: "  " +altitude.toFixed(1)
            color: "white"
            font {
                family: "Arial"
                pixelSize: parent.height * 0.35
                weight: Font.DemiBold
            }
            anchors.centerIn: parent
        }
    }

    SvgElementImage {
        id: altitude_unit_box
        elementName: "altitude-unit-box"
        sceneSize: sceneItem.sceneSize

        visible: qmlWidget.altitudeUnit != 0

        anchors.top: altitude_window.bottom
        anchors.right: altitude_window.right
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        Text {
            id: altitude_unit_text
            text: qmlWidget.altitudeUnit
            color: "cyan"
            font {
                family: "Arial"
                pixelSize: parent.height * 0.6
                weight: Font.DemiBold
            }
            anchors.centerIn: parent
        }
    }
}
