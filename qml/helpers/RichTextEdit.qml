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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtWebEngine 1.10
import QtWebChannel 1.15
import Qt.labs.settings 1.0

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Item {
    id: root

    // RichTextEdit is intented to be used as a drop-in-replacement for FlickableTextArea
    property int tabSequenceIndex: 0

    property bool adjustTextWidthBasedOnScrollBar: false
    property bool editorHasFocus: _scriteWebChannelObject.focus && _webEngineView.activeFocus
    property bool scrollBarRequired: false
    property bool undoRedoEnabled: true

    property alias font: _webEngineView.defaultFont
    property alias placeholderText: _webEngineView.placeholderText
    property alias readOnly: _webEngineView.readOnly
    property alias readonly: _webEngineView.readOnly
    property alias text: _webEngineView.content
    property alias textArea: _webEngineView

    property TabSequenceManager tabSequenceManager

    clip: true

    WebEngineView {
        id: _webEngineView

        // made public by aliasing
        property var content // This is a Delta Object: https://quilljs.com/docs/delta/
        property string placeholderText: "Type something here ..."
        property font defaultFont
        property bool readOnly: false

        // internal private property only
        property bool contentUpdatedFromQuill: false

        // Focus handling
        TabSequenceItem.manager: tabSequenceManager
        TabSequenceItem.sequence: tabSequenceIndex
        TabSequenceItem.onAboutToReceiveFocus: {
            Qt.callLater( () => {
                            _scriteWebChannelObject.requestFocus(true)
                         } )
        }

        Component.onCompleted: {
            font.family = "Verdana"
            font.pointSize = Qt.binding( () => { return Runtime.idealFontMetrics.font.pointSize } )

            if(!Runtime.richTextEditorSettings.languageNoteShown) {
                Runtime.richTextEditorSettings.languageNoteShown = true
                MessageBox.information("Limited Language Support",
                                       "In here, you can only type in English or in languages for which you have configured a text input method from your OS in Scrite Settings.")
            }
        }

        anchors.fill: parent

        backgroundColor: "transparent"
        url: "qrc:/richtexteditor.html"
        webChannel: _scriteWebChannel

        ActionHandler {
            action: ActionHub.editOptions.find("undo")
            enabled: !_webEngineView.readOnly && _scriteWebChannelObject.focus && _webEngineView.activeFocus

            onTriggered: (source) => { _scriteWebChannelObject.requestUndo() }
        }

        ActionHandler {
            action: ActionHub.editOptions.find("redo")
            enabled: !_webEngineView.readOnly && _scriteWebChannelObject.focus && _webEngineView.activeFocus

            onTriggered: (source) => { _scriteWebChannelObject.requestRedo() }
        }

        // Send messages to the HTML side.
        onWidthChanged: Qt.callLater(reportSizeChange)
        onHeightChanged: Qt.callLater(reportSizeChange)
        function reportSizeChange() {
            _scriteWebChannelObject.requestContentSize({"width": width-20, "height": height-20})
        }

        onPlaceholderTextChanged: Qt.callLater(reportPlaceholderTextChange)
        function reportPlaceholderTextChange() {
            _scriteWebChannelObject.requestPlaceholderText(placeholderText)
        }

        onDefaultFontChanged: Qt.callLater(reportFontChange)
        function reportFontChange() {
            _scriteWebChannelObject.requestFont({"family": defaultFont.family, "size": defaultFont.pointSize})
        }

        onContentChanged: if(!contentUpdatedFromQuill) Qt.callLater(reportContentChange)
        function reportContentChange() {
            _scriteWebChannelObject.requestContent(content)
        }

        onReadOnlyChanged: Qt.callLater(reportReadOnlyChange)
        function reportReadOnlyChange() {
            _scriteWebChannelObject.requestReadOnly(_webEngineView.readOnly)
        }
    }

    QtObject {
        id: _scriteWebChannelObject

        WebChannel.id: "scrite"

        readonly property string fontSizeUint: Platform.isMacOSDesktop ? "px" : "pt"
        property bool focus: false

        function contentUpdated(content) {
            _webEngineView.contentUpdatedFromQuill = true
            _webEngineView.content = content
            _webEngineView.contentUpdatedFromQuill = false
        }

        function getInitialParameters() {
            const params = {
                "size": {"width": _webEngineView.width-20, "height": _webEngineView.height-20},
                "placeholderText": _webEngineView.placeholderText,
                "content": _webEngineView.content,
                "font": {
                    "family": _webEngineView.defaultFont.family,
                    "size": _webEngineView.defaultFont.pointSize
                },
                "readOnly": _webEngineView.readOnly
            }
            return params
        }

        function log(text) {
            Gui.log(text)
        }

        function openUrlExternally(url) {
            Qt.openUrlExternally(url)
        }

        signal requestPlaceholderText(string text)
        signal requestContentSize(var size)
        signal requestFont(var font)
        signal requestContent(var content)
        signal requestReadOnly(bool readOnly)
        signal requestFocus(bool focus)
        signal requestUndo()
        signal requestRedo()
    }

    WebChannel {
        id: _scriteWebChannel

        registeredObjects: [_scriteWebChannelObject]
    }
}
