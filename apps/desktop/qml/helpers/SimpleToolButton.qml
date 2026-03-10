/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick

import io.scrite.components

import "../globals"

Rectangle {
    id: root

    property bool down: false
    property bool checked: false
    property alias pressed: _mouseArea.pressed
    property alias hoverEnabled: _mouseArea.hoverEnabled
    property alias containsMouse: _mouseArea.containsMouse
    property alias iconSource: _icon.source

    signal clicked()

    width: implicitWidth
    height: implicitHeight
    implicitWidth: 36
    implicitHeight: 36

    radius: 4
    opacity: enabled ? 1 : 0.5
    color: _mouseArea.pressed || down ? Runtime.colors.primary.button.background : (checked ? Runtime.colors.primary.highlight.background : Qt.rgba(0,0,0,0))

    Image {
        id: _icon
        anchors.fill: parent
        anchors.margins: 4
        mipmap: true
    }

    MouseArea {
        id: _mouseArea
        anchors.fill: parent
        onClicked: parent.clicked()
    }
}
