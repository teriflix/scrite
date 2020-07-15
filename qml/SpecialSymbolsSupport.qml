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
    property Item textEditor
    property bool textEditorHasCursorInterface: false

    signal symbolSelected(string text)

    EventFilter.target: textEditor
    EventFilter.active: textEditor !== null
    EventFilter.events: [6]
    EventFilter.onFilter: {
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
                onSymbolClicked: {
                    if(textEditorHasCursorInterface) {
                        var cp = textEditor.cursorPosition
                        textEditor.insert(textEditor.cursorPosition, text)
                        app.execLater(textEditor, 100, function() { textEditor.cursorPosition = cp + text.length })
                        symbolMenu.close()
                        textEditor.forceActiveFocus()
                    } else {
                        SpecialSymbolsSupport
                        symbolSelected(text)
                    }
                }
            }
        }
    }
}
