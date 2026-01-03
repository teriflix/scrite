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
    id: root

    spacing: 10

    property int indentation: 0
    property int answerLength: FormQuestion.LongParagraph
    property int tabSequenceIndex: 0
    property int nrQuestionDigits: 2

    property bool cursorVisible: _answerItemLoader.lod === _answerItemLoader.LodLoader.LOD.High ? _answerItemLoader.item.cursorVisible : false
    property bool enableUndoRedo: true
    property bool textFieldHasActiveFocus: _answerItemLoader.lod === _answerItemLoader.LodLoader.LOD.High ? _answerItemLoader.item.activeFocus : false

    property real minHeight: _questionRow.height + _answerArea.minHeight + spacing

    property rect cursorRectangle: {
        const cr = _answerItemLoader.lod === _answerItemLoader.LodLoader.LOD.High ? _answerItemLoader.item.cursorRectangle : Qt.rect(0,0,1,12)
        return mapFromItem(_answerItemLoader, cr.x, cr.y, cr.width, cr.height)
    }

    property alias question: _questionText.text
    property alias questionNumber: _questionNumberText.text
    property string answer
    property string questionKey: questionNumber
    property string placeholderText

    property TextArea textFieldItem: _answerItemLoader.item
    property TabSequenceManager tabSequenceManager

    signal focusNextRequest()
    signal focusPreviousRequest()

    function assumeFocus(pos) {
        _answerItemLoader.assumeFocus(pos)
    }

    Row {
        id: _questionRow

        anchors.right: parent.right

        width: parent.width-indentation

        spacing: 10

        VclLabel {
            id: _questionNumberText

            anchors.top: parent.top

            width: Runtime.idealFontMetrics.averageCharacterWidth * nrQuestionDigits

            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
            horizontalAlignment: Text.AlignRight
        }

        VclLabel {
            id: _questionText

            anchors.top: parent.top

            width: parent.width - _questionNumberText.width - parent.spacing

            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
        }
    }

    Rectangle {
        id: _answerArea

        property real minHeight: (Runtime.idealFontMetrics.lineSpacing + Runtime.idealFontMetrics.descent + Runtime.idealFontMetrics.ascent) * (answerLength == FormQuestion.ShortParagraph ? 1.1 : 3)

        anchors.right: parent.right

        width: _questionText.width
        height: Math.max(minHeight, _answerItemLoader.item ? _answerItemLoader.item.height : 0)

        color: Color.translucent(Runtime.colors.primary.c100.background, 0.75)
        border.width: 1
        border.color: Color.translucent(Runtime.colors.primary.borderColor, 0.25)

        LodLoader {
            id: _answerItemLoader

            TabSequenceItem.manager: tabSequenceManager
            TabSequenceItem.sequence: tabSequenceIndex
            TabSequenceItem.onAboutToReceiveFocus: lod = LodLoader.LOD.High

            function assumeFocus(position) {
                if(lod === LodLoader.LOD.Low)
                    lod = LodLoader.LOD.High
                Qt.callLater( (pos) => { item.assumeFocus(pos) }, position )
            }

            width: _answerArea.width
            height: Math.max(_answerArea.minHeight-topPadding-bottomPadding, item ? item.contentHeight+20 : 0)

            lod: LodLoader.LOD.Low

            lowDetailComponent: TextArea {
                id: _textArea

                SyntaxHighlighter.delegates: [
                    LanguageFontSyntaxHighlighterDelegate {
                        enabled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                        defaultFont: _textArea.font
                    },

                    SpellCheckSyntaxHighlighterDelegate {
                        enabled: root.answer !== ""
                        cursorPosition: _textArea.cursorPosition
                    }
                ]
                SyntaxHighlighter.textDocument: textDocument

                text: root.answer === "" ? root.placeholderText : root.answer
                opacity: root.answer === "" ? 0.5 : 1
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

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
                                const position = _answerItemLoader.item.positionAt(mouse.x, mouse.y)
                                _answerItemLoader.assumeFocus(position)
                            }
            }

            highDetailComponent: TextArea {
                id: _answerText

                function assumeFocus(pos) {
                    forceActiveFocus()
                    cursorPosition = pos < 0 ? TextDocument.lastCursorPosition() : pos
                }

                Component.onCompleted: {
                    forceActiveFocus()
                    // enableSpellCheck()
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

                SyntaxHighlighter.delegates: [
                    LanguageFontSyntaxHighlighterDelegate {
                        enabled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                        defaultFont: _answerText.font
                    },

                    SpellCheckSyntaxHighlighterDelegate {
                        enabled: Runtime.screenplayEditorSettings.enableSpellCheck
                        cursorPosition: _answerText.cursorPosition
                    }
                ]
                SyntaxHighlighter.textDocument: textDocument
                SyntaxHighlighter.textDocumentUndoRedoEnabled: enableUndoRedo

                LanguageTransliterator.popup: LanguageTransliteratorPopup {
                    editorFont: _answerText.font
                }
                LanguageTransliterator.option: Runtime.language.activeTransliterationOption
                LanguageTransliterator.enabled: !readOnly

                text: root.answer
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
                    textEditor: _answerText
                    textEditorHasCursorInterface: true
                    enabled: !Scrite.document.readOnly
                }

                ActionHandler {
                    action: ActionHub.editOptions.find("undo")
                    enabled: !_answerText.readOnly && _answerText.activeFocus && enableUndoRedo && _answerText.canUndo

                    onTriggered: (source) => { _answerText.undo() }
                }

                ActionHandler {
                    action: ActionHub.editOptions.find("redo")
                    enabled: !_answerText.readOnly && _answerText.activeFocus && enableUndoRedo && _answerText.canRedo

                    onTriggered: (source) => { _answerText.redo() }
                }

                TextAreaSpellingSuggestionsMenu {
                    textArea: _answerText
                }

                onTextChanged: root.answer = text

                onActiveFocusChanged: {
                    if(!activeFocus && !persistentSelection) {
                        _answerItemLoader.lod = _answerItemLoader.LodLoader.LOD.Low
                    }
                }
            }
        }
    }
}
