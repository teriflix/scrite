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

import QtQuick 2.12
import QtQuick.Controls 2.12
import Scrite 1.0

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
                markupText = SearchAgent.createMarkupText(textViewEdit.text, result.from, result.to, app.palette.highlight, app.palette.highlightedText)
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
            palette: app.palette
            wrapMode: textViewEdit.wrapMode
            horizontalAlignment: textViewEdit.horizontalAlignment
            verticalAlignment: textViewEdit.verticalAlignment
            onTextChanged: textViewEdit.textEdited(text)
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

            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus

            Completer {
                id: completer
                strings: completionStrings
                suggestionMode: Completer.CompleteSuggestion
                completionPrefix: textArea.text
            }

            Item {
                x: parent.cursorRectangle.x
                y: parent.cursorRectangle.y
                width: parent.cursorRectangle.width
                height: parent.cursorRectangle.height
                ToolTip.visible: completer.hasSuggestion && parent.cursorVisible
                ToolTip.text: completer.suggestion
                visible: parent.cursorVisible
            }

            Keys.onEscapePressed: {
                editingFinished()
            }

            Keys.onReturnPressed: {
                if(completer.hasSuggestion) {
                    textArea.text = completer.suggestion
                    textArea.cursorPosition = textArea.length
                } else if(event.modifiers !== Qt.NoModifier)
                    textArea.append("\n")
                else
                    editingFinished()
            }
        }
    }
}
