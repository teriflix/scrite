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
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"

Rectangle {
    id: root

    required property Annotation annotation

    signal resetRequest()

    width: _toolsLayout.width+5
    height: _toolsLayout.height+5

    color: Runtime.colors.primary.c100.background
    border.width: 1
    border.color: Runtime.colors.primary.borderColor

    Row {
        id: _toolsLayout

        anchors.centerIn: parent

        FlatToolButton {
            ToolTip.text: "Edit properties of this annotation"

            down: AnnotationPropertyEditorDock.visible
            iconSource: "qrc:/icons/action/edit.png"

            onClicked: Runtime.structureCanvasSettings.displayAnnotationProperties = !Runtime.structureCanvasSettings.displayAnnotationProperties
        }

        FlatToolButton {
            ToolTip.text: "Bring this annotation to front"

            enabled: Scrite.document.structure.canBringToFront(root.annotation)
            iconSource: "qrc:/icons/action/keyboard_arrow_up.png"

            onClicked: {
                let a = root.annotation
                Scrite.document.structure.bringToFront(a)
            }
        }

        FlatToolButton {
            ToolTip.text: "Send this annotation to back"

            enabled: Scrite.document.structure.canSendToBack(root.annotation)
            iconSource: "qrc:/icons/action/keyboard_arrow_down.png"

            onClicked: {
                root.resetRequest()
                Scrite.document.structure.sendToBack(a)
            }
        }

        FlatToolButton {
            ToolTip.text: "Delete this annotation"

            iconSource: "qrc:/icons/action/delete.png"

            onClicked: {
                root.resetRequest()

                let a = root.annotation
                Scrite.document.structure.removeAnnotation(a)
            }
        }
    }
}

