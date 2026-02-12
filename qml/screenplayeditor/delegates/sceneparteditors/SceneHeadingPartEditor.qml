/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
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
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor/delegates/sceneparteditors/helpers"

AbstractScenePartEditor {
    id: root

    implicitHeight: _layout.height

    RowLayout {
        id: _layout

        width: parent.width

        Item {
            Layout.minimumWidth: root.pageLeftMargin
            Layout.maximumWidth: root.pageLeftMargin

            Row {
                anchors.left: parent.left
                anchors.right: _sceneNumber.left
                anchors.margins: 16 * root.zoomLevel
                anchors.verticalCenter: parent.verticalCenter

                spacing: 4 * root.zoomLevel

                SceneTypeImage {
                    id: _sceneTypeIcon

                    width: root.fontMetrics.height
                    height: root.fontMetrics.height

                    visible: sceneType !== Scene.Standard
                    sceneType: root.scene ? root.scene.type : Scene.Standard
                    showTooltip: false
                    lightBackground: Color.isLight(Runtime.colors.tint(root.scene.color, Runtime.colors.sceneHeadingTint))

                    onClicked: _private.popupMarkSceneAsMenu(_sceneTypeIcon)
                }

                Image {
                    width: root.fontMetrics.height
                    height: root.fontMetrics.height

                    visible: Runtime.screenplayEditorSettings.longSceneWarningEnabled &&
                             root.scene.wordCount > Runtime.screenplayEditorSettings.longSceneWordTreshold
                    smooth: true
                    mipmap: true
                    source: "qrc:/icons/content/warning.png"
                    fillMode: Image.PreserveAspectFit

                    MouseArea {
                        anchors.fill: parent

                        hoverEnabled: enabled

                        ToolTipPopup {
                            text: "" + root.scene.wordCount + " words (limit: " + Runtime.screenplayEditorSettings.longSceneWordTreshold + ").\nRefer Settings > Screenplay > Options tab."
                            visible: container.containsMouse
                        }

                        onClicked: SettingsDialog.launch("Screenplay")
                    }
                }
            }

            TextField {
                id: _sceneNumber

                Keys.onPressed: (event) => {
                                    event.accepted = false

                                    if(event.key === Qt.Key_Escape && root.isCurrent) {
                                        event.accepted = true
                                        _private.editSceneContent.trigger()
                                    }
                                }

                anchors.right: parent.right
                anchors.rightMargin: root.pageLeftMargin * 0.1
                anchors.verticalCenter: parent.verticalCenter

                width: root.fontMetrics.averageCharacterWidth * 5

                text: root.screenplayElement.hasUserSceneNumber ? root.screenplayElement.resolvedSceneNumber : ""
                font: _sceneHeading.font
                readOnly: root.readOnly
                placeholderText: root.scene.heading.enabled ? root.screenplayElement.sceneNumber : "-"

                background: Item { }

                ActionHandler {
                    action: ActionHub.editOptions.find("editSceneNumber")
                    enabled: root.isCurrent && !root.readOnly && !_sceneNumber.activeFocus

                    onTriggered: (source) => {
                                     _sceneNumber.selectAll()
                                     _sceneNumber.forceActiveFocus()
                                 }
                }

                onTextEdited: {
                    root.screenplayElement.userSceneNumber = text
                }

                onEditingFinished: {
                    if(root.isCurrent) {
                        _private.editSceneContent.trigger()
                    }
                }

                onActiveFocusChanged: {
                    if(activeFocus)
                        root.ensureVisible(_sceneNumber, _sceneNumber.cursorRectangle)
                }
            }
        }

        SceneHeadingTextField {
            id: _sceneHeading

            Layout.fillWidth: true

            DelayedProperty.watch: completionHasSuggestions

            Keys.onPressed: (event) => {
                                event.accepted = false

                                if(event.key === Qt.Key_Escape && root.isCurrent) {
                                    event.accepted = true
                                    _private.editSceneContent.trigger()
                                }
                            }

            focus: true
            sceneOmitted: root.screenplayElement.omitted
            sceneHeading: root.scene.heading

            ActionHandler {
                action: ActionHub.paragraphFormats.find("headingParagraph")
                enabled: root.isCurrent && !root.readOnly && !_sceneHeading.activeFocus

                onTriggered: (source) => {
                                 _sceneHeading.selectAll()
                                 _sceneHeading.forceActiveFocus()
                             }
            }

            onEditingComplete: {
                if(!DelayedProperty.value && root.isCurrent) {
                    _private.editSceneContent.trigger()
                }
            }

            onActiveFocusChanged: {
                if(activeFocus)
                    root.ensureVisible(_sceneHeading, _sceneHeading.cursorRectangle)
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: root.pageRightMargin
            Layout.maximumWidth: root.pageRightMargin

            FlatToolButton {
                toolTipText: "Scene Menu"
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
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    QtObject {
        id: _private

        readonly property Action editSceneContent: ActionHub.editOptions.find("editSceneContent")

        property Component markSceneAsMenu: MarkSceneAsMenu {
            scene: root.scene
        }

        function popupMarkSceneAsMenu(parent) {
            let menu = markSceneAsMenu.createObject(root)
            menu.closed.connect(menu.destroy)
            menu.popup()
            return menu
        }

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
