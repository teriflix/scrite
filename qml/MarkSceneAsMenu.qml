/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13

import Scrite 1.0

Menu2 {
    id: msaMenu
    title: "Mark Scene As"

    property Scene scene
    signal triggered(var type)

    Repeater {
        model: app.enumerationModelForType("Scene", "Type")

        MenuItem2 {
            text: modelData.key
            icon.source: modelData.icon
            enabled: scene !== null
            font.bold: scene.type === modelData.value
            onTriggered: {
                scene.type = modelData.value
                msaMenu.triggered(modelData.value)
            }
        }
    }
}