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

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import io.scrite.components


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

        /**
          Not using Row here on purpose.

          The Layout.fillWidth attached property in the first VclText makes this part of the code looks
          so much cleaner and maintainable than having to calculate width manually.

          Besides, we won't have too many break delegates anyway.
          */
        RowLayout {
            id: _layout

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: root.pageLeftMargin
            anchors.rightMargin: root.pageRightMargin

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

    on__FocusIn: () => { }     // TODO
    on__FocusOut: () => { }    // TODO
}


