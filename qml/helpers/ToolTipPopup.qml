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

ToolTip {
    id: root

    readonly property real maximumContentWidth: 350

    property Item container: parent

    DelayedProperty.watch: Runtime.visibleTooltipCount
    DelayedProperty.delay: Runtime.stdAnimationDuration

    x: 20
    y: container.height + 15

    contentWidth: Math.min(Runtime.idealFontMetrics.advanceWidth(text), maximumContentWidth)
    delay: DelayedProperty.value > 0 ? 0 : Qt.styleHints.mousePressAndHoldInterval

    contentItem: Text {
        width: root.contentWidth

        font: Runtime.idealFontMetrics.font
        text: root.text
        color: {
            if(Runtime.applicationSettings.theme === "Material")
                return "white"

            return root.palette.toolTipText
        }
        wrapMode: Text.WordWrap
    }

    exit: null
    enter: null

    onAboutToShow: Runtime.visibleTooltipCount = Runtime.visibleTooltipCount + 1
    onAboutToHide: Runtime.visibleTooltipCount = Runtime.visibleTooltipCount - 1
}
