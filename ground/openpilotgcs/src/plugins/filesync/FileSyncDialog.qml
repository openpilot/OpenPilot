import QtQuick 2.4
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

//import org.openpilot 1.0

//import "functions.js" as Functions

Item {
    width: 500
    height: 300
    id: root
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0

            RowLayout {
                width: 100
                height: 100

                Text {
                    text: qsTr("Device: ")
                }

                ComboBox {
                    enabled: !filesyncManager.disableControls && filesyncManager.boardConnected
                    model: filesyncManager.device
                    Layout.preferredWidth: 120
                    currentIndex: styleData.value
                    onCurrentIndexChanged: {
                        filesyncManager.setDeviceIdx(currentIndex);
                    }
                }

                Button {
                    id: formatButton
                    enabled: !filesyncManager.disableControls && filesyncManager.boardConnected
                    text: qsTr("Format")
                    activeFocusOnPress: true
                    onClicked: filesyncManager.format()
                }
            }

            RowLayout {
                width: 100
                height: 100
                anchors.right: parent.right
                anchors.rightMargin: 0

                Label {
                    id: deviceInfoLabel
                    text: qsTr("Available: ") + filesyncManager.deviceFreeKBytes() + qsTr(" KBytes")
                }
            }
        }

        TableView {
            id: filelist
            Layout.fillWidth: true
            Layout.fillHeight: true
            //Layout.stretchLastSection: true
            Layout.preferredHeight: 1000;
            model: filesyncManager.fileEntries
            enabled: !filesyncManager.disableControls && filesyncManager.boardConnected
            //onClicked: tableView.currentRow = styleData.row
            rowDelegate: Rectangle {
                height: 22
                color: styleData.selected ? "#ccc" : (styleData.alternate ? "#fff" : "#eee")
                //if (styleData.selected)
                //    filelist.currentRow = styleData.row
                MouseArea {
                    //anchors.fill: parent
                    onClicked: (styleData != 'undefined')? filelist.currentRow = styleData.row: filelist.currentRow = -1;
                }
            }

            //itemDelegate: Text {
            //    verticalAlignment: Text.AlignVCenter
            //    text: styleData.value
            //}

            TableViewColumn {
                id: fileSelected
                role: "FileName"; title: qsTr("Name"); width: 200;
                delegate:
                    Text {
                    verticalAlignment: Text.AlignVCenter
                    text: styleData.value
                }
            }

            TableViewColumn {
                role: "FileType"; title: "Type"; width: 100;
                delegate:
                    Text {
                    verticalAlignment: Text.AlignVCenter
                    text: {
                        switch(styleData.value) {
                        case 0 : text: qsTr("Script"); break;
                        case 1 : text: qsTr("Log"); break;
                        case 2 : text: qsTr("UAVO"); break;
                        case 3 : text: qsTr("UAVO(P)"); break;
                        case 4 : text: qsTr("Info"); break;
                        case 5 : text: qsTr("Text"); break;
                        default: text: qsTr("Unknown"); break;
                        }
                    }
                }
            }

            TableViewColumn {
                role: "FileSize";
                title: qsTr("Size");
                width: 50
            }
        }

        RowLayout {
            spacing: 5

            Button {
                id: downloadButton
                enabled: !filesyncManager.disableControls && filesyncManager.boardConnected
                text: qsTr("Download")
                activeFocusOnPress: true
                onClicked: filesyncManager.downloadFile(filelist.currentRow)
            }

            Button {
                id: uploadButton
                enabled: !filesyncManager.disableControls && filesyncManager.boardConnected
                text: qsTr("Upload")
                activeFocusOnPress: true
                onClicked: filesyncManager.uploadFile()
            }

            Button {
                id: deleteButton
                enabled: !filesyncManager.disableControls && filesyncManager.boardConnected
                text: qsTr("Delete")
                activeFocusOnPress: true
                onClicked: filesyncManager.deleteFile(filelist.currentRow)
            }
        }

        RowLayout {
            anchors.right: parent.right
            anchors.rightMargin: 0
            spacing: 5

            Button {
                id: closeButton
                enabled: !filesyncManager.disableControls
                isDefault: true
                text: qsTr("Close")
                activeFocusOnPress: true
                onClicked: { filesyncManager.cancelFileSync(); syncDialog.close(); }
            }
        }
    }
}
