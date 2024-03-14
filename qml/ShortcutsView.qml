/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    property alias backgroundColor: background.color
    property alias backgroundOpacity: background.opacity
    property alias backgroundBorder: background.border

    readonly property real minimumWidth: 375

    Rectangle {
        id: background
        anchors.fill: parent
        color: "white"
        opacity: 0.5
        radius: 8
    }

    ListView {
        id: shortcutsView
        anchors.fill: parent
        anchors.margins: 5
        model: Scrite.shortcuts
        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: VclScrollBar { flickable: shortcutsView }
        section.property: "itemGroup"
        section.criteria: ViewSection.FullString
        section.delegate: Item {
            width: shortcutsView.width-17
            height: 40

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4

                VclText {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    // font.family: "Courier Prime"
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    font.bold: true
                    text: section
                }
            }
        }
        delegate: Item {
            width: shortcutsView.width-17
            height: itemVisible ? 40 : 0
            opacity: itemEnabled ? 1 : 0.5
            visible: itemVisible

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.right: parent.right

                VclText {
                    anchors.verticalCenter: parent.verticalCenter
                    // font.family: "Courier Prime"
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    text: itemTitle
                    width: parent.width * 0.65
                    elide: Text.ElideRight
                }

                VclText {
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Courier Prime"
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize-2
                    text: Scrite.app.polishShortcutTextForDisplay(itemShortcut)
                    width: parent.width * 0.35
                }
            }
        }
    }
}
