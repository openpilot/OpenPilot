/**
 ******************************************************************************
 *
 * @file       aboutdialog.qml
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
import QtQuick 2.1
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.0

Rectangle {
    id: container
    width: 600
    height: 400

    property AuthorsModel authors: AuthorsModel {}

    ColumnLayout {
        id: columnLayout1
        anchors.fill: parent
        spacing: 10
        RowLayout {
            id: rowLayout1
            opacity: 1
            Image {
                id: logo
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.top: parent.top
                anchors.topMargin: 10
                source: "../images/openpilot_logo_128.png"
                z: 100
                fillMode: Image.PreserveAspectFit
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        dialog.openUrl("http://www.openpilot.org")
                    }
                }
            }

            Rectangle {
                anchors.left: logo.right
                anchors.margins: 10
                anchors.right: parent.right
                anchors.top: parent.top
                Layout.fillHeight: true
                Layout.fillWidth: true
                anchors.bottom: parent.bottom
                color: "transparent"

                ColumnLayout {
                    anchors.fill: parent
                    Text {
                        id: headerLabel
                        text: qsTr("OpenPilot Ground Control Station")
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        font.bold: true

                    }
                    Text {
                        id: versionLabel
                        Layout.fillWidth: true
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        text: version
                    }
                    Text {
                        id: licenseLabel
                        Layout.fillWidth: true
                        font.pixelSize: 9
                        wrapMode: Text.WordWrap
                        text: qsTr("This program is free software; you can redistribute it and/or " +
                                   "modify it under the terms of the GNU General Public License " +
                                   "as published by the Free Software Foundation; either version 3 " +
                                   "of the License, or (at your option) any later version.\n" +
                                   "The program is provided AS IS with NO WARRANTY OF ANY KIND, " +
                                   "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.")
                    }

                    Text {
                        id: contributorLabel
                        text: qsTr("Contributors")
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        font.bold: true

                    }
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        frameVisible: true
                        ListView {
                            id: authorsView
                            anchors.fill: parent

                            spacing: 3
                            model: authors
                            delegate: Text {
                                font.pixelSize: 12
                                text: name
                            }
                            clip: true
                        }
                    }
                }
            }
        }
        Button {
            id: button
            text: qsTr("Ok")
            activeFocusOnPress: true
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            onClicked: dialog.close()
        }
    }
}
