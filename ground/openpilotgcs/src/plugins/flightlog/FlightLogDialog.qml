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

        Text {
            id:flights
            height: 40
            text: logStatus.Flight
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
