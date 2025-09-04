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
import "qrc:/qml/floatingdockpanels"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors/helpers"

AbstractScenePartEditor {
    id: root

    signal scrollToNextSceneRequest()
    signal scrollToPreviousSceneRequest()

    height: _sceneTextEditor.height

    TextArea {
        id: _sceneTextEditor

        property bool hasSelection: selectionStart >= 0 && selectionEnd >= 0 && selectionEnd > selectionStart
        property bool userIsTyping: false

        EventFilter.target: Scrite.app
        EventFilter.active: activeFocus
        EventFilter.events: [EventFilter.KeyPress] // Wheel, ShortcutOverride
        EventFilter.onFilter: {
            if(object === _sceneTextEditor) {
                // Enter, Tab and other keys must not trigger
                // Transliteration. Only space should.
                userIsTyping = event.hasText
                // completionModel.actuallyEnable = event.hasText
                result.filter = event.controlModifier && (event.key === Qt.Key_Z || event.key === Qt.Key_Y)
            } else if(event.key === Qt.Key_PageUp || event.key === Qt.Key_PageDown) {
                if(event.key === Qt.Key_PageUp)
                    root.scrollToPreviousSceneRequest()
                else
                    root.scrollToNextSceneRequest()
                result.filter = true
                result.acceptEvent = true
            }
        }

        Transliterator.enabled: root.scene && !root.scene.isBeingReset && userIsTyping
        Transliterator.textDocument: textDocument
        Transliterator.cursorPosition: cursorPosition
        Transliterator.hasActiveFocus: activeFocus
        Transliterator.spellCheckEnabled: false // SceneDocumentBinder handles it separately.
        Transliterator.applyLanguageFonts: false // SceneDocumentBinder handles it separately.
        Transliterator.onAboutToTransliterate: {
            root.scene.beginUndoCapture(false)
            root.scene.undoRedoEnabled = false
        }
        Transliterator.onFinishedTransliterating: {
            root.scene.endUndoCapture()
            root.scene.undoRedoEnabled = true
        }

        width: parent.width

        topPadding: Runtime.sceneEditorFontMetrics.height
        leftPadding: root.pageLeftMargin
        rightPadding: root.pageRightMargin
        bottomPadding: Runtime.sceneEditorFontMetrics.height

        font: Scrite.document.displayFormat.defaultFont2
        palette: Scrite.app.palette
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        placeholderText: activeFocus ? "" : "Click here to type your scene content..."

        readOnly: root.readOnly
        selectByMouse: true
        selectByKeyboard: true
        persistentSelection: true

        background: SceneTextEditorBackground {
            zoomLevel: root.zoomLevel
            sceneTextEditor: _sceneTextEditor
            sceneDocumentBinder: _sceneDocumentBinder
        }
        cursorDelegate: TextEditCursorDelegate {
            textEdit: _sceneTextEditor

            SpecialSymbolsSupport {
                anchors.top: parent.bottom
                anchors.left: parent.left

                enabled: !root.readOnly
                textEditor: _sceneTextEditor
                includeEmojis: true
                textEditorHasCursorInterface: true
            }

            SceneTextEditorCompletionPopup {
                anchors.bottom: parent.bottom
                anchors.verticalCenter: parent.verticalCenter

                sceneTextEditor: _sceneTextEditor
                sceneDocumentBinder: _sceneDocumentBinder
            }

            SceneTextEditorSpellingSuggestionsMenu {
                anchors.bottom: parent.bottom
                anchors.verticalCenter: parent.verticalCenter

                sceneTextEditor: _sceneTextEditor
                sceneDocumentBinder: _sceneDocumentBinder

                onAddToDictionaryRequest: ++_private.numberOfWordsAddedToDict
                onAddToIgnoreListRequest: ++_private.numberOfWordsAddedToDict
            }

            SceneTextEditorContextMenu {
                anchors.bottom: parent.bottom
                anchors.verticalCenter: parent.verticalCenter

                sceneTextEditor: _sceneTextEditor
                sceneDocumentBinder: _sceneDocumentBinder

                splitSceneEnabled: _private.canSplitScene
                mergeWithPreviousSceneEnabled: _private.canJoinToPreviousScene

                onCutRequest: () => { } // TODO
                onCopyRequest: () => { } // TODO
                onPasteRequest: () => { } // TODO
                onReloadSceneContentRequest: () => { } // TODO
                onSplitSceneAtPositionRequest: (position) => { } // TODO
                onMergeWithPreviousSceneRequest: () => { } // TODO
            }
        }

        onActiveFocusChanged: Qt.callLater(_private.handleSceneTextEditorFocusChange)
        onCursorRectangleChanged: root.ensureVisible(_sceneTextEditor, cursorRectangle)
    }

    SceneDocumentBinder {
        id: _sceneDocumentBinder

        function preserveScrollAndReload() {
            var cy = contentView.contentY
            reload()
            contentView.contentY = cy
        }

        function changeCase(textCase) {
            const sstart = _sceneTextEditor.selectionStart
            const send = _sceneTextEditor.selectionEnd
            const cp = _sceneTextEditor.cursorPosition
            changeTextCase(textCase)
            if(sstart >= 0 && send > 0 && send > sstart)
                Utils.execLater(_sceneTextEditor, 150, () => {
                                    _sceneTextEditor.forceActiveFocus()
                                    _sceneTextEditor.select(sstart, send)
                                })
            else if(cp >= 0)
                contentItem.assumeFocusLater(cp, 100)
        }

        scene: root.scene
        shots: Scrite.document.structure.shots
        transitions: Scrite.document.structure.transitions
        textDocument: _sceneTextEditor.textDocument
        characterNames: Scrite.document.structure.characterNames
        cursorPosition: _sceneTextEditor.activeFocus ? _sceneTextEditor.cursorPosition : -1
        applyTextFormat: true
        screenplayFormat: Scrite.document.displayFormat
        screenplayElement: root.screenplayElement
        forceSyncDocument: !_sceneTextEditor.activeFocus
        spellCheckEnabled: !root.readOnly && _private.spellCheckEnabledFlag.value
        applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
        autoPolishParagraphs: !root.readOnly && Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
        selectionEndPosition: _sceneTextEditor.activeFocus ? _sceneTextEditor.selectionEnd : -1
        liveSpellCheckEnabled: _sceneTextEditor.activeFocus
        selectionStartPosition: _sceneTextEditor.activeFocus ? _sceneTextEditor.selectionStart : -1
        autoCapitalizeSentences: !root.readOnly && Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences

        onDocumentInitialized: () => {
            if(!_private.firstInitializationDone && !_private.scrollingBetweenScenes)
                _sceneTextEditor.cursorPosition = 0
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

        property bool canSplitScene: _sceneTextEditor.activeFocus &&
                                     !root.readOnly &&
                                     _sceneDocumentBinder.currentElement &&
                                     _sceneDocumentBinder.currentElementCursorPosition === 0 &&
                                     Runtime.screenplayAdapter.isSourceScreenplay // TODO: We need to replace this with something else when we begin using ScreenplayAdapter for filtered scene editing.
        property bool canJoinToPreviousScene: _sceneTextEditor.activeFocus &&
                                              !root.readOnly &&
                                              _sceneTextEditor.cursorPosition === 0
                                              && root.index > 0

        readonly property ResetOnChange spellCheckEnabledFlag: ResetOnChange {
            to: Runtime.screenplayEditorSettings.enableSpellCheck
            from: false
            delay: 100
            trackChangesOn: _private.numberOfWordsAddedToDict
        }

        property int numberOfWordsAddedToDict: 0

        property bool scrollingBetweenScenes: false
        property bool firstInitializationDone: false

        function handleSceneTextEditorFocusChange() {
            if(!Scrite.window.active)
                return

            const activeFocus = _sceneTextEditor.activeFocus
            if(activeFocus) {
                // completionModel.actuallyEnable = true
                root.ensureVisible(_sceneTextEditor, _sceneTextEditor.cursorRectangle)
                // privateData.changeCurrentIndexTo(contentItem.theIndex)
                Runtime.screenplayEditorToolbar.set(sceneTextEditor, sceneDocumentBinder)
                FloatingMarkupToolsDock.sceneDocumentBinder = sceneDocumentBinder
                // justReceivedFocus = true
                Announcement.shout(Runtime.announcementIds.sceneTextEditorReceivedFocus, _sceneTextEditor)
            } else {
                Runtime.screenplayEditorToolbar.reset(_sceneTextEditor, _sceneDocumentBinder)
                if(FloatingMarkupToolsDock.sceneDocumentBinder === _sceneDocumentBinder)
                    FloatingMarkupToolsDock.sceneDocumentBinder = null
            }
        }

        property Timer reloadSceneContentTimer
        function reloadSceneContent() {
            if(reloadSceneContentTimer)
                reloadSceneContentTimer.restart()
            else
                reloadSceneContentTimer = Utils.execLater(_sceneDocumentBinder, 1000, function() {
                    _sceneDocumentBinder.preserveScrollAndReload()
                } )
        }

        property int currentParagraphType: currentElement ? currentElement.type : SceneHeading.Action
        onCurrentParagraphTypeChanged: {
            // TODO: Get ruler margins out in a better way
            // if(currentParagraphType === SceneElement.Action) {
            //     ruler.paragraphLeftMargin = 0
            //     ruler.paragraphRightMargin = 0
            // } else {
            //     var elementFormat = screenplayEditor.screenplayFormat.elementFormat(_private.currentParagraphType)
            //     ruler.paragraphLeftMargin = ruler.leftMargin + pageLayout.contentWidth * elementFormat.leftMargin * Screen.devicePixelRatio
            //     ruler.paragraphRightMargin = ruler.rightMargin + pageLayout.contentWidth * elementFormat.rightMargin * Screen.devicePixelRatio
            // }
        }
    }
}
