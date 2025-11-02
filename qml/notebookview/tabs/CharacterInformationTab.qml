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

Item {
    id: root

    required property real maxTextAreaSize
    required property real minTextAreaSize

    required property Character character

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 11

        Flickable {
            id: _contentView

            function scrollIntoView(field) {
                const fpos = field.mapToItem(_content, 0, 0)
                if(fpos.y < contentY)
                    contentY = fpos.y
                else if(fpos.y+field.height > contentY+height)
                    contentY = fpos.y+field.height-height
            }

            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical: VclScrollBar { }
            ScrollBar.horizontal: VclScrollBar { }

            contentWidth: _content.width
            contentHeight: _content.height

            RowLayout {
                id: _content

                // Quick Info
                Item {
                    Component.onCompleted: Runtime.execLater(this, 100, function() {
                        _photoSlideView.currentIndex = root.character.hasKeyPhoto ? root.character.keyPhotoIndex : 0
                    } )

                    implicitWidth: _quickInfoLayout.width
                    implicitHeight: _quickInfoLayout.height

                    ColumnLayout {
                        id: _quickInfoLayout

                        width: Runtime.workspaceSettings.showNotebookInStructure ? 300 : Math.max(300, Scrite.window.width*0.3)

                        Rectangle {
                            property bool fillWidth: parent.width < 320

                            width: parent.width-(fillWidth ? 0 : 90)
                            height: width
                            color: _photoSlideView.currentIndex === _photoSlideView.count-1 ? Qt.rgba(0,0,0,0.25) : Qt.rgba(0,0,0,0.75)

                            border.width: 1
                            border.color: Runtime.colors.primary.borderColor

                            SwipeView {
                                id: _photoSlideView

                                anchors.fill: parent
                                anchors.margins: 2

                                clip: true
                                currentIndex: 0

                                Repeater {
                                    model: root.character.photos

                                    Image {
                                        width: _photoSlideView.width
                                        height: _photoSlideView.height

                                        fillMode: Image.PreserveAspectFit
                                        source: "file:///" + modelData
                                    }
                                }

                                Item {
                                    width: _photoSlideView.width
                                    height: _photoSlideView.height

                                    VclButton {
                                        anchors.centerIn: parent

                                        enabled: !Scrite.document.readOnly && _photoSlideView.count <= 6
                                        text: "Add Photo"

                                        onClicked: _fileDialog.open()
                                    }
                                }
                            }

                            FlatToolButton {
                                anchors.left: parent.left
                                anchors.leftMargin: parent.fillWidth ? 0 : -width
                                anchors.verticalCenter: _photoSlideView.verticalCenter

                                enabled: _photoSlideView.currentIndex > 0
                                iconSource: parent.fillWidth ? "qrc:/icons/navigation/arrow_left_inverted.png" : "qrc:/icons/navigation/arrow_left.png"

                                onClicked: _photoSlideView.currentIndex = Math.max(_photoSlideView.currentIndex-1, 0)
                            }

                            FlatToolButton {
                                anchors.right: parent.right
                                anchors.rightMargin: parent.fillWidth ? 0 : -width
                                anchors.verticalCenter: _photoSlideView.verticalCenter

                                enabled: _photoSlideView.currentIndex < _photoSlideView.count-1
                                iconSource: parent.fillWidth ? "qrc:/icons/navigation/arrow_right_inverted.png" : "qrc:/icons/navigation/arrow_right.png"

                                onClicked: _photoSlideView.currentIndex = Math.min(_photoSlideView.currentIndex+1, _photoSlideView.count-1)
                            }

                            FlatToolButton {
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.rightMargin: parent.fillWidth ? 0 : -width

                                iconSource: parent.fillWidth ? "qrc:/icons/action/delete_inverted.png" : "qrc:/icons/action/delete.png"
                                visible: _photoSlideView.currentIndex < _photoSlideView.count-1

                                onClicked: {
                                    var ci = _photoSlideView.currentIndex
                                    root.character.removePhoto(_photoSlideView.currentIndex)
                                    Qt.callLater( function() { _photoSlideView.currentIndex = Math.min(ci,_photoSlideView.count-1) } )
                                }
                            }

                            FlatToolButton {
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.leftMargin: parent.fillWidth ? 0 : -width

                                down: _photoSlideView.currentIndex === root.character.keyPhotoIndex
                                iconSource: parent.fillWidth ? "qrc:/icons/action/pin_inverted.png" : "qrc:/icons/action/pin.png"

                                onClicked: {
                                    if(_photoSlideView.currentIndex === root.character.keyPhotoIndex)
                                        root.character.keyPhotoIndex = 0
                                    else
                                        root.character.keyPhotoIndex = _photoSlideView.currentIndex
                                }
                            }
                        }

                        PageIndicator {
                            Layout.alignment: Qt.AlignHCenter

                            count: _photoSlideView.count
                            currentIndex: _photoSlideView.currentIndex
                            interactive: true

                            onCurrentIndexChanged: _photoSlideView.currentIndex = currentIndex
                        }

                        VclTextField {
                            id: _designationField

                            Layout.fillWidth: true

                            TabSequenceItem.sequence: 0
                            TabSequenceItem.manager: _tabSequence

                            enableTransliteration: true
                            label: "Role / Designation:"
                            labelAlwaysVisible: true
                            maximumLength: 50
                            placeholderText: "Hero/Heroine/Villian/Other <max 50 letters>"
                            readOnly: Scrite.document.readOnly
                            text: root.character.designation

                            onTextEdited: {
                                root.character.designation = text
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true

                            spacing: parent.spacing/2

                            VclTextField {
                                id: _newTagField

                                Layout.fillWidth: true

                                TabSequenceItem.sequence: 1
                                TabSequenceItem.manager: _tabSequence

                                enableTransliteration: true
                                label: "Tags:"
                                labelAlwaysVisible: true
                                maximumLength: 25
                                placeholderText: Platform.isMacOSDesktop ? "<type & hit Return, max 25 chars>" : "<type and hit Enter, max 25 chars>"
                                readOnly: Scrite.document.readOnly

                                onEditingComplete: {
                                    root.character.addTag(text)
                                    clear()
                                }
                            }

                            Flow {
                                id: _tagsFlow

                                Layout.fillWidth: true
                                Layout.bottomMargin: parent.spacing

                                visible: root.character.tags.length > 0

                                Repeater {
                                    model: root.character.tags

                                    TagText {
                                        property var colors: containsMouse ? Runtime.colors.accent.c900 : Runtime.colors.accent.c500

                                        color: colors.background
                                        textColor: colors.text

                                        border.color: colors.text
                                        border.width: 1

                                        closable: Scrite.document.readOnly ? false : true
                                        text: modelData

                                        bottomPadding: 4
                                        leftPadding: 12
                                        rightPadding: 8
                                        topPadding: 4

                                        onCloseRequest: {
                                            if(!Scrite.document.readOnly)
                                                root.character.removeTag(text)
                                        }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true

                            VclLabel {
                                function priority(val) {
                                    var ret = ""
                                    if(val >= -2 && val <= 2)
                                        ret = "Normal"
                                    else if(val >= -6 && val <= -3)
                                        ret = "Low"
                                    else if(val <=-7)
                                        ret = "Very Low"
                                    else if(val>=3 && val <=6)
                                        ret = "High"
                                    else if(val >= 7)
                                        ret = "Very High"

                                    return ret += " (" + val + ")"
                                }

                                Layout.fillWidth: true

                                text: "Priority: " + priority(root.character.priority) + ""
                                elide: Text.ElideMiddle

                                font.pointSize: 2*Runtime.idealFontMetrics.font.pointSize/3
                            }

                            Slider {
                                id: _prioritySlider

                                Layout.fillWidth: true

                                TabSequenceItem.sequence: 2
                                TabSequenceItem.manager: _tabSequence

                                orientation: Qt.Horizontal
                                from: -10
                                to: 10
                                padding: 0
                                stepSize: 1
                                value: root.character.priority

                                onValueChanged: {
                                    root.character.priority = value
                                }
                            }
                        }

                        VclTextField {
                            id: _aliasesField

                            Layout.fillWidth: true

                            TabSequenceItem.sequence: 3
                            TabSequenceItem.manager: _tabSequence

                            enableTransliteration: true
                            label: "Aliases:"
                            labelAlwaysVisible: true
                            maximumLength: 50
                            placeholderText: "<max 50 letters>"
                            readOnly: Scrite.document.readOnly
                            text: root.character.aliases.join(", ")

                            onEditingComplete: {
                                root.character.aliases = text.split(",")
                            }
                        }

                        GridLayout {
                            Layout.fillWidth: true

                            columns: 2

                            VclTextField {
                                id: _typeField

                                Layout.fillWidth: true

                                TabSequenceItem.sequence: 4
                                TabSequenceItem.manager: _tabSequence

                                enableTransliteration: true
                                label: "Type:"
                                labelAlwaysVisible: true
                                maximumLength: 25
                                placeholderText: "Human/Animal/Robot <max 25 letters>"
                                readOnly: Scrite.document.readOnly
                                text: root.character.type

                                onTextEdited: {
                                    root.character.type = text
                                }
                            }

                            VclTextField {
                                id: _genderField

                                Layout.fillWidth: true

                                TabSequenceItem.sequence: 5
                                TabSequenceItem.manager: _tabSequence

                                enableTransliteration: true
                                label: "Gender:"
                                labelAlwaysVisible: true
                                maximumLength: 20
                                placeholderText: "<max 20 letters>"
                                readOnly: Scrite.document.readOnly
                                text: root.character.gender

                                onTextEdited: root.character.gender = text
                            }

                            VclTextField {
                                id: _ageField

                                Layout.fillWidth: true

                                TabSequenceItem.sequence: 6
                                TabSequenceItem.manager: _tabSequence

                                enableTransliteration: true
                                label: "Age:"
                                labelAlwaysVisible: true
                                maximumLength: 20
                                placeholderText: "<max 20 letters>"
                                readOnly: Scrite.document.readOnly
                                text: root.character.age
                                width: (parent.width - parent.spacing)/2

                                onTextEdited: root.character.age = text
                            }

                            VclTextField {
                                id: _bodyTypeField

                                Layout.fillWidth: true

                                TabSequenceItem.sequence: 7
                                TabSequenceItem.manager: _tabSequence

                                enableTransliteration: true
                                label: "Body Type:"
                                labelAlwaysVisible: true
                                maximumLength: 20
                                placeholderText: "<max 20 letters>"
                                readOnly: Scrite.document.readOnly
                                text: root.character.bodyType

                                onTextEdited: root.character.bodyType = text
                            }

                            VclTextField {
                                id: _heightField

                                Layout.fillWidth: true

                                TabSequenceItem.sequence: 8
                                TabSequenceItem.manager: _tabSequence

                                enableTransliteration: true
                                label: "Height:"
                                labelAlwaysVisible: true
                                maximumLength: 20
                                placeholderText: "<max 20 letters>"
                                readOnly: Scrite.document.readOnly
                                text: root.character.height

                                onTextEdited: root.character.height = text
                            }

                            VclTextField {
                                id: _weightField

                                Layout.fillWidth: true

                                TabSequenceItem.sequence: 9
                                TabSequenceItem.manager: _tabSequence

                                enableTransliteration: true
                                label: "Weight:"
                                labelAlwaysVisible: true
                                maximumLength: 20
                                placeholderText: "<max 20 letters>"
                                readOnly: Scrite.document.readOnly
                                text: root.character.weight

                                onTextEdited: root.character.weight = text
                            }
                        }
                    }

                    AttachmentsDropArea {
                        anchors.fill: parent

                        attachmentNoticeSuffix: "Drop here to capture as character pic(s)."
                        allowedType: Attachments.PhotosOnly
                        allowMultiple: true

                        onDropped: {
                            const dus = dropUrls
                            dus.forEach( (url) => { root.character.addPhoto(Url.toPath(url)) } )
                            _photoSlideView.currentIndex = root.character.photos.length - 1
                        }
                    }

                    TabSequenceManager {
                        id: _tabSequence

                        wrapAround: true

                        onCurrentItemChanged: {
                            if(currentItem && currentItem.item)
                                _contentView.scrollIntoView(currentItem.item)
                        }
                    }

                    VclFileDialog {
                        id: _fileDialog

                        folder: Runtime.workspaceSettings.lastOpenPhotosFolderUrl
                        nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
                        selectExisting: true
                        selectFolder: false
                        selectMultiple: false
                        sidebarVisible: true

                        // The default Ctrl+U interfers with underline
                        onFolderChanged: Runtime.workspaceSettings.lastOpenPhotosFolderUrl = folder

                        onAccepted: {
                            if(fileUrl != "") {
                                root.character.addPhoto(Url.toPath(fileUrl))
                                _photoSlideView.currentIndex = root.character.photos.length - 1
                            }
                        }
                    }

                    Connections {
                        target: root

                        function onCharacterChanged() {
                            Runtime.execLater(this, 100, function() {
                                _photoSlideView.currentIndex = root.character.hasKeyPhoto ? root.character.keyPhotoIndex : 0
                            } )
                        }
                    }
                }

                // Character Summary
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    LodLoader {
                        anchors.centerIn: parent
                        anchors.horizontalCenterOffset: -5

                        width: parent.width >= root.maxTextAreaSize+20 ? root.maxTextAreaSize : parent.width-20
                        height: parent.height

                        lod: Runtime.notebookSettings.richTextNotesEnabled ? LodLoader.LOD.High : LodLoader.LOD.Low
                        resetHeightBeforeLodChange: false
                        resetWidthBeforeLodChange: false
                        sanctioned: root.character

                        lowDetailComponent: FlickableTextArea {
                            DeltaDocument {
                                id: _summaryContent
                                content: root.character.summary
                            }

                            placeholderText: "Character Summary"
                            tabSequenceIndex: 10
                            tabSequenceManager: _tabSequence
                            text: _summaryContent.plainText

                            background: Rectangle {
                                color: Runtime.colors.primary.windowColor
                                opacity: 0.15
                            }

                            onTextChanged: {
                                if(textArea.activeFocus)
                                    root.character.summary = text
                            }
                        }

                        highDetailComponent: RichTextEdit {
                            adjustTextWidthBasedOnScrollBar: false
                            placeholderText: "Character Summary"
                            tabSequenceIndex: 10
                            tabSequenceManager: _tabSequence
                            text: root.character.summary

                            onTextChanged: {
                                root.character.summary = text
                            }
                        }
                    }

                    AttachmentsDropArea {
                        anchors.fill: parent

                        allowMultiple: true
                        target: _private.attachments
                    }
                }
            }
        }

        AttachmentsView {
            Layout.fillWidth: true

            attachments: _private.attachments

            AttachmentsDropArea {
                anchors.fill: parent

                noticeWidthFactor: 0.8
                allowMultiple: true

                target: _private.attachments
            }
        }
    }

    QtObject {
        id: _private

        property Attachments attachments: root.character ? root.character.attachments : null
    }
}
