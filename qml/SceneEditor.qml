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

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import Scrite 1.0

Item {
    id: sceneEditor
    property Scene scene
    property bool  readOnly: false
    property SceneElementFormat sceneHeadingFormat: scriteDocument.formatting.elementFormat(SceneElement.Heading)
    property alias binder: sceneDocumentBinder
    property Item  editor: sceneContentEditor
    property bool  editorHasActiveFocus: activeFocusBinder.get
    property real  fullHeight: (sceneHeadingLoader.active ? sceneHeadingArea.height : 0) + (sceneContentEditor ? (sceneContentEditor.totalHeight+contentEditorArea.anchors.topMargin+(sceneContentEditor.length===0 ? 10 : 0)) : 0)
    property color backgroundColor: scene ? Qt.tint(scene.color, "#F7FFFFFF") : "white"
    property bool  scrollable: true
    property bool  showOnlyEnabledSceneHeadings: false
    property bool  allowSplitSceneRequest: false
    property real  sceneHeadingHeight: sceneHeadingArea.height

    signal assumeFocus()
    signal assumeFocusAt(int pos)
    signal requestScrollUp()
    signal requestScrollDown()
    signal splitSceneRequest(SceneElement sceneElement, int textPosition)

    readonly property real margin: Math.max( Math.round((width-sceneEditorFontMetrics.pageWidth)/2), sceneEditorFontMetrics.height*2 )
    readonly property real padding: sceneEditorFontMetrics.paragraphMargin + margin

    DelayedPropertyBinder {
        id: activeFocusBinder
        initial: false
        set: sceneContentEditor.activeFocus
    }

    Item {
        id: sceneHeadingArea
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        height: sceneHeadingLoader.active ? sceneHeadingLoader.height : 0

        Loader {
            id: sceneHeadingLoader
            width: parent.width
            height: loaderHeight.get
            property bool viewOnly: true
            active: scene !== null && scene.heading !== null && (showOnlyEnabledSceneHeadings ? scene.heading.enabled : true)
            sourceComponent: sceneHeadingComponent.get
            clip: true

            DelayedPropertyBinder {
                id: loaderHeight
                initial: 40
                set: Math.max(sceneHeadingLoader.viewOnly && sceneHeadingLoader.item ? sceneHeadingLoader.item.contentHeight : initial, initial)
            }

            DelayedPropertyBinder {
                id: sceneHeadingComponent
                initial: sceneHeadingDisabled
                set: {
                    if(scene !== null && scene.heading !== null && scene.heading.enabled)
                        return sceneHeadingLoader.viewOnly ? sceneHeadingViewer : sceneHeadingEditor
                    return sceneHeadingDisabled
                }
            }
        }
    }

    property TextArea sceneContentEditor

    Rectangle {
        id: contentEditorArea
        anchors.left: parent.left
        anchors.top: sceneHeadingArea.bottom
        anchors.topMargin: sceneHeadingLoader.active ? radius/2 : 0
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: backgroundColor
        radius: 0

        SceneDocumentBinder {
            id: sceneDocumentBinder
            screenplayFormat: scriteDocument.formatting
            scene: sceneEditor.scene
            textDocument: sceneContentEditor.textDocument
            cursorPosition: sceneContentEditor.cursorPosition
            characterNames: scriteDocument.structure.characterNames
            onDocumentInitialized: sceneContentEditor.cursorPosition = 0
            forceSyncDocument: !sceneContentEditor.activeFocus
            onRequestCursorPosition: app.execLater(100, function() { assumeFocusAt(position) })
        }

        Loader {
            id: contentEditorLoader
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: scrollable ? 0 : 10
            active: true
            clip: true
            sourceComponent: scrollable ? scrollableSceneContentEditorComponent : sceneContentEditorComponent
        }
    }

    onSceneChanged: {
        contentEditorLoader.active = false
        contentEditorLoader.active = true
        scene.undoRedoEnabled = Qt.binding( function() {
            return editorHasActiveFocus || sceneHeadingLoader.viewOnly === false
        })
    }

    Component {
        id: scrollableSceneContentEditorComponent

        ScrollView {
            id: sceneContentScrollView
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            ScrollBar.vertical.opacity: ScrollBar.vertical.active ? 1 : 0.2
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            contentWidth: width
            contentHeight: editorLoader.height

            Loader {
                id: editorLoader
                width: sceneContentScrollView.width-20
                height: item ? item.totalHeight : 0
                sourceComponent: sceneContentEditorComponent
            }

            DelayedPropertyBinder {
                initial: Qt.rect(0,0,0,0)
                set: editorLoader.item ? editorLoader.item.cursorRectangle : initial
                onGetChanged: sceneContentScrollView.adjustScroll(get)
            }

            function adjustScroll(rect) {
                if(rect.top < contentItem.contentY)
                    contentItem.contentY = Math.max(rect.top - rect.height, 0)
                else if(rect.bottom > contentItem.contentY + height)
                    contentItem.contentY = Math.min(rect.bottom + rect.height - height, contentHeight)
            }
        }
    }

    Component {
        id: sceneContentEditorComponent

        TextArea {
            id: sceneTextArea
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            renderType: Text.NativeRendering
            readOnly: sceneEditor.readOnly
            property real totalHeight: contentHeight + topPadding + bottomPadding
            background: Rectangle {
                color: backgroundColor

                BorderImage {
                    source: "../icons/content/shadow.png"
                    anchors.fill: contentArea
                    horizontalTileMode: BorderImage.Stretch
                    verticalTileMode: BorderImage.Stretch
                    anchors { leftMargin: -11; topMargin: -11; rightMargin: -10; bottomMargin: -10 }
                    border { left: 21; top: 21; right: 21; bottom: 21 }
                    opacity: sceneTextArea.activeFocus ? 0.25 : 0.1
                }

                Rectangle {
                    id: contentArea
                    radius: 8
                    border.width: 1
                    border.color: sceneTextArea.activeFocus ? backgroundColor : "#cfd8dc"
                    anchors.fill: parent
                    anchors.leftMargin: sceneEditor.margin
                    anchors.rightMargin: sceneEditor.margin
                    anchors.topMargin: sceneEditorFontMetrics.height
                    anchors.bottomMargin: sceneEditorFontMetrics.height
                    color: sceneTextArea.activeFocus ? "white" : Qt.rgba(0,0,0,0)
                }
            }
            leftPadding: sceneEditor.padding
            rightPadding: sceneEditor.padding
            topPadding: sceneEditorFontMetrics.height*2
            bottomPadding: sceneEditorFontMetrics.height*2
            palette: app.palette
            selectByMouse: true
            selectByKeyboard: true
            EventFilter.events: [31,51,6] // Wheel, ShortcutOverride
            EventFilter.onFilter: {
                if(event.type === 31) {
                    result.acceptEvent = false
                    result.filter = !scrollable
                } else if(event.type === 51) {
                    result.acceptEvent = false
                    result.filter = true
                } else if(event.type === 6)
                    sceneTextArea.userIsTyping = event.hasText
            }
            font: scriteDocument.formatting.defaultFont
            property bool userIsTyping: false
            Transliterator.enabled: scene && !scene.isBeingReset && userIsTyping
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
            Transliterator.onAboutToTransliterate: {
                scene.beginUndoCapture(false)
                scene.undoRedoEnabled = false
            }
            Transliterator.onFinishedTransliterating: {
                app.execLater(0, function() {
                    scene.endUndoCapture()
                    scene.undoRedoEnabled = true
                })
            }
            Component.onCompleted: sceneContentEditor = sceneTextArea

            Completer {
                id: completer
                strings: sceneDocumentBinder.autoCompleteHints
                completionPrefix: sceneDocumentBinder.completionPrefix
            }

            Connections {
                target: sceneEditor
                onAssumeFocus: {
                    if(!sceneTextArea.activeFocus)
                        sceneTextArea.forceActiveFocus()
                }
                onAssumeFocusAt: {
                    if(!sceneTextArea.activeFocus)
                        sceneTextArea.forceActiveFocus()
                    if(pos < 0)
                        sceneTextArea.cursorPosition = sceneDocumentBinder.lastCursorPosition()
                    else
                        sceneTextArea.cursorPosition = pos
                }
            }

            cursorDelegate: Item {
                width: sceneTextArea.cursorRectangle.width
                height: sceneTextArea.cursorRectangle.height
                visible: sceneTextArea.activeFocus
                ToolTip.text: '<font name="' + sceneDocumentBinder.currentFont.family + '"><font color="gray">' + sceneDocumentBinder.completionPrefix.toUpperCase() + '</font>' + completer.suggestion.toUpperCase() + '</font>';
                ToolTip.visible: completer.hasSuggestion

                Rectangle {
                    id: blinkingCursor
                    color: primaryColors.borderColor
                    width: 2
                    height: parent.height

                    SequentialAnimation {
                        loops: Animation.Infinite
                        running: sceneTextArea.activeFocus

                        NumberAnimation {
                            target: blinkingCursor
                            property: "opacity"
                            duration: 400
                            easing.type: Easing.Linear
                            from: 0
                            to: 1
                        }

                        NumberAnimation {
                            target: blinkingCursor
                            property: "opacity"
                            duration: 400
                            easing.type: Easing.Linear
                            from: 1
                            to: 0
                        }
                    }
                }
            }
            onActiveFocusChanged: {
                if(activeFocus)
                    sceneHeadingLoader.viewOnly = true
            }
            Keys.onReturnPressed: {
                if(event.modifiers & Qt.ControlModifier) {
                    sceneEditor.splitSceneRequest(sceneDocumentBinder.currentElement, sceneDocumentBinder.currentElementCursorPosition)
                    event.accepted = true
                    return
                }

                if(completer.suggestion !== "") {
                    userIsTyping = false
                    insert(cursorPosition, completer.suggestion)
                    userIsTyping = true
                    Transliterator.enableFromNextWord()
                    event.accepted = true
                } else
                    event.accepted = false
            }
            Keys.onTabPressed: {
                if(completer.suggestion !== "") {
                    userIsTyping = false
                    insert(cursorPosition, completer.suggestion)
                    userIsTyping = true
                    Transliterator.enableFromNextWord()
                    event.accepted = true
                } else
                    sceneDocumentBinder.tab()
            }
            Keys.onBackPressed: sceneDocumentBinder.backtab()
            Keys.onUpPressed: {
                if(sceneDocumentBinder.canGoUp())
                    event.accepted = false
                else {
                    event.accepted = true
                    requestScrollUp()
                }
            }
            Keys.onDownPressed: {
                if(sceneDocumentBinder.canGoDown())
                    event.accepted = false
                else {
                    event.accepted = true
                    requestScrollDown()
                }
            }
            Keys.onPressed: {
                if(event.key === Qt.Key_PageUp) {
                    requestScrollUp()
                    event.accepted = true
                } else if(event.key === Qt.Key_PageDown) {
                    requestScrollDown()
                    event.accepted = true
                } else
                    event.accepted = false
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                enabled: !editorContextMenu.visible && sceneTextArea.activeFocus
                onClicked: {
                    sceneTextArea.persistentSelection = true
                    editorContextMenu.popup()
                    mouse.accept = true
                }
                cursorShape: Qt.IBeamCursor
            }

            Menu2 {
                id: editorContextMenu
                onAboutToHide: sceneTextArea.persistentSelection = false

                MenuItem2 {
                    text: "Cut\t" + app.polishShortcutTextForDisplay("Ctrl+X")
                    enabled: sceneTextArea.selectionEnd > sceneTextArea.selectionStart
                    onClicked: sceneTextArea.cut()
                }

                MenuItem2 {
                    text: "Copy\t" + app.polishShortcutTextForDisplay("Ctrl+C")
                    enabled: sceneTextArea.selectionEnd > sceneTextArea.selectionStart
                    onClicked: sceneTextArea.copy()
                }

                MenuItem2 {
                    text: "Paste\t" + app.polishShortcutTextForDisplay("Ctrl+V")
                    enabled: sceneTextArea.canPaste
                    onClicked: sceneTextArea.paste()
                }

                MenuSeparator {  }

                MenuItem2 {
                    text: "Split Scene"
                    enabled: sceneDocumentBinder && sceneDocumentBinder.currentElement && sceneDocumentBinder.currentElementCursorPosition >= 0 && allowSplitSceneRequest
                    onClicked: sceneEditor.splitSceneRequest(sceneDocumentBinder.currentElement, sceneDocumentBinder.currentElementCursorPosition)
                }

                MenuSeparator {  }

                Menu2 {
                    title: "Format"

                    Repeater {
                        model: [
                            { "value": SceneElement.Action, "display": "Action" },
                            { "value": SceneElement.Character, "display": "Character" },
                            { "value": SceneElement.Dialogue, "display": "Dialogue" },
                            { "value": SceneElement.Parenthetical, "display": "Parenthetical" },
                            { "value": SceneElement.Shot, "display": "Shot" },
                            { "value": SceneElement.Transition, "display": "Transition" }
                        ]

                        MenuItem2 {
                            text: modelData.display + "\t" + app.polishShortcutTextForDisplay("Ctrl+" + (index+1))
                            enabled: sceneDocumentBinder.currentElement !== null
                            onClicked: sceneDocumentBinder.currentElement.type = modelData.value
                        }
                    }
                }

                Menu2 {
                    title: "Translate"
                    enabled: sceneTextArea.selectionEnd > sceneTextArea.selectionStart

                    Repeater {
                        model: app.enumerationModel(app.transliterationEngine, "Language")

                        MenuItem2 {
                            visible: index > 0
                            text: modelData.key
                            onClicked: sceneTextArea.Transliterator.transliterateToLanguage(sceneTextArea.selectionStart, sceneTextArea.selectionEnd, modelData.value)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: sceneHeadingDisabled

        Rectangle {
            color: Qt.tint(scene.color, "#D9FFFFFF")
            property font headingFont: sceneHeadingFormat.font
            onHeadingFontChanged: {
                if(headingFont.pointSize === sceneHeadingFormat.font.pointSize)
                    headingFont.pointSize = headingFont.pointSize+scriteDocument.formatting.fontPointSizeDelta
            }

            Text {
                text: "inherited from previous scene"
                anchors.centerIn: parent
                color: primaryColors.c10.text
                font: headingFont
                opacity: 0.5
            }
        }
    }

    Component {
        id: sceneHeadingEditor

        Rectangle {
            property font headingFont: sceneHeadingFormat.font
            Component.onCompleted: {
                headingFont.pointSize = headingFont.pointSize+scriteDocument.formatting.fontPointSizeDelta
                locTypeEdit.forceActiveFocus()
            }
            color: primaryColors.c100.background
            height: layout.height + 4

            Row {
                id: layout
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: sceneEditor.padding
                anchors.rightMargin: sceneEditor.padding

                TextField2 {
                    id: locTypeEdit
                    font: headingFont
                    width: Math.max(contentWidth, 80)
                    anchors.verticalCenter: parent.verticalCenter
                    text: scene.heading.locationType
                    completionStrings: scriteDocument.structure.standardLocationTypes()
                    onEditingComplete: scene.heading.locationType = text
                    tabItem: locEdit
                }

                Text {
                    id: sep1Text
                    font: headingFont
                    text: ". "
                    anchors.verticalCenter: parent.verticalCenter
                }

                TextField2 {
                    id: locEdit
                    font: headingFont
                    width: parent.width - locTypeEdit.width - sep1Text.width - momentEdit.width - sep2Text.width
                    anchors.verticalCenter: parent.verticalCenter
                    text: scene.heading.location
                    enableTransliteration: true
                    completionStrings: scriteDocument.structure.allLocations()
                    onEditingComplete: scene.heading.location = text
                    tabItem: momentEdit
                }

                Text {
                    id: sep2Text
                    font: headingFont
                    text: "- "
                    anchors.verticalCenter: parent.verticalCenter
                }

                TextField2 {
                    id: momentEdit
                    font: headingFont
                    width: Math.max(contentWidth, 150);
                    anchors.verticalCenter: parent.verticalCenter
                    text: scene.heading.moment
                    completionStrings: scriteDocument.structure.standardMoments()
                    onEditingComplete: scene.heading.moment = text
                    tabItem: sceneContentEditor
                }
            }
        }
    }

    Component {
        id: sceneHeadingViewer

        Rectangle {
            color: Qt.tint(scene.color, "#D9FFFFFF")
            property font headingFont: sceneHeadingFormat.font
            Component.onCompleted: headingFont.pointSize = headingFont.pointSize+scriteDocument.formatting.fontPointSizeDelta
            radius: contentEditorArea.radius
            property real contentHeight: sceneHeadingText.contentHeight

            Text {
                id: sceneHeadingText
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: sceneEditor.padding
                anchors.rightMargin: sceneEditor.padding
                font: parent.headingFont
                text: scene.heading.text
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.WordWrap
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if(readOnly)
                        return
                    app.execLater(0, function() { sceneHeadingLoader.viewOnly = false })
                }
            }
        }
    }
}
