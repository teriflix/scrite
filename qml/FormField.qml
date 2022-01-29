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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

Column {
    id: formField
    spacing: 10

    property string questionKey: questionNumber
    property alias questionNumber: questionNumberText.text
    property alias question: questionText.text
    property string answer
    property string placeholderText
    property bool enableUndoRedo: true
    property rect cursorRectangle: {
        var cr = answerItemLoader.lod === answerItemLoader.eHIGH ? answerItemLoader.item.cursorRectangle : Qt.rect(0,0,1,12)
        return mapFromItem(answerItemLoader, cr.x, cr.y, cr.width, cr.height)
    }
    property bool cursorVisible: answerItemLoader.lod === answerItemLoader.eHIGH ? answerItemLoader.item.cursorVisible : false
    property TextArea textFieldItem: answerText
    property TabSequenceManager tabSequenceManager
    property bool textFieldHasActiveFocus: answerItemLoader.lod === answerItemLoader.eHIGH ? answerItemLoader.item.activeFocus : false
    property real minHeight: questionRow.height + answerArea.minHeight + spacing
    property int tabSequenceIndex: 0
    property int nrQuestionDigits: 2
    property int indentation: 0
    property int answerLength: FormQuestion.LongParagraph

    Row {
        id: questionRow
        width: parent.width-indentation
        anchors.right: parent.right

        spacing: 10

        Label {
            id: questionNumberText
            font.bold: true
            horizontalAlignment: Text.AlignRight
            width: idealAppFontMetrics.averageCharacterWidth * nrQuestionDigits
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
        color: Scrite.app.translucent(primaryColors.c100.background, 0.75)
        border.width: 1
        border.color: Scrite.app.translucent(primaryColors.borderColor, 0.25)
        height: Math.max(minHeight, answerItemLoader.item ? answerItemLoader.item.height : 0)
        property real minHeight: (idealAppFontMetrics.lineSpacing + idealAppFontMetrics.descent + idealAppFontMetrics.ascent) * (answerLength === FormQuestion.ShortParagraph ? 1 : 3)

        MouseArea {
            anchors.fill: parent
            enabled: answerItemLoader.lod === answerItemLoader.eLOW
            onClicked: answerItemLoader.TabSequenceItem.assumeFocus()
        }

        LodLoader {
            id: answerItemLoader
            width: answerArea.width
            height: Math.max(answerArea.minHeight-topPadding-bottomPadding, item ? item.contentHeight+20 : 0)
            lod: eLOW
            TabSequenceItem.manager: tabSequenceManager
            TabSequenceItem.sequence: tabSequenceIndex
            TabSequenceItem.onAboutToReceiveFocus: lod = eHIGH

            lowDetailComponent: TextArea {
                font.pointSize: Scrite.app.idealFontPointSize
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: formField.answer === "" ? formField.placeholderText : formField.answer
                opacity: formField.answer === "" ? 0.5 : 1
                padding: 5
                Transliterator.textDocument: textDocument
                Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
                readOnly: true
                selectByMouse: false
                selectByKeyboard: false
                background: Item { }
                onPressed: Qt.callLater( () => { answerItemLoader.TabSequenceItem.assumeFocus() } )
            }

            highDetailComponent: TextArea {
                id: answerText
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
                Transliterator.textDocumentUndoRedoEnabled: enableUndoRedo
                readOnly: Scrite.document.readOnly
                background: Item { }
                SpecialSymbolsSupport {
                    anchors.top: parent.bottom
                    anchors.left: parent.left
                    textEditor: answerText
                    textEditorHasCursorInterface: true
                    enabled: !Scrite.document.readOnly
                }
                UndoHandler {
                    enabled: !answerText.readOnly && answerText.activeFocus && enableUndoRedo
                    canUndo: answerText.canUndo
                    canRedo: answerText.canRedo
                    onUndoRequest: answerText.undo()
                    onRedoRequest: answerText.redo()
                }

                onActiveFocusChanged: {
                    if(!activeFocus) {
                        if(dialogUnderlay.visible)
                            return
                        answerItemLoader.lod = answerItemLoader.eLOW
                    }
                }
                Component.onCompleted: forceActiveFocus()
                text: formField.answer
                onTextChanged: formField.answer = text
            }
        }
    }
}
