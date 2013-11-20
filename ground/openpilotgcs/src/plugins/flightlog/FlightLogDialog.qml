import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.openpilot 1.0

Rectangle {
    width: 600
    height: 400

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
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
                    font.pixelSize: 12
                    text: "<b>" + qsTr("Flights recorded: ") + "</b>" + logStatus.Flight
                }
                Text {
                    id: totalEntries
                    font.pixelSize: 12
                    text: "<b>" + qsTr("Logs slots used: ") + "</b>" + logStatus.UsedSlots
                }
                Text {
                    id: freeEntries
                    font.pixelSize: 12
                    text: "<b>" + qsTr("Logs slots left: ") + "</b>" + logStatus.FreeSlots
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

        RowLayout {
            Layout.fillWidth: true
            height: 40
            Button {
                id: exportButton
                text: qsTr("Export...")
                activeFocusOnPress: true
                onClicked: logManager.exportLogs()
            }

            Rectangle {
                Layout.fillWidth: true
            }

            Button {
                id: okButton
                text: qsTr("OK")
                activeFocusOnPress: true
                onClicked: dialog.close()
            }
        }
    }
}
