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

Column {
    id: formField
    spacing: 10

    property string questionKey: questionNumber
    property alias questionNumber: questionNumberText.text
    property alias question: questionText.text
    property alias answer: answerText.text
    property alias placeholderText: answerText.placeholderText
    property bool enableUndoRedo: true
    property rect cursorRectangle: {
        var cr = answerText.cursorRectangle
        return mapFromItem(answerText, cr.x, cr.y, cr.width, cr.height)
    }
    property alias cursorVisible: answerText.cursorVisible
    property TextArea textFieldItem: answerText
    property TabSequenceManager tabSequenceManager
    property bool textFieldHasActiveFocus: answerText.activeFocus
    property real minHeight: questionRow.height + answerArea.minHeight + spacing
    property int tabSequenceIndex: 0

    Row {
        id: questionRow
        width: parent.width
        spacing: 10

        Label {
            id: questionNumberText
            font.bold: true
            horizontalAlignment: Text.AlignRight
            width: idealAppFontMetrics.averageCharacterWidth * 2
            anchors.top: parent.top
        }

        Label {
            id: questionText
            font.bold: true
            width: parent.width - questionNumberText.width - parent.spacing
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            anchors.top: parent.top
        }
    }

    Rectangle {
        id: answerArea
        width: questionText.width
        anchors.right: parent.right
        color: app.translucent(primaryColors.c100.background, 0.75)
        border.width: 1
        border.color: app.translucent(primaryColors.borderColor, 0.25)
        height: Math.max(minHeight, answerText.height)
        readonly property real minHeight: 125

        TextArea {
            id: answerText
            width: answerArea.width
            height: Math.max(answerArea.minHeight-topPadding-bottomPadding, contentHeight+20)
            font.pointSize: app.idealFontPointSize
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            selectByMouse: true
            selectByKeyboard: true
            leftPadding: 5; rightPadding: 5
            topPadding: 5; bottomPadding: 5
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
            Transliterator.textDocumentUndoRedoEnabled: enableUndoRedo
            readOnly: scriteDocument.readOnly
            background: Item { }
            SpecialSymbolsSupport {
                anchors.top: parent.bottom
                anchors.left: parent.left
                textEditor: answerText
                textEditorHasCursorInterface: true
                enabled: !scriteDocument.readOnly
            }
            UndoHandler {
                enabled: !answerText.readOnly && answerText.activeFocus && enableUndoRedo
                canUndo: answerText.canUndo
                canRedo: answerText.canRedo
                onUndoRequest: answerText.undo()
                onRedoRequest: answerText.redo()
            }
            TabSequenceItem.manager: tabSequenceManager
            TabSequenceItem.sequence: tabSequenceIndex
        }
    }
}
