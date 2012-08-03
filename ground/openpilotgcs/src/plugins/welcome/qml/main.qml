import QtQuick 1.1

Rectangle {
    id: container
    width: 1024
    height: 768

    color: "#272727"

    Image {
        id: bg
        source: "images/welcome-op-bg.png"
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        // The background OP logo is downscalled to fit the page
        // but not upscalled if page is larger
        property real scale: Math.min(parent.width/sourceSize.width,
                                      parent.height/sourceSize.height,
                                      1.0)

        width: scale*sourceSize.width
        height: scale*sourceSize.height
        smooth: true
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        // distribute a vertical space between the icons blocks an community widget as:
        // top - 48% - Icons - 27% - CommunityWidget - 25% - bottom
        y: (parent.height - buttons.height - communityPanel.height) * 0.48
        width: parent.width
        spacing: (parent.height - buttons.height - communityPanel.height) * 0.27

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 16

            Image {
                source: "images/welcome-op-logo.png"
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -2 //it looks better aligned to icons grid
            }

            Grid {
                id: buttons
                columns: 3
                spacing: 4
                anchors.verticalCenter: parent.verticalCenter

                WelcomePageButton {
                    baseIconName: "flightdata"
                    label: "Flight Data"
                    onClicked: welcomePlugin.openPage("Flight data")
                }

                WelcomePageButton {
                    baseIconName: "config"
                    label: "Configuration"
                    onClicked: welcomePlugin.openPage("Configuration")
                }

                WelcomePageButton {
                    baseIconName: "system"
                    label: "System"
                    onClicked: welcomePlugin.openPage("System")
                }

                WelcomePageButton {
                    baseIconName: "scopes"
                    label: "Scopes"
                    onClicked: welcomePlugin.openPage("Scopes")
                }

                WelcomePageButton {
                    baseIconName: "hitl"
                    label: "HITL"
                    onClicked: welcomePlugin.openPage("HITL")
                }

                WelcomePageButton {
                    baseIconName: "firmware"
                    label: "Firmware"
                    onClicked: welcomePlugin.openPage("Firmware")
                }
            } //icons grid
        } // images row

        CommunityPanel {
            id: communityPanel
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
