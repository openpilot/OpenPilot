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

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.width: 1
            radius: 4
            ColumnLayout {
                anchors.margins: 10
                anchors.fill: parent
                Text {
                    Layout.fillWidth: true
                    text: "<b>" + qsTr("Log entries") + "</b>"
                    font.pixelSize: 12
                }
                TableView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: logManager.logEntries
                    TableViewColumn { role: "Flight"; title: "Flight"; width: 50; horizontalAlignment: Text.AlignRight}
                    TableViewColumn { role: "FlightTime"; title: "Time";width: 50; horizontalAlignment: Text.AlignRight}
                    TableViewColumn { role: "Entry"; title: "#"; width: 50; horizontalAlignment: Text.AlignRight}
                    TableViewColumn { role: "Type"; title: "Contents"; width: 100}
                }

                RowLayout {
                    anchors.margins: 10
                    spacing: 10
                    ColumnLayout {
                        spacing: 10
                        Text {
                            id: totalFlights
                            font.pixelSize: 12
                            text: "<b>" + qsTr("Flights recorded: ") + "</b>" + (logStatus.Flight + 1)
                        }
                        Text {
                            id: totalEntries
                            font.pixelSize: 12
                            text: "<b>" + qsTr("Entries logged (free): ") + "</b>" +
                                  logStatus.UsedSlots + " (" + logStatus.FreeSlots + ")"
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                    }
                    ColumnLayout {
                        spacing: 10
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
