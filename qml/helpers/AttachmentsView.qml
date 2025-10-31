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

import "qrc:/qml/globals"
import "qrc:/qml/controls"

ListView {
    id: root

    property Attachments attachments
    readonly property real delegateSize: 83
    property bool readonly: Scrite.document.readOnly
    property bool scrollBarVisible: contentWidth > width

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    ScrollBar.horizontal: VclScrollBar { flickable: root }

    height: implicitHeight
    implicitHeight: scrollBarVisible ? 100 : 83

    clip: true
    highlightMoveDuration: 0
    highlightResizeDuration: 0
    model: attachments
    orientation: ListView.Horizontal

    highlight: Rectangle {
        color: Runtime.colors.primary.highlight.background
    }

    delegate: Item {
        id: _delegate

        required property var objectItem

        property bool isSelected: root.currentIndex === index

        ToolTip.text: objectItem.originalFileName
        ToolTip.visible: _delegateMouseArea.containsMouse
        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

        width: root.delegateSize
        height: root.delegateSize

        Column {
            anchors.fill: parent
            anchors.margins: 5

            spacing: 0

            Image {
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.height - _ofnLabel.height - parent.spacing
                height: width

                fillMode: Image.PreserveAspectFit
                mipmap: true
                source: "image://fileIcon/" + objectItem.filePath
            }

            VclLabel {
                id: _ofnLabel

                width: parent.width

                color: isSelected ? Runtime.colors.primary.highlight.text : Runtime.colors.primary.c10.text
                elide: Text.ElideMiddle
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 1
                padding: 2
                text: objectItem.originalFileName

                font.pointSize: Runtime.idealFontMetrics.font.pointSize/2
            }
        }

        MouseArea {
            id: _delegateMouseArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true
            onClicked: {
                root.currentIndex = index
                if(mouse.button === Qt.RightButton && !readonly)
                    _attachmentContextMenu.popup()
            }
            onDoubleClicked: objectItem.openAttachmentAnonymously()
        }
    }

    footer: Item {
        width: root.delegateSize
        height: root.delegateSize

        FlatToolButton {
            iconSource: "qrc:/icons/action/attach_file.png"
            anchors.centerIn: parent
            onClicked: if(!readonly) _fileDialog.open()
            visible: !readonly
        }
    }

    Rectangle {
        anchors.fill: parent

        z: -1

        color: Color.translucent(Runtime.colors.primary.windowColor, 0.5)

        border.width: 1
        border.color: Runtime.colors.primary.borderColor

        VclLabel {
            anchors.fill: parent
            anchors.margins: 10
            anchors.leftMargin: root.delegateSize

            opacity: 0.5
            text: "Attachments"
            verticalAlignment: Text.AlignVCenter
            visible: attachments && attachments.attachmentCount === 0

            font.pointSize: Runtime.idealFontMetrics.font.pointSize
        }
    }

    VclFileDialog {
        id: _fileDialog

        nameFilters: attachments ? attachments.nameFilters : ["All Types (*.*)"]
        selectExisting: true
        selectFolder: false
        selectMultiple: false
        sidebarVisible: true

         // The default Ctrl+U interfers with underline
        onAccepted: {
            if(attachments === null)
                return
            if(fileUrl !== "")
                attachments.includeAttachmentFromFileUrl(fileUrl)
        }
    }

    VclMenu {
        id: _attachmentContextMenu

        VclMenuItem {
            text: "Edit"
            enabled: root.currentIndex >= 0

            onClicked: {
                let attm = attachments.attachmentAt(root.currentIndex)
                if(attm)
                    attm.openAttachmentInPlace()
            }
        }

        MenuSeparator { }

        VclMenuItem {
            text: "Remove"
            enabled: root.currentIndex >= 0

            onClicked: {
                attachments.removeAttachment( attachments.attachmentAt(root.currentIndex) )
            }
        }
    }

    onAttachmentsChanged: currentIndex = -1
}
