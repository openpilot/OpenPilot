import QtQuick 2.1
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
            spacing: 10
            ColumnLayout {
                spacing: 10
                height: 40
                Text {
                    id: totalFlights
                    font.pixelSize: 12
                    text: "<b>" + qsTr("Flights recorded: ") + "</b>" + (logStatus.Flight + 1)
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
            Rectangle {
                Layout.fillWidth: true
            }
            ColumnLayout {
                spacing: 10
                height: 40
                RowLayout {
                    Rectangle {
                        Layout.fillWidth: true
                    }
                    Text {
                        font.pixelSize: 12
                        text: "<b>" + qsTr("Flight to download:") + "</b>"
                    }

                    ComboBox {
                        id: flightCombo
                        property ListModel dataModel: ListModel {}
                        model: dataModel
                        Component.onCompleted: {
                            dataModel.append({"value": "All"})
                            for (var a = 0; a <= logStatus.Flight ; a++) {
                                dataModel.append({"value": (a + 1).toString()})
                            }
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Rectangle {
                        Layout.fillWidth: true
                    }
                    Button {
                        text: qsTr("Download logs")
                        activeFocusOnPress: true
                        onClicked: logManager.retrieveLogs(flightCombo.currentIndex - 1)
                    }
                }
            }
        }

        Rectangle {
            border.width: 1
            height: 2
            width: parent.width
            anchors.margins: 20
            anchors.horizontalCenter: parent.horizontalCenter
            border.color: "#adadad"
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
