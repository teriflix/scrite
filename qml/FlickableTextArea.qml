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

Flickable {
    property Item textArea: __textArea
    property bool scrollBarRequired: contentHeight > height
    property bool adjustTextWidthBasedOnScrollBar: true
    property bool undoRedoEnabled: true
    property alias text: __textArea.text
    property alias font: __textArea.font
    property Item tabItem
    property Item backTabItem
    property alias readonly: __textArea.readOnly
    property alias placeholderText: __textArea.placeholderText
    property alias readOnly: __textArea.readOnly
    property alias background: __textArea.background
    property TabSequenceManager tabSequenceManager
    property int tabSequenceIndex: 0
    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

    id: textAreaFlickable
    clip: true
    contentWidth: __textArea.width
    contentHeight: __textArea.height
    ScrollBar.vertical: ScrollBar2 { flickable: textAreaFlickable }

    TextArea {
        id: __textArea
        width: textAreaFlickable.width - (textAreaFlickable.scrollBarRequired && textAreaFlickable.adjustTextWidthBasedOnScrollBar ? 20 : 0)
        height: Math.max(textAreaFlickable.height-topPadding-bottomPadding, contentHeight+20)
        font.pointSize: Scrite.app.idealFontPointSize
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        selectByMouse: true
        selectByKeyboard: true
        leftPadding: 5; rightPadding: 5
        topPadding: 5; bottomPadding: 5
        Transliterator.textDocument: textDocument
        Transliterator.cursorPosition: cursorPosition
        Transliterator.hasActiveFocus: activeFocus
        Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
        Transliterator.textDocumentUndoRedoEnabled: undoRedoEnabled
        readOnly: Scrite.document.readOnly
        KeyNavigation.tab: textAreaFlickable.tabItem
        KeyNavigation.backtab: textAreaFlickable.backTabItem
        KeyNavigation.priority: KeyNavigation.AfterItem
        background: Item { }
        SpecialSymbolsSupport {
            anchors.top: parent.bottom
            anchors.left: parent.left
            textEditor: __textArea
            textEditorHasCursorInterface: true
            enabled: !Scrite.document.readOnly
        }
        UndoHandler {
            enabled: !__textArea.readOnly && __textArea.activeFocus && textAreaFlickable.undoRedoEnabled
            canUndo: __textArea.canUndo
            canRedo: __textArea.canRedo
            onUndoRequest: __textArea.undo()
            onRedoRequest: __textArea.redo()
        }
        onCursorRectangleChanged: {
            var cr = cursorRectangle
            cr = Qt.rect(cr.x, cr.y-4, cr.width, cr.height+8)

            var cy = textAreaFlickable.contentY
            var ch = textAreaFlickable.height
            if(cr.y < cy)
                cy = Math.max(cr.y, 0)
            else if(cr.y + cr.height > cy + ch)
                cy = Math.min(cr.y + cr.height - ch, height-ch)
            else
                return
            textAreaFlickable.contentY = cy
        }
        TabSequenceItem.manager: tabSequenceManager
        TabSequenceItem.sequence: tabSequenceIndex
    }
}
