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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

AbstractScreenplayElementDelegate {
    id: root

    canFocus: false

    content: Rectangle {
        color: Runtime.colors.primary.c200.background

        height: _layout.height

        RowLayout {
            id: _layout

            VclText {
                Layout.fillWidth: true

                font: root.font
                text: {
                    let ret = "[OMITTED]"
                    if(root.scene.heading.enabled)
                        ret += " " + root.screenplayElement.resolvedSceneNumber + ": " + root.scene.heading.displayText
                    return ret
                }
                elide: Text.ElideRight
            }

            VclButton {
                text: "Include"

                onClicked: root.screenplayElement.omitted = false
            }
        }
    }
}


