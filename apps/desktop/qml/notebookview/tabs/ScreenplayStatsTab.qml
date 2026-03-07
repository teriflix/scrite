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
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../../globals"
import "../../helpers"
import "../../controls"
import ".."
import "../../structureview"

Item {
    id: root

    PdfView {
        function generateStatsReport() {
            const fileName = _fileManager.generateUniqueTemporaryFileName("pdf")

            let generator = Scrite.document.createReportGenerator("Statistics Report")
            generator.fileName = fileName
            generator.generate()

            _fileManager.addToAutoDeleteList(fileName)

            pagesPerRow = 1
            source = Url.fromPath(fileName)

            _busyMessage.visible = false
        }

        Component.onCompleted: Qt.callLater(generateStatsReport)

        anchors.fill: parent

        closable: false
        displayRefreshButton: true
        pagesPerRow: 1
        allowFileSave: !Scrite.document.hasCollaborators || Scrite.document.canModifyCollaborators

        onRefreshRequest: {
            _busyMessage.visible = true
            Qt.callLater(generateStatsReport)
        }
    }

    FileManager {
        id: _fileManager
    }

    BusyMessage {
        id: _busyMessage

        message: "Loading Stats ..."
        visible: true
    }
}
