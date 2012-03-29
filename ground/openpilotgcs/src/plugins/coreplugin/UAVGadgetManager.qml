import QtQuick 1.0

Rectangle {
    anchors.fill: parent
    Row {
        id: row1
        width: parent.width
        height: parent.height
        spacing: 10
        Rectangle {
            width: 40
            height: parent.height
            color: "green"
        }
        Text {
            text: "itemName"
        }
    }
}
