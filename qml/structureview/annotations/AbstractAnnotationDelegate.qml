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

import io.scrite.components 1.0

import "qrc:/qml/globals"

Rectangle {
    id: root

    required property int annotationIndex
    required property Annotation annotation

    signal gripRequest(Item delegate, Annotation annotation)

    x: annotation.geometry.x
    y: annotation.geometry.y
    width: annotation.geometry.width
    height: annotation.geometry.height

    color: annotation.attributes.fillBackground ? (annotation.attributes.color ? annotation.attributes.color : annotation.attributes.backgroundColor) : Qt.rgba(0,0,0,0)
    border {
        width: annotation.attributes.borderWidth ? annotation.attributes.borderWidth : 0
        color: annotation.attributes.borderColor ? annotation.attributes.borderColor : Qt.rgba(0,0,0,0)
    }

    opacity: annotation.attributes.opacity / 100

    MouseArea {
        anchors.fill: parent
        enabled: annotationGripLoader.annotationItem !== root
        onClicked: parent.grip()
        onDoubleClicked: {
            root.gripRequest(root, root.annotation)

            Runtime.structureCanvasSettings.displayAnnotationProperties = true
        }
    }

    Component.onCompleted: {
        if(annotation.objectName === "ica") {
            Qt.callLater(gripRequest, root, root.annotation)
            annotation.objectName = ""
        }
    }
}
