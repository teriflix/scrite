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

Loader {
    id: textViewEdit
    property bool readOnly: true
    property string text
    property alias font: fontMetrics.font
    property var horizontalAlignment: Text.AlignHCenter
    property var verticalAlignment: Text.AlignTop
    property var wrapMode: Text.WordWrap
    property var elide: Text.ElideNone
    property SearchEngine searchEngine
    property int searchSequenceNumber: -1
    property bool hasFocus: item ? item.activeFocus : false
    property var completionStrings: []
    property real contentWidth: item ? item.contentWidth : fontMetrics.advanceWidth(text)
    property bool frameVisible: false
    property alias fontAscent: fontMetrics.ascent
    property alias fontDescent: fontMetrics.descent
    property alias fontHeight: fontMetrics.height
    property real leftPadding: 0
    property real rightPadding: 0
    property real topPadding: 0
    property real bottomPadding: 0
    property color textColor: "black"

    signal textEdited(string text)
    signal editingFinished()
    signal highlightRequest()

    FontMetrics {
        id: fontMetrics
    }

    sourceComponent: visible ? (readOnly ? textViewComponent : textEditComponent) : null

    Component {
        id: textViewComponent

        Text {
            property string markupText
            text: markupText !== "" ? markupText : textViewEdit.text
            font: textViewEdit.font
            wrapMode: textViewEdit.wrapMode
            elide: textViewEdit.elide
            horizontalAlignment: textViewEdit.horizontalAlignment
            verticalAlignment: textViewEdit.verticalAlignment
            padding: 5
            color: textColor
            textFormat: markupText === "" ? Text.PlainText : Text.RichText
            leftPadding: textViewEdit.leftPadding
            topPadding: textViewEdit.topPadding
            rightPadding: textViewEdit.rightPadding
            bottomPadding: textViewEdit.bottomPadding

            property var searchResults: []
            SearchAgent.engine: searchEngine
            SearchAgent.sequenceNumber: searchSequenceNumber
            SearchAgent.searchResultCount: searchResults.length
            SearchAgent.onSearchRequest: searchResults = SearchAgent.indexesOf(string, text)
            SearchAgent.onCurrentSearchResultIndexChanged: {
                if(SearchAgent.currentSearchResultIndex < 0)
                    return
                var result = searchResults[SearchAgent.currentSearchResultIndex]
                markupText = SearchAgent.createMarkupText(textViewEdit.text, result.from, result.to, Scrite.app.palette.highlight, Scrite.app.palette.highlightedText)
                textViewEdit.highlightRequest()
            }
            SearchAgent.onClearSearchRequest: searchResults = []
            SearchAgent.onClearHighlight: markupText = ""
        }
    }

    Component {
        id: textEditComponent

        TextArea {
            id: textArea
            text: textViewEdit.text
            font: textViewEdit.font
            palette: Scrite.app.palette
            wrapMode: textViewEdit.wrapMode
            horizontalAlignment: textViewEdit.horizontalAlignment
            verticalAlignment: textViewEdit.verticalAlignment
            onTextChanged: { textViewEdit.textEdited(text); completionModel.allowEnable = true }
            selectByMouse: true
            selectByKeyboard: true
            background: Rectangle {
                visible: frameVisible
                border.width: 1
                border.color: primaryColors.borderColor
            }
            opacity: activeFocus ? 1 : 0.5
            leftPadding: textViewEdit.leftPadding
            topPadding: textViewEdit.topPadding
            rightPadding: textViewEdit.rightPadding
            bottomPadding: textViewEdit.bottomPadding

            Component.onCompleted: {
                selectAll()
                forceActiveFocus()
            }

            Component.onDestruction: {
                textViewEdit.text = text
                textViewEdit.editingFinished()
            }

            onEditingFinished: {
                textViewEdit.text = text
                textViewEdit.editingFinished()
            }

            Transliterator.defaultFont: font
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
            Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts

            onFocusChanged: completionModel.allowEnable = true

            CompletionModel {
                id: completionModel
                property bool allowEnable: true
                property string suggestion: currentCompletion
                property bool hasSuggestion: count > 0
                enabled: allowEnable && textArea.activeFocus
                sortStrings: false
                strings: completionStrings
                completionPrefix: textArea.text
                filterKeyStrokes: textArea.activeFocus
                onRequestCompletion: {
                    textArea.text = currentCompletion
                    textArea.cursorPosition = textArea.length
                    allowEnable = false
                }
                property bool hasItems: count > 0
                onHasItemsChanged: {
                    if(hasItems)
                        completionViewPopup.open()
                    else
                        completionViewPopup.close()
                }
            }

            Popup {
                id: completionViewPopup
                x: parent.cursorRectangle.x - Scrite.app.boundingRect(completionModel.completionPrefix, parent.font).width
                y: parent.cursorRectangle.y + parent.cursorRectangle.height
                width: Scrite.app.largestBoundingRect(completionModel.strings, textArea.font).width + leftInset + rightInset + leftPadding + rightPadding + 20
                height: completionView.contentHeight + topInset + bottomInset + topPadding + bottomPadding
                focus: false
                closePolicy: Popup.NoAutoClose
                contentItem: ListView {
                    id: completionView
                    model: completionModel
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                    keyNavigationEnabled: false
                    delegate: Text {
                        width: completionView.width-1
                        text: string
                        padding: 5
                        font: textArea.font
                        color: index === completionView.currentIndex ? primaryColors.highlight.text : primaryColors.c10.text
                    }
                    highlight: Rectangle {
                        color: primaryColors.highlight.background
                    }
                    currentIndex: completionModel.currentRow
                    height: contentHeight
                }
            }

            Keys.onEscapePressed: {
                editingFinished()
            }

            Keys.onReturnPressed: {
                if(completionModel.hasSuggestion) {
                    textArea.text = completionModel.suggestion
                    textArea.cursorPosition = textArea.length
                    completionModel.allowEnable = false
                } else if(event.modifiers !== Qt.NoModifier)
                    textArea.append("\n")
                else
                    editingFinished()
            }
        }
    }
}
