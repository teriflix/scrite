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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"
import "qrc:/qml/structureview"

Item {
    id: root

    required property real maxTextAreaSize
    required property real minTextAreaSize

    required property Scene scene

    TabSequenceManager {
        id: _sceneTabSequence

        enabled: parent.visible
    }

    ColumnLayout {
        EventFilter.active: _private.sceneSynopsisField !== null
        EventFilter.events: [EventFilter.Wheel]
        EventFilter.onFilter: {
            EventFilter.forwardEventTo(_private.sceneSynopsisField)
            result.filter = true
            result.accepted = true
        }

        anchors.top: parent.top
        anchors.bottom: _sceneAttachments.top
        anchors.margins: 10
        anchors.bottomMargin: 0
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width >= root.maxTextAreaSize+20 ? root.maxTextAreaSize : parent.width-20

        spacing: 10

        VclTextField {
            id: _sceneHeadingField

            TabSequenceItem.manager: _sceneTabSequence
            TabSequenceItem.sequence: 0

            Layout.fillWidth: true

            text: root.scene.heading.text
            label: ""
            wrapMode: Text.WordWrap
            placeholderText: "Scene Heading"
            readOnly: Scrite.document.readOnly
            enabled: root.scene.heading.enabled

            font.capitalization: Font.AllUppercase
            font.family: Scrite.document.formatting.elementFormat(SceneElement.Heading).font.family
            font.pointSize: Runtime.idealFontMetrics.font.pointSize+2

            onEditingComplete: root.scene.heading.parseFrom(text)
        }

        VclTextField {
            id: _sceneTitleField

            TabSequenceItem.manager: _sceneTabSequence
            TabSequenceItem.sequence: 1

            Layout.fillWidth: true

            backTabItem: _sceneHeadingField
            label: ""
            placeholderText: "Scene Title"
            readOnly: Scrite.document.readOnly
            text: root.scene.structureElement.nativeTitle
            wrapMode: Text.WordWrap

            onEditingComplete: root.scene.structureElement.title = text
        }

        Row {
            Layout.fillWidth: true

            spacing: _sceneTagsList.spacing

            FlatToolButton {
                suggestedWidth: _sceneTagsList.label.height
                suggestedHeight: _sceneTagsList.label.height

                enabled: Runtime.appFeatures.structure.enabled
                opacity: enabled ? 1 : 0.5
                iconSource: "qrc:/icons/action/tag.png"
                toolTipText: "Formal Story Beats/Tags"

                onClicked: _private.popupFormalTagsMenu()
            }

            Text {
                font: _sceneTagsList.label.font
                text: "Formal Tags"
                visible: _private.presentableGroupNames === ""
            }

            Link {
                width: Math.min(implicitWidth, root.width*0.9)

                text: _private.presentableGroupNames
                elide: Text.ElideRight
                visible: _private.presentableGroupNames !== ""
                enabled: Runtime.appFeatures.structure.enabled
                opacity: enabled ? 1 : 0.5
                topPadding: 5
                bottomPadding: 5

                onClicked: _private.popupFormalTagsMenu()
            }
        }

        TextListInput {
            id: _sceneTagsList

            Layout.fillWidth: true

            enabled: Runtime.appFeatures.structure.enabled

            addTextButtonTooltip: "Click here to tag the scene with custom keywords."
            completionStrings: Scrite.document.structure.sceneTags
            labelIconSource: "qrc:/icons/action/keyword.png"
            labelIconVisible: true
            labelText: "Keywords"
            readOnly: !Runtime.appFeatures.structure.enabled && Scrite.document.readOnly
            textList: root.scene ? root.scene.tags : 0

            onTextCloseRequest: (text, source) => { root.scene.removeTag(text) }
            onConfigureTextRequest: (text, tag) => { tag.closable = true }
            onNewTextRequest: (text) => {
                                  root.scene.addTag(text)
                              }
        }

        TextListInput {
            id: _sceneCharactersList

            Component.onCompleted: font.capitalization = Font.AllUppercase

            Layout.fillWidth: true

            addTextButtonTooltip: "Click here to capture characters who don't have any dialogues in this scene, but are still required for the scene."
            completionStrings: Scrite.document.structure.characterNames
            labelIconSource: "qrc:/icons/content/persons_add.png"
            labelText: "Characters"
            readOnly: Scrite.document.readOnly
            textList: root.scene ? root.scene.characterNames : 0

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
                              }
        }

        TrapeziumTabView {
            id: _synopsisContentTabView

            Layout.fillWidth: true
            Layout.fillHeight: true

            tabNames: ["Synopsis", "Featured Photo"]
            tabColor: root.scene.color
            currentTabContent: currentTabIndex === 0 ? _sceneSynopsisFieldComponent : _featuredPhotoComponent
            currentTabIndex: Runtime.notebookSettings.sceneSynopsisTabIndex

            onCurrentTabIndexChanged: Runtime.notebookSettings.sceneSynopsisTabIndex = currentTabIndex
        }
    }

    AttachmentsView {
        id: _sceneAttachments

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        attachments: root.scene ? root.scene.attachments : null
    }

    AttachmentsDropArea {
        id: _sceneAttachmentsDropArea

        anchors.fill: _synopsisContentTabView.currentTabIndex === 1 ? _sceneAttachments : parent

        target: root.scene ? root.scene.attachments : null
        allowMultiple: true
    }

    Component {
        id: _sceneSynopsisFieldComponent

        ColumnLayout {
            property alias textArea: _sceneSynopsisField.textArea

            FlickableTextArea {
                id: _sceneSynopsisField

                Layout.fillWidth: true
                Layout.fillHeight: true

                TabSequenceItem.manager: _sceneTabSequence
                TabSequenceItem.sequence: 2
                TabSequenceItem.onAboutToReceiveFocus: Qt.callLater(textArea.forceActiveFocus)

                // Unfortunately, focus scope doesnt really work!
                EventFilter.target: textArea
                EventFilter.events: [EventFilter.KeyPress]
                EventFilter.active: textArea.activeFocus
                EventFilter.onFilter: (watched, event) => {
                                          if(event.key === Qt.Key_Tab)
                                          TabSequenceItem.focusNext()
                                          else if(event.key === Qt.Key_Backtab)
                                          TabSequenceItem.focusPrevious()
                                      }

                Component.onCompleted: _private.sceneSynopsisField = _sceneSynopsisField

                background: Rectangle {
                    color: Runtime.colors.primary.windowColor
                    opacity: 0.15
                }

                text: root.scene.synopsis
                readOnly: Scrite.document.readOnly
                placeholderText: "Scene Synopsis"
                undoRedoEnabled: true
                adjustTextWidthBasedOnScrollBar: false

                onTextChanged: root.scene.synopsis = text
            }

            IndexCardFields {
                id: _indexCardFields

                Layout.fillWidth: true

                lod: LodLoader.LOD.High
                startTabSequence: 3
                structureElement: root.scene.structureElement
                tabSequenceEnabled: true
                tabSequenceManager: _sceneTabSequence
                visible: hasFields
            }
        }
    }

    Component {
        id: _featuredPhotoComponent

        SceneFeaturedImage {
            defaultFillMode: Image.PreserveAspectFit
            fillModeAttrib: "notebookFillMode"
            mipmap: true
            scene: root.scene
        }
    }

    QtObject {
        id: _private

        property FlickableTextArea sceneSynopsisField

        property string presentableGroupNames: Scrite.document.structure.presentableGroupNames(root.scene.groups)

        readonly property Component formalTagsMenu: StructureGroupsMenu {
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

