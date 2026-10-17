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

import Scrite.App

import "../../globals"
import "../../controls"
import "../helpers"

FocusScope {
    id: root

    required property var pageData
    required property NotebookModel notebookModel

    required property real maxTextAreaSize
    required property real minTextAreaSize

    property color backgroundColor: Runtime.colors.primary.c100.background

    Component.onCompleted: Runtime.activeNotePage = root

    function askDeleteConfirmation(message, callback) {
        let popup = _private.deleteConfirmationPopup.createObject(root, {"message": message}) as DeleteConfirmationPopup
        popup.closed.connect(popup.destroy)
        popup.deletionConfirmed.connect(callback)
        popup.open()
    }

    clip: true

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
