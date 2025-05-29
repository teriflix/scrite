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
import QtQuick.Layouts 1.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Rectangle {
    id: root

    property bool fullSize: true
    property int placement: Qt.TopEdge // or Qt.BottomEdge

    implicitWidth: 100
    implicitHeight: layout.height * (fullSize ? 1.75 : 1.25)

    color: Runtime.colors.primary.c100.background

    RowLayout {
        id: layout

        y: placement === Qt.TopEdge ? (root.height-height) : 0
        width: parent.width

        LineItem {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
        }

        VclLabel {
            text: "Page Break"
            font.pointSize: root.fullSize ? Runtime.idealFontMetrics.font.pointSize : Runtime.minimumFontMetrics.font.pointSize
            padding: root.fullSize ? 5 : 2
        }

        LineItem {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
        }
    }

    component LineItem : Image {
        fillMode: Image.TileHorizontally
        source: "qrc:/icons/content/dash-line.png"
        opacity: 0.25
    }
}
