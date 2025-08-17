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

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/structureview"

MouseArea {
    id: root

    required property real scale
    required property string annotationType

    signal done()
    signal createAnnotationRequest(real x, real y, string type)

    EventFilter.target: Scrite.app
    EventFilter.events: [EventFilter.KeyPress]
    EventFilter.active: root.enabled
    EventFilter.onFilter: (object, event, result) => {
                                if(event.key === Qt.Key_Escape) {
                                    Qt.callLater(root.done)
                                }
                                result.accept = false
                                result.filter = false
                            }

    hoverEnabled: true
    acceptedButtons: Qt.LeftButton

    Image {
        property real halfSize: width/2

        x: parent.mouseX - halfSize
        y: parent.mouseY - halfSize
        width: 30/root.scale
        height: width

        sourceSize.width: width
        sourceSize.height: height

        source: "qrc:/icons/action/add_annotation.png"
    }

    onClicked: {
        if(!Scrite.document.readOnly) {
            root.createAnnotationRequest(mouse.x, mouse.y, annotationType)
            Qt.callLater(root.done)
        }
    }
}
