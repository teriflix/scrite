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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Item {
    id: root

    required property string dragDropMimeType

    signal dropEntered(var drag) // When we move to Qt 6.9+, change type to DragEvent
    signal dropExited()
    signal dropRequest(var drop) // When we move to Qt 6.9+, change type to DragEvent

    height: Runtime.sceneEditorFontMetrics.lineSpacing

    DropArea {
        id: _dropArea

        anchors.fill: parent

        keys: [root.dragDropMimeType]

        onEntered: (drag) => {
                       drag.acceptProposedAction()
                       root.dropEntered(drag)
                   }

        onExited: root.dropExited()

        onDropped: (drop) => {
                       drop.acceptProposedAction()
                       root.dropRequest(drop)
                   }
    }

    Rectangle {
        width: parent.width
        height: 2

        color: Runtime.colors.primary.borderColor
        visible: _dropArea.containsDrag
    }
}
