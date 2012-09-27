import Qt 4.7

Item {
    id: sceneItem
    property variant sceneSize

    SvgElementImage {
        id: altitude_bg
        elementName: "altitude-bg"
        sceneSize: sceneItem.sceneSize
        clip: true

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "altitude-bg")

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        SvgElementImage {
            id: altitude_scale

            elementName: "altitude-scale"
            sceneSize: sceneItem.sceneSize

            anchors.verticalCenter: parent.verticalCenter
            // The altitude scale represents 30 meters,
            // move it in 0..5m range
            anchors.verticalCenterOffset: -height/30 * (PositionActual.Down-Math.floor(PositionActual.Down/5)*5)
            anchors.left: parent.left

            property int topNumber: 15-Math.floor(PositionActual.Down/5)*5

            // Altitude numbers
            Column {
                Repeater {
                    model: 7
                    Item {
                        height: altitude_scale.height / 6
                        width: altitude_bg.width

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
    }


    SvgElementImage {
        id: altitude_window
        clip: true

        elementName: "altitude-window"
        sceneSize: sceneItem.sceneSize

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "altitude-window")

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        Text {
            id: altitude_text
            text: Math.floor(-PositionActual.Down).toFixed()
            color: "white"
            font {
                family: "Arial"
                pixelSize: parent.height * 0.6
            }
            anchors.centerIn: parent
        }
    }
}
