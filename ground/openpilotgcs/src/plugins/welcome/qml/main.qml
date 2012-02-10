import QtQuick 1.1

Rectangle {
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
        anchors.centerIn: parent
        width: parent.width
        spacing: 32

        Row {
            //anchors.bottom: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 16

            Image {
                source: "images/welcome-op-logo.png"
                anchors.verticalCenter: parent.verticalCenter
            }

            Grid {
                id: buttons
                columns: 3
                spacing: 4

                WelcomePageButton {
                    baseIconName: "welcome-flightdata"
                    label: "Flight Data"
                    onClicked: welcomePlugin.openPage("Mode1")
                }

                WelcomePageButton {
                    baseIconName: "welcome-flightdata"
                    label: "Configuration"
                    onClicked: welcomePlugin.openPage("Mode2")
                }

                WelcomePageButton {
                    baseIconName: "welcome-flightdata"
                    label: "Flight Planner"
                    onClicked: welcomePlugin.openPage("Mode3")
                }

                WelcomePageButton {
                    baseIconName: "welcome-flightdata"
                    label: "Scopes"
                    onClicked: welcomePlugin.openPage("Mode4")
                }

                WelcomePageButton {
                    baseIconName: "welcome-flightdata"
                    label: "HIL"
                    onClicked: welcomePlugin.openPage("Mode5")
                }

                WelcomePageButton {
                    baseIconName: "welcome-flightdata"
                    label: "Firmware"
                    onClicked: welcomePlugin.openPage("Mode6")
                }
            } //icons grid
        } // images row

        CommunityPanel {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width*0.8
        }
    }
}
