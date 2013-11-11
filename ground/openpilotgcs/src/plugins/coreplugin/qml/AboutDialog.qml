/**
 ******************************************************************************
 *
 * @file       aboutdialog.qml
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.0

GridLayout {
    width: 600
    height: 400
    ColumnLayout {
        id: columnLayout1
        anchors.fill: parent
        spacing: 10
        RowLayout {
            id: rowLayout1
            Image {
                id: logo
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.top: parent.top
                anchors.topMargin: 10
                source: "../images/openpilot_logo_128.png"
                z: 100
                fillMode: Image.PreserveAspectFit
            }
            TabView {
                id: tabs
                anchors.left: logo.right
                anchors.leftMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: parent.top
                anchors.topMargin: 10
                Layout.fillHeight: true
                Layout.fillWidth: true
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                Tab {
                    title: qsTr("About")
                    Text {
                        id: versionLabel
                        anchors.fill: parent
                        anchors.margins: 10
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        text: version
                    }
                }

                Tab {
                    title: qsTr("Contributors")
                    ListView {
                        id: authorsView
                        anchors.fill: parent
                        anchors.margins: 10

                        spacing: 3
                        model: authors
                        delegate: Text {
                            text: name
                        }
                        clip: true
                    }
                }
            }
        }
        Button {
            id: button
            x: 512
            y: 369
            text: qsTr("Ok")
            activeFocusOnPress: true
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
        }
    }
}
