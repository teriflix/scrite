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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch(message) {
        var initialProps = {
            "message": "Please wait ..."
        }
        if(message && typeof message === "string")
            initialProps.message = message

        var dlg = dialogComponent.createObject(root, initialProps)
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        console.log("Couldn't launch WaitDialog")
        return null
    }

    Component {
        id: dialogComponent

        VclDialog {
            id: dialog

            property string message

            title: "Please wait ..."
            closePolicy: Popup.NoAutoClose
            titleBarButtons: null
            width: Math.min(500, Scrite.window.width*0.5)
            height: Math.min(200, Scrite.window.height*0.3)
            appOverrideCursor: Qt.WaitCursor
            appCloseButtonVisible: false
            contentItem: VclLabel {
                text: dialog.message
                padding: 16
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                maximumLineCount: 5
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
