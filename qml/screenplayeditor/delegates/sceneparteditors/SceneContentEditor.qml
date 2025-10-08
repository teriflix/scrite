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
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/floatingdockpanels"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors/helpers"

AbstractScenePartEditor {
    id: root

    readonly property TextArea editor: _sceneTextEditor

    readonly property alias currentParagraphType: _private.currentParagraphType

    signal editSceneHeadingRequest()

    signal jumpToNextScene()
    signal jumpToLastScene()
    signal jumpToFirstScene()
    signal jumpToPreviousScene()
    signal scrollToNextSceneRequest()
    signal scrollToPreviousSceneRequest()

    signal splitSceneRequest(SceneElement paragraph, int cursorPosition)
    signal mergeWithPreviousSceneRequest()

    height: _sceneTextEditor.height

    function assumeFocus() {
        if(!_sceneTextEditor.activeFocus)
            _sceneTextEditor.forceActiveFocus()
        else
            _sceneTextEditor.highlightCursor()
    }

    function assumeFocusAt(cursorPosition) {
        if(!_sceneTextEditor.activeFocus)
            _sceneTextEditor.forceActiveFocus()

        if(cursorPosition === undefined || cursorPosition < 0)
            _sceneTextEditor.cursorPosition = _sceneDocumentBinder.lastCursorPosition()
        else
            _sceneTextEditor.cursorPosition = cursorPosition

        _sceneTextEditor.highlightCursor()
    }

    function beforeZoomLevelChange() {
        _private.beforeZoomLevelChange()
    }

    function afterZoomLevelChange() {
        Utils.execLater(_private, 100, _private.afterZoomLevelChange)
    }

    TextArea {
        id: _sceneTextEditor

        property bool hasSelection: selectionStart >= 0 && selectionEnd >= 0 && selectionEnd > selectionStart
        property bool userIsTyping: false

        signal highlightCursor()

        Keys.onPressed: (event) => { _private.handleSceneTextEditorKeyPressed(event) }
        Keys.onUpPressed: (event) => { _private.handleSceneTextEditorKeyUpPressed(event) }
        Keys.onTabPressed: (event) => { _private.handleSceneTextEditorTabPressed(event) }
        Keys.onDownPressed: (event) => { _private.handleSceneTextEditorKeyDownPressed(event) }
        Keys.onEnterPressed: (event) => { _private.handleSceneTextEditorReturnPressed(event) }
        Keys.onReturnPressed: (event) => { _private.handleSceneTextEditorReturnPressed(event) }

        EventFilter.target: Scrite.app
        EventFilter.active: activeFocus
        EventFilter.events: [EventFilter.KeyPress] // Wheel, ShortcutOverride
        EventFilter.onFilter: (object, event, result) => {
                                  if(activeFocus)
                                    _private.handleSceneTextEditorFilteredEvent(object, event, result)
                              }

        LanguageTransliterator.popup: LanguageTransliteratorPopup {
            editorFont: _sceneTextEditor.font
        }
        LanguageTransliterator.option: Runtime.language.activeTransliterationOption
        LanguageTransliterator.enabled: !readOnly

        width: parent.width

        topPadding: Runtime.sceneEditorFontMetrics.height
        leftPadding: root.pageLeftMargin
        rightPadding: root.pageRightMargin
        bottomPadding: Runtime.sceneEditorFontMetrics.height

        font: Scrite.document.displayFormat.defaultFont2
        palette: Scrite.app.palette
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        placeholderText: activeFocus ? "" : "Click here to type your scene content..."

        focus: true
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
            id: _cursor

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
                id: _completion

                anchors.top: parent.bottom
                anchors.left: parent.left

                fontMetrics: Runtime.sceneEditorFontMetrics
                sceneTextEditor: _sceneTextEditor
                sceneDocumentBinder: _sceneDocumentBinder

                onCompletionRequest: (suggestion) => {
                                         _private.acceptCompletionSuggestion(suggestion)
                                     }
            }

            Connections {
                target: _sceneTextEditor

                function onHighlightCursor() {
                    _cursor.highlight()
                }
            }

            ShortcutsModelRecord {
                group: "Formatting"
                title: _sceneDocumentBinder.nextTabFormatAsString
                enabled: _sceneTextEditor.activeFocus && !root.readOnly && !_completion.model.hasSuggestion
                visible: _sceneTextEditor.activeFocus && !_completion.model.hasSuggestion
                priority: 1
                shortcut: "Tab"
            }
        }

        // For handling context menu popup
        MouseArea {
            anchors.fill: parent

            enabled: !_sceneTextEditor.readOnly && !_spellingSuggestionsMenu.active && !_contextMenu.active && _sceneTextEditor.activeFocus
            cursorShape: Qt.IBeamCursor
            acceptedButtons: Qt.RightButton

            onClicked: (mouse) => {
                           mouse.accepted = true

                           _sceneTextEditor.persistentSelection = true
                           if(!_sceneTextEditor.hasSelection && _sceneDocumentBinder.spellCheckEnabled) {
                               _sceneTextEditor.cursorPosition = _sceneTextEditor.positionAt(mouse.x, mouse.y)
                               if(_sceneDocumentBinder.wordUnderCursorIsMisspelled) {
                                   _spellingSuggestionsMenu.popup()
                                   return
                               }
                           }

                           _sceneTextEditor.persistentSelection = false
                           _contextMenu.popup()
                       }
        }

        onActiveFocusChanged: Qt.callLater(_private.handleSceneTextEditorFocusChange)
        onCursorRectangleChanged: root.ensureVisible(_sceneTextEditor, cursorRectangle)
    }

    SceneTextEditorSpellingSuggestionsMenu {
        id: _spellingSuggestionsMenu

        sceneTextEditor: _sceneTextEditor
        sceneDocumentBinder: _sceneDocumentBinder

        onAddToDictionaryRequest: ++_private.numberOfWordsAddedToDict
        onAddToIgnoreListRequest: ++_private.numberOfWordsAddedToDict
    }

    SceneTextEditorContextMenu {
        id: _contextMenu

        sceneTextEditor: _sceneTextEditor
        sceneDocumentBinder: _sceneDocumentBinder

        splitSceneEnabled: _private.canSplitScene
        mergeWithPreviousSceneEnabled: _private.canJoinToPreviousScene

        onCutRequest: () => { _private.cut() }
        onCopyRequest: () => { _private.copy() }
        onPasteRequest: () => { _private.paste() }
        onReloadSceneContentRequest: () => { } // TODO
        onSplitSceneAtPositionRequest: (position) => { _private.splitSceneAt(position) }
        onMergeWithPreviousSceneRequest: () => { _private.mergeWithPreviousScene(0) }
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
        spellCheckEnabled: !root.readOnly && _spellCheckEnabledFlag.value
        applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
        autoPolishParagraphs: !root.readOnly && Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
        selectionEndPosition: _sceneTextEditor.activeFocus ? _sceneTextEditor.selectionEnd : -1
        liveSpellCheckEnabled: _sceneTextEditor.activeFocus
        selectionStartPosition: _sceneTextEditor.activeFocus ? _sceneTextEditor.selectionStart : -1
        autoCapitalizeSentences: !root.readOnly &&
                                 Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences &&
                                 (Runtime.language.activeTransliterationOption ? !Runtime.language.activeTransliterationOption.inApp : true)

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
                                     root.assumeFocusAt(position)
                                 }
    }

    // Scene content search
    TextDocumentSearch {
        id: _sceneSearch

        Component.onCompleted: Qt.callLater(__highlightSearchResultTextSnippet)

        textDocument: _sceneTextEditor.textDocument
        searchString: __sceneDocumentLoadCount > 0 ? __sceneSearchString : ""
        currentResultIndex: searchResultCount > 0 ? __searchResultIndex : -1

        onHighlightText: (start, end) => {
                             __searchResultSelection = { "start": start, "end": end }
                         }
        onClearHighlight: () => {
                              __searchResultSelection = { "start": -1, "end": -1 }
                          }

        function replaceCurrentSelection(replacementText) {
            if(currentResultIndex >= 0) {
                root.scene.beginUndoCapture()
                replace(replacementText)
                root.scene.endUndoCapture()
            }
        }

        // Internal / Private stuff
        property var __sceneUserData: root.screenplayElement ? root.screenplayElement.userData : undefined
        property var __searchResultSelection: { "start": -1, "end": -1 }

        property int __searchResultIndex: __sceneUserData ? __sceneUserData.sceneResultIndex : -1
        property int __sceneDocumentLoadCount: _sceneDocumentBinder.documentLoadCount

        property string __sceneSearchString: __sceneUserData ? __sceneUserData.searchString : ""

        on__SearchResultSelectionChanged: Qt.callLater(__highlightSearchResultTextSnippet)
        on__SceneDocumentLoadCountChanged: Qt.callLater(__highlightSearchResultTextSnippet)

        function __highlightSearchResultTextSnippet() {
            if(__searchResultSelection.start >= 0 && __searchResultSelection.end >= 0) {
                if(_sceneTextEditor.selectionStart === __searchResultSelection.start &&
                        _sceneTextEditor.selectionEnd === __searchResultSelection.end )
                    return;

                _sceneTextEditor.select(__searchResultSelection.start, __searchResultSelection.end)
                _sceneTextEditor.update()

                Qt.callLater(__scrollToSelection)
            } else {
                _sceneTextEditor.deselect()
            }
        }

        function __scrollToSelection() {
            let rect = Scrite.app.uniteRectangles( _sceneTextEditor.positionToRectangle(__searchResultSelection.start),
                                                     _sceneTextEditor.positionToRectangle(__searchResultSelection.end) )
            rect = Scrite.app.adjustRectangle(rect, -20, -50, 20, 50)

            root.ensureVisible(_sceneTextEditor, rect)
        }
    }

    // All the keyboard shortcuts
    ShortcutsModelRecord {
        id: _splitSceneShortcut

        group: "Edit"
        title: "Split Scene"
        enabled: _private.canSplitScene
        visible: _sceneTextEditor.activeFocus
        priority: 1
        shortcut: Scrite.app.isMacOSPlatform ? "Ctrl+Shift+Return" : "Ctrl+Shift+Enter"

        function trigger() {
            if(enabled)
                _private.splitSceneAt(_sceneTextEditor.cursorPosition)
        }
    }

    ShortcutsModelRecord {
        id: _mergeSceneShortcut

        group: "Edit"
        title: "Join Previous Scene"
        enabled: _private.canJoinToPreviousScene
        visible: _sceneTextEditor.activeFocus
        priority: 1
        shortcut: Scrite.app.isMacOSPlatform ? "Ctrl+Shift+Delete" : "Ctrl+Shift+Backspace"

        function trigger() {
            if(enabled)
                _private.mergeWithPreviousScene(_sceneTextEditor.cursorPosition)
        }
    }

    // Other private objects
    ResetOnChange {
        id: _spellCheckEnabledFlag
        to: Runtime.screenplayEditorSettings.enableSpellCheck
        from: false
        delay: 100
        trackChangesOn: _private.numberOfWordsAddedToDict
    }

    Connections {
        // When a scene is completely reset, we will need to retain the cursor position
        // after reset is complete, and highlight the cursor once done.
        target: root.scene
        enabled: _sceneTextEditor.activeFocus && !_sceneTextEditor.readOnly
        ignoreUnknownSignals: true

        function onSceneRefreshed() {
            _sceneTextEditor.highlightCursor()
        }

        function onModelAboutToBeReset() {
            if(_sceneTextEditor.activeFocus)
                _private.cursorPositionBeforeSceneReset = _sceneTextEditor.cursorPosition
        }

        function onModelReset() {
            if(_private.cursorPositionBeforeSceneReset >= 0) {
                Utils.execLater(_sceneTextEditor, 100, (position) => {
                                    root.assumeFocusAt(position)
                                }, _private.cursorPositionBeforeSceneReset )
                _private.cursorPositionBeforeSceneReset = -1
            }
        }
    }

    // Signal handlers
    on__SearchBarSaysReplaceCurrent: (replacementText, searchAgent) => {
                                         _sceneSearch.replaceCurrentSelection(replacementText)
                                     }

    // Private stuff
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

        property int numberOfWordsAddedToDict: 0
        property int cursorPositionBeforeSceneReset: -1

        property bool scrollingBetweenScenes: false
        property bool firstInitializationDone: false

        property int currentParagraphType: _sceneTextEditor.activeFocus ? (currentElement ? currentElement.type : SceneElement.Action) : -1
        property SceneElement currentElement: _sceneTextEditor.activeFocus ? _sceneDocumentBinder.currentElement : null

        function handleSceneTextEditorKeyUpPressed(event) {
            if(_sceneTextEditor.hasSelection) {
                event.accepted = _sceneTextEditor.cursorPosition === 0
                return
            }

            event.accepted = !_sceneDocumentBinder.canGoUp() || event.modifiers & Qt.ControlModifier
            if(event.accepted) {
                if(_sceneTextEditor.cursorPosition > 0)
                    _sceneTextEditor.cursorPosition = 0
                else
                    root.scrollToPreviousSceneRequest()
            }
        }

        function handleSceneTextEditorKeyDownPressed(event) {
            if(_sceneTextEditor.hasSelection) {
                event.accepted = _sceneTextEditor.cursorPosition >= _sceneTextEditor.length - 1
                return
            }

            event.accepted = !_sceneDocumentBinder.canGoDown() || event.modifiers & Qt.ControlModifier
            if(event.accepted) {
                if(_sceneTextEditor.cursorPosition < _sceneDocumentBinder.lastCursorPosition())
                    _sceneTextEditor.cursorPosition = _sceneDocumentBinder.lastCursorPosition()
                else
                    root.scrollToNextSceneRequest()
            }
        }

        function handleSceneTextEditorReturnPressed(event) {
            event.accepted = false

            // This should be same as
            // if( event.modifiers & Qt.ControlModifier|Qt.ShiftModifier )
            // but for whatever reason, that does not work.
            if(event.modifiers & Qt.ControlModifier && event.modifiers & Qt.ShiftModifier) {
                event.accepted = true
                _splitSceneShortcut.trigger()
                return
            }
        }

        function handleSceneTextEditorTabPressed(event) {
            if(!_sceneTextEditor.readOnly) {
                _sceneDocumentBinder.tab()
                event.accepted = true
            }
        }

        function handleSceneTextEditorKeyPressed(event) {
            event.accepted = false

            // This should be same as
            // if( event.modifiers & Qt.ControlModifier|Qt.ShiftModifier )
            // but for whatever reason, that does not work.
            if(event.modifiers & Qt.ControlModifier && event.modifiers & Qt.ShiftModifier) {
                if( (Scrite.app.isMacOSPlatform && event.key === Qt.Key_Delete) || (event.key === Qt.Key_Backspace) ) {
                    event.accepted = true
                    _mergeSceneShortcut.trigger()
                }
                return
            }

            if(event.modifiers === Qt.ControlModifier) {
                switch(event.key) {
                case Qt.Key_0:
                    event.accepted = true
                    root.editSceneHeadingRequest()
                    break
                case Qt.Key_X:
                    event.accepted = true
                    cut()
                    break
                case Qt.Key_C:
                    event.accepted = true
                    copy()
                    break
                case Qt.Key_V:
                    event.accepted = true
                    paste()
                    break
                case Qt.Key_Home:
                    event.accepted = true
                    root.jumpToFirstScene()
                    break
                case Qt.Key_End:
                    event.accepted = true
                    root.jumpToLastScene()
                    break
                }
            } else {
                switch(event.key) {
                case Qt.Key_PageUp:
                    event.accepted = true
                    root.jumpToPreviousScene()
                    break
                case Qt.Key_PageDown:
                    event.accepted = true
                    root.jumpToNextScene()
                    break
                }
            }
        }

        function handleSceneTextEditorFilteredEvent(object, event, result) {
            if(object === _sceneTextEditor) {
                // Enter, Tab and other keys must not trigger Transliteration. Only space should.
                _sceneTextEditor.userIsTyping = event.hasText
                result.filter = event.controlModifier && (event.key === Qt.Key_Z || event.key === Qt.Key_Y)
            }
        }

        function handleSceneTextEditorFocusChange() {
            if(!Scrite.window.active)
                return

            const activeFocus = _sceneTextEditor.activeFocus
            if(activeFocus) {
                // completionModel.actuallyEnable = true
                root.ensureVisible(_sceneTextEditor, _sceneTextEditor.cursorRectangle)
                // privateData.changeCurrentIndexTo(contentItem.theIndex)
                Runtime.screenplayEditorToolbar.set(_sceneTextEditor, _sceneDocumentBinder)
                _sceneTextEditor.highlightCursor()
                Announcement.shout(Runtime.announcementIds.sceneTextEditorReceivedFocus, _sceneTextEditor)
            } else {
                Runtime.screenplayEditorToolbar.reset(_sceneTextEditor, _sceneDocumentBinder)
            }
        }

        function cut() {
            if(root.readOnly)
                return

            if(_sceneTextEditor.hasSelection) {
                _sceneDocumentBinder.copy(_sceneTextEditor.selectionStart, _sceneTextEditor.selectionEnd)
                _sceneTextEditor.remove(selectionStart, selectionEnd)
                _sceneTextEditor.highlightCursor()
            }
        }

        function copy() {
            if(_sceneTextEditor.hasSelection)
                _sceneDocumentBinder.copy(_sceneTextEditor.selectionStart, _sceneTextEditor.selectionEnd)
        }

        function paste() {
            if(root.readOnly)
                return

            if(_sceneTextEditor.canPaste) {
                // Fix for https://github.com/teriflix/scrite/issues/195
                // [0.5.2 All] Pasting doesnt replace the selected text #195
                if(_sceneTextEditor.hasSelection)
                    _sceneTextEditor.remove(_sceneTextEditor.selectionStart, _sceneTextEditor.selectionEnd)

                const cursorPositionBeforePaste = _sceneTextEditor.cursorPosition
                const cursorPositionAfterPaste = _sceneDocumentBinder.paste(_sceneTextEditor.cursorPosition)
                if(cursorPositionAfterPaste < 0)
                    _sceneTextEditor.paste()
                else
                    _sceneTextEditor.cursorPosition = cursorPositionAfterPaste
                _sceneTextEditor.highlightCursor()
            }
        }

        function reload() {
            _sceneDocumentBinder.preserveScrollAndReload()
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

        function acceptCompletionSuggestion(suggestion) {
            if(suggestion !== "") {
                _sceneTextEditor.userIsTyping = false
                if(_sceneDocumentBinder.hasCompletionPrefixBoundary)
                    _sceneTextEditor.remove(_sceneDocumentBinder.completionPrefixStart, _sceneDocumentBinder.completionPrefixEnd)
                else
                    _sceneTextEditor.remove(_sceneDocumentBinder.currentBlockPosition(), _sceneTextEditor.cursorPosition)
                _sceneTextEditor.insert(_sceneTextEditor.cursorPosition, suggestion)
                _sceneTextEditor.userIsTyping = true
                return true
            }

            return false
        }

        // Splitting and merging should happen from the context of the ScreenplayEditor or the ListView
        // in which all these scenes are shown. So, we should simply emit a signal and cascade it until
        // it reaches that context.
        //
        // Why?
        // Because its possible that this scene-content-editor item may not survive until the end of that
        // operation, and we don't want the split task initiated here to be unceremoniously destroyed before
        // it gets to complete its job.
        function splitSceneAt(cursorPosition) {
            if(!canSplitScene || cursorPosition !== _sceneDocumentBinder.cursorPosition) {
                MessageBox.information("Split Scene Error",
                                    "Scene can be split only when cursor is placed at the start of a paragraph.")
                return
            }

            root.splitSceneRequest(_sceneDocumentBinder.currentElement, cursorPosition)
        }

        function mergeWithPreviousScene(cursorPosition) {
            if(!canJoinToPreviousScene || cursorPosition !== _sceneDocumentBinder.cursorPosition) {
                MessageBox.information("Merge Scene Error",
                                    "Scene can be merged only when cursor is placed at the start of the first paragraph in a scene.")
                return
            }

            root.mergeWithPreviousSceneRequest()
        }

        /**
          It appears that the TextEdit resets its cursorPosition to 0 if font changes on the text
          editor, while it still has active focus. So, we will have to preserve cursor position
          before such changes - and then restore it after the fact. That seems to be the only way
          we can preserve cursorPosition consistently across such changes. This is important for us
          because we have cursorRectangle depending on the cursorPosition, which in turn dictates
          where we show the cursor, the highlighter and so on.
          */
        property int cursorPositionBeforeZoom: -1

        function beforeZoomLevelChange() {
            if(_sceneTextEditor.activeFocus) {
                cursorPositionBeforeZoom = _sceneTextEditor.cursorPosition
                _sceneTextEditor.cursorPosition = cursorPositionBeforeZoom + (cursorPositionBeforeZoom >= _sceneTextEditor.length-1 ? -1 : 1)
            }
        }

        function afterZoomLevelChange() {
            if(cursorPositionBeforeZoom >= 0) {
                _sceneTextEditor.forceActiveFocus()
                _sceneTextEditor.cursorPosition = cursorPositionBeforeZoom
                cursorPositionBeforeZoom = -1
            }
        }
    }
}

