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

    function launch(report, reportInitialProperties) {
        if(_private.dialogImpl.status !== Component.Ready) {
            Scrite.app.log("ReportConfigurationDialog is not ready!")
            return null
        }

        if(!report) {
            Scrite.app.log("No report supplied.")
            return null
        }

        var args = {
            report: null
        }

        if(typeof report === "string")
            args.report = Scrite.document.createReportGenerator(report)
        else if(Scrite.app.verifyType(report, "AbstractReportGenerator"))
            args.report = report
        else {
            Scrite.app.log("No report supplied.")
            return null
        }

        if(reportInitialProperties) {
            for(var member in reportInitialProperties) {
                args.report.setConfigurationValue(member, reportInitialProperties[member])
            }
        }

        var exportConfigDlg = _private.dialogImpl.createObject(root, args)
        if(exportConfigDlg) {
            exportConfigDlg.closed.connect(exportConfigDlg.destroy)
            exportConfigDlg.open()
            return exportConfigDlg
        }

        Scrite.app.log("Couldn't launch ReportConfigurationDialog")
        return null
    }

    QtObject {
        id: _private

        property Component dialogImpl: Qt.createComponent("./reportconfigurationdialog/impl_ReportConfigurationDialog.qml", Component.PreferSynchronous, root)
    }
}
