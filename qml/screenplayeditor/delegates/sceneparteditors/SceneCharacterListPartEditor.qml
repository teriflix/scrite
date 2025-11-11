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


import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor"

AbstractScenePartEditor {
    id: root

    signal newCharacterAdded(string characterName)

    height: _charactersInput.height

    TextListInput {
        id: _charactersInput

        Component.onCompleted: font.capitalization = Font.AllUppercase

        width: parent.width

        leftPadding: root.pageLeftMargin
        rightPadding: root.pageRightMargin

        addTextButtonTooltip: "Click here to capture characters who don't have any dialogues in this scene, but are still required for the scene."
        completionStrings: Scrite.document.structure.characterNames
        font: root.font
        labelIconSource: "qrc:/icons/content/persons_add.png"
        labelText: "Characters"
        readOnly: root.readOnly
        textBorderWidth: root.screenplayElementDelegateHasFocus ? 0 : Math.max(0.5, 1 * zoomLevel)
        textColors: root.screenplayElementDelegateHasFocus ? Runtime.colors.accent.c600 : Runtime.colors.accent.c10
        textList: root.scene ? root.scene.characterNames : 0
        zoomLevel: root.zoomLevel

        onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
        onTextClicked: (text, source) => { _private.popupCharacterMenu(text, source) }
        onTextCloseRequest: (text, source) => { root.scene.removeMuteCharacter(text) }
        onConfigureTextRequest: (text, tag) => {
                                    const chMute = root.scene.isCharacterMute(text)
                                    tag.closable = chMute

                                    const chVisible = Runtime.screenplayEditorSettings.captureInvisibleCharacters ? (chMute || root.scene.isCharacterVisible(text)) : true
                                    tag.font.italic = !chVisible
                                    tag.opacity = chVisible ? 1 : 0.65
                                }
        onNewTextRequest: (text) => {
                              root.scene.addMuteCharacter(text)
                              root.newCharacterAdded(text)

                              if(root.isCurrent) {
                                  _private.editSceneContent.trigger()
                              }
                          }
        onNewTextCancelled: () => {
                                if(root.isCurrent) {
                                    _private.editSceneContent.trigger()
                                }
                            }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("addMuteCharacter")
        enabled: root.isCurrent && !_charactersInput.readOnly && !_charactersInput.acceptingNewText

        onTriggered: (source) => {
                         _charactersInput.acceptNewText()
                     }
    }

    QtObject {
        id: _private

        readonly property Action editSceneContent: ActionHub.editOptions.find("editSceneContent")

        property bool captureInvisibleCharacters: Runtime.screenplayEditorSettings.captureInvisibleCharacters

        property Component characterMenu: ScreenplayEditorCharacterMenu { }

        readonly property Connections sceneConnections: Connections {
            target: root.scene

            function onSceneChanged() {
                _charactersInput.configureTextsLater()
            }
        }

        function popupCharacterMenu(characterName, parent) {
            let menu = characterMenu.createObject(parent, {"characterName": characterName})
            menu.closed.connect(menu.destroy)
            menu.popup()
            return menu
        }
    }
}
