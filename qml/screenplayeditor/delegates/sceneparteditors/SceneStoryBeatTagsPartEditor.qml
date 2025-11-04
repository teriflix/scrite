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

    height: _tagsInput.height

    TextListInput {
        id: _tagsInput

        Announcement.onIncoming: (type,data) => {
            if(!root.screenplayElementDelegateHasFocus || root.readOnly)
                return

            var sdata = "" + data
            var stype = "" + type
            if(stype === Runtime.announcementIds.focusRequest && sdata === Runtime.announcementData.focusOptions.addSceneTag) {
                acceptNewText()
            }
        }

        width: parent.width

        leftPadding: root.pageLeftMargin
        rightPadding: root.pageRightMargin

        enabled: Runtime.appFeatures.structure.enabled

        addTextButtonTooltip: "Click here to add custom scene tags."
        completionStrings: Scrite.document.structure.sceneTags
        font: root.font
        labelIconSource: "qrc:/icons/action/tag.png"
        labelText: "Open Tags"
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
                          }

        header: Row {
            spacing: _tagsInput.spacing

            FlatToolButton {
                ToolTip.text: "Formal Story Beats/Tags"

                suggestedWidth: _tagsInput.label.height
                suggestedHeight: _tagsInput.label.height

                enabled: Runtime.appFeatures.structure.enabled
                opacity: enabled ? 1 : 0.5
                iconSource: "qrc:/icons/action/tag.png"

                onClicked: _private.popupFormalTagsMenu()
            }

            Link {
                width: Math.min(implicitWidth, root.width*0.9)

                text: _private.presentableGroupNames + ", "
                elide: Text.ElideRight
                visible: _private.presentableGroupNames !== ""
                enabled: Runtime.appFeatures.structure.enabled
                opacity: enabled ? 1 : 0.5
                topPadding: 5
                bottomPadding: 5

                font.pointSize: Math.max(root.font.pointSize * root.zoomLevel, Runtime.minimumFontMetrics.font.pointSize)

                onClicked: _private.popupFormalTagsMenu()
            }
        }
    }

    QtObject {
        id: _private

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
