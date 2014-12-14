import QtQuick 2.0

Rectangle {
    color: "#666666"

    SvgElementImage {
        id: background
        elementName: "pfd-window"
        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
        sceneSize: Qt.size(width, height)
        
        Rectangle {
            width: Math.floor(parent.paintedHeight * 1.319)
            height: Math.floor(parent.paintedHeight - parent.paintedHeight * 0.008)
            
            color: "transparent"
            border.color: "white"
            border.width: Math.floor(parent.paintedHeight * 0.008)
            radius: Math.floor(parent.paintedHeight * 0.01)
            anchors.centerIn: parent             
        }

        Item {
            id: sceneItem

            FontLoader {
                id: pt_bold_narrow
                source: "qrc:/pfdqml/fonts/PTN77F.ttf"
            }

            FontLoader {
                id: pt_bold
                source: "qrc:/pfdqml/fonts/PTS75F.ttf"
            }

            width: Math.floor((parent.paintedHeight * 1.32) - (parent.paintedHeight * 0.013))
            height: Math.floor(parent.paintedHeight - parent.paintedHeight * 0.02)
            property variant viewportSize : Qt.size(width, height)

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
                id: side_slip_fixed
                elementName: "sideslip-fixed"
                sceneSize: sceneItem.viewportSize

                x: scaledBounds.x * sceneItem.width
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
                visible: qmlWidget.altitudeUnit != 0
            }

            Info {
                anchors.fill: parent
                sceneSize: sceneItem.viewportSize
            }

            Panels {
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
