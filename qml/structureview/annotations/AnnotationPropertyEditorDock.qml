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

    DelayedProperty.set: Runtime.structureCanvasSettings.displayAnnotationProperties && annotation !== null

    x: 80
    y: Scrite.window.height * 0.15
    width: 375
    height: Scrite.window.height * 0.6
    visible: typeof DelayedProperty.get === "boolean" ? DelayedProperty.get : false

    title: "Annotation Properties"

    content: AnnotationPropertyEditor {
        annotation: root.annotation
        canvasItemsBoundingBox: root.canvasItemsBoundingBox
    }

    onCloseRequest: {
        Runtime.structureCanvasSettings.displayAnnotationProperties = false
    }
}
