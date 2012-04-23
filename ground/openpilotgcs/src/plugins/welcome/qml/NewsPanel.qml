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
        font.pointSize: { if(parent.height < 150) 7; else 13 }

        width: parent.width
        height: font.pixelSize * 1.1
        color: "#303060"
        font {
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
            height: header.font.pixelSize * 2.5

            Column {
                id: column
                spacing: header.font.pixelSize * 0.1
                Text {
                    text: title
                    font.pixelSize: header.font.pixelSize
                    width: view.width
                    elide: Text.ElideRight
                    font.bold: true
                    color: mouseArea.containsMouse ? "darkblue" : "black"
                }

                Text {
                    text: description
                    font.pointSize: header.font.pixelSize * 0.7
                    width: view.width
                    elide: Text.ElideRight
                    color: mouseArea.containsMouse ? "darkblue" : "black"
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
