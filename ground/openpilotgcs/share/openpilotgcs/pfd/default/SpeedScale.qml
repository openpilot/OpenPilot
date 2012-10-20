import Qt 4.7

Item {
    id: sceneItem
    property variant sceneSize
    property real groundSpeed : 3.6 * Math.sqrt(Math.pow(VelocityActual.North,2)+
                                                Math.pow(VelocityActual.East,2))

    SvgElementImage {
        id: speed_bg
        elementName: "speed-bg"
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
                width: speed_bg.width
                anchors.right: speed_scale.right

                Repeater {
                    model: 7
                    Item {
                        height: speed_scale.height / 6
                        width: speed_bg.width

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
    }


    SvgElementImage {
        id: speed_window
        clip: true

        elementName: "speed-window"
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
                pixelSize: parent.height * 0.6
            }
            anchors.centerIn: parent
        }
    }
}
