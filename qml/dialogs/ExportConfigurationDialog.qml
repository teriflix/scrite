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

    function launch(exporter) {
        if(_private.dialogComponent.status !== Component.Ready) {
            console.log("ExportConfigurationDialog is not ready!")
            return null
        }

        if(!exporter) {
            console.log("No exporter supplied.")
            return null
        }

        var args = {
            exporter: null
        }

        if(typeof exporter === "string") {
            if(_private.dialog) {
                if(_private.dialog.exporter.format === exporter)
                    return _private.dialog
                _private.dialog.close()
            }
            args.exporter = Scrite.document.createExporter(exporter)
        } else if(Scrite.app.verifyType(exporter, "AbstractExporter")) {
            if(_private.dialog) {
                if(_private.dialog.exporter === exporter)
                    return _private.dialog
                _private.dialog.close()
            }
            args.exporter = exporter
        } else {
            if(_private.dialog)
                _private.dialog.close()
            console.log("No exporter supplied.")
            return null
        }

        Scrite.app.restoreObjectConfiguration(args.exporter)

        var dlg = _private.dialogComponent.createObject(root, args)
        if(dlg) {
            _private.dialog = dlg
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        console.log("Couldn't launch ExportConfigurationDialog")
        return null
    }

    QtObject {
        id: _private

        property Component dialogComponent: Qt.createComponent("./exportconfigurationdialog/impl_ExportConfigurationDialog.qml", Component.PreferSynchronous, root)
        property VclDialog dialog
    }
}
