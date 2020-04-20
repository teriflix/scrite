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
    property real  fullHeight: (sceneHeadingLoader.active ? sceneHeadingArea.height : 0) + (sceneContentEditor ? (sceneContentEditor.contentHeight+10) : 0) + 10
    property color backgroundColor: scene ? Qt.tint(scene.color, "#E0FFFFFF") : "white"
    property bool  scrollable: true
    property bool  showOnlyEnabledSceneHeadings: false
    signal assumeFocus()
    signal assumeFocusAt(int pos)
    signal requestScrollUp()
    signal requestScrollDown()

    DelayedPropertyBinder {
        id: activeFocusBinder
        initial: false
        set: sceneContentEditor.activeFocus
    }

    Rectangle {
        id: sceneHeadingArea
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        height: sceneHeadingLoader.active ? sceneHeadingLoader.height : 0
        border { width: 1; color: "lightgray" }

        Loader {
            id: sceneHeadingLoader
            width: parent.width
            height: 40
            property bool viewOnly: true
            active: scene !== null && scene.heading !== null && (showOnlyEnabledSceneHeadings ? scene.heading.enabled : true)
            sourceComponent: {
                if(scene !== null && scene.heading !== null && scene.heading.enabled)
                    return viewOnly ? sceneHeadingViewer : sceneHeadingEditor
                return sceneHeadingDisabled
            }
        }
    }

    property TextArea sceneContentEditor

    Rectangle {
        anchors.left: parent.left
        anchors.top: sceneHeadingArea.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: backgroundColor

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
            clip: true
            active: true
            sourceComponent: scrollable ? scrollableSceneContentEditorComponent : sceneContentEditorComponent
        }
    }

    onSceneChanged: {
        contentEditorLoader.active = false
        contentEditorLoader.active = true
    }

    Component {
        id: scrollableSceneContentEditorComponent

        ScrollView {
            id: scrollView
            ScrollBar.vertical.minimumSize: 0.1

            Repeater {
                model: 1
                delegate: sceneContentEditorComponent
            }

            Component.onCompleted: {
                scrollView.ScrollBar.vertical.setPosition(0)
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
            background: Rectangle {
                color: backgroundColor
            }
            palette: app.palette
            selectByMouse: true
            selectByKeyboard: true
            EventFilter.events: [31,51] // Wheel, ShortcutOverride
            EventFilter.onFilter: {
                if(event.type === 31) {
                    result.acceptEvent = false
                    result.filter = !scrollable
                } else if(event.type === 51) {
                    result.acceptEvent = false
                    result.filter = true
                }
            }
            font: scriteDocument.formatting.defaultFont
            Transliterator.enabled: scene && !scene.isBeingReset
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
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
                    color: "black"
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
                if(scene)
                    scene.undoRedoEnabled = activeFocus
            }
            Keys.onReturnPressed: {
                if(completer.suggestion !== "") {
                    insert(cursorPosition, completer.suggestion)
                    event.accepted = true
                } else
                    event.accepted = false
            }
            Keys.onTabPressed: {
                if(completer.suggestion !== "") {
                    insert(cursorPosition, completer.suggestion)
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

            Menu {
                id: editorContextMenu
                onAboutToHide: sceneTextArea.persistentSelection = false

                MenuItem {
                    text: "Cut\t" + app.polishShortcutTextForDisplay("Ctrl+X")
                    enabled: sceneTextArea.selectionEnd > sceneTextArea.selectionStart
                    onClicked: sceneTextArea.cut()
                }

                MenuItem {
                    text: "Copy\t" + app.polishShortcutTextForDisplay("Ctrl+C")
                    enabled: sceneTextArea.selectionEnd > sceneTextArea.selectionStart
                    onClicked: sceneTextArea.copy()
                }

                MenuItem {
                    text: "Paste\t" + app.polishShortcutTextForDisplay("Ctrl+V")
                    enabled: sceneTextArea.canPaste
                    onClicked: sceneTextArea.paste()
                }

                MenuSeparator {  }

                Menu {
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

                        MenuItem {
                            text: modelData.display + "\t" + app.polishShortcutTextForDisplay("Ctrl+" + (index+1))
                            enabled: sceneDocumentBinder.currentElement !== null
                            onClicked: sceneDocumentBinder.currentElement.type = modelData.value
                        }
                    }
                }

                Menu {
                    title: "Translate"
                    enabled: sceneTextArea.selectionEnd > sceneTextArea.selectionStart

                    Repeater {
                        model: app.enumerationModel(app.transliterationSettings, "Language")

                        MenuItem {
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
                color: "gray"
                font: headingFont
            }
        }
    }

    Component {
        id: sceneHeadingEditor

        Row {
            spacing: 10
            property font headingFont: sceneHeadingFormat.font
            onHeadingFontChanged: {
                if(headingFont.pointSize === sceneHeadingFormat.font.pointSize)
                    headingFont.pointSize = headingFont.pointSize+scriteDocument.formatting.fontPointSizeDelta
            }

            Component.onDestruction: scene.heading.location = locationEdit.text

            TextViewEdit {
                id: locationTypeEdit
                text: scene.heading.locationType
                font: headingFont
                width: Math.max(contentWidth, 80)
                readOnly: false
                frameVisible: true
                completionStrings: scriteDocument.structure.standardLocationTypes()
                onEditingFinished: scene.heading.locationType = text
                horizontalAlignment: Qt.AlignLeft
                anchors.verticalCenter: parent.verticalCenter
                KeyNavigation.priority: KeyNavigation.BeforeItem
                KeyNavigation.tab: locationEdit.item
                KeyNavigation.backtab: momentEdit.item
            }

            TextViewEdit {
                id: locationEdit
                font: headingFont
                text: scene.heading.location
                width: parent.width - locationTypeEdit.width - momentEdit.width - 2*parent.spacing
                readOnly: false
                frameVisible: true
                completionStrings: scriteDocument.structure.allLocations()
                onEditingFinished: scene.heading.location = text
                horizontalAlignment: Qt.AlignLeft
                anchors.verticalCenter: parent.verticalCenter
                KeyNavigation.priority: KeyNavigation.BeforeItem
                KeyNavigation.tab: momentEdit.item
                KeyNavigation.backtab: locationTypeEdit.item
            }

            TextViewEdit {
                id: momentEdit
                font: headingFont
                text: scene.heading.moment
                width: Math.max(contentWidth, 150)
                readOnly: false
                frameVisible: true
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Qt.AlignLeft
                completionStrings: scriteDocument.structure.standardMoments()
                onEditingFinished: scene.heading.moment = text
                KeyNavigation.priority: KeyNavigation.BeforeItem
                KeyNavigation.tab: locationTypeEdit.item
                KeyNavigation.backtab: locationEdit.item
            }
        }
    }

    Component {
        id: sceneHeadingViewer

        Rectangle {
            color: Qt.tint(scene.color, "#D9FFFFFF")
            property font headingFont: sceneHeadingFormat.font
            Component.onCompleted: headingFont.pointSize = headingFont.pointSize+8

            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                font: parent.headingFont
                text: scene.heading.text
                anchors.verticalCenter: parent.verticalCenter
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if(readOnly)
                        return
                    sceneHeadingLoader.viewOnly = false
                }
            }
        }
    }
}
