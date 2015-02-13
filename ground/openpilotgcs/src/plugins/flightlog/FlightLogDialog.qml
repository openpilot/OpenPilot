import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.openpilot 1.0

import "functions.js" as Functions

Rectangle {
    width: 700
    height: 400
    id: root
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
                id: exportTab
                anchors.margins: 10
                anchors.fill: parent
                visible: true
                Text {
                    Layout.fillWidth: true
                    text: "<b>" + qsTr("Log entries") + "</b>"
                }
                TableView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredHeight: 1000;
                    model: logManager.logEntries
                    enabled: !logManager.disableControls && logManager.boardConnected

                    rowDelegate: Rectangle {
                        height: 22
                        color: styleData.selected ? "#ccc" : (styleData.alternate ? "#fff" : "#eee")
                    }

                    itemDelegate: Text {
                        verticalAlignment: Text.AlignVCenter
                        text: styleData.value
                    }

                    TableViewColumn {
                        role: "Flight"; title: qsTr("Flight"); width: 50;
                        delegate:
                            Text {
                                verticalAlignment: Text.AlignVCenter
                                text: styleData.value + 1
                            }

                    }
                    TableViewColumn {
                        role: "FlightTime"; title: qsTr("Time"); width: 100;
                        delegate:
                            Text {
                                verticalAlignment: Text.AlignVCenter
                                text: Functions.microsToTime(styleData.value)
                            }
                    }
                    TableViewColumn {
                        role: "Type"; title: "Type"; width: 50;
                        delegate:
                            Text {
                                verticalAlignment: Text.AlignVCenter
                                text: {
                                    switch(styleData.value) {
                                    case 0 : text: qsTr("Empty"); break;
                                    case 1 : text: qsTr("Text"); break;
                                    case 2 : text: qsTr("UAVO"); break;
                                    case 3 : text: qsTr("UAVO(P)"); break;
                                    default: text: qsTr("Unknown"); break;
                                    }
                                }
                            }

                    }
                    TableViewColumn {
                        role: "LogString";
                        title: qsTr("Data");
                        width: 280
                    }
                }

                RowLayout {
                    anchors.margins: 10
                    spacing: 10

                    ColumnLayout {
                        spacing: 10
                        Text {
                            id: totalFlights
                            text: "<b>" + qsTr("Flights recorded:") + "</b> " + (logStatus.Flight + 1)
                        }
                        Text {
                            id: totalEntries
                            text: "<b>" + qsTr("Entries downloaded:") + "</b> " + logManager.logEntriesCount
                        }
                        Rectangle {
                            Layout.fillHeight: true
                        }
                        CheckBox {
                            id: exportRelativeTimeCB
                            enabled: !logManager.disableControls && !logManager.disableExport && logManager.boardConnected
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
                                enabled: !logManager.disableControls && logManager.boardConnected
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
                                enabled: !logManager.disableControls && logManager.boardConnected
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
                                enabled: !logManager.disableControls && logManager.boardConnected
                                text: qsTr("Clear all logs")
                                activeFocusOnPress: true
                                onClicked: logManager.clearAllLogs()
                            }
                            Button {
                                id: exportButton
                                enabled: !logManager.disableControls && !logManager.disableExport && logManager.boardConnected
                                text: qsTr("Export logs...")
                                activeFocusOnPress: true
                                onClicked: logManager.exportLogs()
                            }
                        }
                    }
                }
            }
            ColumnLayout {
                id: settingsTab
                visible: false
                anchors.margins: 10
                anchors.fill: parent
                Text {
                    Layout.fillWidth: true
                    text: "<b>" + qsTr("Settings") + "</b>"
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Text {
                        text: qsTr("When to log: ")
                    }
                    ComboBox {
                        enabled: !logManager.disableControls && logManager.boardConnected
                        model: logManager.logStatuses
                        Layout.preferredWidth: 200
                        currentIndex: logManager.loggingEnabled
                        onCurrentIndexChanged: {
                            logManager.setLoggingEnabled(currentIndex);
                        }
                    }

                }

                Component {
                    id: comboEditableDelegate
                    Item {

                        Text {
                            width: parent.width
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            elide: styleData.elideMode
                            text: styleData.value !== undefined ? logManager.logSettings[styleData.value] : ""
                            color: logManager.uavoEntries[styleData.row].dirty ? "#f00" : styleData.textColor
                            visible: !styleData.selected
                        }
                        Loader {
                            id: loaderEditor
                            anchors.fill: parent
                            Connections {
                                target: loaderEditor.item
                                onCurrentIndexChanged: {
                                    logManager.uavoEntries[styleData.row].setting = loaderEditor.item.currentIndex
                                }
                            }
                            sourceComponent: styleData.selected ? editor : null
                            Component {
                                id: editor
                                ComboBox {
                                    id: combo
                                    model: logManager.logSettings
                                    currentIndex: styleData.value
                                }
                            }
                        }
                    }
                }

                Component {
                    id: spinnerEditableDelegate
                    Item {

                        Text {
                            width: parent.width
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            elide: styleData.elideMode
                            text: styleData.value !== undefined &&
                                  (logManager.uavoEntries[styleData.row].setting === 1 || logManager.uavoEntries[styleData.row].setting === 3) ?
                                  parseInt(logManager.uavoEntries[styleData.row].period) + " ms" : "-"
                            color: logManager.uavoEntries[styleData.row].dirty ? "#f00" : styleData.textColor
                            //visible: !styleData.selected && (logManager.uavoEntries[styleData.row].setting <= 1)
                            enabled: (logManager.uavoEntries[styleData.row].setting > 1)
                        }
                        Loader {
                            id: loaderEditor
                            anchors.fill: parent
                            Connections {
                                target: loaderEditor.item
                                onValueChanged: {
                                    logManager.uavoEntries[styleData.row].period = loaderEditor.item.value
                                }
                            }
                            sourceComponent: styleData.selected &&
                                             (logManager.uavoEntries[styleData.row].setting === 1 || logManager.uavoEntries[styleData.row].setting === 3) ? editor : null
                            Component {
                                id: editor
                                SpinBox {
                                    id: spinner
                                    decimals: 0
                                    minimumValue: 0
                                    maximumValue: 1000 * 60 * 60 //1h
                                    suffix: "ms"
                                    stepSize: 10
                                    value: styleData.value
                                }
                            }
                        }
                    }
                }

                TableView {
                    id: settingsTable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredHeight: 1000;
                    model: logManager.uavoEntries
                    enabled: !logManager.disableControls && logManager.boardConnected

                    rowDelegate: Rectangle {
                        height: 22
                        color: styleData.selected ? "#ccc" : (styleData.alternate ? "#fff" : "#eee")
                    }

                    TableViewColumn {
                        role: "name";
                        title: qsTr("UAVObject");
                        width: 250;
                        delegate:
                            Text {
                                verticalAlignment: Text.AlignVCenter
                                anchors.leftMargin: 5
                                color: logManager.uavoEntries[styleData.row].dirty ? "#f00" : styleData.textColor
                                text: styleData.value
                            }

                    }

                    TableViewColumn {
                        role: "setting";
                        title: qsTr("Settings");
                        width: 160;
                        delegate: comboEditableDelegate
                    }

                    TableViewColumn {
                        role: "period";
                        title: qsTr("Period");
                        width: 120;
                        delegate: spinnerEditableDelegate
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Button {
                        enabled: !logManager.disableControls && logManager.boardConnected
                        text: qsTr("Load...")
                        tooltip: qsTr("Loads settings for all objects from a file.")
                        activeFocusOnPress: true
                        onClicked: logManager.loadSettings()
                    }
                    Button {
                        enabled: !logManager.disableControls && logManager.boardConnected
                        text: qsTr("Save...")
                        tooltip: qsTr("Saves settings for all objects in a file.")
                        activeFocusOnPress: true
                        onClicked: logManager.saveSettings()
                    }
                    Button {
                        enabled: !logManager.disableControls && logManager.boardConnected
                        text: qsTr("Reset")
                        tooltip: qsTr("Resets all settings to the values currently set on the board.")
                        activeFocusOnPress: true
                        onClicked: logManager.resetSettings(false)
                    }
                    Button {
                        enabled: !logManager.disableControls && logManager.boardConnected
                        text: qsTr("Clear")
                        tooltip: qsTr("Clears all settings to default values.")
                        activeFocusOnPress: true
                        onClicked: logManager.resetSettings(true)
                    }
                    Rectangle {
                        Layout.fillWidth: true
                    }
                    Button {
                        enabled: !logManager.disableControls && logManager.boardConnected
                        text: qsTr("Save to board")
                        tooltip: qsTr("Saves the logging configurations to the boards flash memory.")
                        activeFocusOnPress: true
                        onClicked: logManager.saveSettingsToBoard()
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            height: 40
            Button {
                id: settingsButton
                enabled: !logManager.disableControls
                text: qsTr("Settings...")
                activeFocusOnPress: true
                property bool showSettings: false
                onClicked: {
                    showSettings = !showSettings;
                    settingsTab.visible = showSettings;
                    exportTab.visible = !showSettings;
                    text = (showSettings ? qsTr("Logs...") : qsTr("Settings..."));
                }
            }
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
                onClicked: logDialog.close()
            }
        }
    }
}
