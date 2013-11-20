import QtQuick 2.0
import QtQuick.Controls 1.0

Rectangle {
    width: 100
    height: 62

    Button {
        id: button1
        x: 8
        y: 18
        text: qsTr("OK")
        onClicked: dialog.close()
    }
}
