// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: container
    width: 100
    height: 62

    signal clicked(string url)

    Text {
        id: header
        text: "Project News"
        width: parent.width
        color: "#44515c"
        font {
            pointSize: 14
            weight: Font.Bold
        }
    }

    ListView {
        id: view
        width: parent.width
        anchors { top: header.bottom; topMargin: 14; bottom: parent.bottom }
        model: xmlModel
        delegate: listDelegate
        clip: true
    }

    ScrollDecorator {
        flickableItem: view
    }

    XmlListModel {
        id: xmlModel
        source: "http://www.openpilot.org/feed/"
        query: "/rss/channel/item"

        XmlRole { name: "title"; query: "title/string()" }
        XmlRole { name: "description"; query: "description/string()" }
        XmlRole { name: "link"; query: "link/string()" }
    }

    Component {
        id: listDelegate
        Item {
            width: view.width
            height: column.height + 16

            Column {
                id: column
                spacing: 4
                Text {
                    text: title
                    width: view.width
                    textFormat: text.indexOf("&") > 0 ? Text.StyledText : Text.PlainText
                    elide: Text.ElideRight
                    font.bold: true
                    color: mouseArea.containsMouse ? "#224d81" : "black"
                }

                Text {
                    text: description
                    width: view.width
                    textFormat: text.indexOf("&") > 0 ? Text.StyledText : Text.PlainText
                    elide: Text.ElideRight
                    color: mouseArea.containsMouse ? "#224d81" : "black"
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    console.log(link)
                    container.clicked(link)
                }
            }
        }
    }
}
