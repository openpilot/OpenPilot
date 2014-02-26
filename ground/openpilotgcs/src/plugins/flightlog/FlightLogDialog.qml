import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.openpilot 1.0

import "functions.js" as Functions

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
                }
                TableView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredHeight: 1000;
                    model: logManager.logEntries

                    itemDelegate: Text {
                        anchors.fill: parent
                        anchors.margins: 2
                        anchors.leftMargin: 5
                        font.pixelSize: 12
                        text: styleData.value
                    }

                    TableViewColumn {
                        role: "Flight"; title: qsTr("Flight"); width: 50;
                        delegate:
                            Text {
                                anchors.fill: parent
                                anchors.margins: 2
                                anchors.leftMargin: 5
                                font.pixelSize: 12
                                text: styleData.value + 1
                            }

                    }
                    TableViewColumn {
                        role: "FlightTime"; title: qsTr("Time"); width: 80;
                        delegate:
                            Text {
                                anchors.fill: parent
                                anchors.margins: 2
                                anchors.leftMargin: 5
                                font.pixelSize: 12
                                text: Functions.millisToTime(styleData.value)
                            }
                    }
                    TableViewColumn {
                        role: "Type"; title: "Type"; width: 50;
                        delegate:
                            Text {
                                anchors.fill: parent
                                anchors.margins: 2
                                anchors.leftMargin: 5
                                font.pixelSize: 12
                                text: {
                                    switch(styleData.value) {
                                    case 0 : text: qsTr("Empty"); break;
                                    case 1 : text: qsTr("Text"); break;
                                    case 2 : text: qsTr("UAVO"); break;
                                    default: text: qsTr("Unknown"); break;
                                    }
                                }
                            }

                    }
                    TableViewColumn { role: "LogString"; title: qsTr("Data"); width: 280}
                }

                RowLayout {
                    anchors.margins: 10
                    spacing: 10

                    ColumnLayout {
                        spacing: 10
                        Text {
                            id: totalFlights
                            text: "<b>" + qsTr("Flights recorded: ") + "</b>" + (logStatus.Flight + 1)
                        }
                        Text {
                            id: totalEntries
                            text: "<b>" + qsTr("Entries logged (free): ") + "</b>" +
                                  logStatus.UsedSlots + " (" + logStatus.FreeSlots + ")"
                        }
                        Rectangle {
                            Layout.fillHeight: true
                        }
                        CheckBox {
                            id: exportRelativeTimeCB
                            enabled: !logManager.disableControls && !logManager.disableExport
                            text: qsTr("Adjust timestamps")
                            activeFocusOnPress: true
                            checked: logManager.adjustExportedTimestamps
                            onCheckedChanged: logManager.setAdjustExportedTimestamps(checked)
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
                                text: "<b>" + qsTr("Flight to download:") + "</b>"
                            }

                            ComboBox {
                                id: flightCombo
                                enabled: !logManager.disableControls
                                model: logManager.flightEntries
                            }
                        }
                        RowLayout {
                            spacing: 10
                            Rectangle {
                                Layout.fillWidth: true
                            }
                            Button {
                                text: qsTr("Download logs")
                                enabled: !logManager.disableControls
                                activeFocusOnPress: true
                                onClicked: logManager.retrieveLogs(flightCombo.currentIndex - 1)
                            }
                        }
                        Rectangle {
                            Layout.fillHeight: true
                        }
                        RowLayout {
                            Rectangle {
                                Layout.fillWidth: true
                            }
                            Button {
                                id: clearButton
                                enabled: !logManager.disableControls
                                text: qsTr("Clear all logs")
                                activeFocusOnPress: true
                                onClicked: logManager.clearAllLogs()
                            }
                            Button {
                                id: exportButton
                                enabled: !logManager.disableControls && !logManager.disableExport
                                text: qsTr("Export logs...")
                                activeFocusOnPress: true
                                onClicked: logManager.exportLogs()
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            height: 40
            Rectangle {
                Layout.fillWidth: true
            }
            Button {
                id: cancelButton
                enabled: logManager.disableControls
                text: qsTr("Cancel")
                activeFocusOnPress: true
                onClicked: logManager.cancelExportLogs()
            }
            Button {
                id: okButton
                enabled: !logManager.disableControls
                text: qsTr("OK")
                isDefault: true
                activeFocusOnPress: true
                onClicked: dialog.close()
            }
        }
    }
}
