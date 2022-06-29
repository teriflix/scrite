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
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

Item {
    id: sceneFeaturedPhotoItem
    property Scene scene

    property Attachments sceneAttachments: scene.attachments
    property Attachment featuredAttachment: sceneAttachments.featuredAttachment
    property Attachment featuredImage: featuredAttachment && featuredAttachment.type === Attachment.Photo ? featuredAttachment : null
    property string fillModeAttrib: "fillMode"
    property int defaultFillMode: Image.PreserveAspectCrop
    property bool mipmap: false

    Image {
        anchors.fill: parent
        fillMode: {
            if(!featuredImage)
                return defaultFillMode
            const ud = featuredImage.userData
            if(ud[fillModeAttrib])
                return ud[fillModeAttrib] === "fit" ? Image.PreserveAspectFit : Image.PreserveAspectCrop
            return defaultFillMode
        }
        source: featuredImage ? featuredImage.fileSource : ""
        visible: featuredImage
        mipmap: sceneFeaturedPhotoItem.mipmap

        RoundButton {
            icon.source: parent.fillMode === Image.PreserveAspectCrop ? "../icons/navigation/zoom_fit.png" : "../icons/navigation/zoom_one.png"
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 5
            hoverEnabled: true
            opacity: hovered ? 1 : 0.5
            enabled: !removeFeaturedImageDialog.active
            onClicked: {
                if(parent.fillMode === Image.PreserveAspectFit)
                    parent.fillMode = Image.PreserveAspectCrop
                else
                    parent.fillMode = Image.PreserveAspectFit

                var ud = featuredImage.userData
                ud[fillModeAttrib] = parent.fillMode === Image.PreserveAspectFit ? "fit" : "crop"
                featuredImage.userData = ud
            }
        }

        RoundButton {
            icon.source: "../icons/action/delete.png"
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 5
            anchors.rightMargin: 10
            hoverEnabled: true
            opacity: hovered ? 1 : 0.5
            onClicked: removeFeaturedImageDialog.active = true
            enabled: !removeFeaturedImageDialog.active
        }

        Loader {
            id: removeFeaturedImageDialog
            anchors.fill: parent
            active: false
            sourceComponent: Rectangle {
                color: Scrite.app.translucent(primaryColors.c600.background,0.85)

                MouseArea {
                    anchors.fill: parent
                }

                Column {
                    width: parent.width-40
                    anchors.centerIn: parent
                    spacing: 40

                    Text {
                        text: "Are you sure you want to remove this photo?"
                        font.bold: true
                        font.pointSize: Scrite.app.idealFontPointSize
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        color: primaryColors.c600.text
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 20

                        Button2 {
                            text: "Yes"
                            focusPolicy: Qt.NoFocus
                            onClicked: {
                                Qt.callLater( () => {
                                                 sceneAttachments.removeAttachment(featuredImage)
                                                 removeFeaturedImageDialog.active = false
                                             } )
                            }
                        }

                        Button2 {
                            text: "No"
                            focusPolicy: Qt.NoFocus
                            onClicked: removeFeaturedImageDialog.active = false
                        }
                    }
                }
            }
        }
    }

    AttachmentsDropArea2 {
        anchors.fill: parent
        allowedType: Attachments.PhotosOnly
        target: sceneAttachments
        onDropped: {
            attachment.featured = true
            allowDrop()
        }
        visible: !featuredAttachment
        attachmentNoticeSuffix: "Drop this photo to tag it as featured image for this scene."

        Column {
            width: parent.width - 20
            anchors.centerIn: parent
            spacing: 10
            visible: !parent.active

            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                font.pointSize: Scrite.app.idealFontPointSize
                text: sceneFeaturedPhotoItem.height > 150 ? "Drag & Drop a Photo\n\n-- OR --" : "Drag & Drop a Photo"
            }

            Button2 {
                text: "Select Photo"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: featuredAttachmentFileDialog.open()
                visible: sceneFeaturedPhotoItem.height > 150
            }
        }

        FileDialog {
            id: featuredAttachmentFileDialog
            nameFilters: sceneAttachments.nameFilters
            selectMultiple: false
            selectExisting: true
            dirUpAction.shortcut: "Ctrl+Shift+U" // The default Ctrl+U interfers with underline
            onAccepted: {
                const attachment = sceneAttachments.includeAttachment( Scrite.app.urlToLocalFile(fileUrl) )
                if(attachment)
                    attachment.featured = true
            }
        }
    }
}

