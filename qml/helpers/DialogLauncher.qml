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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: root

    property string name: "DialogLauncher"
    property Component dialogComponent
    property bool singleInstanceOnly: true

    parent: Scrite.window.contentItem

    function doLaunch(initialProperties) {
        if(singleInstanceOnly && _private.dialog) {
            if(initialProperties) {
                for(let member in initialProperties)
                    _private.dialog[member] = initialProperties[member]
            }
            return _private.dialog
        }

        if(!dialogComponent) {
            console.log("No dialog component was supplied for " + name)
            return null
        }

        if(dialogComponent.status !== Component.Ready) {
            console.log(name +" is not ready!")
            return null
        }

        if(!dialogComponent.parent) {
            Object.reparent(dialogComponent, root)
        }

        var dlg = initialProperties ? dialogComponent.createObject(root,initialProperties) : dialogComponent.createObject(root)
        if(dlg) {
            if(singleInstanceOnly)
                _private.dialog = dlg

            dlg.closed.connect(dlg.destroy)
            dlg.open()

            if(root.name !== "DialogLauncher")
                Runtime.showHelpTip(root.name)

            return dlg
        }

        console.log("Couldn't launch " + root.name)
        return null
    }

    function closeSingleInstance() {
        if(singleInstanceOnly && _private.dialog)
            _private.dialog.close()
    }

    onSingleInstanceOnlyChanged: {
        if(!singleInstanceOnly) {
            if(_private.dialog)
                _private.dialog = null
        }
    }

    QtObject {
        id: _private

        property Dialog dialog
    }
}
