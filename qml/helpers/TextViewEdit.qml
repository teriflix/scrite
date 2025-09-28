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

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Loader {
    id: root

    property var completionStrings: []

    property int elide: Text.ElideNone
    property int wrapMode: Text.WordWrap
    property int verticalAlignment: Text.AlignTop
    property int horizontalAlignment: Text.AlignHCenter
    property int searchSequenceNumber: -1

    property bool readOnly: true
    property bool hasFocus: item ? item.activeFocus : false
    property bool frameVisible: false

    property real topPadding: 0
    property real leftPadding: 0
    property real contentWidth: item ? item.contentWidth : _private.fontMetrics.advanceWidth(text)
    property real rightPadding: 0
    property real bottomPadding: 0

    property color textColor: "black"

    property alias font: _private.fontMetrics.font
    property alias fontHeight: _private.fontMetrics.height
    property alias fontAscent: _private.fontMetrics.ascent
    property alias fontDescent: _private.fontMetrics.descent

    property string text

    property SearchEngine searchEngine

    signal textEdited(string text)
    signal editingFinished()
    signal highlightRequest()

    sourceComponent: visible ? (readOnly ? _private.textViewComponent : _private.textEditComponent) : null

    QtObject {
        id: _private

        readonly property FontMetrics fontMetrics: FontMetrics { }

        readonly property Component textViewComponent: VclLabel {
            readonly property bool editorKind: false

            property var searchResults: []
            property string markupText

            SearchAgent.engine: root.searchEngine
            SearchAgent.sequenceNumber: root.searchSequenceNumber
            SearchAgent.searchResultCount: searchResults.length

            SearchAgent.onSearchRequest: (string) => { root.searchResults = SearchAgent.indexesOf(string, text) }
            SearchAgent.onClearHighlight: markupText = ""
            SearchAgent.onClearSearchRequest: searchResults = []
            SearchAgent.onCurrentSearchResultIndexChanged: {
                if(SearchAgent.currentSearchResultIndex < 0)
                    return
                var result = searchResults[SearchAgent.currentSearchResultIndex]
                markupText = SearchAgent.createMarkupText(root.text, result.from, result.to, Scrite.app.palette.highlight, Scrite.app.palette.highlightedText)
                root.highlightRequest()
            }

            text: markupText !== "" ? markupText : root.text
            font: root.font
            elide: root.elide
            color: textColor
            wrapMode: root.wrapMode
            textFormat: markupText === "" ? Text.PlainText : Text.RichText
            horizontalAlignment: root.horizontalAlignment
            verticalAlignment: root.verticalAlignment

            padding: 5
            topPadding: root.topPadding
            leftPadding: root.leftPadding
            rightPadding: root.rightPadding
            bottomPadding: root.bottomPadding
        }

        readonly property Component textEditComponent: TextArea {
            id: _textArea

            readonly property bool editorKind: true

            Component.onCompleted: {
                selectAll()
                forceActiveFocus()
            }

            Component.onDestruction: {
                root.text = text
                root.editingFinished()
            }

            onEditingFinished: {
                root.text = text
                root.editingFinished()
            }

            Transliterator.defaultFont: font
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
            Transliterator.applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts

            text: root.text
            font: root.font
            palette: Scrite.app.palette
            wrapMode: root.wrapMode
            verticalAlignment: root.verticalAlignment
            horizontalAlignment: root.horizontalAlignment

            selectByMouse: true
            selectByKeyboard: true
            background: Rectangle {
                visible: frameVisible
                border.width: 1
                border.color: Runtime.colors.primary.borderColor
            }
            opacity: activeFocus ? 1 : 0.5
            leftPadding: root.leftPadding
            topPadding: root.topPadding
            rightPadding: root.rightPadding
            bottomPadding: root.bottomPadding

            onTextChanged: {
                root.textEdited(text);
                _completionModel.allowEnable = true
            }

            onFocusChanged: _completionModel.allowEnable = true

            CompletionModel {
                id: _completionModel

                property bool hasItems: count > 0
                property bool allowEnable: true
                property bool hasSuggestion: count > 0

                property string suggestion: currentCompletion

                enabled: allowEnable && _textArea.activeFocus
                strings: completionStrings
                sortStrings: false
                completionPrefix: _textArea.text
                filterKeyStrokes: _textArea.activeFocus

                onRequestCompletion: {
                    _textArea.text = currentCompletion
                    _textArea.cursorPosition = _textArea.length
                    allowEnable = false
                }

                onHasItemsChanged: {
                    if(hasItems)
                        _completionViewPopup.open()
                    else
                        _completionViewPopup.close()
                }
            }

            Popup {
                id: _completionViewPopup

                x: parent.cursorRectangle.x - Scrite.app.boundingRect(_completionModel.completionPrefix, parent.font).width
                y: parent.cursorRectangle.y + parent.cursorRectangle.height
                width: Scrite.app.largestBoundingRect(_completionModel.strings, _textArea.font).width + leftInset + rightInset + leftPadding + rightPadding + 20
                height: _completionView.contentHeight + topInset + bottomInset + topPadding + bottomPadding

                focus: false
                closePolicy: Popup.NoAutoClose

                contentItem: ListView {
                    id: _completionView

                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                    height: contentHeight

                    model: _completionModel
                    currentIndex: _completionModel.currentRow
                    keyNavigationEnabled: false

                    delegate: VclLabel {
                        required property int index
                        required property string completionString

                        width: _completionView.width-1

                        text: completionString
                        font: _textArea.font
                        color: _root_elementIndex === _completionView.currentIndex ? Runtime.colors.primary.highlight.text : Runtime.colors.primary.c10.text
                        padding: 5
                    }

                    highlight: Rectangle {
                        color: Runtime.colors.primary.highlight.background
                    }
                }
            }

            Keys.onEscapePressed: {
                editingFinished()
            }

            Keys.onReturnPressed: {
                if(_completionModel.hasSuggestion) {
                    _textArea.text = _completionModel.suggestion
                    _textArea.cursorPosition = _textArea.length
                    _completionModel.allowEnable = false
                } else if(event.modifiers !== Qt.NoModifier)
                    _textArea.append("\n")
                else
                    editingFinished()
            }
        }
    }
}
