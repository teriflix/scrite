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

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Scrite.App

import "../../../../helpers"
import "../../../../globals"
import "../../../../controls"
import "../../../../structureview"
import "../../.."

MenuLoader {
    id: root

    required property TextEdit sceneTextEditor
    required property SceneDocumentBinder sceneDocumentBinder

    property bool splitSceneEnabled: false
    property bool mergeWithPreviousSceneEnabled: false

    signal cutRequest()
    signal copyRequest()
    signal pasteRequest()
    signal reloadSceneContentRequest()
    signal splitSceneAtPositionRequest(int position)
    signal mergeWithPreviousSceneRequest()
    signal translateSelection()

    menu: SctMenu {
        id: _menu

        property bool splitSceneEnabled: false
        property bool mergeWithPreviousSceneEnabled: false
        property int sceneTextEditorCursorPosition: -1

        property TextFormat sceneTextFormat: root.sceneDocumentBinder.textFormat
        property SceneElement sceneCurrentElement

        onAboutToShow: {
            splitSceneEnabled = root.splitSceneEnabled
            mergeWithPreviousSceneEnabled = root.mergeWithPreviousSceneEnabled
            sceneCurrentElement = root.sceneDocumentBinder.currentElement
            sceneTextEditorCursorPosition = root.sceneTextEditor.cursorPosition
            root.sceneTextEditor.persistentSelection = true
        }
        onAboutToHide: root.sceneTextEditor.persistentSelection = false

        SctMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Cut\t" + ActionHub.editOptions.find("cut").shortcut
            enabled: root.sceneTextEditor.selectionEnd > root.sceneTextEditor.selectionStart
            onClicked: { root.cutRequest(); root.close() }
        }

        SctMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Copy\t" + ActionHub.editOptions.find("copy").shortcut
            enabled: root.sceneTextEditor.selectionEnd > root.sceneTextEditor.selectionStart
            onClicked: { root.copyRequest(); root.close() }
        }

        SctMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Paste\t" + ActionHub.editOptions.find("paste").shortcut
            enabled: root.sceneTextEditor.canPaste
            onClicked: { root.pasteRequest(); root.close() }
        }

        MenuSeparator {  }

        SctMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Split Scene\t" + ActionHub.editOptions.find("splitScene").shortcut
            enabled: _menu.splitSceneEnabled
            onClicked: {
                root.splitSceneAtPositionRequest(_menu.sceneTextEditorCursorPosition)
                root.close()
            }
        }

        SctMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Join Previous Scene\t" + ActionHub.editOptions.find("mergeScene").shortcut
            enabled: _menu.mergeWithPreviousSceneEnabled
            onClicked: {
                root.mergeWithPreviousSceneRequest()
                root.close()
            }
        }

        SctMenuItem {
            readonly property Action txAction: ActionHub.editOptions.find("translateToActiveLanguage") as Action

            text: "Transliterate to " + Runtime.language.active.name + "\t" + txAction.shortcut
            enabled: root.sceneTextEditor.selectedText !== "" && Runtime.language.textSelectionTransliterationEnabled
            focusPolicy: Qt.NoFocus

            onTriggered: () => {
                             root.translateSelection()
                         }
        }

        SctMenuItem {
            readonly property bool isGrouped: _menu.sceneCurrentElement &&
                                             root.sceneDocumentBinder.scene &&
                                             root.sceneDocumentBinder.scene.dualDialogueContaining(_menu.sceneCurrentElement) !== null

            text: isGrouped ? "Undo Dual Dialogue\t" + ActionHub.editOptions.find("toggleDualDialogue").shortcut
                           : "Make Dual Dialogue\t" + ActionHub.editOptions.find("toggleDualDialogue").shortcut
            enabled: _menu.sceneCurrentElement !== null
            focusPolicy: Qt.NoFocus

            onClicked: {
                root.sceneDocumentBinder.toggleDualDialogue()
                root.close()
            }
        }

        MenuSeparator {  }

        SctMenu {
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

                delegate: SctMenuItem {
                    required property int index
                    required property var modelData

                    focusPolicy: Qt.NoFocus
                    text: modelData.display + "\tCtrl+" + (index+1)
                    enabled: _menu.sceneCurrentElement !== null
                    onClicked: {
                        _menu.sceneCurrentElement.type = modelData.value
                        root.close()
                    }
                }
            }
        }
    }
}
