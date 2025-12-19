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
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: _attachmentsView.top
        anchors.margins: 11

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Runtime.colors.primary.c10.background
            border.color: Runtime.colors.primary.borderColor
            border.width: _contentView.height < _contentView.contentHeight ? 1 : 0

            Flickable {
                id: _contentView

                function scrollIntoView(field) {
                    const fpos = field.mapToItem(_content, 0, 0)
                    if(fpos.y < contentY)
                        contentY = fpos.y
                    else if(fpos.y+field.height > contentY+height)
                        contentY = fpos.y+field.height-height
                }

                ScrollBar.vertical: VclScrollBar { }

                anchors.fill: parent
                anchors.margins: 1
                anchors.leftMargin: height < contentHeight ? 11 : 1

                contentWidth: _content.width
                contentHeight: _content.height

                clip: true

                RowLayout {
                    id: _content

                    width: _contentView.width

                    // Quick Info
                    Item {
                        readonly property real minimumWidth: 300

                        Component.onCompleted: Runtime.execLater(this, 100, function() {
                            _photoSlideView.currentIndex = root.character.hasKeyPhoto ? root.character.keyPhotoIndex : 0
                        } )

                        implicitWidth: Runtime.showNotebookInStructure ? minimumWidth : Math.max(Scrite.window.width * 0.15, minimumWidth)
                        implicitHeight: Math.max(_quickInfoLayout.height, _contentView.height)

                        ColumnLayout {
                            id: _quickInfoLayout

                            width: parent.width

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: width

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

                                        delegate: Image {
                                            required property int index
                                            required property string modelData

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
                            }

                            PageIndicator {
                                Layout.alignment: Qt.AlignHCenter

                                count: _photoSlideView.count
                                currentIndex: _photoSlideView.currentIndex
                                interactive: true
                                visible: _photoSlideView.count >= 4

                                onCurrentIndexChanged: _photoSlideView.currentIndex = currentIndex
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.bottomMargin: 20

                                FlatToolButton {
                                    down: _photoSlideView.currentIndex === root.character.keyPhotoIndex
                                    enabled: root.character.photos.length > 0
                                    iconSource: parent.fillWidth ? "qrc:/icons/action/pin_inverted.png" : "qrc:/icons/action/pin.png"

                                    onClicked: {
                                        if(_photoSlideView.currentIndex === root.character.keyPhotoIndex)
                                            root.character.keyPhotoIndex = 0
                                        else
                                            root.character.keyPhotoIndex = _photoSlideView.currentIndex
                                    }
                                }

                                Item {
                                    Layout.fillWidth: true
                                }

                                FlatToolButton {
                                    enabled: _photoSlideView.currentIndex > 0
                                    iconSource: parent.fillWidth ? "qrc:/icons/navigation/arrow_left_inverted.png" : "qrc:/icons/navigation/arrow_left.png"

                                    onClicked: _photoSlideView.currentIndex = Math.max(_photoSlideView.currentIndex-1, 0)
                                }

                                FlatToolButton {
                                    enabled: _photoSlideView.currentIndex < _photoSlideView.count-1
                                    iconSource: parent.fillWidth ? "qrc:/icons/navigation/arrow_right_inverted.png" : "qrc:/icons/navigation/arrow_right.png"

                                    onClicked: _photoSlideView.currentIndex = Math.min(_photoSlideView.currentIndex+1, _photoSlideView.count-1)
                                }

                                Item {
                                    Layout.fillWidth: true
                                }

                                FlatToolButton {
                                    enabled: _photoSlideView.currentIndex < _photoSlideView.count-1
                                    iconSource: parent.fillWidth ? "qrc:/icons/action/delete_inverted.png" : "qrc:/icons/action/delete.png"

                                    onClicked: {
                                        var ci = _photoSlideView.currentIndex
                                        root.character.removePhoto(_photoSlideView.currentIndex)
                                        Qt.callLater( function() { _photoSlideView.currentIndex = Math.min(ci,_photoSlideView.count-1) } )
                                    }
                                }
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

                                        delegate: TagText {
                                            required property int index
                                            required property string modelData

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
        }
    }

    AttachmentsView {
        id: _attachmentsView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        attachments: _private.attachments

        AttachmentsDropArea {
            anchors.fill: parent

            noticeWidthFactor: 0.8
            allowMultiple: true

            target: _private.attachments
        }
    }

    QtObject {
        id: _private

        property Attachments attachments: root.character ? root.character.attachments : null
    }
}
