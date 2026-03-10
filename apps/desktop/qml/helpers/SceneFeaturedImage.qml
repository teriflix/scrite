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

import QtQml
import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../controls"

Item {
    id: root

    required property Scene scene

    property int defaultFillMode: Image.PreserveAspectCrop
    property bool mipmap: false
    property string fillModeAttrib: "fillMode"

    property Attachment featuredImage: featuredAttachment && featuredAttachment.type === Attachment.Photo ? featuredAttachment : null
    property Attachment featuredAttachment: sceneAttachments ? sceneAttachments.featuredAttachment : null
    property Attachments sceneAttachments: scene ? scene.attachments : null

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
        mipmap: root.mipmap

        RoundButton {
            icon.source: parent.fillMode === Image.PreserveAspectCrop ? "qrc:/icons/navigation/zoom_fit.png" : "qrc:/icons/navigation/zoom_one.png"
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 5
            hoverEnabled: true
            opacity: hovered ? 1 : 0.5
            enabled: !_removeFeaturedImageDialog.active
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
            icon.source: "qrc:/icons/action/delete.png"
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 5
            anchors.rightMargin: 10
            hoverEnabled: true
            opacity: hovered ? 1 : 0.5
            onClicked: _removeFeaturedImageDialog.active = true
            enabled: !_removeFeaturedImageDialog.active
        }

        Loader {
            id: _removeFeaturedImageDialog
            anchors.fill: parent
            active: false
            sourceComponent: Rectangle {
                color: Color.translucent(Runtime.colors.primary.c600.background,0.85)

                MouseArea {
                    anchors.fill: parent
                }

                Column {
                    width: parent.width-40
                    anchors.centerIn: parent
                    spacing: 40

                    VclLabel {
                        text: "Are you sure you want to remove this photo?"
                        font.bold: true
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        color: Runtime.colors.primary.c600.text
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 20

                        VclButton {
                            text: "Yes"
                            focusPolicy: Qt.NoFocus
                            onClicked: {
                                Qt.callLater( () => {
                                                 sceneAttachments.removeAttachment(featuredImage)
                                                 _removeFeaturedImageDialog.active = false
                                             } )
                            }
                        }

                        VclButton {
                            text: "No"
                            focusPolicy: Qt.NoFocus
                            onClicked: _removeFeaturedImageDialog.active = false
                        }
                    }
                }
            }
        }
    }

    AttachmentsDropArea {
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

            VclLabel {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: root.height > 150 ? "Drag & Drop a Photo\n\n-- OR --" : "Drag & Drop a Photo"
            }

            VclButton {
                text: "Select Photo"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: _featuredAttachmentFileDialog.open()
                visible: root.height > 150
            }
        }

        VclFileDialog {
            id: _featuredAttachmentFileDialog
            nameFilters: sceneAttachments ? sceneAttachments.nameFilters : []
            onAccepted: {
                const attachment = sceneAttachments.includeAttachment( Url.toPath(selectedFile) )
                if(attachment)
                    attachment.featured = true
            }
        }
    }
}

