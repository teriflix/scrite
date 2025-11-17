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

Link {
    id: root

    required property string description
    required property string portableShortcut

    property string placeholderText: "None Set"
    property string nativeShortcut: Gui.nativeShortcut(portableShortcut)

    function editShortcut() {
        if(enabled)
            ShortcutInputDialog.launch(portableShortcut, description, shortcutEdited)
    }

    signal shortcutEdited(string newShortcut)

    defaultColor: Runtime.colors.primary.c10.text

    padding: 8
    text: root.nativeShortcut === "" ? placeholderText : root.nativeShortcut

    font.family: Runtime.shortcutFontMetrics.font.family
    font.pointSize: Runtime.shortcutFontMetrics.font.pointSize
    font.underline: containsMouse

    onClicked: editShortcut()
}
