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

Item {
    id: specialSymbolsSupport
    property Item textEditor
    property bool textEditorHasCursorInterface: false
    property bool includeEmojis: true
    property bool showingSymbols: symbolMenu.visible

    signal symbolSelected(string text)

    EventFilter.target: textEditor
    EventFilter.active: textEditor !== null && specialSymbolsSupport.enabled && textEditor.activeFocus
    EventFilter.events: [6]
    EventFilter.onFilter: {
        if(!specialSymbolsSupport.enabled)
            return

        if(textEditorHasCursorInterface && textEditor.readOnly)
            return

        if(event.key === Qt.Key_F3) {
            symbolMenu.visible = true
            result.filer = true
            result.accepted = true
        }
    }

    Menu2 {
        id: symbolMenu
        width: 514
        focus: false

        MenuItem2 {
            width: symbolMenu.width
            height: 400
            focusPolicy: Qt.NoFocus
            background: Item { }
            contentItem: SpecialSymbolsPanel {
                includeEmojis: specialSymbolsSupport.includeEmojis
                onSymbolClicked: {
                    if(!specialSymbolsSupport.enabled)
                        return

                    if(textEditorHasCursorInterface) {
                        if(textEditor.readOnly)
                            return

                        var cp = textEditor.cursorPosition
                        textEditor.insert(textEditor.cursorPosition, text)
                        Scrite.app.execLater(textEditor, 250, function() { textEditor.cursorPosition = cp + text.length })
                        symbolMenu.close()
                        textEditor.forceActiveFocus()
                    } else {
                        symbolMenu.close()
                        symbolSelected(text)
                    }
                }
            }
        }
    }
}
