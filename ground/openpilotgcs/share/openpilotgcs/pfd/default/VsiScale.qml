import Qt 4.7

Item {
    id: sceneItem
    property variant sourceSize

    Image {
        id: vsi_bg
        source: "image://svg/pfd.svg!vsi-bg"
        sourceSize: sceneItem.sourceSize
        clip: true

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "vsi-bg")

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height

        Image {
            id: vsi_bar

            source: "image://svg/pfd.svg!vsi-bar"
            sourceSize: sceneItem.sourceSize

            //the scale in 1000 ft/min with height == 5200 ft/min
            height: (-VelocityActual.Down*3.28*60/1000)*(vsi_scale.height/5.2)


            anchors.bottom: parent.verticalCenter
            anchors.left: parent.left
        }

        Image {
            id: vsi_scale

            source: "image://svg/pfd.svg!vsi-scale"
            sourceSize: sceneItem.sourceSize

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left

            //Text labels
            Column {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.right

                Repeater {
                    model: [2, 1, 0, 1, 2]
                    Item {
                        height: vsi_scale.height / 5.2 //the scale height is 5200
                        width: vsi_bg.width - vsi_scale.width //fill area right to scale

                        Text {
                            text: modelData
                            visible: modelData !== 0 //hide "0" label
                            color: "white"
                            font.pixelSize: parent.height / 4
                            font.family: "Arial"

                            anchors.centerIn: parent
                        }
                    }
                }
            }
        }
    }


    Image {
        id: vsi_window
        clip: true
        smooth: true

        source: "image://svg/pfd.svg!vsi-window"
        sourceSize: sceneItem.sourceSize

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "vsi-window")

        x: scaledBounds.x * sceneItem.width
        y: scaledBounds.y * sceneItem.height
        width: scaledBounds.width * sceneItem.width
        height: scaledBounds.height * sceneItem.height
    }
}
