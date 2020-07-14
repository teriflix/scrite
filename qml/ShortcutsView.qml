/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    property alias backgroundColor: background.color
    property alias backgroundOpacity: background.opacity
    property alias backgroundBorder: background.border

    readonly property real minimumWidth: 350

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
        model: shortcutsModel
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {
            policy: shortcutsView.contentHeight > shortcutsView.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            minimumSize: 0.1
            palette {
                mid: Qt.rgba(0,0,0,0.25)
                dark: Qt.rgba(0,0,0,0.75)
            }
            opacity: active ? 1 : 0.2
        }
        section.property: "itemGroup"
        section.criteria: ViewSection.FullString
        section.delegate: Item {
            width: shortcutsView.width-17
            height: 40

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    // font.family: "Courier Prime"
                    font.pointSize: app.idealFontPointSize
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
                anchors.rightMargin: 5

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    // font.family: "Courier Prime"
                    font.pointSize: app.idealFontPointSize
                    text: itemTitle
                    width: parent.width * 0.7
                    elide: Text.ElideRight
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Courier Prime"
                    font.pointSize: app.idealFontPointSize-2
                    text: app.polishShortcutTextForDisplay(itemShortcut)
                    width: parent.width * 0.3
                }
            }
        }
    }
}
