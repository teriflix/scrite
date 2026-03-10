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
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"

VclMenu {
    id: root

    title: "Mark Scene As"

    property Scene scene
    property bool enableValidation: true

    signal triggered(var type)

    Repeater {
        model: EnumerationModel {
            className: "Scene"
            enumeration: "Type"
        }

        delegate: VclMenuItem {
            required property int index
            required property int enumValue
            required property string enumKey
            required property string enumIcon

            text: enumKey + (scene ? (font.bold ? " ✔" : "") : "")
            icon.source: enumIcon
            enabled: enableValidation ? scene : true
            font.bold: scene ? (enabled ? scene.type === enumValue : false) : false

            onClicked: {
                if(scene)
                    scene.type = enumValue
                root.triggered(enumValue)
            }
        }
    }
}
