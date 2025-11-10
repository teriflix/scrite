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

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: root

    property alias shortcut: _text.text
    property alias placeholderText: _placeholder.text

    signal shortcutEdited(string newShortcut)

    implicitWidth: Math.max(_text.width, _placeholder.width)
    implicitHeight: Math.max(_text.height, _placeholder.height)

    ShortcutInputHandler.handleInput: activeFocus
    ShortcutInputHandler.onShortcutCaptured: (newShortcut) => { shortcutEdited(newShortcut) }

    Rectangle {
        id: _background

        anchors.fill: parent

        visible: parent.activeFocus
        border.width: 1
        border.color: Runtime.colors.accent.c300.background

        SequentialAnimation {
            loops: Animation.Infinite
            running: root.activeFocus

            ColorAnimation {
                to: Runtime.colors.accent.c200.background
                from: Runtime.colors.primary.c10.background
                target: _background
                duration: Qt.styleHints.cursorFlashTime
                properties: "color"
            }

            ColorAnimation {
                to: Runtime.colors.primary.c10.background
                from: Runtime.colors.accent.c200.background
                target: _background
                duration: Qt.styleHints.cursorFlashTime
                properties: "color"
            }
        }
    }

    VclText {
        id: _text

        padding: 8

        font.family: {
            // We need ZERO and the letter O to be rendered distinctly
            // We also need small-L and capital-I and digit-1 to look disctinct.
            switch(Platform.type) {
            case Platform.WindowsDesktop: return "Consolas"
            case Platform.MacOSDesktop: return "Monaco"
            case Platform.LinuxDesktop: return "DejaVu Sans Mono"
            }
            return "Courier Prime"
        }
    }

    VclText {
        id: _placeholder

        text: "No shortcut set."
        padding: _text.padding
        visible: _text.text === ""
    }

    MouseArea {
        id: _mouseArea

        anchors.fill: parent

        onClicked: parent.forceActiveFocus()
    }
}
