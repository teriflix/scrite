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
        var waitDlg = waitDialogComponent.createObject(root, (message && typeof message === "string" ? {"message": message} : undefined))
        if(waitDlg) {
            waitDlg.closed.connect(waitDlg.destroy)
            waitDlg.open()
            return waitDlg
        }

        Scrite.app.log("Couldn't launch ProgressDialog")
        return null
    }

    Component {
        id: waitDialogComponent

        VclDialog {
            id: waitDialog

            property string message: "Please wait ..."

            title: "Please wait ..."
            closePolicy: Popup.NoAutoClose
            titleBarButtons: null
            width: Math.min(500, Scrite.window.width*0.5)
            height: Math.min(200, Scrite.window.height*0.3)
            appOverrideCursor: Qt.WaitCursor
            appCloseButtonVisible: false
            contentItem: VclText {
                text: waitDialog.message
                padding: 16
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                maximumLineCount: 5
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            function closeLater(delay) {
                Utils.execLater(waitDialog, delay ? delay : 1500, close)
            }
        }
    }
}
