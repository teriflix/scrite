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

import QtQuick 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.15

import io.scrite.components 1.0

ListView {
    id: attachmentsView
    property Attachments attachments
    readonly property real delegateSize: 83
    property bool readonly: Scrite.document.readOnly
    orientation: ListView.Horizontal
    model: attachments
    onAttachmentsChanged: currentIndex = -1
    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
    highlight: Rectangle {
        color: primaryColors.highlight.background
    }
    clip: true
    highlightMoveDuration: 0
    highlightResizeDuration: 0
    height: scrollBarVisible ? 100 : 83

    Rectangle {
        anchors.fill: parent
        color: Scrite.app.translucent(primaryColors.windowColor, 0.5)
        border.width: 1
        border.color: primaryColors.borderColor
        z: -1

        Text {
            anchors.fill: parent
            anchors.margins: 10
            anchors.leftMargin: attachmentsView.delegateSize
            verticalAlignment: Text.AlignVCenter
            font.pointSize: Scrite.app.idealFontPointSize
            opacity: 0.5
            text: "Attachments"
            visible: attachments && attachments.attachmentCount === 0
        }
    }

    delegate: Item {
        property bool isSelected: attachmentsView.currentIndex === index
        width: attachmentsView.delegateSize
        height: attachmentsView.delegateSize

        Column {
            anchors.fill: parent
            anchors.margins: 5
            spacing: 0

            Image {
                width: parent.height - ofnLabel.height - parent.spacing
                height: width
                fillMode: Image.PreserveAspectFit
                mipmap: true
                source: "image://fileIcon/" + objectItem.filePath
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id: ofnLabel
                width: parent.width
                elide: Text.ElideMiddle
                padding: 2
                font.pointSize: Scrite.app.idealFontPointSize/2
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 1
                text: objectItem.originalFileName
                color: isSelected ? primaryColors.highlight.text : primaryColors.c10.text
            }
        }

        ToolTip.text: objectItem.originalFileName
        ToolTip.visible: itemMouseArea.containsMouse
        ToolTip.delay: 1000

        MouseArea {
            id: itemMouseArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true
            onClicked: {
                attachmentsView.currentIndex = index
                if(mouse.button === Qt.RightButton && !readonly)
                    attachmentContextMenu.popup()
            }
            onDoubleClicked: objectItem.openAttachmentAnonymously()
        }
    }

    footer: Item {
        width: attachmentsView.delegateSize
        height: attachmentsView.delegateSize

        ToolButton3 {
            iconSource: "../icons/action/attach_file.png"
            anchors.centerIn: parent
            onClicked: if(!readonly) fileDialog.open()
            visible: !readonly
        }
    }

    property bool scrollBarVisible: contentWidth > width
    ScrollBar.horizontal: ScrollBar2 { flickable: attachmentsView }

    FileDialog {
        id: fileDialog
        nameFilters: attachments ? attachments.nameFilters : ["All Types (*.*)"]
        selectFolder: false
        selectMultiple: false
        sidebarVisible: true
        selectExisting: true
        dirUpAction.shortcut: "Ctrl+Shift+U" // The default Ctrl+U interfers with underline
        onAccepted: {
            if(attachments === null)
                return
            if(fileUrl !== "")
                attachments.includeAttachmentFromFileUrl(fileUrl)
        }
    }

    Menu2 {
        id: attachmentContextMenu

        MenuItem2 {
            text: "Edit"
            enabled: attachmentsView.currentIndex >= 0
            onClicked: {
                var attm = attachments.attachmentAt(attachmentsView.currentIndex)
                if(attm)
                    attm.openAttachmentInPlace()
            }
        }

        MenuSeparator { }

        MenuItem2 {
            text: "Remove"
            enabled: attachmentsView.currentIndex >= 0
            onClicked: attachments.removeAttachment( attachments.attachmentAt(attachmentsView.currentIndex) )
        }
    }
}
