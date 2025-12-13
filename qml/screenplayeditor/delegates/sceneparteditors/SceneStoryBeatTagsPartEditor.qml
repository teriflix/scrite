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
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor"

AbstractScenePartEditor {
    id: root

    signal sceneTagAdded(string tagName)
    signal sceneTagClicked(string tagName)

    implicitHeight: _layout.height

    ColumnLayout {
        id: _layout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: root.pageLeftMargin
        anchors.rightMargin: root.pageRightMargin

        RowLayout {
            Layout.fillWidth: true

            spacing: _tagsInput.spacing

            FlatToolButton {
                toolTipText: "Formal Story Beats/Tags"
                suggestedWidth: _tagsInput.label.height
                suggestedHeight: _tagsInput.label.height

                enabled: Runtime.appFeatures.structure.enabled
                opacity: enabled ? 1 : 0.5
                iconSource: "qrc:/icons/action/tag.png"

                onClicked: _private.popupFormalTagsMenu()
            }

            Text {
                font: _tagsInput.label.font
                text: "Formal Tags"
                visible: _private.presentableGroupNames === ""
            }

            Link {
                Layout.fillWidth: true

                text: _private.presentableGroupNames
                font: _tagsInput.label.font
                elide: Text.ElideRight
                visible: _private.presentableGroupNames !== ""
                enabled: Runtime.appFeatures.structure.enabled
                opacity: enabled ? 1 : 0.5
                topPadding: 5
                bottomPadding: 5

                onClicked: _private.popupFormalTagsMenu()
            }

            Image {
                Layout.preferredWidth: _tagsInput.label.height
                Layout.preferredHeight: _tagsInput.label.height

                source: "qrc:/icons/content/add_box.png"

                opacity: enabled ? 1 : 0.5
                visible: enabled && _private.presentableGroupNames === ""
                enabled: !Scrite.document.readOnly

                MouseArea {
                    anchors.fill: parent

                    onClicked: _private.popupFormalTagsMenu()
                }
            }
        }

        TextListInput {
            id: _tagsInput

            Layout.fillWidth: true

            enabled: Runtime.appFeatures.structure.enabled

            addTextButtonTooltip: "Click here to tag the scene with custom keywords."
            completionStrings: Scrite.document.structure.sceneTags
            font: root.font
            labelIconSource: "qrc:/icons/action/keyword.png"
            labelIconVisible: true
            labelText: "Keywords"
            readOnly: !Runtime.appFeatures.structure.enabled && root.readOnly
            textBorderWidth: root.screenplayElementDelegateHasFocus ? 0 : Math.max(0.5, 1 * zoomLevel)
            textColors: root.screenplayElementDelegateHasFocus ? Runtime.colors.accent.c600 : Runtime.colors.accent.c10
            textList: root.scene ? root.scene.tags : 0
            zoomLevel: root.zoomLevel

            onEnsureVisible: (item, area) => { root.ensureVisible(item, area) }
            onTextClicked: (text, source) => { root.sceneTagClicked(text) }
            onTextCloseRequest: (text, source) => { root.scene.removeTag(text) }
            onConfigureTextRequest: (text, tag) => { tag.closable = true }
            onNewTextRequest: (text) => {
                                  root.scene.addTag(text)
                                  root.sceneTagAdded(text)

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
    }

    ActionHandler {
        action: ActionHub.editOptions.find("addOpenTag")
        enabled: root.isCurrent && !_tagsInput.readOnly && !_tagsInput.acceptingNewText

        onTriggered: (source) => {
                         _tagsInput.acceptNewText()
                     }
    }

    QtObject {
        id: _private

        readonly property Action editSceneContent: ActionHub.editOptions.find("editSceneContent")

        property string presentableGroupNames: Scrite.document.structure.presentableGroupNames(root.scene.groups)

        property Component formalTagsMenu: StructureGroupsMenu {
            sceneGroup: SceneGroup {
                scenes: [root.scene]
                structure: Scrite.document.structure
            }
        }

        function popupFormalTagsMenu(parent) {
            let menu = formalTagsMenu.createObject(root)
            menu.closed.connect(menu.destroy)
            menu.popup()
            return menu
        }
    }
}
