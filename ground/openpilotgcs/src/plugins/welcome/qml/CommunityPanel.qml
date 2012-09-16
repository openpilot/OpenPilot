// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    property alias sourceSize: background.sourceSize
    width: sourceSize.width
    height: 300

    BorderImage {
        id: background
        anchors.fill: parent

        border { left: 30; top: 30; right: 30; bottom: 30 }
        source: "images/welcome-news-bg.png"
    }

    NewsPanel {
        id: newsPanel
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width*0.6
        anchors.margins: 32

        onClicked: welcomePlugin.openUrl(url)
    }

    //better to use image instead
    Rectangle {
        id: separator
        width: 1
        height: parent.height*0.7
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: newsPanel.right
        anchors.margins: 16
        color: "#A0A0A0"
    }

    SitesPanel {
        anchors.left: newsPanel.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 32

        onClicked: welcomePlugin.openUrl(url)
    }
}
