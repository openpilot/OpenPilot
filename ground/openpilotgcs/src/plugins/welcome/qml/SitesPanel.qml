// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: container
    width: 100
    height: 62

    signal clicked(string url)

    Text {
        id: header
        text: "OpenPilot Websites"
        width: parent.width
        color: "#303060"
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font {
            pointSize: { if(parent.height < 150) 7; else 13 }
            weight: Font.Bold
        }
    }

    ListModel {
        id: sitesModel
        ListElement { title: "OpenPilot Home"; link: "http://www.openpilot.org" }
        ListElement { title: "OpenPilot Wiki"; link: "http://wiki.openpilot.org" }
        ListElement { title: "OpenPilot Store"; link: "http://www.openpilot.org/hardware/get-hardware/" }
        ListElement { title: "OpenPilot Forums"; link: "http://forums.openpilot.org" }
        ListElement { title: "OpenPilot Code Reviews"; link: "http://git.openpilot.org" }
        ListElement { title: "OpenPilot Progress Tracker"; link: "http://progress.openpilot.org" }
    }

    ListView {
        id: view
        width: parent.width
        anchors { top: header.bottom; topMargin: 14; bottom: parent.bottom }
        model: sitesModel
        spacing: { if(parent.height < 150) 4; else 8 }
        clip: true

        delegate: Text {
            text: title
            width: view.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            font {
                pointSize: header.font.pointSize
                weight: Font.Bold
            }

            color: mouseArea.containsMouse ? "darkblue" : "black"

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
