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

Item {
    // RichTextEdit is intented to be used as a drop-in-replacement for FlickableTextArea
    property alias textArea: webEngineView
    property bool scrollBarRequired: false
    property bool adjustTextWidthBasedOnScrollBar: false
    property bool undoRedoEnabled: true
    property alias text: webEngineView.content
    property alias font: webEngineView.defaultFont
    property Item tabItem
    property Item backTabItem
    property alias readonly: webEngineView.readOnly
    property alias placeholderText: webEngineView.placeholderText
    property alias readOnly: webEngineView.readOnly
    property Item background
    property TabSequenceManager tabSequenceManager
    property int tabSequenceIndex: 0
    property bool editorHasFocus: scriteWebChannelObject.focus && webEngineView.activeFocus
    clip: true

    onBackgroundChanged: if(background) background.visible = false

    WebEngineView {
        id: webEngineView
        anchors.fill: parent
        anchors.margins: 1
        backgroundColor: "transparent"
        url: "qrc:/richtexteditor.html"
        webChannel: scriteWebChannel

        UndoHandler {
            enabled: !webEngineView.readOnly && scriteWebChannelObject.focus && webEngineView.activeFocus
            canUndo: true
            canRedo: true
            onUndoRequest: scriteWebChannelObject.requestUndo()
            onRedoRequest: scriteWebChannelObject.requestRedo()
        }

        TransliterationHints.allowedMechanisms: TransliterationHints.TextInputSourceMechanism

        // Focus handling
        TabSequenceItem.manager: tabSequenceManager
        TabSequenceItem.sequence: tabSequenceIndex
        TabSequenceItem.onAboutToReceiveFocus: {
            Qt.callLater( () => {
                            scriteWebChannelObject.requestFocus(true)
                         } )
        }

        // made public by aliasing
        property var content // This is a Delta Object: https://quilljs.com/docs/delta/
        property string placeholderText: "Type something here ..."
        property font defaultFont
        property bool readOnly: false

        Component.onCompleted: {
            font.family = "Verdana"
            font.pointSize = Qt.binding( () => { return Scrite.app.idealFontPointSize } )
        }

        // internal private property only
        property bool contentUpdatedFromQuill: false

        // Send messages to the HTML side.
        onWidthChanged: Qt.callLater(reportSizeChange)
        onHeightChanged: Qt.callLater(reportSizeChange)
        function reportSizeChange() {
            scriteWebChannelObject.requestContentSize({"width": width-20, "height": height-20})
        }

        onPlaceholderTextChanged: Qt.callLater(reportPlaceholderTextChange)
        function reportPlaceholderTextChange() {
            scriteWebChannelObject.requestPlaceholderText(placeholderText)
        }

        onDefaultFontChanged: Qt.callLater(reportFontChange)
        function reportFontChange() {
            scriteWebChannelObject.requestFont({"family": defaultFont.family, "size": defaultFont.pointSize})
        }

        onContentChanged: if(!contentUpdatedFromQuill) Qt.callLater(reportContentChange)
        function reportContentChange() {
            scriteWebChannelObject.requestContent(content)
        }

        onReadOnlyChanged: Qt.callLater(reportReadOnlyChange)
        function reportReadOnlyChange() {
            scriteWebChannelObject.requestReadOnly(webEngineView.readOnly)
        }
    }

    QtObject {
        id: scriteWebChannelObject

        WebChannel.id: "scrite"

        readonly property string fontSizeUint: Scrite.app.isMacOSPlatform ? "px" : "pt"
        property bool focus: false

        function contentUpdated(content) {
            webEngineView.contentUpdatedFromQuill = true
            webEngineView.content = content
            webEngineView.contentUpdatedFromQuill = false
        }

        function getInitialParameters() {
            const params = {
                "size": {"width": webEngineView.width-20, "height": webEngineView.height-20},
                "placeholderText": webEngineView.placeholderText,
                "content": webEngineView.content,
                "font": {
                    "family": webEngineView.defaultFont.family,
                    "size": webEngineView.defaultFont.pointSize
                },
                "readOnly": webEngineView.readOnly
            }
            return params
        }

        function log(text) {
            Scrite.app.log(text)
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
        id: scriteWebChannel
        registeredObjects: [scriteWebChannelObject]
    }

    Settings {
        id: richTextEditorSettings
        fileName: Scrite.app.settingsFilePath
        category: "Rich Text Editor"
        property bool languageNoteShown: false
    }

    Loader {
        anchors.fill: parent
        active: richTextEditorSettings.languageNoteShown === false
        sourceComponent: Item {
            Rectangle {
                anchors.fill: parent
                color: primaryColors.c600.background
                opacity: 0.7
            }

            MouseArea {
                anchors.fill: parent
            }

            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(500, parent.width * 0.75)
                spacing: 40

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: "In here, you can only type in English or in languages for which you have configured a text input method from your OS in Scrite Settings."
                    font.bold: true
                    font.pointSize: Scrite.app.idealFontPointSize + 3
                    color: primaryColors.c600.text
                }

                Button2 {
                    text: "Got it!"
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: richTextEditorSettings.languageNoteShown = true
                }
            }
        }
    }
}
