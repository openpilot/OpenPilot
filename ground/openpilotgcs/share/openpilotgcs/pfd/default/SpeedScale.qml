import QtQuick 2.0

Item {
    id: sceneItem
    property variant sceneSize
    property real groundSpeed : qmlWidget.speedFactor * Math.sqrt(Math.pow(VelocityState.North,2)+
                                                Math.pow(VelocityState.East,2))

    SvgElementImage {
        id: speed_window
        elementName: "speed-window"
        sceneSize: sceneItem.sceneSize
        clip: true

        visible: qmlWidget.speedUnit != 0

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        SvgElementImage {
            id: speed_scale

            elementName: "speed-scale"
            sceneSize: sceneItem.sceneSize

            anchors.verticalCenter: parent.verticalCenter
            // The speed scale represents 10 units,
            // move using decimal term from value to display
            anchors.verticalCenterOffset: height/10 * (sceneItem.groundSpeed-Math.floor(sceneItem.groundSpeed))
            anchors.right: parent.right

            property int topNumber: Math.floor(sceneItem.groundSpeed)+5

            // speed numbers
            Column {
                width: speed_window.width
                anchors.right: speed_scale.right

                Repeater {
                    model: 10
                    Item {
                        height: speed_scale.height / 10
                        width: speed_window.width

                        Text {
                            //don't show negative numbers
                            text: speed_scale.topNumber - index
                            color: "white"
                            visible: speed_scale.topNumber - index >= 0

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
            id: speed_waypoint
            elementName: "speed-waypoint"
            sceneSize: sceneItem.sceneSize
            visible: PathDesired.EndingVelocity !== 0.0

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            anchors.verticalCenterOffset: speed_scale.height/10 * (sceneItem.groundSpeed - (PathDesired.EndingVelocity * qmlWidget.speedFactor))
        }
    }

    SvgElementImage {
        id: speed_box
        clip: true
        elementName: "speed-box"
        sceneSize: sceneItem.sceneSize

        visible: qmlWidget.speedUnit != 0

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        Text {
            id: speed_text
            text: sceneItem.groundSpeed.toFixed(1)+"  "
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
        id: speed_unit_box
        elementName: "speed-unit-box"
        sceneSize: sceneItem.sceneSize

        visible: qmlWidget.speedUnit != 0

        anchors.top: speed_window.bottom
        anchors.right: speed_window.right
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        Text {
           id: speed_unit_text
           text: qmlWidget.speedUnit
           color: "white"
           font {
                family: "Arial"
                pixelSize: parent.height * 0.6
                weight: Font.DemiBold
            }
            anchors.centerIn: parent
        }
    }
}
