/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
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

    required property Action ensureCursorCenteredAction
    readonly property TextArea editor: _sceneTextEditor
    required property ListView listView // This must be the list-view in which the delegate which creates this part is placed

    readonly property alias currentParagraphType: _private.currentParagraphType
    readonly property alias cursorPosition: _sceneTextEditor.cursorPosition

    signal splitSceneRequest(SceneElement paragraph, int cursorPosition)
    signal mergeWithPreviousSceneRequest()
    signal ensureCentered(Item item, rect area)

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
            _sceneTextEditor.cursorPosition = _sceneTextEditor.length
        else
            _sceneTextEditor.cursorPosition = cursorPosition

        _sceneTextEditor.highlightCursor()
    }

    function beforeZoomLevelChange() {
        _private.beforeZoomLevelChange()
    }

    function afterZoomLevelChange() {
        Runtime.execLater(_private, 100, _private.afterZoomLevelChange)
    }

    TextArea {
        id: _sceneTextEditor

        property bool hasSelection: selectionStart >= 0 && selectionEnd >= 0 && selectionEnd > selectionStart
        property bool controlModifierPressed: false

        signal highlightCursor()

        Keys.onTabPressed: (event) => { _private.handleSceneTextEditorTabPressed(event) }
        Keys.onPressed: (event) => { _private.handleSceneTextEditorKeyPressed(event) }
        Keys.onUpPressed: (event) => { _private.handleSceneTextEditorKeyUpPressed(event) }
        Keys.onDownPressed: (event) => { _private.handleSceneTextEditorKeyDownPressed(event) }

        EventFilter.events: [EventFilter.KeyPress,EventFilter.KeyRelease,EventFilter.FocusOut]
        EventFilter.onFilter: (object, event, result) => {
                                  result.filter = false
                                  result.acceptEvent = false
                                  if(event.type === EventFilter.FocusOut) {
                                      controlModifierPressed = false
                                  } else {
                                      if(event.modifiers & Qt.ControlModifier) {
                                          controlModifierPressed = event.type === EventFilter.KeyPress
                                      }
                                  }
                              }

        DiacriticHandler.enabled: Runtime.allowDiacriticEditing && activeFocus

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

            ActionHandler {
                action: ActionHub.editOptions.find("showCursor")
                enabled: _sceneTextEditor.activeFocus

                onTriggered: (source) => { _cursor.highlight() }
            }

            ActionHandler {
                action: root.ensureCursorCenteredAction
                enabled: _sceneTextEditor.activeFocus

                onTriggered: (source) => { root.ensureCentered(_sceneTextEditor, _sceneTextEditor.cursorRectangle) }
            }

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

            ActionHandler {
                id: _nextFormatHandler

                action: ActionHub.paragraphFormats.find("nextFormat")
                enabled: _sceneTextEditor.activeFocus && !root.readOnly && !_completion.model.hasSuggestion
                onTriggered: (source) => {  } // Do nothing, since we already handle this in Keys.onTabPressed()
            }
        }

        // For handling context menu popup and hyperlinks
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

        onLinkActivated: (link) => {
                             if(activeFocus && controlModifierPressed) {
                                 controlModifierPressed = false
                                 const maxWidth = Math.min(500, Scrite.window.width * 0.5) - 40
                                 const elidedLink = Runtime.idealFontMetrics.elidedText(link, Text.ElideMiddle, maxWidth)
                                 MessageBox.question("Link clicked",
                                                     "The following link was activated. Do you want to open it?\n\n" +
                                                     elidedLink, ["Yes", "No"], (answer) => {
                                                         if(answer === "Yes") {
                                                             Qt.openUrlExternally(link);
                                                         }
                                                     })
                             }
                         }

        onActiveFocusChanged: () => {
                                  Qt.callLater(_private.handleSceneTextEditorFocusChange)
                              }

        onCursorRectangleChanged: () => {
                                      Qt.callLater(_private.ensureSceneTextEditorCursorIsVisible)
                                  }
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

        onCutRequest: () => {
                          _sceneTextEditor.forceActiveFocus()
                          _private.cut()
                      }
        onCopyRequest: () => {
                           _sceneTextEditor.forceActiveFocus()
                           _private.copy()
                       }
        onPasteRequest: () => {
                            _sceneTextEditor.forceActiveFocus()
                            _private.paste()
                        }
        onReloadSceneContentRequest: () => {
                                         _private.reload()
                                     }
        onSplitSceneAtPositionRequest: (position) => {
                                           _sceneTextEditor.forceActiveFocus()
                                           _private.splitSceneAt(position)
                                       }
        onMergeWithPreviousSceneRequest: () => {
                                             _sceneTextEditor.forceActiveFocus()
                                             _private.mergeWithPreviousScene(0)
                                         }

        onTranslateSelection: () => {
                                  _sceneTextEditor.forceActiveFocus()
                                  _private.translateToActiveLanguage.trigger()
                              }
    }

    SceneDocumentBinder {
        id: _sceneDocumentBinder

        readonly property TextArea textArea: _sceneTextEditor

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
                Runtime.execLater(_sceneTextEditor, 150, () => {
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
        cursorPosition: _sceneTextEditor.activeFocus && !root.readOnly ? _sceneTextEditor.cursorPosition : -1
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
                                   if(!_private.firstInitializationDone && !_private.scrollingBetweenScenes) {
                                       _sceneTextEditor.cursorPosition = 0
                                   }
                                   _private.firstInitializationDone = true
                               }

        onRequestCursorPosition: (position) => {
                                     _sceneTextEditor.cursorPosition = position < 0 ? _sceneTextEditor.length : position
                                 }
    }

    Connections {
        target: Runtime.screenplayEditor ? Runtime.screenplayEditor.searchBar : null

        enabled: _sceneSearch.currentResultIndex >= 0

        function onReplaceCurrentRequest(replacementText, searchAgent) {
            _sceneSearch.replaceCurrentSelection(replacementText)
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
            let rect = GMath.uniteRectangles( _sceneTextEditor.positionToRectangle(__searchResultSelection.start),
                                              _sceneTextEditor.positionToRectangle(__searchResultSelection.end) )
            rect = GMath.adjustRectangle(rect, -20, -50, 20, 50)

            root.ensureVisible(_sceneTextEditor, rect)
        }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("cut")
        enabled: !root.readOnly && root.isCurrent && _sceneTextEditor.hasSelection && _sceneTextEditor.activeFocus

        onTriggered: (source) => {
                         _private.cut()
                     }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("copy")
        enabled: !root.readOnly && root.isCurrent && _sceneTextEditor.hasSelection && _sceneTextEditor.activeFocus

        onTriggered: (source) => {
                         _private.copy()
                     }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("paste")
        enabled: !root.readOnly && root.isCurrent && _sceneTextEditor.activeFocus

        onTriggered: (source) => {
                         _private.paste()
                     }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("editSceneContent")
        enabled: !root.readOnly && root.isCurrent && !_sceneTextEditor.activeFocus

        onTriggered: (source) => {
                         _sceneTextEditor.forceActiveFocus()
                     }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("splitScene")
        enabled: !root.readOnly && _private.canSplitScene

        onTriggered: (source) => {
                         Qt.callLater(_private.splitSceneAt, _sceneTextEditor.cursorPosition)
                     }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("mergeScene")
        enabled: !root.readOnly && _private.canJoinToPreviousScene

        onTriggered: (source) => {
                         Qt.callLater(_private.mergeWithPreviousScene, _sceneTextEditor.cursorPosition)
                     }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("pageUp")
        enabled: _sceneTextEditor.activeFocus

        onTriggered: (source) => {
                         if(_sceneTextEditor.cursorPosition > 0) {
                             const pageHeight = root.listView.height * 0.85
                             const cursorRect = _sceneTextEditor.cursorRectangle
                             if(cursorRect.y < pageHeight) {
                                 _private.placeCursorAt(0)
                             } else {
                                 const nextPosition = _sceneTextEditor.positionAt(cursorRect.x, cursorRect.y - pageHeight)
                                 _private.placeCursorAt(nextPosition)
                                 _private.ensureSceneTextEditorCursorIsVisible()
                             }
                         } else {
                             if(_private.scrollPreviousScene.enabled) {
                                 _private.scrollPreviousScene.trigger()
                             }
                         }
                     }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("pageDown")
        enabled: _sceneTextEditor.activeFocus

        onTriggered: (source) => {
                         if(_sceneTextEditor.cursorPosition < _sceneTextEditor.length) {
                             const pageHeight = root.listView.height * 0.85
                             const cursorRect = _sceneTextEditor.cursorRectangle
                             if(cursorRect.y + pageHeight > _sceneTextEditor.height) {
                                 _private.placeCursorAt(_sceneTextEditor.length)
                             } else {
                                 const nextPosition = _sceneTextEditor.positionAt(cursorRect.x, cursorRect.y + pageHeight)
                                 _private.placeCursorAt(nextPosition)
                                 _private.ensureSceneTextEditorCursorIsVisible()
                             }
                         } else {
                             if(_private.scrollNextScene.enabled) {
                                _private.scrollNextScene.trigger()
                             }
                         }
                     }
    }

    ActionHandler {
        property string text: "Translate to " + Runtime.language.active.name

        action: _private.translateToActiveLanguage
        enabled: Runtime.screenplayEditorSettings.allowSelectedTextTranslation && _sceneTextEditor.activeFocus &&
                 _sceneTextEditor.selectionEnd > _sceneTextEditor.selectionStart && _sceneTextEditor.selectionStart >= 0

        onTriggered: (source) => {
                         if(!enabled) return

                         const option = Runtime.language.active.preferredTransliterationOption()
                         if(option && option.inApp) {
                             if(_sceneTextEditor.selectionEnd >= 0 &&
                                _sceneTextEditor.selectionStart >= 0 &&
                                _sceneTextEditor.selectionEnd > _sceneTextEditor.selectionStart) {
                                 const pos = _sceneTextEditor.selectionStart
                                 const txText = option.transliterateParagraph(_sceneTextEditor.selectedText)
                                 if(txText !== "" && txText !== _sceneTextEditor.selectedText) {
                                     _sceneTextEditor.remove(_sceneTextEditor.selectionStart, _sceneTextEditor.selectionEnd)
                                     _sceneTextEditor.insert(pos, txText)
                                 } else {
                                     MessageBox.information("Translation Error", "Couldn't translate the selected text to " + Runtime.language.active.name + ".")
                                 }
                             }
                         }
                     }
    }

    ActionHandler {
        action: ActionHub.markupTools.find("link")

        enabled: _sceneTextEditor.activeFocus && Runtime.allowAppUsage

        onTriggered: (source) => {
                         let cursorPosition = -1
                         let selectedText = _sceneDocumentBinder.selectedText
                         let selectionStart = _sceneTextEditor.selectionStart
                         let selectionEnd = _sceneTextEditor.selectionEnd
                         if(selectedText === "") {
                             selectedText = _sceneDocumentBinder.wordUnderCursor
                             selectionStart = _sceneDocumentBinder.hyperlinkUnderCursorStartPosition
                             selectionEnd = _sceneDocumentBinder.hyperlinkUnderCursorEndPosition
                             cursorPosition = _sceneTextEditor.cursorPosition
                         }
                         if(selectedText === "") {
                             MessageBox.information("Link Error", "Cannot add hyperlink to unselected text.")
                             return
                         }

                         EditHyperlinkDialog.launch(selectedText, _sceneDocumentBinder.textFormat.link, (newLink) => {
                                                        _sceneTextEditor.forceActiveFocus()
                                                        _sceneTextEditor.select(selectionStart, selectionEnd)
                                                        Qt.callLater( () => {
                                                                         _sceneDocumentBinder.textFormat.link = newLink
                                                                         _sceneTextEditor.deselect()
                                                                         if(cursorPosition >= 0) {
                                                                             _sceneTextEditor.cursorPosition = cursorPosition
                                                                         }
                                                                     })
                                                    })
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

        /*function onModelAboutToBeReset() {
            if(_sceneTextEditor.activeFocus)
                _private.cursorPositionBeforeSceneReset = _sceneTextEditor.cursorPosition
        }

        function onModelReset() {
            if(_private.cursorPositionBeforeSceneReset >= 0) {
                Runtime.execLater(_sceneTextEditor, 100, (position) => {
                                    root.assumeFocusAt(position)
                                }, _private.cursorPositionBeforeSceneReset )
                _private.cursorPositionBeforeSceneReset = -1
            }
        }*/

        function onSceneAboutToReset() {
            _private.captureCursorOffset()
        }

        function onSceneReset() {
            Runtime.execLater(_private, Runtime.stdAnimationDuration, _private.restoreCursorOffset)
        }
    }

    // Signal handlers
    onIsCurrentChanged: () => {
                            if(!isCurrent)
                                _sceneTextEditor.deselect()
                        }

    on__SearchBarSaysReplaceCurrent: (replacementText, searchAgent) => {
                                         _sceneSearch.replaceCurrentSelection(replacementText)
                                     }

    // Private stuff
    QtObject {
        id: _private

        readonly property Action scrollNextScene: ActionHub.editOptions.find("scrollNextScene")
        readonly property Action scrollPreviousScene: ActionHub.editOptions.find("scrollPreviousScene")
        readonly property Action focusCursorPosition: ActionHub.editOptions.find("focusCursorPosition")
        readonly property Action translateToActiveLanguage: ActionHub.editOptions.find("translateToActiveLanguage")

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

        onFirstInitializationDoneChanged: {
            if(firstInitializationDone) {
                Qt.callLater(placeFocusCursor)
            }
        }

        property bool focusCursorIsOnMe: focusCursorPosition.sceneElementIndex === root.index
        onFocusCursorIsOnMeChanged: {
            if(firstInitializationDone) {
                Qt.callLater(placeFocusCursor)
            }
        }

        function placeFocusCursor() {
            const cursorPosition = focusCursorPosition.get(root.index)
            if(cursorPosition >= -1)
                root.assumeFocusAt(cursorPosition)
        }

        property int currentParagraphType: _sceneTextEditor.activeFocus ? (currentElement ? currentElement.type : SceneElement.Action) : -1
        property SceneElement currentElement: _sceneTextEditor.activeFocus ? _sceneDocumentBinder.currentElement : null

        function handleSceneTextEditorTabPressed(event) {
            event.accepted = true
            _sceneDocumentBinder.tab()
        }

        function handleSceneTextEditorKeyPressed(event) {
            event.accepted = false

            const shortcut = Gui.shortcut(event.modifiers + event.key)
            const action = ActionHub.editOptions.findByShortcut(shortcut)
            if(action) {
                if(action.enabled) {
                    action.trigger()
                    event.accepted = true
                }
            }
        }

        function handleSceneTextEditorKeyUpPressed(event) {
            if(_sceneTextEditor.hasSelection) {
                event.accepted = _sceneTextEditor.cursorPosition === 0
                return
            }

            event.accepted = !_sceneDocumentBinder.canGoUp() || event.modifiers & Qt.ControlModifier
            if(event.accepted) {
                if(_sceneTextEditor.cursorPosition > 0)
                    _sceneTextEditor.cursorPosition = 0
                else if(scrollPreviousScene.enabled) {
                    _sceneTextEditor.focus = false
                    scrollPreviousScene.trigger()
                }
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
                else if(scrollNextScene.enabled) {
                    _sceneTextEditor.focus = false
                    scrollNextScene.trigger()
                }
            }
        }

        function handleSceneTextEditorFocusChange() {
            if(!Scrite.window.active)
                return

            const activeFocus = _sceneTextEditor.activeFocus
            if(activeFocus) {
                root.ensureVisible(_sceneTextEditor, _sceneTextEditor.cursorRectangle)
                ActionHub.setBinder(_sceneDocumentBinder)
                _sceneTextEditor.highlightCursor()
                Runtime.shoutout(Runtime.announcementIds.sceneTextEditorReceivedFocus, _sceneTextEditor)
            } else {
                ActionHub.resetBinder(_sceneDocumentBinder)
            }
        }

        function ensureSceneTextEditorCursorIsVisible() {
            if(_sceneTextEditor.activeFocus) {
                root.ensureVisible(_sceneTextEditor, _sceneTextEditor.cursorRectangle)
            }
        }

        function ensureSceneTextEditorCursorIsCentered() {
            if(_sceneTextEditor.activeFocus) {
                root.ensureCentered(_sceneTextEditor, _sceneTextEditor.cursorRectangle)
                _sceneTextEditor.highlightCursor()
            }
        }

        function cut() {
            if(root.readOnly)
                return

            if(_sceneTextEditor.hasSelection && _sceneTextEditor.activeFocus) {
                captureCursorOffset()
                _sceneDocumentBinder.copy(_sceneTextEditor.selectionStart, _sceneTextEditor.selectionEnd)
                _sceneTextEditor.remove(_sceneTextEditor.selectionStart, _sceneTextEditor.selectionEnd)
                Runtime.execLater(_private, Runtime.stdAnimationDuration, _private.restoreCursorOffset)
            }
        }

        function copy() {
            if(_sceneTextEditor.hasSelection && _sceneTextEditor.activeFocus)
                _sceneDocumentBinder.copy(_sceneTextEditor.selectionStart, _sceneTextEditor.selectionEnd)
        }

        function paste() {
            if(root.readOnly)
                return

            if(_sceneTextEditor.canPaste && _sceneTextEditor.activeFocus) {
                captureCursorOffset()

                // Fix for https://github.com/teriflix/scrite/issues/195
                // [0.5.2 All] Pasting doesnt replace the selected text #195
                if(_sceneTextEditor.hasSelection)
                    _sceneTextEditor.remove(_sceneTextEditor.selectionStart, _sceneTextEditor.selectionEnd)

                const cursorPositionBeforePaste = _sceneTextEditor.cursorPosition
                const cursorPositionAfterPaste = _sceneDocumentBinder.paste(_sceneTextEditor.cursorPosition)
                if(cursorPositionAfterPaste < 0) {
                    _sceneTextEditor.paste()
                    discardCursorOffset()
                } else {
                    _sceneTextEditor.cursorPosition = 0
                    placeCursorAt(cursorPositionAfterPaste)
                    Runtime.execLater(_private, Runtime.stdAnimationDuration, _private.restoreCursorOffset)
                }
            }
        }

        function reload() {
            _sceneDocumentBinder.preserveScrollAndReload()
        }

        function placeCursorAt(position) {
            if(_sceneTextEditor.activeFocus) {
                _sceneTextEditor.cursorPosition = position > 0 ? 0 : _sceneTextEditor.length
                Qt.callLater( () => {
                                 _sceneTextEditor.cursorPosition = position
                                 _sceneTextEditor.highlightCursor()
                             })
            }
        }

        property real globalCursorY: 0
        property rect localCursorRect: Qt.rect(0,0,0,0)

        function captureCursorOffset() {
            globalCursorY = listView.mapFromItem(_sceneTextEditor, _sceneTextEditor.cursorRectangle).y
            localCursorRect = _sceneTextEditor.cursorRectangle
        }

        function restoreCursorOffset() {
            let localCursorYDelta = _sceneTextEditor.cursorRectangle.y - localCursorRect.y
            let expectedGlobalCursorY = globalCursorY + localCursorYDelta

            let expectedGlobalCursorYDiff = Math.abs(listView.mapFromItem(_sceneTextEditor, _sceneTextEditor.cursorRectangle).y - expectedGlobalCursorY)
            if( expectedGlobalCursorYDiff < Runtime.idealFontMetrics.lineSpacing/2 ) {
                discardCursorOffset()
                if(expectedGlobalCursorY < Runtime.idealFontMetrics.lineSpacing || expectedGlobalCursorY > listView.height-Runtime.idealFontMetrics.lineSpacing)
                    ensureSceneTextEditorCursorIsCentered()
                return
            }

            ensureSceneTextEditorCursorIsCentered()
            discardCursorOffset()
        }

        function discardCursorOffset() {
            globalCursorY = 0
            localCursorRect = Qt.rect(0,0,0,0)
        }

        property Timer reloadSceneContentTimer
        function reloadSceneContent() {
            if(reloadSceneContentTimer)
                reloadSceneContentTimer.restart()
            else
                reloadSceneContentTimer = Runtime.execLater(_sceneDocumentBinder, 1000, function() {
                    _sceneDocumentBinder.preserveScrollAndReload()
                } )
        }

        function acceptCompletionSuggestion(suggestion) {
            if(suggestion !== "") {
                if(_sceneDocumentBinder.hasCompletionPrefixBoundary)
                    _sceneTextEditor.remove(_sceneDocumentBinder.completionPrefixStart, _sceneDocumentBinder.completionPrefixEnd)
                else
                    _sceneTextEditor.remove(_sceneDocumentBinder.currentBlockPosition(), _sceneTextEditor.cursorPosition)
                _sceneTextEditor.insert(_sceneTextEditor.cursorPosition, suggestion)
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
                // MessageBox.information("Split Scene Error",
                //                     "Scene can be split only when cursor is placed at the start of a paragraph.")
                return
            }

            root.splitSceneRequest(_sceneDocumentBinder.currentElement, cursorPosition)
        }

        function mergeWithPreviousScene(cursorPosition) {
            if(!canJoinToPreviousScene || cursorPosition !== _sceneDocumentBinder.cursorPosition) {
                // MessageBox.information("Merge Scene Error",
                //                     "Scene can be merged only when cursor is placed at the start of the first paragraph in a scene.")
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

