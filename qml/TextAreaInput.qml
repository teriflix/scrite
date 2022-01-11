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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import io.scrite.components 1.0

TextArea {
    id: txtAreaInput
    palette: Scrite.app.palette
    selectByKeyboard: true
    selectByMouse: true
    // renderType: Text.NativeRendering
    Material.primary: primaryColors.key
    Material.accent: accentColors.key
    selectedTextColor: accentColors.c700.text
    selectionColor: accentColors.c700.background
    background: Rectangle {
        color: enabled ? primaryColors.c10.background : primaryColors.button.background

        Rectangle {
            width: parent.width
            height: txtAreaInput.activeFocus ? 2 : 1
            color: accentColors.c700.background
            visible: txtAreaInput.enabled
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 4
        }
    }
    Keys.onReturnPressed: Transliterator.transliterateLastWord()
    Transliterator.textDocument: textDocument
    Transliterator.cursorPosition: cursorPosition
    Transliterator.hasActiveFocus: activeFocus
    Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
}
