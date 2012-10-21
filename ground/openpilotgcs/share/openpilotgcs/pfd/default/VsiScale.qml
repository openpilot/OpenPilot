import Qt 4.7

Item {
    id: sceneItem
    property variant sceneSize

    SvgElementImage {
        id: vsi_bg
        elementName: "vsi-bg"
        sceneSize: sceneItem.sceneSize
        clip: true

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        SvgElementImage {
            id: vsi_bar

            elementName: "vsi-bar"
            sceneSize: sceneItem.sceneSize

            //the scale in 1000 ft/min with height == 5200 ft/min
            height: (-VelocityActual.Down*3.28*60/1000)*vsi_scale.unitHeight


            anchors.bottom: parent.verticalCenter
            anchors.left: parent.left
        }

        SvgElementImage {
            id: vsi_scale

            elementName: "vsi-scale"
            sceneSize: sceneItem.sceneSize

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            property real unitHeight: vsi_scale.height / 5.2 //the scale height is 5200

            //Text labels
            Column {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.right

                Repeater {
                    model: [2, 1, 0, 1, 2]
                    Item {
                        height: vsi_scale.unitHeight
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

        SvgElementImage {
            id: vsi_desired

            elementName: "vsi-desired"
            border: 1
            sceneSize: sceneItem.sceneSize

            anchors.verticalCenter: parent.verticalCenter
            //the scale in 1000 ft/min with height == 5200 ft/min
            anchors.verticalCenterOffset: (VelocityDesired.Down*3.28*60/1000)*vsi_scale.unitHeight
            anchors.left: parent.left

            visible: VelocityDesired.Down != 0
        }
    }


    SvgElementImage {
        id: vsi_window
        clip: true
        smooth: true

        elementName: "vsi-window"
        sceneSize: sceneItem.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }
}
