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
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Rectangle {
    id: busyOverlay

    color: Runtime.colors.primary.windowColor
    opacity: 0.9
    visible: false

    onVisibleChanged: if(parent) parent.enabled = !visible

    property string busyMessage: "Busy Doing Something..."

    Rectangle {
        anchors.fill: busyOverlayNotice
        anchors.margins: -30
        radius: 4
        color: Runtime.colors.primary.c700.background

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: false
        }
    }

    Column {
        id: busyOverlayNotice
        spacing: 10
        width: parent.width * 0.8
        anchors.centerIn: parent

        BusyIcon {
            running: busyOverlay.visible
            anchors.horizontalCenter: parent.horizontalCenter
            forDarkBackground: true
        }

        VclLabel {
            width: parent.width
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            horizontalAlignment: Text.AlignHCenter
            text: busyMessage
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            color: Runtime.colors.primary.c700.text
        }
    }

    MouseArea {
        anchors.fill: parent
    }
}
