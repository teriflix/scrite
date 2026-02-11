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

import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

BasicAttachmentsDropArea {
    id: root

    property real noticeWidthFactor: 0.5
    property string attachmentNoticeSuffix: "Add as attachment by dropping it here."

    enabled: !Scrite.document.readOnly

    Rectangle {
        anchors.fill: parent

        color: Color.translucent(Runtime.colors.primary.c500.background, 0.5)
        visible: root.active

        Rectangle {
            anchors.fill: _notice
            anchors.margins: -30

            color: Runtime.colors.primary.c700.background
            radius: 4
        }

        VclLabel {
            id: _notice

            anchors.centerIn: parent

            width: parent.width * noticeWidthFactor

            color: Runtime.colors.primary.c700.text
            horizontalAlignment: Text.AlignHCenter
            text: parent.visible ? "<b>" + root.attachment.title + "</b><br/><br/>" + attachmentNoticeSuffix : ""
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            font.pointSize: Runtime.idealFontMetrics.font.pointSize
        }
    }
}
