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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Item {
    id: root

    required property string description
    required property string portableShortcut
    property string nativeShortcut: Gui.nativeShortcut(portableShortcut)

    property alias placeholderText: _placeholder.text

    signal shortcutEdited(string newShortcut)

    implicitWidth: Math.max(_text.contentWidth, _placeholder.contentWidth)
    implicitHeight: Math.max(_text.height, _placeholder.height)

    VclText {
        id: _text

        padding: 8
        text: root.nativeShortcut

        font.family: Runtime.shortcutFontMetrics.font.family
        font.pointSize: Runtime.shortcutFontMetrics.font.pointSize
        font.underline: _mouseArea.containsMouse
    }

    VclText {
        id: _placeholder

        text: "No shortcut set."
        padding: _text.padding
        visible: _text.text === ""

        font.underline: _mouseArea.containsMouse
        font.pointSize: Runtime.idealFontMetrics.font.pointSize
    }

    MouseArea {
        id: _mouseArea

        anchors.fill: parent

        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onClicked: ShortcutInputDialog.launch(root.portableShortcut, root.description, root.shortcutEdited)
    }
}
