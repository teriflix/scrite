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
import QtQuick.Layouts 1.15
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

    delay: DelayedProperty.value > 0 ? 0 : Qt.styleHints.mousePressAndHoldInterval
    contentWidth: Math.min(_content.implicitWidth, maximumContentWidth)

    contentItem: RowLayout {
        id: _content

        property var fields: {
            let label = "", shortcut = ""

            const text = root.text
            const boIndex = text.indexOf('(')
            if(boIndex > 0) {
                const bcIndex = text.lastIndexOf(')')
                shortcut = text.slice(boIndex+1, bcIndex > boIndex ? bcIndex : text.length-1).trim()
                label = text.slice(0, boIndex).trim()
            } else {
                const comps = text.split("\t")
                label = comps[0].trim()
                shortcut = comps.length > 1 ? comps[comps.length-1].trim() : ""
            }

            if(shortcut === "undefined")
                return [label]
            if(shortcut === "" || Gui.portableShortcut(shortcut) === "")
                return [text]
            return [label, Gui.portableShortcut(shortcut)]
        }

        spacing: 20

        Text {
            Layout.fillWidth: true

            font: Runtime.idealFontMetrics.font
            text: _content.fields[0]
            color: {
                if(Runtime.applicationSettings.theme === "Material")
                    return "white"

                return root.palette.toolTipText
            }
            wrapMode: Text.WordWrap
        }

        ShortcutField {
            visible: _content.fields.length > 1 && portableShortcut !== ""
            readOnly: true
            description: ""
            portableShortcut: _content.fields.length > 1 ? _content.fields[_content.fields.length-1] : ""
        }
    }

    exit: null
    enter: null

    onAboutToShow: Runtime.visibleTooltipCount = Runtime.visibleTooltipCount + 1
    onAboutToHide: Runtime.visibleTooltipCount = Runtime.visibleTooltipCount - 1
}
