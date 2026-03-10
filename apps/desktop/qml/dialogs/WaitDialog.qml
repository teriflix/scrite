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

pragma Singleton

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components


import "../globals"
import "../controls"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch(message, progressReport) {
        var initialProps = {
            "message": "Please wait ..."
        }
        if(message && typeof message === "string")
            initialProps.message = message
        if(progressReport && Object.isOfType(progressReport, "ProgressReport"))
            initialProps.progressReport = progressReport

        var dlg = _dialogComponent.createObject(root, initialProps)
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        console.log("Couldn't launch WaitDialog")
        return null
    }

    Component {
        id: _dialogComponent

        VclDialog {
            id: _dialog

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

                    text: _dialog.message
                    elide: _dialog.progressReport ? Text.ElideMiddle : Text.ElideRight
                    wrapMode: _dialog.progressReport ? Text.NoWrap : Text.WrapAtWordBoundaryOrAnywhere
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
                    value: _dialog.progressReport ? _dialog.progressReport.progress : 0
                    visible: _dialog.progressReport !== null
                }

                VclLabel {
                    Layout.fillWidth: true

                    text: _dialog.progressReport ? (_dialog.progressReport.progressText + " (" + Math.round(_dialog.progressReport.progress*100,0) + "%)") : " - "
                    elide: Text.ElideMiddle
                    padding: 8
                    visible: _dialog.progressReport !== null
                    bottomPadding: 16
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
