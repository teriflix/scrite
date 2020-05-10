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

Rectangle {
    color: primaryColors.c10.background
    border.width: 1
    border.color: primaryColors.borderColor
    radius: height/2

    property alias text: textItem.text
    property alias font: textItem.font
    property alias textColor: textItem.color
    property alias padding: textItem.padding
    property alias topPadding: textItem.topPadding
    property alias leftPadding: textItem.leftPadding
    property alias rightPadding: textItem.rightPadding
    property alias bottomPadding: textItem.bottomPadding
    property alias closable: closeButton.active
    property alias hoverEnabled: tagMouseArea.hoverEnabled
    property alias containsMouse: tagMouseArea.containsMouse

    width: textItem.width + (closeButton.active ? (height+2-textItem.rightPadding) : 0)
    height: textItem.height

    signal closeRequest()
    signal clicked()

    Text {
        id: textItem
        padding: 5
    }

    MouseArea {
        id: tagMouseArea
        hoverEnabled: true
        anchors.fill: parent
        onClicked: parent.clicked()
    }

    Loader {
        id: closeButton
        width: parent.height-8; height: parent.height-8
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 4
        active: false

        sourceComponent: Rectangle {
            color: closeButtonMouseArea.pressed ? accentColors.c600.background : accentColors.c100.background
            radius: height/2

            Image {
                source: closeButtonMouseArea.pressed ? "../icons/navigation/close_inverted.png" : "../icons/navigation/close.png"
                height: parent.height * 0.85; width: height
                smooth: true
                anchors.centerIn: parent
            }

            MouseArea {
                id: closeButtonMouseArea
                anchors.fill: parent
                onClicked: closeRequest()
            }
        }
    }
}
