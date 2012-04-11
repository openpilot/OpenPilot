import QtQuick 1.1

Rectangle {
    id: welcome
    width: 1024
    height: 768

    color: "#272727"

    Image {
        id: bg
        source: "images/welcome-op-bg.png"
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Column {
        id: column1
        x: 66
        y: 66
        width: parent.width * 0.8
        height: parent.height * 0.8
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32

        Row {
            id: pager
            width: parent.width * 0.7
            height: parent.height * 0.4
            //anchors.bottom: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 16

            Image {
                height: parent.height
                fillMode: Image.PreserveAspectFit
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.top: parent.top
                anchors.topMargin: 0
                source: "images/welcome-op-logo.png"
            }

            Flow {
                id: buttons
                x: 173
                y: 52
                width: 453
                height: parent.height

                WelcomePageButton {
                    width: parent.height / 2
                    height: parent.height / 2
                    baseIconName: "welcome-flightdata"
                    label: "Flight Data"
                    onClicked: welcome.changePage(1)
                }

                WelcomePageButton {
                    width: parent.height / 2
                    height: parent.height / 2
                    baseIconName: "welcome-flightdata"
                    label: "Configuration"
                    onClicked: welcome.changePage(2)
                }

                WelcomePageButton {
                    width: parent.height / 2
                    height: parent.height / 2
                    baseIconName: "welcome-flightdata"
                    label: "Flight Planner"
                    onClicked: welcome.changePage(3)
                }

                WelcomePageButton {
                    width: parent.height / 2
                    height: parent.height / 2
                    baseIconName: "welcome-flightdata"
                    label: "Scopes"
                    onClicked: welcome.changePage(4)
                }

                WelcomePageButton {
                    width: parent.height / 2
                    height: parent.height / 2
                    baseIconName: "welcome-flightdata"
                    label: "HIL"
                    onClicked: welcome.changePage(5)
                }

                WelcomePageButton {
                    width: parent.height / 2
                    height: parent.height / 2
                    baseIconName: "welcome-flightdata"
                    label: "Firmware"
                    onClicked: welcome.changePage(6)
                }
            } //icons grid
        } // images row

        CommunityPanel {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width*0.8
            height: parent.height * 0.4
            anchors.bottom: parent.bottom
        }
    }

    signal changePage(int index)
}
