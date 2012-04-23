// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: welcomeButton
//    width: 116
//    height: 116

    property string baseIconName
    property alias label : labelText.text

    signal clicked
    width: 64
    height: 64

    Image {
        id: icon
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        source: "images/"+baseIconName+".png"
    }

    Image {
        id: hoveredIcon
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        source: "images/"+baseIconName+"-hover.png"
        opacity: 0
    }

    Image {
        id: labelImage
        y: -parent.height / 3
        width: parent.width * 1.5
        height: parent.height * 0.75
        z: 1
        source: "images/button-label.png"
        opacity: 0

        anchors.horizontalCenter: parent.horizontalCenter

        Text {
            id: labelText
            anchors.baseline: parent.verticalCenter
            anchors.baselineOffset: -4
            anchors.horizontalCenter: parent.horizontalCenter

            font {
                weight: Font.DemiBold
            }
            color: "#272727"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: parent.height * 0.25
        }
    }

    states: State {
        name: "hovered"
        PropertyChanges { target: hoveredIcon; opacity: 1.0 }
        PropertyChanges { target: labelImage; opacity: 1.0 }
    }

    transitions: Transition {
        from: ""; to: "hovered"; reversible: true
        NumberAnimation { targets: [hoveredIcon, labelImage]; property: "opacity"; duration: 150; easing.type: Easing.InOutQuad }
    }

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        anchors.fill: parent

        onEntered: welcomeButton.state = "hovered"
        onExited: welcomeButton.state = ""
        onClicked: welcomeButton.clicked()
    }
}
