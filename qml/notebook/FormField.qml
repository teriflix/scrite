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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

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

    signal focusNextRequest()
    signal focusPreviousRequest()

    function assumeFocus(pos) {
        answerItemLoader.assumeFocus(pos)
    }

    Row {
        id: questionRow
        width: parent.width-indentation
        anchors.right: parent.right

        spacing: 10

        VclLabel {
            id: questionNumberText
            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
            horizontalAlignment: Text.AlignRight
            width: Runtime.idealFontMetrics.averageCharacterWidth * nrQuestionDigits
            anchors.top: parent.top
        }

        VclLabel {
            id: questionText
            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
            width: parent.width - questionNumberText.width - parent.spacing
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            anchors.top: parent.top
        }
    }

    Rectangle {
        id: answerArea
        width: questionText.width
        anchors.right: parent.right
        color: Scrite.app.translucent(Runtime.colors.primary.c100.background, 0.75)
        border.width: 1
        border.color: Scrite.app.translucent(Runtime.colors.primary.borderColor, 0.25)
        height: Math.max(minHeight, answerItemLoader.item ? answerItemLoader.item.height : 0)
        property real minHeight: (Runtime.idealFontMetrics.lineSpacing + Runtime.idealFontMetrics.descent + Runtime.idealFontMetrics.ascent) * (answerLength == FormQuestion.ShortParagraph ? 1.1 : 3)

        LodLoader {
            id: answerItemLoader
            width: answerArea.width
            height: Math.max(answerArea.minHeight-topPadding-bottomPadding, item ? item.contentHeight+20 : 0)
            lod: eLOW
            TabSequenceItem.manager: tabSequenceManager
            TabSequenceItem.sequence: tabSequenceIndex
            TabSequenceItem.onAboutToReceiveFocus: lod = eHIGH

            function assumeFocus(position) {
                if(lod === eLOW)
                    lod = eHIGH
                Qt.callLater( (pos) => { item.assumeFocus(pos) }, position )
            }

            lowDetailComponent: TextArea {
                id: _textArea

                Transliterator.enabled: false
                Transliterator.defaultFont: font
                Transliterator.textDocument: textDocument
                Transliterator.applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                Transliterator.spellCheckEnabled: formField.answer !== ""

                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: formField.answer === "" ? formField.placeholderText : formField.answer
                opacity: formField.answer === "" ? 0.5 : 1

                topPadding: 5
                leftPadding: 5
                rightPadding: 5
                bottomPadding: 5

                readOnly: true
                selectByMouse: false
                selectByKeyboard: false

                font.pointSize: Runtime.idealFontMetrics.font.pointSize

                background: Item { }

                onPressed:  (mouse) => {
                                const position = answerItemLoader.item.positionAt(mouse.x, mouse.y)
                                answerItemLoader.assumeFocus(position)
                            }
            }

            highDetailComponent: TextArea {
                id: answerText

                function assumeFocus(pos) {
                    forceActiveFocus()
                    cursorPosition = pos < 0 ? TextDocument.lastCursorPosition() : pos
                }

                Component.onCompleted: {
                    forceActiveFocus()
                    enableSpellCheck()
                }


                Keys.onUpPressed: (event) => {
                                      if(TextDocument.canGoUp())
                                          event.accepted = false
                                      else {
                                          event.accepted = true
                                          Qt.callLater(focusPreviousRequest)
                                      }
                                  }

                Keys.onDownPressed: (event) => {
                                        if(TextDocument.canGoDown())
                                            event.accepted = false
                                        else {
                                            event.accepted = true
                                            Qt.callLater(focusNextRequest)
                                        }
                                    }

                Transliterator.enabled: false
                Transliterator.defaultFont: font
                Transliterator.textDocument: textDocument
                Transliterator.cursorPosition: cursorPosition
                Transliterator.hasActiveFocus: activeFocus
                Transliterator.applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                Transliterator.textDocumentUndoRedoEnabled: enableUndoRedo
                Transliterator.spellCheckEnabled: Runtime.screenplayEditorSettings.enableSpellCheck

                LanguageTransliterator.popup: LanguageTransliteratorPopup {
                    editorFont: answerText.font
                }
                LanguageTransliterator.option: Runtime.language.activeTransliterationOption
                LanguageTransliterator.enabled: !readOnly

                text: formField.answer
                readOnly: Scrite.document.readOnly
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                selectByMouse: true
                selectByKeyboard: true

                topPadding: 5
                leftPadding: 5
                rightPadding: 5
                bottomPadding: 5

                font.pointSize: Runtime.idealFontMetrics.font.pointSize

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

                TextAreaSpellingSuggestionsMenu { }

                onTextChanged: formField.answer = text

                onActiveFocusChanged: {
                    if(!activeFocus && !persistentSelection) {
                        answerItemLoader.lod = answerItemLoader.eLOW
                    }
                }
            }
        }
    }
}
