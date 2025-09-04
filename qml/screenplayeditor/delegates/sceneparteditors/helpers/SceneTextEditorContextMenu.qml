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

    menu: VclMenu {
        id: _menu

        property int sceneTextEditorCursorPosition: -1

        property TextFormat sceneTextFormat: sceneDocumentBinder.textFormat
        property SceneElement sceneCurrentElement

        onAboutToShow: {
            sceneCurrentElement = sceneDocumentBinder.currentElement
            sceneTextEditorCursorPosition = sceneTextEditor.cursorPosition
            sceneTextEditor.persistentSelection = true
        }
        onAboutToHide: sceneTextEditor.persistentSelection = false

        VclMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Cut\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+X")
            enabled: sceneTextEditor.selectionEnd > sceneTextEditor.selectionStart
            onClicked: { root.cutRequest(); root.close() }
        }

        VclMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Copy\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+C")
            enabled: sceneTextEditor.selectionEnd > sceneTextEditor.selectionStart
            onClicked: { root.copyRequest(); root.close() }
        }

        VclMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Paste\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+V")
            enabled: sceneTextEditor.canPaste
            onClicked: { root.pasteRequest(); root.close() }
        }

        MenuSeparator {  }

        VclMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Split Scene"
            enabled: root.splitSceneEnabled
            onClicked: {
                root.splitSceneAtPositionRequest(_menu.sceneTextEditorCursorPosition)
                root.close()
            }
        }

        VclMenuItem {
            focusPolicy: Qt.NoFocus
            text: "Join Previous Scene"
            enabled: root.mergeWithPreviousSceneEnabled
            onClicked: {
                root.mergeWithPreviousSceneRequest()
                root.close()
            }
        }

        MenuSeparator {  }

        VclMenu {
            title: "Format"
            width: 250

            Repeater {
                model: [
                    { "value": SceneElement.Action, "display": "Action" },
                    { "value": SceneElement.Character, "display": "Character" },
                    { "value": SceneElement.Dialogue, "display": "Dialogue" },
                    { "value": SceneElement.Parenthetical, "display": "Parenthetical" },
                    { "value": SceneElement.Shot, "display": "Shot" },
                    { "value": SceneElement.Transition, "display": "Transition" }
                ]

                VclMenuItem {
                    required property var modelData

                    focusPolicy: Qt.NoFocus
                    text: modelData.display + "\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+" + (index+1))
                    enabled: _menu.sceneCurrentElement !== null
                    onClicked: {
                        _menu.sceneCurrentElement.type = modelData.value
                        root.close()
                    }
                }
            }
        }

        VclMenu {
            title: "Translate"
            enabled: sceneTextEditor.hasSelection

            Repeater {
                model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")

                VclMenuItem {
                    text: modelData.key
                    visible: index >= 0
                    enabled: modelData.value !== TransliterationEngine.English
                    focusPolicy: Qt.NoFocus

                    onClicked: {
                        root.close()
                        sceneTextEditor.forceActiveFocus()
                        sceneTextEditor.scene.beginUndoCapture()
                        sceneTextEditor.Transliterator.transliterateToLanguage(sceneTextEditor.selectionStart, sceneTextEditor.selectionEnd, modelData.value)
                        sceneTextEditor.scene.endUndoCapture()
                        root.reloadSceneContentRequest()
                    }
                }
            }
        }
    }
}
