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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors"

AbstractScreenplayElementDelegate {
    id: root

    content: Item {
        height: _layout.height

        ColumnLayout {
            id: _layout

            // Scene Heading Area
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: _headingLayout.height + root.zoomLevel * _headingLayout.spacing * 2

                color: Runtime.tintSceneHeadingColor(root.scene.color)

                ColumnLayout {
                    id: _headingLayout

                    anchors.verticalCenter: parent.verticalCenter

                    width: parent.width

                    spacing: 5

                    SceneHeadingPartEditor {
                        Layout.fillWidth: true

                        index: root.index
                        sceneID: root.sceneID
                        screenplayElement: root.screenplayElement
                        screenplayElementDelegateHasFocus: root.hasFocus

                        zoomLevel: root.zoomLevel
                        fontMetrics: root.fontMetrics
                        pageMargins: root.pageMargins
                        placeholderMode: root.placeholderMode

                        onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
                    }

                    SceneCharacterListPartEditor {
                        Layout.fillWidth: true

                        index: root.index
                        sceneID: root.sceneID
                        screenplayElement: root.screenplayElement
                        screenplayElementDelegateHasFocus: root.hasFocus

                        zoomLevel: root.zoomLevel
                        fontMetrics: root.fontMetrics
                        pageMargins: root.pageMargins
                        placeholderMode: root.placeholderMode

                        // TODO
                        additionalCharacterMenuItems: []

                        onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }

                        // TODO
                        onNewCharacterAdded: (characterName) => { }
                        onAdditionalCharacterMenuItemClicked: (characterName, menuItemName) => { }
                    }

                    SceneStoryBeatTagsPartEditor {
                        Layout.fillWidth: true

                        index: root.index
                        sceneID: root.sceneID
                        screenplayElement: root.screenplayElement
                        screenplayElementDelegateHasFocus: root.hasFocus

                        zoomLevel: root.zoomLevel
                        fontMetrics: root.fontMetrics
                        pageMargins: root.pageMargins
                        placeholderMode: root.placeholderMode

                        onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }

                        // TODO
                        onSceneTagAdded: (tagName) => { }
                        onSceneTagClicked: (tagName) => { }
                    }

                    SceneSynopsisPartEditor {
                        Layout.fillWidth: true

                        index: root.index
                        sceneID: root.sceneID
                        screenplayElement: root.screenplayElement
                        screenplayElementDelegateHasFocus: root.hasFocus

                        zoomLevel: root.zoomLevel
                        fontMetrics: root.fontMetrics
                        pageMargins: root.pageMargins
                        placeholderMode: root.placeholderMode

                        onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
                    }
                }
            }

            SceneContentEditor {
                Layout.fillWidth: true

                index: root.index
                sceneID: root.sceneID
                screenplayElement: root.screenplayElement
                screenplayElementDelegateHasFocus: root.hasFocus

                zoomLevel: root.zoomLevel
                fontMetrics: root.fontMetrics
                pageMargins: root.pageMargins
                placeholderMode: root.placeholderMode

                onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
            }
        }

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom

            width: 10

            color: root.scene.color

            visible: root.hasFocus
        }
    }
}
