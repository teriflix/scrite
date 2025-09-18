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
import "qrc:/qml/screenplayeditor"

AbstractScenePartEditor {
    id: root

    property var additionalCharacterMenuItems: []

    signal newCharacterAdded(string characterName)
    signal additionalCharacterMenuItemClicked(string characterName, string menuItemName)

    height: _layout.height

    Flow {
        id: _layout

        width: parent.width

        flow: Flow.LeftToRight
        spacing: 5
        leftPadding: root.pageLeftMargin
        rightPadding: root.pageRightMargin

        VclLabel {
            id: _label

            text: "Characters: "

            visible: !root.scene.hasCharacters
            topPadding: 5
            bottomPadding: 5

            font.bold: true
            font.pointSize: _private.tagFontPointSize
        }

        Repeater {
            id: _characterTags
            model: root.scene ? root.scene.characterNames : 0

            TagText {
                id: _characterTag

                required property string modelData

                property string characterName: modelData

                property var colors: {
                    if(containsMouse)
                        return Runtime.colors.accent.c900
                    return root.screenplayElementDelegateHasFocus ? Runtime.colors.accent.c600 : Runtime.colors.accent.c10
                }

                Component.onCompleted: determineFlags()

                border.color: colors.text
                border.width: root.screenplayElementDelegateHasFocus ? 0 : Math.max(0.5, 1 * zoomLevel)

                text: characterName
                color: colors.background
                enabled: !root.readOnly
                textColor: colors.text
                topPadding: Math.max(5, 5 * root.zoomLevel)
                leftPadding: Math.max(10, 10 * root.zoomLevel)
                rightPadding: leftPadding
                bottomPadding: topPadding

                font.family: Runtime.idealFontMetrics.font.family
                font.pointSize: _private.tagFontPointSize
                font.capitalization: Font.AllUppercase

                onClicked: _private.popupCharacterMenu(characterName, _characterTag)

                onCloseRequest: {
                    if(!root.readOnly)
                        root.scene.removeMuteCharacter(characterName)
                }

                function determineFlags() {
                    const chMute = root.scene.isCharacterMute(characterName)
                    closable = chMute

                    const chVisible = Runtime.screenplayEditorSettings.captureInvisibleCharacters ? (chMute || root.scene.isCharacterVisible(characterName)) : true
                    font.italic = !chVisible
                    opacity = chVisible ? 1 : 0.65
                }
            }
        }

        Loader {
            id: _newCharacterInputLoader

            active: false
            visible: active

            sourceComponent: VclTextField {
                Component.onCompleted: {
                    forceActiveFocus()
                    root.ensureVisible(_newCharacterInputLoader, Qt.rect(0,0,width,height))
                }

                Keys.onEscapePressed: {
                    text = ""
                    _newCharacterInputLoader.active = false
                }

                readOnly: false
                completionStrings: Scrite.document.structure.characterNames

                font.pointSize: _private.tagFontPointSize
                font.capitalization: Font.AllUppercase

                onEditingComplete: {
                    if(text.length > 0) {
                        root.scene.addMuteCharacter(text)
                        root.newCharacterAdded(text)
                    }

                    _newCharacterInputLoader.active = false
                }
            }

            onStatusChanged: {
                if(status === Loader.Null) {
                    Scrite.app.resetObjectProperty(_newCharacterInputLoader, "width")
                    Scrite.app.resetObjectProperty(_newCharacterInputLoader, "height")
                }
            }
        }

        Image {
            source: "qrc:/icons/content/add_box.png"

            width: _label.height
            height: width

            opacity: 0.5
            visible: enabled
            enabled: !root.readOnly

            MouseArea {
                ToolTip.text: "Click here to capture characters who don't have any dialogues in this scene, but are still required for the scene."
                ToolTip.delay: 1000
                ToolTip.visible: containsMouse

                anchors.fill: parent

                hoverEnabled: true

                onClicked: _newCharacterInputLoader.active = true
                onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
            }

            Announcement.onIncoming: (type,data) => {
                if(!root.screenplayElementDelegateHasFocus || root.readOnly)
                    return

                var sdata = "" + data
                var stype = "" + type
                if(stype === Runtime.announcementIds.focusRequest && sdata === Runtime.announcementData.focusOptions.addMuteCharacter) {
                    _newCharacterInputLoader.active = true
                }
            }
        }
    }

    QtObject {
        id: _private

        property real tagFontPointSize: Math.max(sceneHeadingFormat.font2.pointSize*0.7, 6)
        property SceneElementFormat sceneHeadingFormat: Scrite.document.displayFormat.elementFormat(SceneElement.Heading)

        property bool captureInvisibleCharacters: Runtime.screenplayEditorSettings.captureInvisibleCharacters
        onCaptureInvisibleCharactersChanged: scheduleDetermineFlagsInTags()

        property Component characterMenu: ScreenplayEditorCharacterMenu {
            additionalCharacterMenuItems: root.additionalCharacterMenuItems

            onAdditionalCharacterMenuItemClicked: (characterName, menuItemName) => {
                                                      root.additionalCharacterMenuItemClicked(characterName, menuItemName)
                                                  }
        }

        readonly property Connections sceneConnections: Connections {
            target: root.scene

            function onSceneChanged() {
                _private.scheduleDetermineFlagsInTags()
            }
        }

        function popupCharacterMenu(characterName, parent) {
            let menu = characterMenu.createObject(parent, {"characterName": characterName})
            menu.closed.connect(menu.destroy)
            menu.popup()
            return menu
        }

        function determineFlagsInTags() {
            const nrTags = _characterTags.count
            for(let i=0; i<nrTags; i++) {
                let tag = _characterTags.itemAt(i)
                tag.determineFlags()
            }
        }

        function scheduleDetermineFlagsInTags() {
            Qt.callLater(determineFlagsInTags)
        }
    }
}
