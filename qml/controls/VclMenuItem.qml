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
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"

MenuItem {
    id: root

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    focusPolicy: Qt.NoFocus
    font.pointSize: Runtime.idealFontMetrics.font.pointSize

    contentItem: RowLayout {
        id: _content

        property real arrowPadding: root.subMenu && root.arrow ? root.arrow.width + root.spacing : 0
        property real indicatorPadding: root.checkable && root.indicator ? root.indicator.width + root.spacing : 0
        property var fields: {
            if(root.action) {
                if(root.action.shortcut && root.action.shortcut !== "")
                    return [root.action.text,root.action.shortcut]
                return [root.action.text]
            }

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

        RowLayout {
            Layout.leftMargin: !root.mirrored ? _content.indicatorPadding : _content.arrowPadding
            Layout.fillWidth: true

            Image {
                Layout.preferredWidth: _label.height
                Layout.preferredHeight: _label.height

                visible: root.icon.source !== "" && status === Image.Ready
                source: root.icon.source
                fillMode: Image.PreserveAspectFit
            }

            VclLabel {
                id: _label

                Layout.fillWidth: true

                text: _content.fields[0]
                horizontalAlignment: Qt.AlignLeft
            }
        }

        ShortcutField {
            Layout.rightMargin: root.mirrored ? _content.indicatorPadding : _content.arrowPadding

            visible: _content.fields.length > 1 && portableShortcut !== ""
            readOnly: true
            description: ""
            portableShortcut: _content.fields.length > 1 ? _content.fields[_content.fields.length-1] : ""
        }
    }
}
