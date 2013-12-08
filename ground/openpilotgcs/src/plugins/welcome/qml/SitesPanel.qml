// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 2.0

Item {
    id: container
    width: 100
    height: 62

    signal clicked(string url)

    Text {
        id: header
        text: qsTr("OpenPilot Websites")
        width: parent.width
        color: "#44515c"
        font {
            pointSize: 14
            weight: Font.Bold
        }
    }

    ListModel {
        id: sitesModel
        ListElement { title: "Home"; link: "http://www.openpilot.org" }
        ListElement { title: "Wiki"; link: "http://wiki.openpilot.org" }
        ListElement { title: "Store"; link: "http://www.openpilot.org/hardware/get-hardware/" }
        ListElement { title: "Forums"; link: "http://forums.openpilot.org" }
        ListElement { title: "Code Reviews"; link: "http://git.openpilot.org" }
        ListElement { title: "Progress Tracker"; link: "http://progress.openpilot.org" }
    }

    ListView {
        id: view
        width: 839
        anchors.topMargin: 30
        anchors { top: parent.top; bottom: parent.bottom }
        orientation: ListView.Horizontal
        model: sitesModel
        spacing: 80

        delegate: Text {
            text: title
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font {
                pointSize: 12
                weight: Font.Bold
            }

            color: mouseArea.containsMouse ? "#224d81" : "black"

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
