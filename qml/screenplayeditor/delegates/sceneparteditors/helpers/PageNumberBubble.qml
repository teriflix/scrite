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

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

PainterPathItem {
    id: root

    property string pageNumber: "-1"

    readonly property var colors: Runtime.colors.primary.c600

    VclLabel {
        id: _sceneNumberText

        anchors.centerIn: parent

        font: Runtime.idealFontMetrics.font
        text: pageNumber
        color: colors.text
        topPadding: 3
        leftPadding: 4
        rightPadding: 4
        bottomPadding: 1
    }

    width: Math.max(_sceneNumberText.contentWidth * 1.5, 30)
    height: _sceneNumberText.height

    fillColor: colors.background
    renderType: PainterPathItem.OutlineAndFill
    outlineColor: colors.background
    outlineWidth: 1

    painterPath: PainterPath {
        id: _bubblePath

        property real  arrowSize: _sceneNumberText.height/4
        property point p1: Qt.point(itemRect.left, itemRect.top)
        property point p2: Qt.point(itemRect.right, itemRect.top)
        property point p3: Qt.point(itemRect.right, itemRect.center.y - arrowSize)
        property point p4: Qt.point(itemRect.right+arrowSize, itemRect.center.y)
        property point p5: Qt.point(itemRect.right, itemRect.center.y + arrowSize)
        property point p6: Qt.point(itemRect.right, itemRect.bottom)
        property point p7: Qt.point(itemRect.left, itemRect.bottom)

        MoveTo { x: _bubblePath.p1.x; y: _bubblePath.p1.y }
        LineTo { x: _bubblePath.p2.x; y: _bubblePath.p2.y }
        LineTo { x: _bubblePath.p3.x; y: _bubblePath.p3.y }
        LineTo { x: _bubblePath.p4.x; y: _bubblePath.p4.y }
        LineTo { x: _bubblePath.p5.x; y: _bubblePath.p5.y }
        LineTo { x: _bubblePath.p6.x; y: _bubblePath.p6.y }
        LineTo { x: _bubblePath.p7.x; y: _bubblePath.p7.y }
        CloseSubpath { }
    }
}
