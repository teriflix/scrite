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

    property string placeholderText: "None Set"
    property string nativeShortcut: Gui.nativeShortcut(portableShortcut)

    property font font: fontMetrics.font
    property FontMetrics fontMetrics: Runtime.shortcutFontMetrics

    property var keyCombinations: Gui.keyCombinations(portableShortcut)

    readonly property string delimiter: " + "

    function editShortcut() {
        if(enabled)
            ShortcutInputDialog.launch(portableShortcut, description, shortcutEdited)
    }

    signal shortcutEdited(string newShortcut)

    implicitWidth: _layout.width
    implicitHeight: _layout.height

    clip: width < _layout.width

    RowLayout {
        id: _layout

        spacing: fontMetrics.averageCharacterWidth * 0.4

        opacity: enabled ? 1 : 0.5

        Repeater {
            id: _modifiers

            model: keyCombinations.modifiers

            KeyboardKey {
                required property string modelData

                Layout.minimumWidth: __minimumKeyWidth
                Layout.minimumHeight: __minimumKeyHeight

                text: modelData
            }
        }

        Repeater {
            model: keyCombinations.keys

            KeyboardKey {
                required property string modelData

                Layout.minimumWidth: __minimumKeyWidth
                Layout.minimumHeight: __minimumKeyHeight

                text: modelData
            }
        }

        Link {
            text: placeholderText
            font: root.font
            visible: portableShortcut === ""
        }
    }

    MouseArea {
        id: _mouseArea

        anchors.fill: _layout

        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true

        onClicked: editShortcut()
    }

    property real __minimumKeyWidth: fontMetrics.boundingRect("Ctrl").width + 6
    property real __minimumKeyHeight: fontMetrics.lineSpacing

    component KeyboardKey : Rectangle {
        property string text

        implicitWidth: _keyText.width
        implicitHeight: _keyText.height

        color: _mouseArea.containsMouse ? Runtime.colors.primary.c300.background : Runtime.colors.primary.c200.background
        border.width: enabled ? 1 : 0
        border.color: Runtime.colors.primary.c700.background

        Text {
            id: _keyText

            anchors.centerIn: parent

            color: Color.textColorFor(parent.color)
            font: root.font
            text: parent.text
            leftPadding: 3
            rightPadding: 3
            topPadding: 2
            bottomPadding: 2
        }
    }
}
