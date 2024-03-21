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
import "qrc:/qml/dialogs/homescreen"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch(mode) {
        var dlg = dialogComponent.createObject(root, {"mode": mode})
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        console.log("Couldn't launch HomeScreen")
        return null
    }

    Component {
        id: dialogComponent

        VclDialog {
            id: dialog

            property string mode

            width: Math.min(800, Scrite.window.width*0.9)
            height: Math.min(width, Scrite.window.height*0.9)
            title: "Home"

            contentItem: HomeScreen {
                mode: dialog.mode
                onCloseRequest: Qt.callLater(dialog.close)
            }
        }
    }
}
