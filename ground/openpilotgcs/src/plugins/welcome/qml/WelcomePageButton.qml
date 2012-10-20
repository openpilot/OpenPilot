// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: welcomeButton
    width: Math.max(116, icon.width)
    height: icon.height
    z: 0

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
        property bool displayBelowButton: false
        source: displayBelowButton ? "images/button-label-bottom.png" : "images/button-label.png"
        opacity: 0
        visible: labelText.text.length > 0 //don't show label bg without text


        anchors.horizontalCenter: parent.horizontalCenter
        y: displayBelowButton ? parent.height-8 : -height+8

        Text {
            id: labelText
            anchors.baseline: parent.bottom
            //text baseline depends on label image orientation,
            //0.3 and 0.55 constants have to be adjusted when button-label.png is modified
            anchors.baselineOffset: labelImage.displayBelowButton ? -parent.height*0.3 : -parent.height*0.55
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
        //update the tooltip position before it's displayed,
        //since we don't have a property to bind directly
        StateChangeScript {
            name: "updateLabelPosition"
            script: labelImage.displayBelowButton = welcomeButton.mapToItem(null,0,0).y < labelImage.height
        }
        PropertyChanges { target: hoveredIcon; opacity: 1.0 }
        PropertyChanges { target: labelImage; opacity: 1.0 }
        //raise this button, so tooltip is not obscured by the next items in the grid
        PropertyChanges { target: welcomeButton; z: 1 }
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
