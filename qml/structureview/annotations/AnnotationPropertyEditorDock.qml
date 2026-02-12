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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/floatingdockpanels"

FloatingDock {
    id: root

    property Annotation annotation
    property BoundingBoxEvaluator canvasItemsBoundingBox

    Component.onCompleted: { Qt.callLater( () => { _private.enableSaveCoordinates = true } ) }

    x: adjustedX(Runtime.structureCanvasSettings.annotationDockX)
    y: adjustedY(Runtime.structureCanvasSettings.annotationDockY)
    width: 375
    height: Scrite.window.height * 0.6
    visible: _private.dockVisibility

    title: "Annotation Properties"

    content: AnnotationPropertyEditor {
        annotation: root.annotation
        canvasItemsBoundingBox: root.canvasItemsBoundingBox
    }

    onXChanged: Qt.callLater(_private.saveCoordinates)
    onYChanged: Qt.callLater(_private.saveCoordinates)
    onCloseRequest: Runtime.structureCanvasSettings.displayAnnotationProperties = false

    // Private section
    QtObject {
        id: _private

        property bool enableSaveCoordinates: false
        property bool dockVisibility: Runtime.structureCanvasSettings.displayAnnotationProperties && root.annotation !== null && Runtime.structureView !== null

        function saveCoordinates() {
            if(enableSaveCoordinates) {
                Runtime.structureCanvasSettings.annotationDockX = Math.round(root.x)
                Runtime.structureCanvasSettings.annotationDockY = Math.round(root.y)
            }
        }

        onDockVisibilityChanged: {
            if(dockVisibility)
                root.open()
            else
                root.close()
        }
    }
}
