/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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
    property bool hasFocus: item.activeFocus

    signal textEdited(string text)
    signal editingFinished()
    signal highlightRequest()

    FontMetrics {
        id: fontMetrics
    }

    sourceComponent: readOnly ? textViewComponent : textEditComponent

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
            textFormat: markupText === "" ? Text.PlainText : Text.RichText

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
            text: textViewEdit.text
            font: textViewEdit.font
            palette: app.palette
            wrapMode: textViewEdit.wrapMode
            horizontalAlignment: textViewEdit.horizontalAlignment
            verticalAlignment: textViewEdit.verticalAlignment
            onTextChanged: textViewEdit.textEdited(text)
            Keys.onReturnPressed: editingFinished()
            Component.onCompleted: {
                selectAll()
                forceActiveFocus()
            }
            onEditingFinished: textViewEdit.editingFinished()
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
        }
    }
}
