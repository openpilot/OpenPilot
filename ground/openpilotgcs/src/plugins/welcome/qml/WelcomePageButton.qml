// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: welcomeButton
    width: 116
    height: 116

    property string baseIconName
    property alias label : labelText.text

    signal clicked

    Image {
        id: icon
        source: "images/"+baseIconName+"-off.png"
        anchors.centerIn: parent
    }

    Image {
        id: hoveredIcon
        source: "images/"+baseIconName+"-on.png"
        anchors.centerIn: parent
        opacity: 0
    }

    Image {
        id: labelImage
        source: "images/button-label.png"
        opacity: 0

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: hoveredIcon.top
        anchors.bottomMargin: -8

        Text {
            id: labelText
            anchors.baseline: parent.verticalCenter
            anchors.baselineOffset: -4
            anchors.horizontalCenter: parent.horizontalCenter

            font {
                weight: Font.DemiBold
                pointSize: 14
            }
            color: "#272727"
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
