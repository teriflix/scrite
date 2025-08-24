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

AbstractScenePartEditor {
    id: root

    signal sceneTagAdded(string tagName)
    signal sceneTagClicked(string tagName)

    height: _layout.height

    Flow {
        id: _layout

        flow: Flow.LeftToRight
        spacing: 5
        leftPadding: root.pageLeftMargin
        rightPadding: root.pageRightMargin

        Link {
            text: Scrite.document.structure.presentableGroupNames(root.scene.groups)

            width: Math.min(contentWidth, root.width*0.35)
            elide: Text.ElideRight
            visible: text !== ""
            topPadding: 5
            bottomPadding: 5
            font.pointSize: _private.tagFontPointSize

            onClicked: _private.popupFormalTagsMenu()
        }

        VclLabel {
            id: _openTagsLabel

            text: "Open Tags: "

            visible: root.scene.groups.length === 0
            topPadding: 5
            bottomPadding: 5

            font.bold: true
            font.pointSize: _private.tagFontPointSize
        }

        Repeater {
            model: root.scene ? root.scene.groups : 0

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
                enabled: !Scrite.document.readOnly
                closable: true
                textColor: colors.text
                topPadding: Math.max(5, 5 * root.zoomLevel)
                leftPadding: Math.max(10, 10 * root.zoomLevel)
                rightPadding: leftPadding
                bottomPadding: topPadding

                font.family: Runtime.idealFontMetrics.font.family
                font.pointSize: _private.tagFontPointSize
                font.capitalization: Font.AllUppercase

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

            sourceComponent: Item {
                property alias contentWidth: _newOpenTagInput.contentWidth

                height: _newOpenTagInput.height

                Component.onCompleted: root.ensureVisible(_newOpenTagInputLoader, Qt.rect(0,0,width,height))

                TextViewEdit {
                    id: _newOpenTagInput

                    y: fontDescent
                    width: parent.width

                    wrapMode: Text.NoWrap
                    readOnly: false
                    completionStrings: Scrite.document.structure.sceneTags
                    horizontalAlignment: Text.AlignLeft

                    font.pointSize: _private.tagFontPointSize

                    onEditingFinished: {
                        root.scene.addTag(text)
                        root.sceneTagAdded(text)
                        _newOpenTagInputLoader.active = false
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: parent.fontHeight - parent.fontAscent - parent.fontHeight*0.25

                        height: 1

                        color: Runtime.colors.accent.borderColor
                    }
                }
            }
        }

        Image {
            source: "qrc:/icons/content/add_box.png"

            width: _openTagsLabel.height
            height: width

            opacity: 0.5
            visible: enabled
            enabled: !Scrite.document.readOnly

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

        property real tagFontPointSize: Math.max(sceneHeadingFormat.font2.pointSize*0.7, 6)
        property SceneElementFormat sceneHeadingFormat: Scrite.document.displayFormat.elementFormat(SceneElement.Heading)

        property SceneGroup formalTags: SceneGroup {
            scenes: [root.scene]
            structure: Scrite.document.structure
        }

        property Component formalTagsMenu: StructureGroupsMenu {
            sceneGroup: _private.formalTags
        }

        function popupFormalTagsMenu(parent) {
            let menu = formalTagsMenu.createObject(parent)
            menu.closed.connect(menu.destroy)
            menu.popup()
            return menu
        }
    }
}
