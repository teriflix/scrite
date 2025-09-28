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
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors/helpers"

AbstractScenePartEditor {
    id: root

    property alias additionalSceneMenuItems: _sceneMenu.additionalSceneMenuItems

    signal additionalSceneMenuItemClicked(string name)

    height: _layout.height

    RowLayout {
        id: _layout

        width: parent.width

        Item {
            Layout.minimumWidth: root.pageLeftMargin
            Layout.maximumWidth: root.pageLeftMargin

            TextField {
                anchors.right: parent.right
                anchors.rightMargin: root.pageLeftMargin * 0.1
                anchors.verticalCenter: parent.verticalCenter

                width: root.fontMetrics.averageCharacterWidth * 5

                text: root.screenplayElement.hasUserSceneNumber ? root.screenplayElement.resolvedSceneNumber : ""
                font: root.font
                placeholderText: root.scene.heading.enabled ? root.screenplayElement.sceneNumber : ("#" + (root.index+1))

                background: Item { }
            }
        }

        SceneHeadingTextField {
            Layout.fillWidth: true

            focus: true
            sceneOmitted: root.screenplayElement.omitted
            sceneHeading: root.scene.heading

            Announcement.onIncoming: (type,data) => {
                if(!root.screenplayElementDelegateHasFocus || root.readOnly)
                    return

                var sdata = "" + data
                var stype = "" + type
                if(stype === Runtime.announcementIds.focusRequest && sdata === Runtime.announcementData.focusOptions.sceneHeading) {
                    forceActiveFocus()
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: root.pageRightMargin
            Layout.maximumWidth: root.pageRightMargin

            FlatToolButton {
                ToolTip.text: "Formal Story Beats/Tags"

                Layout.preferredWidth: suggestedWidth
                Layout.preferredHeight: suggestedHeight

                suggestedWidth: Runtime.iconImageSize
                suggestedHeight: Runtime.iconImageSize

                enabled: Runtime.appFeatures.structure.enabled
                opacity: enabled ? 1 : 0.5
                iconSource: "qrc:/icons/action/tag.png"

                onClicked: _private.popupFormalTagsMenu()
            }

            FlatToolButton {
                ToolTip.text: "Scene Menu"

                Layout.preferredWidth: suggestedWidth
                Layout.preferredHeight: suggestedHeight

                suggestedWidth: Runtime.iconImageSize
                suggestedHeight: Runtime.iconImageSize

                iconSource: "qrc:/icons/navigation/menu.png"

                onClicked: _sceneMenu.popup()

                SceneMenu {
                    id: _sceneMenu

                    anchors.top: parent.bottom
                    anchors.left: parent.left

                    index: root.index
                    screenplayElement: root.screenplayElement
                    screenplayAdapter: root.screenplayAdapter

                    onAdditionalSceneMenuItemClicked: (name) => { root.additionalSceneMenuItemClicked(name) }
                }
            }

            Image {
                Layout.fillWidth: true
            }
        }
    }

    QtObject {
        id: _private

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
