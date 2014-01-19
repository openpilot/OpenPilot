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

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        SvgElementImage {
            id: speed_scale

            elementName: "speed-scale"
            sceneSize: sceneItem.sceneSize

            anchors.verticalCenter: parent.verticalCenter
            // The speed scale represents 30 meters,
            // move it in 0..5m range
            anchors.verticalCenterOffset: height/30 * (sceneItem.groundSpeed-Math.floor(sceneItem.groundSpeed/5)*5)
            anchors.right: parent.right

            property int topNumber: Math.floor(sceneItem.groundSpeed/5)*5+15

            // speed numbers
            Column {
                width: speed_window.width
                anchors.right: speed_scale.right

                Repeater {
                    model: 7
                    Item {
                        height: speed_scale.height / 6
                        width: speed_window.width

                        Text {
                            //don't show negative numbers
                            text: speed_scale.topNumber - index*5
                            color: "white"
                            visible: speed_scale.topNumber - index*5 >= 0

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
            id: speed_waypoint
            elementName: "speed-waypoint"
            sceneSize: sceneItem.sceneSize
            visible: PathDesired.EndingVelocity !== 0.0

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            anchors.verticalCenterOffset: speed_scale.height/30 * (sceneItem.groundSpeed - PathDesired.EndingVelocity)
        }
    }

    SvgElementImage {
        id: speed_box
        clip: true

        elementName: "speed-box"
        sceneSize: sceneItem.sceneSize

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        Text {
            id: speed_text
            text: Math.round(sceneItem.groundSpeed).toFixed()
            color: "white"
            font {
                family: "Arial"
                pixelSize: parent.height * 0.4
            }
            anchors.centerIn: parent
        }
    }

    Text {
        id: speed_unit_text
        text: qmlWidget.speedUnit
        color: "white"
        font {
            family: "Arial"
            pixelSize: sceneSize.height * 0.025
        }
        anchors.top: speed_window.bottom
        anchors.right: speed_window.right
        anchors.margins: font.pixelSize * 0.3
    }
}
