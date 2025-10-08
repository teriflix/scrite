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

Rectangle {
    id: root

    property real padding: 0
    property real topPadding: padding
    property real leftPadding: padding
    property real rightPadding: padding
    property real bottomPadding: padding

    property alias text: _text.text
    property alias font: _text.font
    property alias closable: _closeButtonLoader.active
    property alias textColor: _text.color
    property alias hoverEnabled: _mouseArea.hoverEnabled
    property alias containsMouse: _mouseArea.containsMouse

    signal clicked()
    signal closeRequest()

    width: _container.width + (_closeButtonLoader.active ? (height+2-rightPadding) : 0)
    height: _container.height

    color: Runtime.colors.primary.c10.background
    radius: height/2
    border.width: 1
    border.color: Runtime.colors.primary.borderColor

    Item {
        id: _container

        width: _text.width + parent.leftPadding + parent.rightPadding
        height: _text.height + parent.topPadding + parent.bottomPadding

        VclText {
            id: _text

            x: root.leftPadding
            y: root.topPadding
            width: contentWidth
            height: contentHeight

            color: Runtime.colors.primary.c10.text
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
        }
    }

    MouseArea {
        id: _mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: parent.clicked()
    }

    Loader {
        id: _closeButtonLoader

        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.verticalCenter: parent.verticalCenter

        width: parent.height-8
        height: parent.height-8
        active: false

        sourceComponent: Rectangle {
            color: _closeButtonMouseArea.pressed ? Runtime.colors.accent.c600.background : Runtime.colors.accent.c100.background
            radius: height/2

            Image {
                anchors.centerIn: parent

                width: height
                height: parent.height * 0.85

                source: _closeButtonMouseArea.pressed ? "qrc:/icons/navigation/close_inverted.png" : "qrc:/icons/navigation/close.png"
                smooth: true
            }

            MouseArea {
                id: _closeButtonMouseArea

                anchors.fill: parent

                onClicked: root.closeRequest()
            }
        }
    }
}
