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
        width: parent.width * 0.7
        height: parent.height * 0.7
        spacing: height * 0.1
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        Row {
            id: pager
            y: 0
            width: parent.width
            height: parent.height * 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            //anchors.bottom: parent.verticalCenter
            spacing: 16

            Image {
                height: parent.height
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                source: "images/welcome-op-logo.png"
            }

            Flow {
                id: buttons
                x: 173
                width: 453
                height: parent.height
                anchors.top: parent.top
                anchors.topMargin: 0
                flow: Flow.LeftToRight

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
            width: parent.width
            height: parent.height - pager.height
        }
    }

    signal changePage(int index)
}
