// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 2.0

Item {
    property alias sourceSize: background.sourceSize
    width: sourceSize.width
    height: 400

    BorderImage {
        id: background
        x: 0
        y: 0
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0
        anchors.fill: parent

        border { left: 30; top: 30; right: 30; bottom: 30 }
        source: "images/welcome-news-bg.png"
    }

    SitesPanel {
        id: sites
        height: 50
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 32

        onClicked: welcomePlugin.openUrl(url)
    }

    Rectangle {
       id: vertSeparator
       height: 1
       width: sites.width-20
       anchors.horizontalCenter: sites.horizontalCenter
       anchors.top: sites.bottom
       anchors.margins: 16
       color: "#A0A0A0"
   }

    NewsPanel {
        id: newsPanel
        anchors.left: parent.left
        anchors.top: sites.bottom
        anchors.bottom: parent.bottom
        width: parent.width*0.45
        anchors.margins: 32

        onClicked: welcomePlugin.openUrl(url)
    }

     Rectangle {
        id: horizSeparator
        width: 1
        height: newsPanel.height-20
        anchors.verticalCenter: newsPanel.verticalCenter
        anchors.left: newsPanel.right
        anchors.margins: 16
        color: "#A0A0A0"
    }

    ActivityPanel {
        id: activityPanel
        anchors.left: newsPanel.right
        anchors.top: sites.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 32

        onClicked: welcomePlugin.openUrl(url)
    }
}
