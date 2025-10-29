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

    height: _layout.height

    Flow {
        id: _layout

        width: parent.width

        flow: Flow.LeftToRight
        enabled: Runtime.appFeatures.structure.enabled
        opacity: enabled ? 1 : 0.5
        spacing: 5
        leftPadding: root.pageLeftMargin
        rightPadding: root.pageRightMargin

        FlatToolButton {
            ToolTip.text: "Formal Story Beats/Tags"

            suggestedWidth: _openTagsLabel.height
            suggestedHeight: _openTagsLabel.height

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

        VclLabel {
            id: _openTagsLabel

            text: "Open Tags: "

            topPadding: 5
            bottomPadding: 5

            font.bold: true
            font.pointSize: Math.max(root.font.pointSize * root.zoomLevel, Runtime.minimumFontMetrics.font.pointSize)
        }

        Repeater {
            model: root.scene ? root.scene.tags : 0

            TagText {
                id: _openTag

                required property string modelData

                property string tagName: modelData

                property var colors: {
                    if(containsMouse)
                        return Runtime.colors.accent.c900
                    return root.screenplayElementDelegateHasFocus ? Runtime.colors.accent.c600 : Runtime.colors.accent.c10
                }

                border.color: colors.text
                border.width: root.screenplayElementDelegateHasFocus ? 0 : Math.max(0.5, 1 * zoomLevel)

                text: tagName
                color: colors.background
                enabled: !Scrite.document.readOnly && Runtime.appFeatures.structure.enabled
                opacity: enabled ? 1 : 0.5
                closable: true
                textColor: colors.text
                topPadding: Math.max(5, 5 * root.zoomLevel)
                leftPadding: Math.max(10, 10 * root.zoomLevel)
                rightPadding: leftPadding
                bottomPadding: topPadding

                font.family: root.font.family
                font.pointSize: Math.max(root.font.pointSize * root.zoomLevel, Runtime.minimumFontMetrics.font.pointSize)

                onClicked: root.sceneTagClicked(tagName)

                onCloseRequest: {
                    if(!Scrite.document.readOnly)
                        root.scene.removeTag(tagName)
                }
            }
        }

        Loader {
            id: _newOpenTagInputLoader

            width: active && item ? Math.max(item.contentWidth, 100) : 0

            active: false

            sourceComponent: VclTextField {
                Component.onCompleted: {
                    forceActiveFocus()
                    root.ensureVisible(_newOpenTagInputLoader, Qt.rect(0,0,width,height))
                }

                Keys.onEscapePressed: {
                    text = ""
                    _newOpenTagInputLoader.active = false
                }

                readOnly: false
                completionStrings: Scrite.document.structure.sceneTags

                font.pointSize: Math.max(root.font.pointSize * root.zoomLevel, Runtime.minimumFontMetrics.font.pointSize)

                onEditingComplete: {
                    if(text.length > 0) {
                        root.scene.addTag(text)
                        root.sceneTagAdded(text)
                    }

                    _newOpenTagInputLoader.active = false
                }
            }

            onStatusChanged: {
                if(status === Loader.Null) {
                    Object.resetProperty(_newOpenTagInputLoader, "width")
                    Object.resetProperty(_newOpenTagInputLoader, "height")
                }
            }
        }

        Image {
            source: "qrc:/icons/content/add_box.png"

            width: _openTagsLabel.height
            height: width

            enabled: !Scrite.document.readOnly
            visible: enabled && Runtime.appFeatures.structure.enabled

            MouseArea {
                ToolTip.text: "Click here to add custom scene tags."
                ToolTip.delay: 1000
                ToolTip.visible: containsMouse

                anchors.fill: parent

                hoverEnabled: true

                onClicked: _newOpenTagInputLoader.active = true
                onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
            }

            Announcement.onIncoming: (type,data) => {
                if(!root.screenplayElementDelegateHasFocus || root.readOnly)
                    return

                var sdata = "" + data
                var stype = "" + type
                if(stype === Runtime.announcementIds.focusRequest && sdata === Runtime.announcementData.focusOptions.addSceneTag) {
                    _newOpenTagInputLoader.active = true
                }
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
