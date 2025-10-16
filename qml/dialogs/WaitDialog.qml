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

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch(message, progressReport) {
        var initialProps = {
            "message": "Please wait ..."
        }
        if(message && typeof message === "string")
            initialProps.message = message
        if(progressReport && Scrite.app.verifyType(progressReport, "ProgressReport"))
            initialProps.progressReport = progressReport

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
            property ProgressReport progressReport

            width: Math.min(500, Scrite.window.width*0.5)
            height: Math.min(200, Scrite.window.height*0.3)

            title: "Please wait ..."
            closePolicy: Popup.NoAutoClose
            titleBarButtons: null
            appOverrideCursor: Qt.WaitCursor
            appCloseButtonVisible: false

            contentItem: ColumnLayout {
                spacing: 20

                VclLabel {
                    Layout.fillWidth: true

                    text: dialog.message
                    elide: dialog.progressReport ? Text.ElideMiddle : Text.ElideRight
                    wrapMode: dialog.progressReport ? Text.NoWrap : Text.WrapAtWordBoundaryOrAnywhere
                    padding: 16
                    maximumLineCount: 5
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }

                ProgressBar {
                    Layout.fillWidth: true
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20

                    to: 1
                    from: 0
                    value: dialog.progressReport ? dialog.progressReport.progress : 0
                    visible: dialog.progressReport !== null
                }

                VclLabel {
                    Layout.fillWidth: true

                    text: dialog.progressReport ? (dialog.progressReport.progressText + " (" + Math.round(dialog.progressReport.progress*100,0) + "%)") : " - "
                    elide: Text.ElideMiddle
                    padding: 8
                    visible: dialog.progressReport !== null
                    bottomPadding: 16
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
