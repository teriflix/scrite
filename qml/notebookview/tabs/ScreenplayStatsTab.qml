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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"
import "qrc:/qml/structureview"

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
