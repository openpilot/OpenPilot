import QtQuick 1.0

Rectangle {
//    property alias items: modeList.model
    property alias currentIndex: modeList.currentIndex
    signal indexChanged(int index)

    id: hostRect
    anchors.fill: parent
//    width: 200
//    height: 200

    Column{
        id: column1
//        width: parent.width
//        height: parent.height
        anchors.fill: parent
        Rectangle{
            id: stackHeader
            width: parent.width
            height: headerText.height
            color: "yellow"
            border.color: "#000000"
            Text {
                id: headerText
                font.pointSize: 8
                text: qsTr("ModeStack Header: ") + modeList.count + " items"
            }
        }

        ListView {
            id: modeList
            width: parent.width
            height: parent.height - stackHeader.height
            flickableDirection: Flickable.AutoFlickDirection
            orientation: ListView.Horizontal
            snapMode: ListView.SnapOneItem
            clip: true
            delegate: Item {
                width: modeList.width
                height: modeList.height
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
                    Loader{
                        id: pageLoader
                        width: parent.width
                        height: parent.height
                        source: itemQml
                    }
//                    Text {
//                        anchors.verticalCenter: parent.verticalCenter
//                        text: qsTr(itemName)
//                    }
                }
            }
//            model: testModel
            model: items

            onCurrentIndexChanged: hostRect.indexChanged(currentIndex)
        }
    }
    ListModel{
        id: testModel
        ListElement{
            itemName: "Dave"
            itemQml: ""
        }
        ListElement{
            itemName: "Bob"
            itemQml: ""
        }
        ListElement{
            itemName: "Mo"
            itemQml: ""
        }
    }
}
