import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.openpilot 1.0

Rectangle {
    width: 600
    height: 400

    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10
                height: 40
                Text {
                    id: totalFlights
                    text: "<b>" + qsTr("Flights recorded: ") + "</b>" + logStatus.Flight
                }
                Text {
                    id: totalEntries
                    text: "<b>" + qsTr("Logs recorded: ") + "</b>" + logStatus.UsedSlots
                }
                Text {
                    id: freeEntries
                    text: "<b>" + qsTr("Logs left: ") + "</b>" + logStatus.FreeSlots
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            frameVisible: true
            ListView {
                id: authorsView
                anchors.fill: parent
                spacing: 3
                model: logManager.logEntries
                delegate: Text {
                    font.pixelSize: 12
                    text: Flight
                }
                clip: true
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 40
            Button {
                id: button1
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 10
                text: qsTr("OK")
                onClicked: dialog.close()
            }
        }
    }
}
