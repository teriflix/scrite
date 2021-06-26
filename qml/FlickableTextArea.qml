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
import QtQuick.Controls.Material 2.12

import Scrite 1.0

Flickable {
    property Item textArea: __textArea
    property bool scrollBarRequired: contentHeight > height
    property bool undoRedoEnabled: true
    property alias text: __textArea.text
    property alias font: __textArea.font
    property Item tab
    property Item backtab
    property alias readonly: __textArea.readOnly
    property alias placeholderText: __textArea.placeholderText

    id: flickable
    clip: true
    contentWidth: __textArea.width
    contentHeight: __textArea.height
    ScrollBar.vertical: ScrollBar {
        policy: flickable.scrollBarRequired ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
        minimumSize: 0.1
        palette {
            mid: Qt.rgba(0,0,0,0.25)
            dark: Qt.rgba(0,0,0,0.75)
        }
        opacity: active ? 1 : 0.2
        Behavior on opacity {
            enabled: screenplayEditorSettings.enableAnimations
            NumberAnimation { duration: 250 }
        }
    }

    TextArea {
        id: __textArea
        width: flickable.width - (flickable.scrollBarRequired ? 20 : 0)
        height: Math.max(flickable.height-topPadding-bottomPadding, contentHeight+50)
        font.pointSize: app.idealFontPointSize
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        selectByMouse: true
        selectByKeyboard: true
        leftPadding: 5; rightPadding: 5
        topPadding: 5; bottomPadding: 5
        Transliterator.textDocument: textDocument
        Transliterator.cursorPosition: cursorPosition
        Transliterator.hasActiveFocus: activeFocus
        Transliterator.textDocumentUndoRedoEnabled: undoRedoEnabled
        readOnly: scriteDocument.readOnly
        KeyNavigation.tab: flickable.tab
        KeyNavigation.backtab: flickable.backtab
        KeyNavigation.priority: KeyNavigation.AfterItem
        background: Item { }
        SpecialSymbolsSupport {
            anchors.top: parent.bottom
            anchors.left: parent.left
            textEditor: __textArea
            textEditorHasCursorInterface: true
            enabled: !scriteDocument.readOnly
        }
        UndoHandler {
            enabled: !__textArea.readOnly && __textArea.activeFocus && flickable.undoRedoEnabled
            canUndo: __textArea.canUndo
            canRedo: __textArea.canRedo
            onUndoRequest: __textArea.undo()
            onRedoRequest: __textArea.redo()
        }
        onCursorRectangleChanged: {
            var cr = cursorRectangle
            cr = Qt.rect(cr.x, cr.y-4, cr.width, cr.height+8)

            var cy = flickable.contentY
            var ch = flickable.height
            if(cr.y < flickable.contentY)
                cy = Math.max(cr.y, 0)
            else if(cr.y + cr.height > cy + ch)
                cy = Math.min(cr.y + cr.height - ch, height-ch)
            else
                return
            flickable.contentY = cy
        }
    }
}
