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

Menu2 {
    id: msaMenu
    title: "Mark Scene As"

    property Scene scene
    property bool enableValidation: true
    signal triggered(var type)

    Repeater {
        model: Scrite.app.enumerationModelForType("Scene", "Type")

        MenuItem2 {
            text: modelData.key + (scene ? (font.bold ? " ✔" : "") : "")
            icon.source: modelData.icon
            enabled: enableValidation ? scene : true
            font.bold: scene ? (enabled ? scene.type === modelData.value : false) : false
            onTriggered: {
                if(scene)
                    scene.type = modelData.value
                msaMenu.triggered(modelData.value)
            }
        }
    }
}
