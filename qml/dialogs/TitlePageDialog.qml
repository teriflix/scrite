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

pragma Singleton

import QtQuick 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

DialogLauncher {
    id: root

    parent: Scrite.window.contentItem

    function launch() { return doLaunch() }

    name: "TitlePageDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        title: "Title Page"
        width: Math.min(Scrite.window.width-80, 1050)
        height: Math.min(Scrite.window.height-80, 750)

        content: Item {
            implicitHeight: titlePageSettingsLayout.implicitHeight+60

            TabSequenceManager {
                id: titlePageFieldsTabSequence
                wrapAround: true
            }

            ColumnLayout {
                id: titlePageSettingsLayout
                width: parent.width-160
                spacing: 30
                anchors.centerIn: parent

                // Cover page photo field
                Rectangle {
                    id: coverPageEdit
                    /*
                          At best we can paint a 464x261 point photo on the cover page. Nothing more.
                          So, we need to provide a image preview in this aspect ratio.
                          */
                    color: dialog.background.item.color
                    border.width: Scrite.document.screenplay.coverPagePhoto === "" ? 1 : 0
                    border.color: "black"
                    Layout.preferredWidth: 400
                    Layout.preferredHeight: 255
                    Layout.alignment: Qt.AlignHCenter

                    Loader {
                        id: coverPagePhotoLoader
                        anchors.fill: parent
                        anchors.margins: 1
                        active: Scrite.document.screenplay.coverPagePhoto !== ""
                        sourceComponent: Item {
                            Image {
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                source: coverPagePhoto.source
                                visible: (coverPagePhoto.go && coverPagePhoto.status === Image.Ready) && (coverPagePhoto.paintedWidth < width || coverPagePhoto.paintedHeight < height)
                                opacity: 0.1 * coverPagePhoto.opacity
                                cache: false
                            }

                            Image {
                                id: coverPagePhoto
                                anchors.fill: parent
                                smooth: true; mipmap: true
                                asynchronous: true
                                fillMode: Image.PreserveAspectFit
                                source: go ? "file:///" + Scrite.document.screenplay.coverPagePhoto : ""
                                opacity: coverPagePhotoMouseArea.containsMouse ? 0.25 : 1
                                cache: false
                                property bool go: false

                                BusyIndicator {
                                    anchors.centerIn: parent
                                    running: parent.status === Image.Loading || !parent.go
                                }

                                Component.onCompleted: Utils.execLater(coverPagePhoto, 400, () => {
                                                                           coverPagePhoto.go = true
                                                                       } )
                            }
                        }
                    }

                    VclLabel {
                        anchors.fill: parent
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        opacity: coverPagePhotoMouseArea.containsMouse ? 1 : (Scrite.document.screenplay.coverPagePhoto === "" ? 0.5 : 0)
                        text: Scrite.document.screenplay.coverPagePhoto === "" ? "Click here to set the cover page photo" : "Click here to change the cover page photo"
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    }

                    MouseArea {
                        id: coverPagePhotoMouseArea
                        anchors.fill: parent
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        hoverEnabled: true
                        enabled: !Scrite.document.readOnly
                        onClicked: fileDialog.open()
                    }

                    AttachmentsDropArea {
                        anchors.fill: parent
                        attachmentNoticeSuffix: "Drop to set as cover page photo."
                        visible: !Scrite.document.readOnly
                        allowedType: Attachments.PhotosOnly
                        onDropped: {
                            Scrite.document.screenplay.clearCoverPagePhoto()
                            var filePath = attachment.filePath
                            Qt.callLater( function(fp) {
                                Scrite.document.screenplay.setCoverPagePhoto(fp)
                            }, filePath)
                        }
                    }

                    Column {
                        spacing: 0
                        anchors.left: parent.right
                        anchors.leftMargin: 20
                        visible: Scrite.document.screenplay.coverPagePhoto !== ""
                        enabled: visible && !Scrite.document.readOnly

                        VclLabel {
                            text: "Cover Photo Size"
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            topPadding: 5
                            bottomPadding: 5
                            color: Runtime.colors.primary.c300.text
                            opacity: enabled ? 1 : 0.5
                        }

                        VclRadioButton {
                            text: "Small"
                            checked: Scrite.document.screenplay.coverPagePhotoSize === Screenplay.SmallCoverPhoto
                            onToggled: Scrite.document.screenplay.coverPagePhotoSize = Screenplay.SmallCoverPhoto
                        }

                        VclRadioButton {
                            text: "Medium"
                            checked: Scrite.document.screenplay.coverPagePhotoSize === Screenplay.MediumCoverPhoto
                            onToggled: Scrite.document.screenplay.coverPagePhotoSize = Screenplay.MediumCoverPhoto
                        }

                        VclRadioButton {
                            text: "Large"
                            checked: Scrite.document.screenplay.coverPagePhotoSize === Screenplay.LargeCoverPhoto
                            onToggled: Scrite.document.screenplay.coverPagePhotoSize = Screenplay.LargeCoverPhoto
                        }

                        VclButton {
                            text: "Remove"
                            onClicked: Scrite.document.screenplay.clearCoverPagePhoto()
                        }
                    }

                    VclFileDialog {
                        id: fileDialog
                        nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
                        selectFolder: false
                        selectMultiple: false
                        sidebarVisible: true
                        selectExisting: true
                         // The default Ctrl+U interfers with underline
                        onAccepted: {
                            if(fileUrl != "")
                            Scrite.document.screenplay.setCoverPagePhoto(Scrite.app.urlToLocalFile(fileUrl))
                        }
                        folder: Runtime.workspaceSettings.lastOpenPhotosFolderUrl
                        onFolderChanged: Runtime.workspaceSettings.lastOpenPhotosFolderUrl = folder
                    }
                }

                Grid {
                    id: titlePageFields
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    rowSpacing: 5
                    columnSpacing: 40
                    enabled: !Scrite.document.readOnly
                    flow: Flow.TopToBottom
                    columns: 2

                    Repeater {
                        model: ListModel {
                            ListElement { name: "Title";      fieldSize: 100;    key: "title"       }
                            ListElement { name: "Subtitle";   fieldSize: 100;    key: "subtitle"    }
                            ListElement { name: "Based on";   fieldSize: 100;    key: "basedOn"     }
                            ListElement { name: "Version";    fieldSize: 20 ;    key: "version"     }
                            ListElement { name: "Written by"; fieldSize: 100;    key: "authorValue" }
                            ListElement { name: "Contact";    fieldSize: 100;    key: "contact"     }
                            ListElement { name: "Address";    fieldSize: 100;    key: "address"     }
                            ListElement { name: "Email";      fieldSize: 100;    key: "email"       }
                            ListElement { name: "Phone";      fieldSize: 100;    key: "phoneNumber" }
                            ListElement { name: "Website";    fieldSize: 100;    key: "website"     }
                        }

                        Item {
                            required property int index
                            required property string name
                            required property int fieldSize
                            required property string key

                            width: (titlePageFields.width-titlePageFields.columnSpacing)/2
                            height: _tpfRow.height

                            TextLimiter {
                                id: _tpfLimiter
                                maxWordCount: fieldSize
                                maxLetterCount: fieldSize
                                countMode: TextLimiter.CountInText
                                text: _tpfScreenplayProperty.value
                            }

                            PropertyAlias {
                                id: _tpfScreenplayProperty
                                sourceObject: Scrite.document.screenplay
                                sourceProperty: key
                            }

                            Row {
                                id: _tpfRow
                                width: parent.width
                                spacing: 10

                                VclLabel {
                                    width: _private.fieldLabelWidth
                                    horizontalAlignment: Text.AlignRight
                                    text: name
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize-2
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: Runtime.colors.primary.c800.background
                                }

                                VclTextField {
                                    id: _tpfField
                                    width: parent.width-parent.spacing-_private.fieldLabelWidth
                                    text: _tpfScreenplayProperty.value
                                    selectByMouse: true
                                    placeholderText: activeFocus ?
                                                         (text === "" ? ("(max " + _tpfLimiter.maxLetterCount + " letters)") : (_tpfLimiter.letterCount + "/" + _tpfLimiter.maxLetterCount)) :
                                                         ""
                                    labelColor: _tpfLimiter.limitReached ? "red" : "gray"
                                    labelTextAlign: Text.AlignRight
                                    onTextEdited: {
                                        _tpfLimiter.text = text
                                        _tpfScreenplayProperty.value = text
                                    }
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize+1
                                    enableTransliteration: true
                                    TabSequenceItem.manager: titlePageFieldsTabSequence
                                    TabSequenceItem.sequence: index
                                }
                            }
                        }
                    }
                }

                Item {
                    id: titlePageOptions

                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: useAsDefaultsButton.height

                    VclCheckBox {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter

                        text: "Include Title Page In Preview"
                        checked: Runtime.screenplayEditorSettings.includeTitlePageInPreview
                        onToggled: Runtime.screenplayEditorSettings.includeTitlePageInPreview = checked
                    }

                    VclButton {
                        id: useAsDefaultsButton
                        anchors.centerIn: parent

                        text: "Use As Defaults"
                        hoverEnabled: true
                        ToolTip.visible: hovered && defaultsSavedNotice.opacity === 0
                        ToolTip.text: "Click this button to use Address, Author, Contact, Email, Phone and Website field values from this dialogue as default from now on."
                        ToolTip.delay: 1000
                        onClicked: {
                            Runtime.titlePageSettings.author = Scrite.document.screenplay.author
                            Runtime.titlePageSettings.contact = Scrite.document.screenplay.contact
                            Runtime.titlePageSettings.address = Scrite.document.screenplay.address
                            Runtime.titlePageSettings.email = Scrite.document.screenplay.email
                            Runtime.titlePageSettings.phone = Scrite.document.screenplay.phoneNumber
                            Runtime.titlePageSettings.website = Scrite.document.screenplay.website
                            defaultsSavedNotice.opacity = 1
                        }
                    }

                    VclCheckBox {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        text: "Include Timestamp"
                        checked: Runtime.titlePageSettings.includeTimestamp
                        onToggled: Runtime.titlePageSettings.includeTimestamp = checked
                    }
                }

            }

            VclLabel {
                id: defaultsSavedNotice
                anchors.top: titlePageSettingsLayout.bottom
                anchors.topMargin: 10
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Address, Author, Contact, Email, Phone and Website field values saved as default."
                opacity: 0
                onOpacityChanged: {
                    if(opacity > 0)
                    Utils.execLater(defaultsSavedNotice, 2500, function() { defaultsSavedNotice.opacity = 0 })
                }
            }
        }

        QtObject {
            id: _private

            readonly property real fieldLabelWidth: 60
        }
    }
}
