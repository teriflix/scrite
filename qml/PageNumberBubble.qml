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

PainterPathItem {
    property string pageNumber: "-1"
    readonly property var colors: primaryColors.c600

    Text {
        id: sceneNumberText
        anchors.centerIn: parent
        font: defaultFontMetrics.font
        text: parent.pageNumber
        color: colors.text
        leftPadding: 4; rightPadding: 4
        topPadding: 3; bottomPadding: 1
    }

    width: Math.max(sceneNumberText.contentWidth * 1.5, 30)
    height: sceneNumberText.height

    renderType: PainterPathItem.OutlineAndFill
    fillColor: colors.background
    outlineWidth: 1
    outlineColor: colors.background

    painterPath: PainterPath {
        id: bubblePath
        property real  arrowSize: sceneNumberText.height/4
        property point p1: Qt.point(itemRect.left, itemRect.top)
        property point p2: Qt.point(itemRect.right, itemRect.top)
        property point p3: Qt.point(itemRect.right, itemRect.center.y - arrowSize)
        property point p4: Qt.point(itemRect.right+arrowSize, itemRect.center.y)
        property point p5: Qt.point(itemRect.right, itemRect.center.y + arrowSize)
        property point p6: Qt.point(itemRect.right, itemRect.bottom)
        property point p7: Qt.point(itemRect.left, itemRect.bottom)

        MoveTo { x: bubblePath.p1.x; y: bubblePath.p1.y }
        LineTo { x: bubblePath.p2.x; y: bubblePath.p2.y }
        LineTo { x: bubblePath.p3.x; y: bubblePath.p3.y }
        LineTo { x: bubblePath.p4.x; y: bubblePath.p4.y }
        LineTo { x: bubblePath.p5.x; y: bubblePath.p5.y }
        LineTo { x: bubblePath.p6.x; y: bubblePath.p6.y }
        LineTo { x: bubblePath.p7.x; y: bubblePath.p7.y }
        CloseSubpath { }
    }
}
