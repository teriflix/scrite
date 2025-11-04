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
import "qrc:/qml/controls"
import "qrc:/qml/notebookview/helpers"

FocusScope {
    id: root

    required property var pageData
    required property NotebookModel notebookModel

    property real maxTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 80
    property real minTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 20

    property color backgroundColor: Runtime.colors.primary.c100.background

    clip: true

    function askDeleteConfirmation(message, callback) {
        let popup = _private.deleteConfirmationPopup.createObject(root, {"message": message})
        popup.closed.connect(popup.destroy)
        popup.deletionConfirmed.connect(callback)
        popup.open()
    }

    Rectangle {
        id: _background

        anchors.fill: parent

        z: -1
        color: Color.translucent(root.backgroundColor, 0.5)

        border.color: Runtime.colors.primary.borderColor
        border.width: 1
    }

    QtObject {
        id: _private

        readonly property Component deleteConfirmationPopup: DeleteConfirmationPopup {
            anchors.centerIn: parent
        }
    }
}
