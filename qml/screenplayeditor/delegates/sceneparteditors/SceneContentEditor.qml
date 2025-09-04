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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/screenplayeditor/delegates/sceneeditorparts/helpers"

AbstractScenePartEditor {
    id: root

    TextArea {
        id: sceneTextEditor

        background: SceneTextEditorBackground { }
        cursorDelegate: TextEditCursorDelegate { }
    }

    SceneDocumentBinder {
        id: sceneDocumentBinder

        function preserveScrollAndReload() {
            var cy = contentView.contentY
            reload()
            contentView.contentY = cy
        }

        function changeCase(textCase) {
            const sstart = sceneTextEditor.selectionStart
            const send = sceneTextEditor.selectionEnd
            const cp = sceneTextEditor.cursorPosition
            changeTextCase(textCase)
            if(sstart >= 0 && send > 0 && send > sstart)
                Utils.execLater(sceneTextEditor, 150, () => {
                                    sceneTextEditor.forceActiveFocus()
                                    sceneTextEditor.select(sstart, send)
                                })
            else if(cp >= 0)
                contentItem.assumeFocusLater(cp, 100)
        }

        scene: root.scene
        shots: Scrite.document.structure.shots
        transitions: Scrite.document.structure.transitions
        textDocument: sceneTextEditor.textDocument
        characterNames: Scrite.document.structure.characterNames
        cursorPosition: sceneTextEditor.activeFocus ? sceneTextEditor.cursorPosition : -1
        applyTextFormat: true
        screenplayFormat: screenplayEditor.screenplayFormat
        screenplayElement: root.screenplayElement
        forceSyncDocument: !sceneTextEditor.activeFocus
        spellCheckEnabled: !root.readOnly && _private.spellCheckEnabledFlag.value
        applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
        autoPolishParagraphs: !root.readOnly && Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
        selectionEndPosition: sceneTextEditor.activeFocus ? sceneTextEditor.selectionEnd : -1
        liveSpellCheckEnabled: sceneTextEditor.activeFocus
        selectionStartPosition: sceneTextEditor.activeFocus ? sceneTextEditor.selectionStart : -1
        autoCapitalizeSentences: !root.readOnly && Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences

        onDocumentInitialized: () => {
            if(!_private.firstInitializationDone && !_private.scrollingBetweenScenes)
                sceneTextEditor.cursorPosition = 0
            _private.firstInitializationDone = true
        }

        onRequestCursorPosition: (position) => {
                                     /* Upon receipt of this signal, lets immediately reset cursor position.
                                        if there is a need for delayed setting of cursor position, let that be
                                        a separate signal emission from the backend. */
                                     // if(position >= 0)
                                     //    contentItem.assumeFocusLater(position, 100)
                                     contentItem.assumeFocusAt(position)
                                 }
    }

    QtObject {
        id: _private

        readonly property ResetOnChange spellCheckEnabledFlag: ResetOnChange {
            to: Runtime.screenplayEditorSettings.enableSpellCheck
            from: false
            delay: 100
            trackChangesOn: _private.numberOfWordsAddedToDict
        }

        property int numberOfWordsAddedToDict: 0
        property int currentParagraphType: currentElement ? currentElement.type : SceneHeading.Action

        property bool scrollingBetweenScenes: false
        property bool firstInitializationDone: false

        onCurrentParagraphTypeChanged: {
            if(currentParagraphType === SceneElement.Action) {
                ruler.paragraphLeftMargin = 0
                ruler.paragraphRightMargin = 0
            } else {
                var elementFormat = screenplayEditor.screenplayFormat.elementFormat(_private.currentParagraphType)
                ruler.paragraphLeftMargin = ruler.leftMargin + pageLayout.contentWidth * elementFormat.leftMargin * Screen.devicePixelRatio
                ruler.paragraphRightMargin = ruler.rightMargin + pageLayout.contentWidth * elementFormat.rightMargin * Screen.devicePixelRatio
            }
        }
    }
}
