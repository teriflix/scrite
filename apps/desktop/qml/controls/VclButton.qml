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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../helpers"

Button {
    id: root

    property bool toolTipVisible: hovered
    property string toolTipText

    Layout.minimumWidth: implicitWidth

    Material.roundedScale: Material.NotRounded

    font.pointSize: Runtime.idealFontMetrics.font.pointSize

    implicitWidth: Math.max(GMath.horizontalAdvance(text, font) + leftPadding + rightPadding + 20, 120)

    ToolTipPopup {
        container: root
        text: root.toolTipText
        visible: text !== "" && root.toolTipVisible
    }
}
