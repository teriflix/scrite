/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13

import Scrite 1.0

ListView {
    id: attachmentsView
    property Attachments attachments
    property real delegateSize: orientation === ListView.Horizontal ? height : width
    orientation: ListView.Horizontal
    model: attachments
    highlight: Rectangle {
        color: primaryColors.highlight.background
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
                width: parent.width * 0.7
                height: width
                fillMode: Image.PreserveAspectFit
                mipmap: true
                source: "image://fileIcon/" + objectItem.filePath
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                padding: 5
                font.pointSize: app.idealFontPointSize-2
                maximumLineCount: 1
                text: objectItem.title
                color: isSelected ? primaryColors.highlight.text : primaryColors.c10.text
            }
        }

        ToolTip.text: objectItem.originalFileName
        ToolTip.visible: itemMouseArea.containsMouse

        MouseArea {
            id: itemMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: attachmentsView.currentIndex = index
            onDoubleClicked: objectItem.openAttachmentAnonymously()
        }
    }
}
