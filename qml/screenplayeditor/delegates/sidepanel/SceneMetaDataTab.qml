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


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors"

AbstractScenePartEditor {
    id: root

    Flickable {
        id: _flickable

        anchors.fill: parent
        anchors.margins: 5
        anchors.rightMargin: 0

        clip: interactive
        contentY: 0
        interactive: contentHeight > height
        contentWidth: _layout.width
        contentHeight: _layout.height
        flickableDirection: Flickable.VerticalFlick

        ScrollBar.vertical: VclScrollBar { }

        Column {
            id: _layout

            width: _flickable.ScrollBar.vertical.needed ? _flickable.width-20 : _flickable.width

            Loader {
                width: parent.width

                active: !Runtime.screenplayEditorSettings.displaySceneCharacters
                visible: active

                sourceComponent: SceneStoryBeatTagsPartEditor {
                    index: root.index
                    sceneID: root.sceneID
                    screenplayElement: root.screenplayElement
                    screenplayElementDelegateHasFocus: root.hasFocus

                    partName: "StoryBeats-SidePanel"
                    zoomLevel: root.zoomLevel * 0.75
                    fontMetrics: root.fontMetrics
                    pageMargins: root.pageMargins
                    screenplayAdapter: root.screenplayAdapter

                    onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }

                    // TODO
                    onSceneTagAdded: (tagName) => { }
                    onSceneTagClicked: (tagName) => { }
                }
            }

            Loader {
                width: parent.width

                active: !Runtime.screenplayEditorSettings.displaySceneCharacters
                visible: active

                sourceComponent: SceneCharacterListPartEditor {
                    index: root.index
                    sceneID: root.sceneID
                    screenplayElement: root.screenplayElement
                    screenplayElementDelegateHasFocus: root.hasFocus

                    partName: "CharacterList-SidePanel"
                    zoomLevel: root.zoomLevel * 0.75
                    fontMetrics: root.fontMetrics
                    pageMargins: root.pageMargins
                    screenplayAdapter: root.screenplayAdapter

                    // TODO
                    additionalCharacterMenuItems: []

                    onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }

                    // TODO
                    onNewCharacterAdded: (characterName) => { }
                    onAdditionalCharacterMenuItemClicked: (characterName, menuItemName) => { }
                }
            }

            Loader {
                width: parent.width

                active: !Runtime.screenplayEditorSettings.displaySceneSynopsis
                visible: active

                sourceComponent: SceneSynopsisPartEditor {
                    Layout.fillWidth: true

                    index: root.index
                    sceneID: root.sceneID
                    screenplayElement: root.screenplayElement
                    screenplayElementDelegateHasFocus: root.hasFocus

                    partName: "Synopsis-SidePanel"
                    zoomLevel: root.zoomLevel
                    fontMetrics: root.fontMetrics
                    pageMargins: root.pageMargins
                    screenplayAdapter: root.screenplayAdapter

                    onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
                }
            }
        }
    }
}
