import QtQuick 2.0

Rectangle{
    property alias text : textLabel.text
    property bool active
    property bool done
    height: 25
    color : "transparent"
    Rectangle{
        id: line
        x: parent.height / 2 -1
        y: 0
        width: 2
        height: parent.height
        color: "#ff0000"
    }
    Rectangle{
        x: 3
        y: 3
        id: circle
        width: parent.height - (y * 2)
        height: width
        color: parent.active ? "red" : ( parent.done ? "green" : "grey")
        border.color: "transparent"
        border.width: 0
        radius: parent.active ? 0 : width * 0.5
        rotation: 45
    }
    Text{
        id: textLabel
        x: parent.height + 4
        y: 0 //parent.height
    }
}
