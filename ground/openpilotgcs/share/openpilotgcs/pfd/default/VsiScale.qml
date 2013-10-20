import Qt 4.7

Item {
    id: sceneItem
    property variant sceneSize

    SvgElementImage {
        id: vsi_window
        elementName: "vsi-window"
        sceneSize: sceneItem.sceneSize
        clip: true

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)

        property double scaleSteps : 8
        property double scaleStepValue : 1000
        property double scaleStepHeight : height/scaleSteps

        SvgElementImage {
            id: vsi_bar

            elementName: "vsi-bar"
            sceneSize: sceneItem.sceneSize

            //the scale in 1000 ft/min, convert from VelocityState.Down value in m/s
            height: (-VelocityState.Down*3.28*60/vsi_window.scaleStepValue)*vsi_window.scaleStepHeight

            anchors.bottom: parent.verticalCenter
            anchors.left: parent.left
        }

        SvgElementImage {
            id: vsi_scale

            elementName: "vsi-scale"
            sceneSize: sceneItem.sceneSize

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left

            //Text labels
            Column {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.right

                Repeater {
                    model: [3, 2, 1, 0, 1, 2, 3]
                    Item {
                        height: vsi_window.scaleStepHeight
                        width: vsi_window.width - vsi_scale.width //fill area right to scale

                        Text {
                            text: modelData
                            visible: modelData !== 0 //hide "0" label
                            color: "white"
                            font.pixelSize: parent.height * 0.5
                            font.family: "Arial"

                            anchors.centerIn: parent
                        }
                    }
                }
            }
        }
    }


    SvgElementImage {
        id: vsi_centerline
        clip: true
        smooth: true

        elementName: "vsi-centerline"
        sceneSize: sceneItem.sceneSize

        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }

    Text {
        id: vsi_unit_text
        text: "ft / m"

        color: "white"
        font {
            family: "Arial"
            pixelSize: sceneSize.height * 0.02
        }
        anchors.top: vsi_window.bottom
        anchors.left: vsi_window.left
        anchors.margins: font.pixelSize * 0.5
    }
}
