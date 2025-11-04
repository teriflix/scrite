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

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"
import "qrc:/qml/structureview/annotations"
import "qrc:/qml/structureview/structureelements"

VclMenu {
    id: root

    property alias coloredSceneColor: _colorMenu.selectedColor

    signal createElementRequest(real x, real y, color sceneColor)
    signal createAnnotationRequest(real x, real y, string type)

    VclMenuItem {
        text: "New Scene"
        enabled: !Scrite.document.readOnly

        onClicked: {
            Qt.callLater(root.close)
            root.createElementRequest(root.x-130, root.y-22, Runtime.workspaceSettings.defaultSceneColor)
        }
    }

    ColorMenu {
        id: _colorMenu

        title: "Colored Scene"
        enabled: !Scrite.document.readOnly
        selectedColor: root.coloredSceneColor

        onMenuItemClicked: (color) => {
                               Qt.callLater(root.close)
                               root.coloredSceneColor = color
                               root.createElementRequest(root.x-130, root.y-22, color)
                           }
    }

    MenuSeparator { }

    VclMenu {
        title: "Annotation"

        Repeater {
            model: AnnotationFactory.keys

            VclMenuItem {
                required property var modelData

                text: modelData.title
                enabled: !Scrite.document.readOnly && modelData.what !== ""

                onClicked: {
                    Qt.callLater(root.close)
                    root.createAnnotationRequest(root.x, root.y, modelData.what)
                }
            }
        }
    }
}

